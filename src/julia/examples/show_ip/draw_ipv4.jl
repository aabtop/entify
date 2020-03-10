TOP = 0.8
MIDDLE = 0.0
BOTTOM = -TOP
LEFT = -0.4
RIGHT = -LEFT
TL = vec2(LEFT, TOP)
TR = vec2(RIGHT, TOP)
ML = vec2(LEFT, MIDDLE)
MR = vec2(RIGHT, MIDDLE)
BL = vec2(LEFT, BOTTOM)
BR = vec2(RIGHT, BOTTOM)

struct ClosedPath
  path::Vector{vec2}
end

OpenOrClosedPath = Union{Vector{vec2}, ClosedPath}

ONE_PATHS = Vector{OpenOrClosedPath}([
    [vec2(-0.3, 0.6), vec2(0.0, TOP), vec2(0.0, BOTTOM)],
    [vec2(-0.3, BOTTOM), vec2(0.3, BOTTOM)]])
TWO_PATHS = Vector{OpenOrClosedPath}([[TL, TR, MR, ML, BL, BR]])
THREE_PATHS = Vector{OpenOrClosedPath}([[TL, TR, BR, BL], [MR, ML]])
FOUR_PATHS = Vector{OpenOrClosedPath}([[TL, ML, MR, BR], [TR, MR]])
FIVE_PATHS = Vector{OpenOrClosedPath}([[TR, TL, ML, MR, BR, BL]])
SIX_PATHS = Vector{OpenOrClosedPath}([[TL, BL, BR, MR, ML]])
SEVEN_PATHS = Vector{OpenOrClosedPath}([[TL, TR, vec2(0.0, BOTTOM)]])
EIGHT_PATHS = Vector{OpenOrClosedPath}(
    [ClosedPath([TL, TR, MR, ML]), [ML, BL, BR, MR]])
NINE_PATHS = Vector{OpenOrClosedPath}([[MR, ML, TL, TR, BR]])
ZERO_PATHS = Vector{OpenOrClosedPath}([ClosedPath([TL, TR, BR, BL])])

function PathsToDrawCalls(
    paths::Vector{OpenOrClosedPath}, path_width::AbstractFloat,
    transform_matrix::mat4)
  Path(x::Vector{vec2}) = x
  Path(x::ClosedPath) = x.path
  IsClosed(::Vector{vec2}) = false
  IsClosed(::ClosedPath) = true

  return DrawSet(map(
             x -> LinePath(Path(x), path_width, vec4(1.0, 1.0, 1.0, 1.0),
                           transform_matrix, closed_path=IsClosed(x)),
             paths))
end

function SymbolToDrawCall(c::Char, transform_matrix::mat4)
  PATH_WIDTH = 0.1

  if c == '.'
    return FilledCircle(
        vec2(0.0, BOTTOM), PATH_WIDTH * 2, vec4(1.0, 1.0, 1.0, 1.0),
        transform_matrix)
  elseif c == ':'
    return DrawSet([
        FilledCircle(
            vec2(0.0, -0.45), PATH_WIDTH * 2, vec4(1.0, 1.0, 1.0, 1.0),
            transform_matrix),
        FilledCircle(
            vec2(0.0, 0.45), PATH_WIDTH * 2, vec4(1.0, 1.0, 1.0, 1.0),
            transform_matrix)])
  end

  SYMBOL_TO_PATHS = Dict(
    '0' => ZERO_PATHS,
    '1' => ONE_PATHS,
    '2' => TWO_PATHS,
    '3' => THREE_PATHS,
    '4' => FOUR_PATHS,
    '5' => FIVE_PATHS,
    '6' => SIX_PATHS,
    '7' => SEVEN_PATHS,
    '8' => EIGHT_PATHS,
    '9' => NINE_PATHS)
  return PathsToDrawCalls(SYMBOL_TO_PATHS[c], PATH_WIDTH, transform_matrix)
end

function DrawIPv4(ip::IPv4, port::UInt16, aspect_ratio::Float32)
  ip_str = string(ip) * (port > 0 ? (":" * string(port)) : "")
  ip_length = length(ip_str)
  mid_length::Float32 = (ip_length * 0.5) + 0.5
  CHAR_SCALE = 0.068f0
  CHAR_SPACING = 1.45
  transform_matrix = scale(1.0, aspect_ratio)

  PositionFromIndex(x::Integer)::Float32 =
      (x - mid_length) * CHAR_SCALE * CHAR_SPACING
  TransformMatrixFromIndex(x::Integer)::mat4 =
      translateX(PositionFromIndex(x)) * scale2d(CHAR_SCALE) *
      transform_matrix

  return DrawSet(
      map(x -> SymbolToDrawCall(x[2], TransformMatrixFromIndex(x[1])),
          enumerate(ip_str)))
end
