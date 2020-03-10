module Lines

using ....Entify
using ....Entify.Shaders
using ....Entify.ShaderCommonFunctions

using LinearAlgebra

export LinePath, Polygon

# Simply set the pixel color as determined by the vertex shader.
fragment_shader = @GLSLFragmentShaderFromBuilder(
    (v_color::vec4,), (), v_color)

# Pass through the color and position to the fragment shader.
vertex_shader = @GLSLVertexShaderFromBuilder(
    (a_position::vec2, a_color::vec4), (u_transform::mat4,), (),
    ((v_color = a_color),),
    u_transform * ShaderVec(a_position, 0.0, 1.0))

pipeline = Pipeline(vertex_shader, fragment_shader)

# Returns a normal pointing to the "left" of the segment.
_PerpVector(vec::vec2) = vec2(-vec[2], vec[1])

function _ComputeSegmentNormal(prev_point::vec2, next_point::vec2)
  diff = next_point - prev_point
  perpendicular = _PerpVector(diff)
  return perpendicular / norm(perpendicular)
end

function _ComputeSegmentNormals(points::Vector{vec2}, closed_path::Bool)
  num_segments = length(points) - (closed_path ? 0 : 1)
  segment_normals = Vector{vec2}(undef, num_segments)
  for p in 1:(length(points) - 1)
    segment_normals[p] = _ComputeSegmentNormal(points[p], points[p + 1])
  end
  if closed_path
    segment_normals[end] = _ComputeSegmentNormal(points[end], points[1])
  end
  return segment_normals
end

_ThickPointsFromSegmentNormal(point::vec2, normal::vec2, radius::Float32) =
    (point + normal * radius, point - normal * radius)

function _ComputeThickPoints(
    point::vec2, prev_segment::vec2, next_segment::vec2,
    radius::Float32)::Tuple{vec2, vec2}
  # We use Cramer's rule to compute the intersection of two lines described
  # given a point an a normal.
  det(x::vec2, y::vec2) = x[1] * y[2] - x[2] * y[1]
  determinant = det(prev_segment, next_segment)
  if iszero(determinant)
    return _ThickPointsFromSegmentNormal(point, next_segment, radius)
  end
  inv_determinant = 1.0 / determinant

  function _IntersectLines(c::vec2)
    return vec2(
        det(c, vec2(prev_segment[2], next_segment[2])) * inv_determinant,
        det(vec2(prev_segment[1], next_segment[1]), c) * inv_determinant)
  end

  c = vec2(dot(point, prev_segment), dot(point, next_segment))

  return (_IntersectLines(c + radius), _IntersectLines(c - radius))
end

function _ComputeThickPoints(
    points::Vector{vec2}, segment_normals::Vector{vec2},
    width_radius::Float32)::Vector{Tuple{vec2, vec2}}
  thick_points = Vector{Tuple{vec2, vec2}}(undef, length(points))
  closed_path = (length(points) == length(segment_normals))

  thick_points[1] = closed_path ?
      _ComputeThickPoints(points[1], segment_normals[end], segment_normals[1],
                          width_radius) :
      _ThickPointsFromSegmentNormal(points[1], segment_normals[1], width_radius)

  for p in 2:(length(points) - 1)
    thick_points[p] = _ComputeThickPoints(
        points[p], segment_normals[p - 1], segment_normals[p], width_radius)
  end

  thick_points[end] = closed_path ?
      _ComputeThickPoints(
          points[end], segment_normals[end - 1], segment_normals[end],
          width_radius) :
      _ThickPointsFromSegmentNormal(
          points[end], segment_normals[end], width_radius)

  return thick_points
end

function GetLinePathTriangleVertices(
    points::Vector{vec2}, width_radius::AbstractFloat, color::vec4,
    closed_path::Bool)
  Vertex = Tuple{vec2, vec4}
  segment_normals = _ComputeSegmentNormals(points, closed_path)
  thick_points = _ComputeThickPoints(
      points, segment_normals, Float32(width_radius))
  vertices = Vector{Vertex}(undef, 6 * length(segment_normals))

  function PopulateVertex!(p_start, p_end)
    vertex_index = (p_start - 1) * 6
    vertices[vertex_index + 1] = (thick_points[p_start][1], color)
    vertices[vertex_index + 2] = (thick_points[p_end][2], color)
    vertices[vertex_index + 3] = (thick_points[p_end][1], color)
    vertices[vertex_index + 4] = (thick_points[p_start][2], color)
    vertices[vertex_index + 5] = (thick_points[p_end][2], color)
    vertices[vertex_index + 6] = (thick_points[p_start][1], color)
  end

  for p in 1:(length(points) - 1)
    PopulateVertex!(p, p + 1)
  end

  if closed_path
    PopulateVertex!(length(points), 1)
  end

  return vertices
end

function LinePath(
    points::Vector{vec2}, width_radius::AbstractFloat, color::vec4,
    transform_matrix::mat4; closed_path::Bool = false)
  return DrawCall(pipeline,
                  VertexBuffer(GetLinePathTriangleVertices(
                      points, width_radius, color, closed_path)),
                  UniformValues(transform_matrix), nothing)
end

Polygon(points::Vector{vec2}, width_radius::AbstractFloat, color::vec4,
        transform_matrix::mat4) = LinePath(
    points, width_radius, color, transform_matrix, closed_path=true)

end
