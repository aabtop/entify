function MakeDrawTree(window_width, window_height, reference_provider)
  aspect_ratio = Float32(window_height / window_width)
  to_screen_space = scale(aspect_ratio, 1.0)
  
  return function(time_in_seconds::Float32)
    kRotationsPerSecond = 1/10
    camera_matrix = to_screen_space * rotateZ(
        time_in_seconds * kRotationsPerSecond * 2 * pi)

    return BlitDirect(GaussianBlur(
        2.0,
        window_width, window_height,
        DrawSequence([
          Polygon(
              Star(0.6, 0.3, 5), 0.01,
              vec4(1.0, 0.3, 0.6, 1.0), camera_matrix),
          Polygon(
              Pentagon(0.6), 0.02,
              vec4(0.2, 0.6, 1.0, 1.0), camera_matrix),
          Polygon(
              Pentagram(0.4), 0.02,
              vec4(1.0, 0.1, 0.2, 1.0), camera_matrix)])))
  end
end
