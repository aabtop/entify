module Convolution

using ....Entify
using ....Entify.Shaders
using ....Entify.ShaderCommonFunctions
using ....Entify.Composites.Blit

using ColorTypes
using Distributions: Truncated, Normal, cdf
using FixedPointNumbers

export CreateConvolutionBlitter, CreateSeparableConvolutionBlitter,
       DiscreteGaussian1DKernel, DiscreteGaussian2DKernel, ByteKernel,
       GaussianBlur, Glow

ConvolutionFunction(kernel_width, kernel_height) =
    CachedShaderFunction(
        vec4,
        "Convolution" * string(kernel_width) * "x" * string(kernel_height),
        [("kernel_lookup", Sampler), ("source_step_size", vec2),
         ("kernel_value_origin", Float32), ("kernel_value_scale", Float32),
         ("source", Sampler), ("texcoord", vec2)],
        reduce(replace, ["__KERNEL_WIDTH__" => kernel_width,
                         "__KERNEL_HEIGHT__" => kernel_height], init=
"""
  #define HORIZONTAL_RADIUS (__KERNEL_WIDTH__ / 2)
  #define VERTICAL_RADIUS (__KERNEL_HEIGHT__ / 2)

  vec4 total_color = vec4(0.0, 0.0, 0.0, 0.0);

  for (int y = -VERTICAL_RADIUS; y <= VERTICAL_RADIUS; ++y) {
    for (int x = -HORIZONTAL_RADIUS; x <= HORIZONTAL_RADIUS; ++x) {
      vec4 color = texture2D(
          source, vec2(texcoord.x + float(x) * source_step_size.x,
                          texcoord.y + float(y) * source_step_size.y));
      const vec2 kRadiusVec = vec2(HORIZONTAL_RADIUS, VERTICAL_RADIUS);
      const vec2 kDiameterVec = vec2(__KERNEL_WIDTH__, __KERNEL_HEIGHT__);

      vec2 lookup_pos = (vec2(x, y) + kRadiusVec + 0.5) / kDiameterVec;
      float raw_value = texture2D(kernel_lookup, lookup_pos).r;
      float weight = raw_value * kernel_value_scale + kernel_value_origin;

      total_color += color * weight;
    }
  }

  return total_color;
"""), [])


# Stores the kernel data, but compressed down to 1-byte per pixel.
struct ByteKernel{N}
  data::Array{N0f8, N}
  origin::Float32
  scale::Float32
end

function ByteKernel(input::Array{Float32, N}; normalize=false) where {N}
  maybe_normalized_input = normalize ? input / sum(input) : input

  origin = minimum(maybe_normalized_input)
  origin_zero_input = maybe_normalized_input .- origin
  scale = maximum(origin_zero_input)
  scaled_input = N0f8.(origin_zero_input / scale)

  if normalize
    # We do one more round of normalization, now that the data has been
    # rescaled (and during the process may have lost its normalizedness).
    total = sum(_ApplyOriginAndScale.(scaled_input, origin, scale))
    scale *= (1 - origin) / (total - origin)
  end

  return ByteKernel{N}(scaled_input, origin, scale)
end

_ApplyOriginAndScale(data, origin, scale) = (data * scale) .+ origin

# Some helper methods for making code that works with ByteKernels look more
# like its working with regular Arrays.
Base.reshape(x::ByteKernel{N}, args...) where {N} =
    ByteKernel(reshape(x.data, args...), x.origin, x.scale)
Base.length(x::ByteKernel{N}) where {N} = length(x.data)
Base.size(x::ByteKernel{N}) where {N} = size(x.data)
Base.Array{Float32, N}(input::ByteKernel{N}) where {N} =
    _ApplyOriginAndScale(input.data, input.origin, input.scale)
    

function CreateConvolutionBlitter(kernel::ByteKernel{2})
  kernel_width = size(kernel)[2]
  kernel_height = size(kernel)[1]

  convolution_shader = @GLSLFragmentShaderFromBuilder(
      (v_texcoord::vec2,),
      (u_kernel_lookup::Sampler, u_source_step_size::vec2,
       u_kernel_value_origin::Float32, u_kernel_value_scale::Float32,
       u_source::Sampler),
      ConvolutionFunction(kernel_width, kernel_height)(
          u_kernel_lookup, u_source_step_size, u_kernel_value_origin,
          u_kernel_value_scale, u_source, v_texcoord))

  blitter = CreateBlitter(convolution_shader)

  kernel_sampler = Sampler(
      PixelData(map(x -> RGBA(x, x, x, x), kernel.data)),
      wrap=SamplerWrapTypeTypeClamp, filt=SamplerFilterTypeTypeNearest)

  return function(source::Sampler, vertex_buffer=FullscreenBlitterVertexBuffer)
    source_step_size = vec2(1.0 / source.texture.width_in_pixels,
                            1.0 / source.texture.height_in_pixels)
    return blitter(UniformValues(kernel_sampler, source_step_size,
                                 kernel.origin, kernel.scale, source),
                   vertex_buffer)
  end
end

function CreateSeparableConvolutionBlitter(kernel::ByteKernel{1})
  convolve_y = CreateConvolutionBlitter(reshape(kernel, (length(kernel), 1)))
  convolve_x = CreateConvolutionBlitter(reshape(kernel, (1, length(kernel))))

  return function(source::Sampler, vertex_buffer=FullscreenBlitterVertexBuffer)
    return convolve_x(
        Sampler(
            RenderTarget(
                source.texture.width_in_pixels, source.texture.height_in_pixels,
                convolve_y(source)),
            wrap=SamplerWrapTypeTypeClamp),
        vertex_buffer)

  end
end

function _DiscreteGaussian1DKernelFloat32(
    pixel_radius, sigma)::Vector{Float32}
  diameter = 1 + 2 * pixel_radius
  dist = Truncated(Normal(1 + pixel_radius - 0.5, sigma), 0, diameter)
  return Float32.(map(x -> cdf(dist, x) - cdf(dist, x - 1), 1:diameter))
end

function DiscreteGaussian1DKernel(pixel_radius, sigma)::ByteKernel{1}
  return ByteKernel(_DiscreteGaussian1DKernelFloat32(pixel_radius, sigma),
                    normalize=true)
end

function DiscreteGaussian2DKernel(pixel_radius, sigma)::ByteKernel{2}
  gaussian1d = Vector{Float32}(DiscreteGaussian1DKernel(pixel_radius, sigma))
  return ByteKernel(gaussian1d * gaussian1d', normalize=true)
end


function GaussianBlurInternal(
    stdev, width, height, texture::Texture,
    kernel_radius_per_iteration::Integer,
    apply_source_each_iteration::Bool)::RenderTarget
  # Since most of a Gaussian kernel fits in 2 standard deviations, we use that
  # as a balance between accuracy and computation.
  KERNEL_RADIUS_PER_STDEV = 2

  MAX_STDEV_PER_ITERATION =
      kernel_radius_per_iteration / KERNEL_RADIUS_PER_STDEV

  # Determine the number of iterations we should apply based on the fact that
  # repeatedly applying Gaussians results in a single Gaussian with variances
  # summed, thus the amount of times we must iterate is squared for the desired
  # total stdev to be achieved.
  num_iterations = (stdev ÷ MAX_STDEV_PER_ITERATION) ^ 2

  if num_iterations > 0
    # Note that if the blur is large enough, we only support a granularity
    # of MAX_STDEV_PER_ITERATION for |stdev|.  This is so that we only need
    # a single shader to implement the effect.
    stdev = MAX_STDEV_PER_ITERATION
  else
    num_iterations = 1
  end

  blur_blitter =
      CreateSeparableConvolutionBlitter(
          ByteKernel(Array{Float32, 1}(
              DiscreteGaussian1DKernel(kernel_radius_per_iteration,
                                       stdev ^ 2))))

  function Iteration(num_iterations, texture)
    if num_iterations > 1
      blur_source =
          Iteration(num_iterations - 1, texture)
    else
      blur_source = texture
    end


    return RenderTarget(
        width, height,
        DrawSequence(vcat(
            [blur_blitter(Sampler(blur_source, wrap=SamplerWrapTypeTypeClamp))],
            apply_source_each_iteration ?
                BlitDirect(texture) : Vector{DrawTree}())))
  end

  return Iteration(num_iterations, texture)
end


# Takes a source mask given as a draw call and recursively applies a Gaussian
# blur with the original source image on top of it until it has the desired
# spread.  The blurring is done on a render target of the specified width and
# height, and a render target of the same width and height is returned.
GaussianBlur(stdev, width, height, draw_call::DrawTree) =
    GaussianBlur(stdev, RenderTarget(width, height, draw_call))

GaussianBlur(stdev, texture::Texture)::RenderTarget =
    GaussianBlurInternal(stdev, texture.width_in_pixels,
                         texture.height_in_pixels, texture, 10, false)

# Similar to a GaussianBlur but we re-apply the source each iteration.
Glow(stdev, width, height, draw_call::DrawTree) =
    Glow(stdev, RenderTarget(width, height, draw_call))

Glow(stdev, texture::Texture)::RenderTarget =
    GaussianBlurInternal(stdev, texture.width_in_pixels,
                         texture.height_in_pixels, texture, 5, true)

end
