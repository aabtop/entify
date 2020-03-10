module PathShapes

using ....Entify

export Star
export RegularStarPolygon, Pentagram
export RegularPolygon, Pentagon, Hexagon, Septagon, Octagon
export Rectangle, Square

function Star(
    outer_radius::AbstractFloat, inner_radius::AbstractFloat,
    num_points::Signed)::Vector{vec2}
  points = Vector{vec2}(undef, num_points * 2)
  radians_per_point = (2.0 * pi) / length(points)

  for i in 1:2:length(points)
    theta_outer = radians_per_point * (i - 1)
    theta_inner = theta_outer + radians_per_point
    points[i] = vec2(cos(theta_outer), sin(theta_outer)) * outer_radius
    points[i + 1] = vec2(cos(theta_inner), sin(theta_inner)) * inner_radius
  end

  return points
end

function RegularStarPolygon(
    radius::AbstractFloat, num_points::Signed,
    vertex_step::Signed)::Vector{vec2}
  radians_per_point = (2.0 * pi) / num_points
  return map(function(x)
               theta = radians_per_point * ((x * vertex_step) % num_points)
               return vec2(cos(theta), sin(theta)) * radius
             end,
             0:(num_points - 1))
end
Pentagram(radius) = RegularStarPolygon(radius, 5, 2)

RegularPolygon(radius, num_sides) = RegularStarPolygon(radius, num_sides, 1)
Pentagon(radius) = RegularPolygon(radius, 5)
Hexagon(radius) = RegularPolygon(radius, 6)
Septagon(radius) = RegularPolygon(radius, 7)
Octagon(radius) = RegularPolygon(radius, 8)

Rectangle(left::AbstractFloat, top::AbstractFloat,
          right::AbstractFloat, bottom::AbstractFloat)::Vector{vec2} =
    Vector{vec2}([(left, bottom), (left, top), (right, top), (right, bottom)])

Rectangle(width::AbstractFloat, height::AbstractFloat) =
    let half_width = width / 2,
        half_height = height / 2
      Rectangle(-half_width, half_height, half_width, -half_height)
    end

Square(width) = Rectangle(width, width)

end
