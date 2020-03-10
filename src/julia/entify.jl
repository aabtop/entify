module Entify

# Make sure we use the FlatBuffers that we package ourselves.
pushfirst!(LOAD_PATH, @__DIR__)

include("./entify_lib.jl")
include("./renderer_definitions_generated.jl")

using ColorTypes
using FixedPointNumbers
using FlatBuffers
using SHA
using StaticArrays

vec2 = SVector{2, Float32}
vec3 = SVector{3, Float32}
vec4 = SVector{4, Float32}
mat4 = SMatrix{4, 4, Float32, 16}
export vec2, vec3, vec4, mat4

macro ImportEnum(enum_name, do_export = :nothing)
  get_export_line(symbol) =
      do_export == :no_export ? nothing : :(export $(symbol))

  result = esc(quote
    # First import the enum type name itself
    $enum_name = entify.renderer.$enum_name
    $(get_export_line(enum_name))

    # Then import the enum values.
    $(map(x -> quote
              $(Symbol(string(x))) = entify.renderer.$(Symbol(string(x)))
              $(get_export_line(Symbol(string(x))))
          end,
          instances(eval(:(entify.renderer.$enum_name))))...)
  end)
  return result
end

@ImportEnum(PrimitiveType, no_export)
NamedPrimitiveType = entify.renderer.NamedPrimitiveType

@ImportEnum(BlendCoefficient)
BlendParameters = entify.renderer.BlendParameters
export BlendParameters

@ImportEnum(SamplerWrapType)
@ImportEnum(SamplerFilterType)
@ImportEnum(PixelType)

_RendererNode(x::entify.renderer.RendererNodeUnion) =
    entify.renderer.RendererNode(
        FlatBuffers.typeorder(entify.renderer.RendererNodeUnion, typeof(x)), x)
_DrawTree(x::entify.renderer.DrawTreeUnion) = _RendererNode(
    entify.renderer.DrawTree(
    FlatBuffers.typeorder(entify.renderer.DrawTreeUnion, typeof(x)), x))
_Texture(x::entify.renderer.TextureUnion) = _RendererNode(
    entify.renderer.Texture(
    FlatBuffers.typeorder(entify.renderer.TextureUnion, typeof(x)), x))


abstract type Node end
abstract type DrawTree <: Node end
abstract type Texture <: Node end

export Node, DrawTree, Texture

struct NodeInfo
  data::Vector{UInt8}
  id::Lib.EntifyId
  children::Vector{NodeInfo}
end

# Searches through the parameters tuple and finds all children parameters (e.g.
# "Node"s), and maps them to their EntifyIds.  Also returns the list of
# children.
function _MapChildren(params_with_types_expr)
  children = Vector{Expr}()

  function MapParam(p)
    if p.head == :(=)
      # If a default value is specified, only look at the left side of it.
      p = p.args[1]
    end

    @assert (p.head == :(::))
    param_name = p.args[1]
    type_name = p.args[2]

    if eval(type_name) <: Node
      push!(children, :($(param_name).node_info))
      return :($param_name.node_info.id)
    elseif eval(type_name) <: Array{<:Node}
      push!(children, :(map(x -> x.node_info, $(param_name))...))
      return :(map(x -> x.node_info.id, $(param_name)))
    elseif eval(type_name) <: Union{T, Nothing} where T <: Node
      GetNodeId(x::T where T <: Node) = x.node_info.id
      GetNodeId(x::Nothing) = 0
      GetNodeInfoAsList(x::T where T <: Node) = [x.node_info]
      GetNodeInfoAsList(x::Nothing) = []
      push!(children, :($(GetNodeInfoAsList)($(param_name))...))
      return :($(GetNodeId)($(param_name)))
    else
      return param_name
    end
  end

  mapped = map(MapParam, params_with_types_expr)

  return (mapped, children)
end

function _GetDataHash(data::Vector{UInt8})
  data_hash = Vector{UInt8}(undef, sizeof(Lib.EntifyId))

  blake2s_result = Lib.blake2s(
      pointer(data_hash), pointer(data), C_NULL, sizeof(data_hash),
      sizeof(data), 0)

  @assert (blake2s_result == 0) "Failed to hash."

  return reinterpret(Lib.EntifyId, data_hash)[1]
end

macro MakeWrapper(node_name, base_type, flatbuffer_constructor, params)
  return :(@MakeWrapper(
      $node_name, $node_name, $base_type, $flatbuffer_constructor, $params))
end

macro MakeWrapper(
    input_node_name, output_node_name, base_type, flatbuffer_constructor,
    params)
  flatbuffer_struct_expr = :(entify.renderer.$input_node_name)
  flatbuffer_struct = eval(flatbuffer_struct_expr)

  if params == :nothing
    params = Expr(:tuple, map(
        x -> :(($(x[1])::$(x[2]))),
        zip(fieldnames(flatbuffer_struct), flatbuffer_struct.types))...)
  end

  @assert (params.head == :tuple)
  flatbuffer_params, children = _MapChildren(params.args)

  expr = :(
      struct $output_node_name <: $base_type
        node_info::NodeInfo

        $(params.args...)

        function $output_node_name($(params.args...))
          data = copy(FlatBuffers.bytes(FlatBuffers.build!(
              $flatbuffer_constructor(
                  $flatbuffer_struct_expr($(flatbuffer_params...))))))

          return new(NodeInfo(data, _GetDataHash(data), [$(children...)]),
                     $(map(x -> x.args[1], params.args)...))
        end
      end)
  return expr
end

macro MakeNodeWrapper(node_name, params=:nothing)
  return :(@MakeWrapper($node_name, Node, _RendererNode, $params))
end

macro MakeTextureWrapper(node_name, params=:nothing)
  return :(@MakeWrapper($node_name, Texture, _Texture, $params))
end

# Represents a handle to data already submitted to the backend that we'd like
# to forget in the frontend.  You might use this for example if you're trying
# you are building an infinite chain of render targets where each one is
# a transformation of the previous one, but don't want to pay an infinite
# cost of storing the description of the infinite chain in memory.
# Note that creation of this object implies an allocation (via SubmitReference),
# and it must be ensured somehow that this allocation is deallocated (via
# Lib.EntifyReleaseReference).
macro DefineReferenceNode(abstract_type, keep_fields)
  @assert eval(abstract_type) <: Node

  return esc(:(
      struct ReferenceNode{T<:$abstract_type} <: $abstract_type
        node_info::NodeInfo
        ref::Ptr{Lib.EntifyReference}

        $(keep_fields.args...)

        function ReferenceNode(
            context::Ptr{Lib.EntifyContext}, node::$abstract_type)
          reference = SubmitReference(context, node)

          return new{$abstract_type}(
              NodeInfo(Vector{UInt8}(), node.node_info.id, Vector{NodeInfo}()),
              reference,
              $(map(x -> :(node.$(x.args[1])), keep_fields.args)...))
        end
      end))
end
@DefineReferenceNode(
    Texture, (width_in_pixels::Signed, height_in_pixels::Signed))
export ReferenceNode

@MakeTextureWrapper(PixelData)
# The provided |image| is in column-major format with origin in bottom left.
function PixelData(image::Array{T, 2} where T)
  image = transpose(image)[:,end:-1:1]
  return PixelData(
      Int32(size(image)[1]), Int32(size(image)[2]),
      Int32(size(image)[1] * sizeof(typeof(image).parameters[1])),
      ColorTypeToEntify(typeof(image).parameters[1]),
      Vector{UInt8}(reinterpret(UInt8, reshape(image, length(image)))))
end
export PixelData

@MakeTextureWrapper(RenderTarget, (
    width_in_pixels::Signed, height_in_pixels::Signed, draw_tree::DrawTree))
export RenderTarget

@MakeNodeWrapper(Sampler, (
    texture::Texture,
    wrap_s::SamplerWrapType,
    wrap_t::SamplerWrapType,
    min_filter::SamplerFilterType,
    max_filter::SamplerFilterType))
Sampler(texture::Texture;
        wrap=SamplerWrapTypeTypeRepeat, filt=SamplerFilterTypeTypeLinear) =
    Sampler(
        texture, wrap, wrap, filt, filt)
export Sampler

DataTypeToPrimitiveType = Dict(
  Sampler => PrimitiveTypeSampler,
  Float32 => PrimitiveTypeFloat32V1,
  SVector{2, Float32} => PrimitiveTypeFloat32V2,
  SVector{3, Float32} => PrimitiveTypeFloat32V3,
  SVector{4, Float32} => PrimitiveTypeFloat32V4,
  SMatrix{4, 4, Float32, 16} => PrimitiveTypeFloat32M44,
  UInt8 => PrimitiveTypeUInt8V1,
  SVector{2, UInt8} => PrimitiveTypeUInt8V2,
  SVector{3, UInt8} => PrimitiveTypeUInt8V3,
  SVector{4, UInt8} => PrimitiveTypeUInt8V4)
ToPrimitiveType(t) = DataTypeToPrimitiveType[typeof(t)]

NamedPrimitiveTypeFromTuple(t) =
    NamedPrimitiveType(DataTypeToPrimitiveType[t[1]], t[2])
PrimitiveTypeFromTuple(t) = DataTypeToPrimitiveType[t[1]]

DataTypeToGLSLString = Dict(
  Float32 => "float",
  SVector{2, Float32} => "vec2",
  SVector{3, Float32} => "vec3",
  SVector{4, Float32} => "vec4",
  SMatrix{4, 4, Float32, 16} => "mat4",
  Sampler => "sampler2D")
GetGLSLDeclarationLine(type::String, decl::Tuple{DataType, String}) =
    "$type $(DataTypeToGLSLString[decl[1]]) $(decl[2]);\n"

abstract type AbstractGLSLVertexShader <: Node end
@MakeWrapper(
    GLSLVertexShader, GLSLVertexShaderUntyped, AbstractGLSLVertexShader,
    _RendererNode, nothing)
function GLSLVertexShaderUntyped(
    input_attributes::Tuple{Vararg{Tuple{DataType, String}}},
    output_attributes::Tuple{Vararg{Tuple{DataType, String}}},
    uniforms::Tuple{Vararg{Tuple{DataType, String}}},
    source::String)
  # Automatically add declarations to the shader source code for all
  # input/output attributes and uniforms.
  shader_declarations =
      join(GetGLSLDeclarationLine.("attribute", input_attributes), "\n") *
      "\n" *
      join(GetGLSLDeclarationLine.("varying", output_attributes)) *
      "\n" *
      join(GetGLSLDeclarationLine.("uniform", uniforms)) *
      "\n"

  return GLSLVertexShaderUntyped(
      Vector{NamedPrimitiveType}(
          [NamedPrimitiveTypeFromTuple.(input_attributes)...]),
      Vector{PrimitiveType}(
          [PrimitiveTypeFromTuple.(output_attributes)...]),
      Vector{NamedPrimitiveType}(
          [NamedPrimitiveTypeFromTuple.(uniforms)...]),
      shader_declarations * source)
end
struct GLSLVertexShader{i, o, u} <: AbstractGLSLVertexShader
  node_info::NodeInfo

  function GLSLVertexShader(
      input_attributes::Tuple{Vararg{Tuple{DataType, String}}},
      output_attributes::Tuple{Vararg{Tuple{DataType, String}}},
      uniforms::Tuple{Vararg{Tuple{DataType, String}}},
      source::String)
    return new{
        Tuple{map(x -> x[1], input_attributes)...},
        Tuple{map(x -> x[1], output_attributes)...},
        Tuple{map(x -> x[1], uniforms)...}}(
      GLSLVertexShaderUntyped(
          input_attributes, output_attributes, uniforms, source).node_info)
  end
end

export GLSLVertexShaderUntyped
export GLSLVertexShader

abstract type AbstractGLSLFragmentShader <: Node end
@MakeWrapper(
    GLSLFragmentShader, GLSLFragmentShaderUntyped, AbstractGLSLFragmentShader,
    _RendererNode, nothing)
function GLSLFragmentShaderUntyped(
    input_attributes::Tuple{Vararg{Tuple{DataType, String}}},
    uniforms::Tuple{Vararg{Tuple{DataType, String}}},
    source::String)
  shader_preamble = "precision highp float;\n"
  # Automatically add declarations to the shader source code for all
  # input attributes and uniforms.
  shader_declarations =
      join(GetGLSLDeclarationLine.("varying", input_attributes)) *
      "\n" *
      join(GetGLSLDeclarationLine.("uniform", uniforms)) *
      "\n"

  return GLSLFragmentShaderUntyped(
      Vector{PrimitiveType}(
          [PrimitiveTypeFromTuple.(input_attributes)...]),
      Vector{NamedPrimitiveType}(
          [NamedPrimitiveTypeFromTuple.(uniforms)...]),
      shader_preamble * shader_declarations * source)
end
struct GLSLFragmentShader{i, u} <: AbstractGLSLFragmentShader
  node_info::NodeInfo

  function GLSLFragmentShader(
      input_attributes::Tuple{Vararg{Tuple{DataType, String}}},
      uniforms::Tuple{Vararg{Tuple{DataType, String}}},
      source::String)
    return new{
        Tuple{map(x -> x[1], input_attributes)...},
        Tuple{map(x -> x[1], uniforms)...}}(
      GLSLFragmentShaderUntyped(input_attributes, uniforms, source).node_info)
  end
end

export GLSLFragmentShaderUntyped
export GLSLFragmentShader

abstract type AbstractPipeline <: Node end
@MakeWrapper(
    Pipeline, PipelineUntyped, AbstractPipeline,
    _RendererNode, (
    vertex_shader::AbstractGLSLVertexShader,
    fragment_shader::AbstractGLSLFragmentShader,
    blend_parameters::BlendParameters))
PipelineUntyped(vertex_shader::AbstractGLSLVertexShader,
                fragment_shader::AbstractGLSLFragmentShader) =
    PipelineUntyped(vertex_shader, fragment_shader,
             Entify.BlendParameters(
                 Entify.BlendCoefficientOne,
                 Entify.BlendCoefficientOneMinusSrcAlpha,
                 Entify.BlendCoefficientOne,
                 Entify.BlendCoefficientOneMinusSrcAlpha))
struct Pipeline{i, vu, fu} <: AbstractPipeline
  node_info::NodeInfo

  function Pipeline(
      vertex_shader::GLSLVertexShader{vi, vo, vu},
      fragment_shader::GLSLFragmentShader{vo, fu}) where {vi, vo, vu, fu}
    return new{vi, vu, fu}(
        PipelineUntyped(vertex_shader, fragment_shader).node_info)
  end
end

export PipelineUntyped
export Pipeline

abstract type AbstractVertexBuffer <: Node end
@MakeWrapper(
    VertexBuffer, VertexBufferUntyped, AbstractVertexBuffer,
    _RendererNode, nothing)
function VertexBufferUntyped(vertices::Vector{<:Tuple})
  @assert (length(vertices) > 0)
  types = ToPrimitiveType.(vertices[1])
  offsets = accumulate(+, sizeof.([vertices[1]...]))
  return VertexBufferUntyped([types...], Int32(offsets[end]),
                      Vector{UInt8}(reinterpret(UInt8, vertices)),
                      Int32.(vcat([0], offsets[1:end-1])))
end

struct VertexBuffer{t} <: AbstractVertexBuffer
  node_info::NodeInfo

  function VertexBuffer(vertices::Vector{<:Tuple})
    return new{Tuple{typeof.(vertices[1])...}}(
        VertexBufferUntyped(vertices).node_info)
  end
end
export VertexBufferUntyped
export VertexBuffer

function ColorTypeToEntify(type::Type)::PixelType
  if type <: ColorTypes.RGB{FixedPointNumbers.Normed{UInt8,8}}
    return PixelTypeRGB
  elseif type <: ColorTypes.RGBA{FixedPointNumbers.Normed{UInt8,8}}
    return PixelTypeRGBA
  else
    error("Invalid pixel type")
  end
end

abstract type AbstractUniformValues <: Node end
@MakeWrapper(
    UniformValues, UniformValuesUntyped, AbstractUniformValues,
    _RendererNode, (
        types::Vector{PrimitiveType},
        data::Vector{UInt8},
        samplers::Vector{Sampler}))
function UniformValuesUntyped(values...)
  entify_types = map(ToPrimitiveType, [values...])
  non_sampler_data = Vector{UInt8}()
  samplers = Vector{Sampler}()

  AppendData!(value::Sampler) = push!(samplers, value)
  AppendData!(value::T where T) =
      append!(non_sampler_data, reinterpret(UInt8, [value]))

  for value in values
    AppendData!(value)
  end

  return UniformValuesUntyped(entify_types, non_sampler_data, samplers)
end
struct UniformValues{t} <: AbstractUniformValues
  node_info::NodeInfo

  function UniformValues(values...)
    return new{Tuple{map(typeof, [values...])...}}(
        UniformValuesUntyped(values...).node_info)
  end
end
export UniformValuesUntyped
export UniformValues

abstract type AbstractDrawCall <: DrawTree end
@MakeWrapper(
    DrawCall, DrawCallUntyped, AbstractDrawCall, _DrawTree,
    ( pipeline::AbstractPipeline, vertex_buffer::AbstractVertexBuffer,
      vertex_uniform_values::Union{AbstractUniformValues, Nothing},
      fragment_uniform_values::Union{AbstractUniformValues, Nothing}))
DrawCallUntyped(pipeline::AbstractPipeline,
                vertex_buffer::AbstractVertexBuffer) =
    DrawCallUntyped(pipeline, vertex_buffer, nothing, nothing)
struct DrawCall{vi, vu, fu} <: AbstractDrawCall
  node_info::NodeInfo

  function DrawCall(
      pipeline::Pipeline{vi, vu, fu}, vertex_buffer::VertexBuffer{vi},
      vertex_uniform_values::Union{UniformValues{vu}, Nothing},
      fragment_uniform_values::Union{UniformValues{fu}, Nothing}) where
          {vi, vu, fu}
    return new{vi, vu, fu}(DrawCallUntyped(
        pipeline, vertex_buffer, vertex_uniform_values,
        fragment_uniform_values).node_info)
  end
end

DrawCall(pipeline, vertex_buffer) =
    DrawCall(pipeline, vertex_buffer, nothing, nothing)

export DrawCallUntyped
export DrawCall

@MakeWrapper(DrawSequence, DrawTree, _DrawTree, (draw_trees::Vector{<:DrawTree},))
export DrawSequence

@MakeWrapper(DrawSet, DrawTree, _DrawTree, (draw_trees::Vector{<:DrawTree},))
export DrawSet

struct ParseError
  message::String
end
function Base.showerror(io::IO, err::ParseError)
  print(io, "Entify.ParseError: ")
  print(io, err.message)
end
export ParseError

function SubmitReference(context::Ptr{Lib.EntifyContext},
                         node_info::NodeInfo)::Ptr{Lib.EntifyReference}
  reference = Lib.EntifyTryGetReferenceFromId(context, node_info.id)
  if reference != C_NULL
    return reference
  end

  child_references::Vector{Ptr{Lib.EntifyReference}} = []
  try
    for x in node_info.children
      push!(child_references, SubmitReference(context, x))
    end

    reference = Lib.EntifyCreateReferenceFromFlatBuffer(
        context, node_info.id, node_info.data, sizeof(node_info.data))
    error_message::Vector{Cstring} = [C_NULL]
    if Lib.EntifyGetLastError(context, error_message) != 0
      throw(ParseError(unsafe_string(error_message[1])))
    end
  finally
    for child_reference in child_references
      Lib.EntifyReleaseReference(context, child_reference)
    end
  end

  return reference
end

SubmitReference(context::Ptr{Lib.EntifyContext}, node::Node) =
    SubmitReference(context, node.node_info)

export SubmitReference

function Submit(context::Ptr{Lib.EntifyContext},
                render_target::Ptr{Lib.EntifyRenderTarget},
                draw_tree::DrawTree)
  draw_tree_reference = SubmitReference(context, draw_tree.node_info)

  Lib.EntifySubmit(context, draw_tree_reference, render_target)

  Lib.EntifyReleaseReference(context, draw_tree_reference)
end
export Submit

include("shaders.jl")
include("affine.jl")
include("shader_common_functions.jl")

include("./composites/composites.jl")

end
