module SoftMaxModule

using ....Entify
using ....Entify.Shaders

export SoftMax

@DefineShaderFunction(
    SoftMax(values::vec4)::vec4,
"""
  // To avoid division by zero, we ensure that the inputs are at least a minimum
  // value.
  vec4 clamped_inputs = max(values, vec4(0.000001));

  const float base = 20.0;
  vec4 numerator = pow(vec4(base), clamped_inputs * 6.0);
  float denominator = dot(numerator, vec4(1.0));
  return numerator / denominator;
""")

end
