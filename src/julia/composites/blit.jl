module Blit

using ....Entify
using ....Entify.Shaders
using ....Entify.ShaderCommonFunctions

export CreateBlitter, CreateBlitterVertexBufferFromRect,
       FullscreenBlitterVertexBuffer, BlitDirect

_blitter_vertex_shader = @GLSLVertexShaderFromBuilder(
    (a_position::vec4, a_texcoord::vec2), (), (),
    ((v_texcoord = a_texcoord),),
    a_position)

CreateBlitterVertexBufferFromRect(left::AbstractFloat, top::AbstractFloat,
                                  right::AbstractFloat, bottom::AbstractFloat) =
    VertexBuffer(
        Vector{Tuple{vec4, vec2}}(
            [ ((left, top, 0.0, 1.0), (0.0, 1.0)),
              ((left, bottom, 0.0, 1.0), (0.0, 0.0)),
              ((right, top, 0.0, 1.0), (1.0, 1.0)),
              ((right, top, 0.0, 1.0), (1.0, 1.0)),
              ((left, bottom, 0.0, 1.0), (0.0, 0.0)),
              ((right, bottom, 0.0, 1.0), (1.0, 0.0))]))

FullscreenBlitterVertexBuffer =
    CreateBlitterVertexBufferFromRect(-1.0, 1.0, 1.0, -1.0)

function CreateBlitter(
    fragment_shader::GLSLFragmentShader{Tuple{vec2}, U}) where {U}
  pipeline = Pipeline(_blitter_vertex_shader, fragment_shader)

  function Blit(fragment_shader_uniforms::UniformValues{U},
                vertex_buffer::VertexBuffer{Tuple{vec4, vec2}}=
                    FullscreenBlitterVertexBuffer)
    return DrawCall(pipeline, vertex_buffer, nothing, fragment_shader_uniforms)
  end
  function Blit(fragment_shader_uniforms::UniformValues{U},
                left::AbstractFloat, top::AbstractFloat,
                right::AbstractFloat, bottom::AbstractFloat)
    return Blit(fragment_shader_uniforms,
                CreateBlitterVertexBufferFromRect(left, top, right, bottom))
  end

  return Blit
end

DIRECT_BLITTER = CreateBlitter(@GLSLFragmentShaderFromBuilder(
    (v_texcoord::vec2,), (u_source_image::Sampler,),
    Sample(u_source_image, v_texcoord)))

BlitDirect(texture::Texture, vertex_buffer=FullscreenBlitterVertexBuffer) =
    DIRECT_BLITTER(
        UniformValues(Sampler(texture, wrap=SamplerWrapTypeTypeClamp)),
        vertex_buffer)

BlitDirect(texture::Texture, left, top, right, bottom) = DIRECT_BLITTER(
    UniformValues(Sampler(texture, wrap=SamplerWrapTypeTypeClamp)),
    left, top, right, bottom)

end
