module Shapes

export FilledCircle

using ....Entify
using ....Entify.Shaders
using ....Entify.ShaderCommonFunctions

using LinearAlgebra

filled_circle_fragment_shader = @GLSLFragmentShaderFromBuilder(
    (v_color::vec4, v_normalized_position::vec2), (),
    v_color *
        (dot(v_normalized_position, v_normalized_position) < ShaderVec(1.0)))
filled_circle_vertex_shader = @GLSLVertexShaderFromBuilder(
    (a_color::vec4, a_position::vec4, a_normalized_position::vec2), (), (),
    ((v_color = a_color),
     (v_normalized_position = a_normalized_position)),
    a_position)
filled_circle_pipeline = Pipeline(
    filled_circle_vertex_shader, filled_circle_fragment_shader)


function FilledCircle(
    center::vec2, radius::AbstractFloat, color::vec4, transform_matrix::mat4)
  TransformCorner(x) =
      let point = center + x
        transform_matrix * vec4(point[1], point[2], 0.0, 1.0)
      end

  tl = (color, TransformCorner(vec2(-radius, radius)), vec2(-1.0, 1.0))
  tr = (color, TransformCorner(vec2(radius, radius)), vec2(1.0, 1.0))
  br = (color, TransformCorner(vec2(radius, -radius)), vec2(1.0, -1.0))
  bl = (color, TransformCorner(vec2(-radius, -radius)), vec2(-1.0, -1.0))

  vertex_buffer = VertexBuffer(
      Vector{Tuple{vec4, vec4, vec2}}([bl, tr, tl, bl, br, tr]))

  return DrawCall(filled_circle_pipeline, vertex_buffer)
end

end
