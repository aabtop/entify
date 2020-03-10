module Affine

using ..Entify
using LinearAlgebra
using Rotations

export scale, scale2d, scale3d, translate, translateX, translateY, translateZ,
       rotateX, rotateY, rotateZ, identity_matrix

identity_matrix = mat4([1 0 0 0;
                        0 1 0 0;
                        0 0 1 0;
                        0 0 0 1])

scale(x::AbstractFloat, y::AbstractFloat, z::AbstractFloat = 1.0f0) = mat4([
    x 0 0 0;
    0 y 0 0;
    0 0 z 0;
    0 0 0 1])

scale2d(s::AbstractFloat) = scale(s, s)
scale3d(s::AbstractFloat) = scale(s, s, s)

translate(x::AbstractFloat, y::AbstractFloat, z::AbstractFloat = 0.0f0) = mat4([
    1 0 0 x;
    0 1 0 y;
    0 0 1 z;
    0 0 0 1])
translateX(x::AbstractFloat) = translate(x, 0.0f0, 0.0f0)
translateY(y::AbstractFloat) = translate(0.0f0, y, 0.0f0)
translateZ(z::AbstractFloat) = translate(0.0f0, 0.0f0, z)

function rotateX(theta_in_radians::AbstractFloat)
  m44 = Matrix{Float32}(I, 4, 4)
  m44[1:3, 1:3] = RotX(theta_in_radians)
  return mat4(m44)
end
function rotateY(theta_in_radians::AbstractFloat)
  m44 = Matrix{Float32}(I, 4, 4)
  m44[1:3, 1:3] = RotY(theta_in_radians)
  return mat4(m44)
end
function rotateZ(theta_in_radians::AbstractFloat)
  m44 = Matrix{Float32}(I, 4, 4)
  m44[1:3, 1:3] = RotZ(theta_in_radians)
  return mat4(m44)
end

end
