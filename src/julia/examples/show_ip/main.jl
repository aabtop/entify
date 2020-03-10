using Sockets

include("./draw_ipv4.jl")

@DefineShaderFunction(
    IncreaseContrast(value::Float32)::Float32,
    SoftMax(ShaderVec(value, ShaderVec(0.5) - value, 0.0, 0.0))[1])

@DefineShaderFunction(
    BlendEdges(noise::Float32, source::Float32)::Float32,
    mix(ShaderVec(0.0), noise + ShaderVec(0.6) * max(ShaderVec(0.0), (source - ShaderVec(0.5))),
        ShaderVec(1.0) - pow(ShaderVec(1.0) - source, ShaderVec(8.0))))

cloudy_edge_shader = @GLSLFragmentShaderFromBuilder(
    (v_texcoord::vec2,),
    (u_gradient_lookup::Sampler, u_aspect_ratio::Float32, u_time::Float32,
     u_source_image::Sampler),
    ShaderSource{vec4}("vec4({{x}})",
        ["x" => IncreaseContrast(BlendEdges(
            SimplexNoise(7, u_gradient_lookup,
                         ShaderVec(v_texcoord[1] * u_aspect_ratio,
                                   v_texcoord[2], u_time)
                             * ShaderVec(4.0, 4.0, 0.6)),
            Sample(u_source_image, v_texcoord)[1]))]))

function MakeDrawTree(
    window_width, window_height, reference_provider::ReferenceProvider,
    address::IPv4, port::UInt16)
  text_height = Int32(window_height / 3)
  text_width = Int32(window_width)
  text_width_fraction = text_width / window_width
  text_height_fraction = text_height / window_height
  text_aspect_ratio = Float32(text_width / text_height)

  cloudy_edge_blitter = CreateBlitter(cloudy_edge_shader)

  # Remove the sharp edges around the text by applying a glow to it.
  smooth_text_image = GaussianBlur(
      1, text_width, text_height, DrawIPv4(address, port, text_aspect_ratio))

  # Setup a faded mask of the text within which we'll apply the cloudy shader.
  cloudy_mask_image = Glow(20, smooth_text_image)

  blurred_background =
      UpdatableTextureReference(reference_provider, cloudy_mask_image)

  background_blur_blitter =
      CreateSeparableConvolutionBlitter(
          ByteKernel(Array{Float32, 1}(
              DiscreteGaussian1DKernel(15, 14.0)) * 0.95f0))

  color_multiply_blitter = CreateBlitter(
      @GLSLFragmentShaderFromBuilder(
          (v_texcoord::vec2,), (u_scale::vec4, u_source_image::Sampler),
          u_scale * Sample(u_source_image, v_texcoord)))

  text_region_on_fullscreen_vertex_buffer = CreateBlitterVertexBufferFromRect(
      -text_width_fraction, text_height_fraction,
      text_width_fraction, -text_height_fraction)

  return function(time_in_seconds::Float32)
    blurred_background_draw_call = DrawSequence([
        background_blur_blitter(
            Sampler(blurred_background.texture,
                    wrap=SamplerWrapTypeTypeClamp)),
        cloudy_edge_blitter(
            UniformValues(kGradientLookupSampler, text_aspect_ratio,
                          time_in_seconds * 0.2f0,
                          Sampler(cloudy_mask_image,
                                  wrap=SamplerWrapTypeTypeClamp)))])

    UpdateTexture!(blurred_background, blurred_background_draw_call)

    draw_call = DrawSequence([
        color_multiply_blitter(UniformValues(
                                  vec4(0.55, 0.85, 0.55, 1.0),
                                  Sampler(blurred_background.texture,
                                          wrap=SamplerWrapTypeTypeClamp)),
                               text_region_on_fullscreen_vertex_buffer),
        BlitDirect(
            smooth_text_image, text_region_on_fullscreen_vertex_buffer)])
  end
end

MakeDrawTree(window_width, window_height,
             reference_provider::ReferenceProvider) = MakeDrawTree(
    window_width, window_height, reference_provider, getipaddr(), UInt16(0))
