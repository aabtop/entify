module Shaders

import ..Entify
import MacroTools
using Memoize

export ShaderDeclaration, CachedShaderDeclaration, ShaderExpr, ShaderSource,
       ShaderFunction, CachedShaderFunction, ShaderFunctionCall,
       @DefineInlineShaderFunction, @DefineShaderFunction,
       @GLSLFragmentShaderFromBuilder, @GLSLVertexShaderFromBuilder

abstract type ShaderExpr{T} end

struct ShaderSource{T} <: ShaderExpr{T}
  source_template::String
  substitutions::Vector{Pair{String, ShaderExpr}}

  ShaderSource{T}(source_template, substitutions=[]) where T =
      new{T}(source_template, substitutions)
end

function _EscapeTypeInColonExpression(expr)
  _CheckForColonExpression(expr)
  return :($(expr.args[1])::$(esc(expr.args[2])))
end

struct FunctionDecl
  name::Symbol
  return_type::Expr
  typed_parameters::Vector{Expr}

  function FunctionDecl(decl_expr)
    if decl_expr.head != :(::) error("Invalid parameters.") end
    if decl_expr.args[1].head != :call error("Invalid parameters.") end
    return new(decl_expr.args[1].args[1], esc(decl_expr.args[2]),
               _EscapeTypeInColonExpression.(decl_expr.args[1].args[2:end]))
  end
end

function _WrapTypeInColonExpression(expr)
  _CheckForColonExpression(expr)
  return :($(expr.args[1])::ShaderExpr{$(expr.args[2])})
end

macro DefineInlineShaderFunction(decl, source_template)
  parsed_decl = FunctionDecl(decl)
  expr = :(function $(esc(parsed_decl.name))(
               $(_WrapTypeInColonExpression.(parsed_decl.typed_parameters)...))
             return ShaderSource{$(parsed_decl.return_type)}(
                 $source_template,
                 [$(map(x -> :($(string(x.args[1])) => $(x.args[1])),
                        parsed_decl.typed_parameters)...)])
           end)
  return expr
end

# Just arbitrary shader code that should appear before ShaderFunction
# declarations.
struct ShaderDeclaration
  source::String
end

@memoize Dict function CachedShaderDeclaration(source)
  return ShaderDeclaration(source)
end

struct ShaderFunction{T}
  name::String
  arguments::Vector{Tuple{String, DataType}}
  function_body::Union{String, ShaderExpr}
  helper_declarations::Vector{ShaderDeclaration}
end

@memoize Dict function CachedShaderFunction(
    return_type, name, arguments, function_body, helper_declarations)
  return ShaderFunction{return_type}(name, arguments, function_body,
                                     helper_declarations)
end

struct ShaderFunctionCall{T} <: ShaderExpr{T}
  decl::ShaderFunction{T}
  params::Vector{ShaderExpr}
end

# Make ShaderFunctions callable.
function (shader_function::ShaderFunction)(params...)
  # Do some manual type checking here.
  for i in 1:length(params)
    if !(typeof(params[i]) <: ShaderExpr{shader_function.arguments[i][2]})
      error("Type mismatch for parameter " * shader_function.arguments[i][1]
            * ".  Expected " * string(shader_function.arguments[i][2])
            * ", got " * string(typeof(params[i])))
    end
  end

  return ShaderFunctionCall{typeof(shader_function).parameters[1]}(
      shader_function, [params...])
end

function _CheckForColonExpression(expr)
  if expr.head != :(::)
    error("Expect a list of typed symbols.")
  end
end

macro DefineShaderFunction(function_decl, function_body, declaration_code=:(""))
  parsed_decl = FunctionDecl(function_decl)
  shader_function = gensym(String(parsed_decl.name) * "Declaration")

  if typeof(function_body) == String
    translated_function_body = function_body
  else
    inputs_dict = _ToNameTypeDict(function_decl.args[1].args[2:end])
    translated_function_body = _TranslateShaderExpr(inputs_dict, function_body)
  end

  expr = :(
    $(esc(parsed_decl.name)) = CachedShaderFunction($(parsed_decl.return_type),
        $(String(parsed_decl.name)),
        [$(map(x -> :(($(String(x.args[1])), $(x.args[2]))),
               parsed_decl.typed_parameters)...)],
        $translated_function_body,
        [CachedShaderDeclaration($declaration_code)])
  )
  return expr
end

function _ToNameTypeDict(colon_expr_tuple::Expr)
  if colon_expr_tuple.head != :tuple
    error("Invalid parameters in " * repr(colon_expr_tuple) * ".")
  end
  return _ToNameTypeDict(colon_expr_tuple.args)
end

function _ToNameTypeDict(named_type_list::Vector)
  function ToNameTypeTuple(x::Expr)
    _CheckForColonExpression(x)
    return (String(x.args[1]), esc(x.args[2]))
  end

  return Dict(map(ToNameTypeTuple, named_type_list))
end

function _ColonToTuple(x::Expr)
  _CheckForColonExpression(x)
  return :(($(esc(x.args[2])), $(String(x.args[1]))))
end


# Translate references to inputs/uniforms into ShaderSources, and
# escape all other symbols.
_TranslateShaderExpr(input_names_dict, shader_expr) =
    MacroTools.postwalk(
      function(x)
        if x isa Symbol
          x_str = String(x)
          if x_str in keys(input_names_dict)
            input_type = input_names_dict[x_str]
            return :(ShaderSource{$input_type}($x_str))
          else
            return esc(x)
          end
        else
          return x
        end
      end,
      shader_expr)

macro GLSLFragmentShaderFromBuilder(inputs, uniforms, output_shader_expr)
  inputs_dict = _ToNameTypeDict(inputs)
  uniforms_dict = _ToNameTypeDict(uniforms)
  all_inputs_dict = merge(inputs_dict, uniforms_dict)

  translated_shader_expr =
      _TranslateShaderExpr(all_inputs_dict, output_shader_expr)

  expr = :(
    Entify.GLSLFragmentShader(
      $(Expr(:tuple, _ColonToTuple.(inputs.args)...)),
      $(Expr(:tuple, _ColonToTuple.(uniforms.args)...)),
      BuildShaderString(
          [("gl_FragColor", $translated_shader_expr)])))
  return expr
end

macro GLSLVertexShaderFromBuilder(
    inputs, uniforms, intermediate_assignments, output_assignments,
    position_output_expression)
  inputs_dict = _ToNameTypeDict(inputs)
  uniforms_dict = _ToNameTypeDict(uniforms)
  all_inputs_dict = merge(inputs_dict, uniforms_dict)

  assignments = []
  outputs_tuple = []
  symbol_assignments = []
  function ProcessAssignment!(assignment, is_output)
    if assignment.head != :(=) error("Expected an assignment operator.") end

    lvalue_string = String(assignment.args[1])
    lvalue_symbol = gensym(lvalue_string)
    lvalue_type = :(typeof($lvalue_symbol).parameters[1])

    push!(symbol_assignments,
          :($lvalue_symbol =
                $(_TranslateShaderExpr(all_inputs_dict, assignment.args[2]))))

    push!(assignments, (
        :($(is_output ? "" : :(Entify.DataTypeToGLSLString[$lvalue_type] * " "))
              * $lvalue_string),
        lvalue_symbol))

    if is_output push!(outputs_tuple, :(($lvalue_type, $lvalue_string))) end

    # And now make it referencable by subsequent assignment rvalues.
    all_inputs_dict[lvalue_string] = lvalue_type
  end

  for assignment in intermediate_assignments.args
    ProcessAssignment!(assignment, false)
  end
  for assignment in output_assignments.args
    ProcessAssignment!(assignment, true)
  end

  push!(assignments, (
       "gl_Position",
       _TranslateShaderExpr(all_inputs_dict, position_output_expression)))

  nest_lets(assignments, expr) = (assignments == []
      ? expr
      : :(let $(assignments[1])
            $(nest_lets(assignments[2:end], expr))
          end))

  expr = nest_lets(symbol_assignments,
      :(Entify.GLSLVertexShader(
          $(Expr(:tuple, _ColonToTuple.(inputs.args)...)),
          $(Expr(:tuple, outputs_tuple...)),
          $(Expr(:tuple, _ColonToTuple.(uniforms.args)...)),
          BuildShaderString(
              [$(map(x -> :(($(x[1]), $(x[2]))), assignments)...)]))))
  return expr
end

struct ShaderBuilderContext
  referenced_shader_functions::Set{ShaderFunction}
  ShaderBuilderContext() = new(Set())
end

function PrintFunctionBodyCode!(buffer::IOBuffer, f::ShaderFunction{T} where T)
  glsl_return_type = Entify.DataTypeToGLSLString[typeof(f).parameters[1]]
  glsl_arguments = join(
      map(x -> "$(Entify.DataTypeToGLSLString[x[2]]) $(x[1])",
          f.arguments),
      ", ")

  print(buffer, "$glsl_return_type $(f.name)($glsl_arguments) {\n")

  function PrintBody(body::String)
    print(buffer, f.function_body)
  end
  function PrintBody(body::ShaderExpr)
    print(buffer,
          "return ", GetMainGLSLCallCode!(ShaderBuilderContext(), body), ";")
  end
  PrintBody(f.function_body)
  print(buffer, "\n")
  print(buffer, "}\n")
end

function _FindFunctionReferences(f::ShaderFunction)::Set{ShaderFunction}
  references = Set{ShaderFunction}()
  function _PushReferences(::Any) end
  function _PushReferences(function_call::ShaderFunctionCall)
    push!(references, function_call.decl)
    for p in function_call.params
      _PushReferences(p)
    end
  end
  function _PushReferences(shader_source::ShaderSource)
    for s in shader_source.substitutions
      _PushReferences(s.second)
    end
  end

  _PushReferences(f.function_body)
  return references
end

# Recursively looks through a set of referenced shader functions and finds
# the shader functions they reference, returning a list of sets of shader
# functions in order of their dependencies (functions with no dependencies
# come first).
function _ExpandIntoDependencyLayers(
    referenced_shader_functions::Set{ShaderFunction})
  layers = Vector{Set{ShaderFunction}}()
  function_to_layer = Dict{ShaderFunction, Set{ShaderFunction}}()

  next_referenced_functions = copy(referenced_shader_functions)
  while !isempty(next_referenced_functions)
    push!(layers, next_referenced_functions)

    function_to_layer = map(x -> x => layers[end], collect(layers[end]))

    nf = _FindFunctionReferences.(collect(next_referenced_functions))
    next_referenced_functions =
        union(_FindFunctionReferences.(next_referenced_functions)...)
    for f in next_referenced_functions
      if f in function_to_layer
        # If the function is referenced in a previous layer, pull it out so
        # that it will appear in the next layer.
        pop!(function_to_layer[f], f)
      end
    end
  end
  return reverse(layers)
end

function GetDeclarationCode(referenced_shader_functions::Set{ShaderFunction})
  shader_function_layers =
      _ExpandIntoDependencyLayers(referenced_shader_functions)

  declaration_buffer = IOBuffer()

  declarations = Set{ShaderDeclaration}()
  for layer in shader_function_layers
    for f in layer
      union!(declarations, Set(f.helper_declarations))
    end
  end
  for helper_declaration in declarations
    if !isempty(helper_declaration.source)
      print(declaration_buffer, helper_declaration.source)
      print(declaration_buffer, "\n\n")
    end
  end

  for layer in shader_function_layers
    for f in layer
      PrintFunctionBodyCode!(declaration_buffer, f)
    end
  end
  return String(take!(declaration_buffer))
end

function GetMainGLSLCallCode!(context::ShaderBuilderContext,
                              inlined_source::ShaderSource)
  string_substitutions = map(
      x -> ("{{" * x.first * "}}" => GetMainGLSLCallCode!(context, x.second)),
      inlined_source.substitutions)

  return reduce(replace, string_substitutions,
                init=inlined_source.source_template)
end

function GetMainGLSLCallCode!(context::ShaderBuilderContext,
                              function_call::ShaderFunctionCall)
  push!(context.referenced_shader_functions, function_call.decl)
  parameter_list =
      join(map(x -> GetMainGLSLCallCode!(context, x), function_call.params),
           ", ")
  return "$(function_call.decl.name)($parameter_list)"
end

function GetDeclarationAndMainCode(output_assignments)
  context = ShaderBuilderContext()
  main_source_buffer = IOBuffer()

  for (lvalue, rvalue) in output_assignments
    expr_code = GetMainGLSLCallCode!(context, rvalue)
    print(main_source_buffer, "  $lvalue = $expr_code;\n")
  end
  
  return NamedTuple{(:declaration_code, :main_body_code)}((
      GetDeclarationCode(context.referenced_shader_functions),
      String(take!(main_source_buffer))))
end

function BuildShaderString(output_assignments)
  source = GetDeclarationAndMainCode(output_assignments)
  full_shader_code =
      source.declaration_code * "\n" *
      "void main() {\n" *
      source.main_body_code * "\n" *
      "}\n"
  return full_shader_code
end

end
