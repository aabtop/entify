module SimplexNoiseModule

using ....Entify
using ....Entify.Shaders
using ....Entify.ShaderCommonFunctions

using ColorTypes
using FixedPointNumbers

export SimplexNoise, kGradientLookupSampler

_SimplexNoiseWithOctavesFunction(num_octaves) =
    CachedShaderFunction(
        Float32, "SimplexNoise" * string(num_octaves),
        [("gradient_lookup_sampler", Sampler), ("position", vec3)], replace(
"""
  vec3 x = vec3(position.xy, position.z + 100.0);

  float v = 0.0;
  float total_a = 0.0;
  float a = 0.5;
  vec3 shift = vec3(100);
  for (int i = 0; i < __NUM_OCTAVES__; ++i) {
    v += a * my_noise(gradient_lookup_sampler, x);
    total_a += a;
    x = x * 2.0 + shift;
    a *= 0.5;
  }

  return v / total_a;
""", "__NUM_OCTAVES__" => string(num_octaves)), [CachedShaderDeclaration(
"""
//  Simplex 3D Noise 
//  by Ian McEwan, Ashima Arts
//
vec4 permute(vec4 x){return mod(((x*34.0)+1.0)*x, 289.0);}

vec3 LookupGradientVector(sampler2D gradient_lookup, float index) {
  return (texture2D(gradient_lookup, vec2(index, 0.5)).xyz * 2.0 - 1.0);
}

// Returns values in [-1, 1].
float snoise(sampler2D gradient_lookup, vec3 v){
  const float kSkewFactor = 1.0 / 3.0;
  const float kUnskewFactor = 1.0 / 6.0;

  const vec4  D = vec4(0.0, 0.5, 1.0, 2.0);

  // First corner
  vec3 i  = floor(v + dot(v, vec3(kSkewFactor)));
  vec3 x0 =   v - i + dot(i, vec3(kUnskewFactor)) ;

  // Other corners
  vec3 g = step(x0.yzx, x0.xyz);
  vec3 l = 1.0 - g;
  vec3 i1 = min(g.xyz, l.zxy);
  vec3 i2 = max(g.xyz, l.zxy);

  //  x0 = x0 - 0. + 0.0 * vec3(kUnskewFactor)
  vec3 x1 = x0 - i1 + 1.0 * vec3(kUnskewFactor);
  vec3 x2 = x0 - i2 + 2.0 * vec3(kUnskewFactor);
  vec3 x3 = x0 - 1. + 3.0 * vec3(kUnskewFactor);

  // Permutations
  vec4 p = permute(permute(permute( 
             i.z + vec4(0.0, i1.z, i2.z, 1.0))
           + i.y + vec4(0.0, i1.y, i2.y, 1.0)) 
           + i.x + vec4(0.0, i1.x, i2.x, 1.0)) / 12.0;

  // Gradients
  vec3 p0 = LookupGradientVector(gradient_lookup, p.x);
  vec3 p1 = LookupGradientVector(gradient_lookup, p.y);
  vec3 p2 = LookupGradientVector(gradient_lookup, p.z);
  vec3 p3 = LookupGradientVector(gradient_lookup, p.w);

  // Mix final noise value
  vec4 m = max(0.5 - vec4(dot(x0, x0),
                          dot(x1, x1),
                          dot(x2, x2),
                          dot(x3, x3)),
               0.0);
  m = m * m;

  return 300.0 * dot(m * m, vec4(dot(p0, x0),
                                 dot(p1, x1), 
                                 dot(p2, x2),
                                 dot(p3, x3)));
}

float my_noise(sampler2D gradient_lookup, vec3 position) {
  // Rescale from [-1, 1] to [0, 1].
  // Also add a fudge factor to make it darker.
  return snoise(gradient_lookup, position) * 0.15 + 0.15;
}
""")])

SimplexNoise(
    num_octaves::Signed, gradient_lookup::ShaderExpr{Sampler},
    x::ShaderExpr{vec3}) =
    _SimplexNoiseWithOctavesFunction(num_octaves)(gradient_lookup, x)

const kGradientLookupSampler =
    # We want to work in the space of [-1.0, 1.0] here, but since we are passing
    # this in as a texture we'll define it in terms of [0.0, 1.0] and then
    # convert back in the shader.
    let kInvSqrt2::Float32 = 0.70710678118 / 2.0 + 0.5,
        kNegInvSqrt2::Float32 = 1.0 - kInvSqrt2
        kZero::Float32 = 0.5
        sampler_data = [
          RGB{N0f8}(kInvSqrt2, kInvSqrt2, kZero),
          RGB{N0f8}(kNegInvSqrt2, kInvSqrt2, kZero),
          RGB{N0f8}(kInvSqrt2, kNegInvSqrt2, kZero),
          RGB{N0f8}(kNegInvSqrt2, kNegInvSqrt2, kZero),
          RGB{N0f8}(kInvSqrt2, kZero, kInvSqrt2),
          RGB{N0f8}(kNegInvSqrt2, kZero, kInvSqrt2),
          RGB{N0f8}(kInvSqrt2, kZero, kNegInvSqrt2),
          RGB{N0f8}(kNegInvSqrt2, kZero, kNegInvSqrt2),
          RGB{N0f8}(kZero, kInvSqrt2, kInvSqrt2),
          RGB{N0f8}(kZero, kNegInvSqrt2, kInvSqrt2),
          RGB{N0f8}(kZero, kInvSqrt2, kNegInvSqrt2),
          RGB{N0f8}(kZero, kNegInvSqrt2, kNegInvSqrt2),
        ]
      Sampler(PixelData(reshape(sampler_data, (1, length(sampler_data)))),
              filt=SamplerFilterTypeTypeNearest)
    end

end
