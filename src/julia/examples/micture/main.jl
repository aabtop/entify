import Images

function MakeDrawTree(window_width, window_height, reference_provider)
  # First we define how to generate the 4-channel simplex noise.
  NUM_OCTAVES = 3
  @DefineShaderFunction(
      FourChannelSimplexNoise(gradient_lookup::Sampler, position::vec2,
                              scaled_time::Float32)::vec4,
      ShaderVec(
          SimplexNoise(
              NUM_OCTAVES, gradient_lookup,
              ShaderVec(position, scaled_time)),
          SimplexNoise(
              NUM_OCTAVES, gradient_lookup,
              ShaderVec(position, scaled_time + ShaderVec(101.0))),
          SimplexNoise(
              NUM_OCTAVES, gradient_lookup,
              ShaderVec(position, scaled_time + ShaderVec(457.0))),
          SimplexNoise(
              NUM_OCTAVES, gradient_lookup,
              ShaderVec(position, scaled_time + ShaderVec(1327.0)))))

  noise_cloud_fragment_shader = @GLSLFragmentShaderFromBuilder(
      (v_texcoord::vec2,),
      (gradient_lookup::Sampler, u_aspect_ratio::Float32, u_time::Float32),
      SoftMax(FourChannelSimplexNoise(
          gradient_lookup,
          ShaderVec(v_texcoord[1] * u_aspect_ratio, v_texcoord[2]),
          ShaderVec(0.05) * u_time)))

  noise_blitter = CreateBlitter(noise_cloud_fragment_shader)

  # Next we define how to blend the images together using the noise as weights.
  @DefineShaderFunction(
      Blend(weights::vec4, c1::vec4, c2::vec4, c3::vec4, c4::vec4)::vec4,
      weights[1] * c1 + weights[2] * c2 + weights[3] * c3 + weights[4] * c4)

  blend_fragment_shader = @GLSLFragmentShaderFromBuilder(
      (v_texcoord::vec2,),
      (noise_cloud::Sampler, u_sampler_1::Sampler, u_sampler_2::Sampler,
       u_sampler_3::Sampler, u_sampler_4::Sampler),
      Blend(Sample(noise_cloud, v_texcoord),
            Sample(u_sampler_1, v_texcoord),
            Sample(u_sampler_2, v_texcoord),
            Sample(u_sampler_3, v_texcoord),
            Sample(u_sampler_4, v_texcoord)))

  blend_blitter = CreateBlitter(blend_fragment_shader)

  # Now we load our image data into Sampler objects.
  samplers = map(x -> Sampler(PixelData(Images.load(x))), [
      "slovakia-1.jpg",
      "slovakia-2.jpg",
      "slovakia-3.jpg",
      "slovakia-4.jpg"])

  # And then finally assemble everything together into an animated draw tree.
  return function(time_in_seconds::Float32)
    return blend_blitter(UniformValues(
               Sampler(RenderTarget(
                   window_width ÷ 2, window_height ÷ 2,
                   noise_blitter(UniformValues(
                        kGradientLookupSampler,
                       Float32(window_width / window_height),
                       time_in_seconds)))),
               samplers[1], samplers[2], samplers[3], samplers[4]))
  end
end
