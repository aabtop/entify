module ShaderCommonFunctions

import Base.*
import Base./
import Base.+
import Base.-
import Base.<
import Base.getindex
import Base.length
import Base.max
import Base.min
import LinearAlgebra.dot
using ..Entify
using ..Entify.Shaders
using StaticArrays

export ShaderVec, Sample, mix, pow

@DefineInlineShaderFunction(*(m::mat4, x::vec4)::vec4, "{{m}} * {{x}}")

*(x::ShaderExpr{Float32}, y::ShaderExpr{T}) where {T} =
    ShaderSource{T}("({{x}}) * ({{y}})", ["x" => x, "y" => y])

*(x::ShaderExpr{T}, y::ShaderExpr{Float32}) where {T<:SVector} = y * x

*(x::ShaderExpr{T}, y::ShaderExpr{T}) where {T<:SVector} =
    ShaderSource{T}("({{x}}) * ({{y}})", ["x" => x, "y" => y])

/(x::ShaderExpr{Float32}, y::ShaderExpr{T}) where {T} =
    ShaderSource{T}("({{x}}) / ({{y}})", ["x" => x, "y" => y])

/(x::ShaderExpr{T}, y::ShaderExpr{Float32}) where {T<:SVector} = y / x

/(x::ShaderExpr{T}, y::ShaderExpr{T}) where {T<:SVector} =
    ShaderSource{T}("({{x}}) / ({{y}})", ["x" => x, "y" => y])

+(x::ShaderExpr{T}, y::ShaderExpr{T}) where {T} =
    ShaderSource{T}("({{x}}) + ({{y}})", ["x" => x, "y" => y])

-(x::ShaderExpr{T}, y::ShaderExpr{T}) where {T} =
    ShaderSource{T}("({{x}}) - ({{y}})", ["x" => x, "y" => y])

length(x::ShaderExpr{<:SVector}) =
    ShaderSource{Float32}("length({{x}})", ["x" => x])
dot(x::ShaderExpr{T}, y::ShaderExpr{T}) where {T<:SVector} =
    ShaderSource{Float32}("dot({{x}}, {{y}})", ["x" => x, "y" => y])

<(x::ShaderExpr{Float32}, y::ShaderExpr{Float32}) = ShaderSource{Float32}(
    "(({{x}}) < ({{y}}) ? 1.0 : 0.0)", ["x" => x, "y" => y])

min(x::ShaderExpr{Float32}, y::ShaderExpr{Float32}) = ShaderSource{Float32}(
    "min({{x}}, {{y}})", ["x" => x, "y" => y])

max(x::ShaderExpr{Float32}, y::ShaderExpr{Float32}) = ShaderSource{Float32}(
    "max({{x}}, {{y}})", ["x" => x, "y" => y])

mix(a::ShaderExpr{T}, b::ShaderExpr{T}, x::ShaderExpr{Float32}) where {T} =
    ShaderSource{T}(
        "mix({{a}}, {{b}}, {{x}})", ["a" => a, "b" => b, "x" => x])

pow(x::ShaderExpr{T}, y::ShaderExpr{Float32}) where {T} = ShaderSource{T}(
    "pow({{x}}, {{y}})", ["x" => x, "y" => y])

getindex(x::ShaderExpr{<:SVector}, i::Integer) =
    ShaderSource{typeof(x).parameters[1].parameters[2]}("{{x}}[$(i - 1)]",
                                                        ["x" => x])

function ShaderVec(values...)
  ToExpr(x::ShaderExpr) = x
  ToExpr(x::Float32) = ShaderSource{Float32}(string(Float64(x)))
  ToExpr(x::Float64) = ShaderSource{Float32}(string(x))
  ToExpr(x::SVector{N, Float32}) where {N} = ShaderSource{SVector{N, Float32}}(
      Entify.DataTypeToGLSLString[typeof(x)] * "(" * join(string.(x), ", ") *
      ")")

  NumComponents(::ShaderExpr{Float32}) = 1
  NumComponents(::ShaderExpr{SVector{N, Float32}}) where {N} = N

  values_as_exprs = ToExpr.(values)
  total_components = reduce(+, NumComponents.(values_as_exprs))

  if total_components == 1 return values_as_exprs[1] end

  enumerated_values = enumerate(values_as_exprs)
  return_type = SVector{total_components, Float32}
  return ShaderSource{return_type}(
      Entify.DataTypeToGLSLString[return_type] * "(" *
      join(map(x -> "{{$(x[1])}}", enumerated_values), ", ") *  ")",
      [map(x -> string(x[1]) => x[2], enumerated_values)...])
end

@DefineInlineShaderFunction(Sample(sampler::Sampler, texcoords::vec2)::vec4,
                            "texture2D({{sampler}}, {{texcoords}})")

end
