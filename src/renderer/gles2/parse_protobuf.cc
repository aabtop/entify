#include "src/renderer/gles2/parse_protobuf.h"

#include <memory>

#include "entify/renderer_definitions.pb.h"
#include "src/external_reference.h"
#include "src/renderer/gles2/lookup_utils.h"
#include "src/renderer/gles2/render_tree/draw_call.h"
#include "src/renderer/gles2/render_tree/draw_sequence.h"
#include "src/renderer/gles2/render_tree/fragment_shader.h"
#include "src/renderer/gles2/render_tree/pipeline.h"
#include "src/renderer/gles2/render_tree/program.h"
#include "src/renderer/gles2/render_tree/types.h"
#include "src/renderer/gles2/render_tree/uniform_values.h"
#include "src/renderer/gles2/render_tree/vertex_buffer.h"
#include "src/renderer/gles2/render_tree/vertex_shader.h"

namespace entify {
namespace renderer {
namespace gles2 {

namespace {

render_tree::Type FromProtoType(int32_t in) {
  switch (in) {
    case entify_renderer::PrimitiveTypeFloat32V1:
        return render_tree::TypeFloat32V1;
    case entify_renderer::PrimitiveTypeFloat32V2:
        return render_tree::TypeFloat32V2;
    case entify_renderer::PrimitiveTypeFloat32V3:
        return render_tree::TypeFloat32V3;
    case entify_renderer::PrimitiveTypeFloat32V4:
        return render_tree::TypeFloat32V4;
    case entify_renderer::PrimitiveTypeFloat32M44:
        return render_tree::TypeFloat32M44;
    case entify_renderer::PrimitiveTypeUInt8V1:
        return render_tree::TypeUInt8V1;
    case entify_renderer::PrimitiveTypeUInt8V2:
        return render_tree::TypeUInt8V2;
    case entify_renderer::PrimitiveTypeUInt8V3:
        return render_tree::TypeUInt8V3;
    case entify_renderer::PrimitiveTypeUInt8V4:
        return render_tree::TypeUInt8V4;
    case entify_renderer::PrimitiveTypeSampler:
        return render_tree::TypeSampler;
  }

  return render_tree::TypeInvalid;
}

render_tree::PixelType FromProtoPixelType(int32_t in) {
  switch (in) {
    case entify_renderer::PixelTypeRGB: return render_tree::kPixelTypeRGB;
    case entify_renderer::PixelTypeRGBA: return render_tree::kPixelTypeRGBA;
  }

  assert(false);
  return static_cast<render_tree::PixelType>(0);
}

render_tree::TypeTuple FromProtoTypeTuple(
    const entify_renderer::PrimitiveTypeTuple& in) {
  render_tree::TypeTuple out;
  out.reserve(in.types_size());
  std::transform(in.types().begin(), in.types().end(), std::back_inserter(out),
                 &FromProtoType);
  return std::move(out);
}

std::pair<std::vector<std::string>, render_tree::TypeTuple>
FromNamedProtoTypeTuple(
    const entify_renderer::NamedPrimitiveTypeTuple& in) {
  std::pair<std::vector<std::string>, render_tree::TypeTuple> out;
  out.first.reserve(in.named_types_size());
  out.second.reserve(in.named_types_size());
  for (const auto& i : in.named_types()) {
    out.first.push_back(i.name());
    out.second.push_back(FromProtoType(i.type()));        
  }
  return std::move(out);
}

std::shared_ptr<render_tree::VertexBuffer> ParseVertexBuffer(
    const entify_renderer::VertexBuffer& vertex_buffer) {
  std::vector<int32_t> data_offsets;
  data_offsets.reserve(vertex_buffer.offsets_size());
  std::copy(vertex_buffer.offsets().begin(), vertex_buffer.offsets().end(),
            std::back_inserter(data_offsets));

  return std::make_shared<render_tree::VertexBuffer>(
      vertex_buffer.data().c_str(), vertex_buffer.data().size(),
      vertex_buffer.stride_in_bytes(),
      std::move(data_offsets),
      FromProtoTypeTuple(vertex_buffer.types()));
}

std::vector<std::shared_ptr<render_tree::Sampler>> ParseRepeatedSamplerField(
    const ::google::protobuf::RepeatedField<::google::protobuf::int64>&
        repeated_field,
    const ExternalReferenceLookup& reference_lookup) {
  std::vector<std::shared_ptr<render_tree::Sampler>> samplers;
  samplers.reserve(repeated_field.size());
  for (const auto& sampler_id : repeated_field) {
    auto sampler = LookupNode<render_tree::Sampler>(
        reference_lookup, sampler_id);
    assert(sampler);
    samplers.push_back(sampler);
  }
  return samplers;
}

std::shared_ptr<render_tree::UniformValues> ParseUniformValues(
    const entify_renderer::UniformValues& uniform_values,
    const ExternalReferenceLookup& reference_lookup) {
  std::vector<char> data(
      uniform_values.data().begin(), uniform_values.data().end());
  return std::make_shared<render_tree::UniformValues>(
      FromProtoTypeTuple(uniform_values.types()), std::move(data),
      ParseRepeatedSamplerField(uniform_values.sampler_ids(),
                                reference_lookup));
}

std::shared_ptr<render_tree::VertexShader> ParseGLSLVertexShader(
    const entify_renderer::GLSLVertexShader& glsl_vertex_shader) {

  return std::make_shared<render_tree::VertexShader>(
      glsl_vertex_shader.source(),
      FromNamedProtoTypeTuple(glsl_vertex_shader.input_types()),
      FromProtoTypeTuple(glsl_vertex_shader.output_types()),
      FromNamedProtoTypeTuple(glsl_vertex_shader.uniform_types()));
}

std::shared_ptr<render_tree::FragmentShader> ParseGLSLFragmentShader(
    const entify_renderer::GLSLFragmentShader& glsl_fragment_shader) {
  return std::make_shared<render_tree::FragmentShader>(
      glsl_fragment_shader.source(),
      FromProtoTypeTuple(glsl_fragment_shader.input_types()),
      FromNamedProtoTypeTuple(glsl_fragment_shader.uniform_types()));
}

namespace {
GLenum FromProtoBlendCoefficient(int32_t in) {
  switch (in) {
    case entify_renderer::BlendCoefficientZero: return GL_ZERO;
    case entify_renderer::BlendCoefficientOne: return GL_ONE;
    case entify_renderer::BlendCoefficientSrcAlpha: return GL_SRC_ALPHA;
    case entify_renderer::BlendCoefficientOneMinusSrcAlpha:
        return GL_ONE_MINUS_SRC_ALPHA;
  }
  assert(false);
  return 0;
}
}  // namespace

std::shared_ptr<render_tree::Pipeline> ParsePipeline(
    const entify_renderer::Pipeline& pipeline,
    const ExternalReferenceLookup& reference_lookup) {
  auto vertex_shader = LookupNode<render_tree::VertexShader>(
      reference_lookup, pipeline.vertex_shader_id());
  assert(vertex_shader);

  auto fragment_shader = LookupNode<render_tree::FragmentShader>(
      reference_lookup, pipeline.fragment_shader_id());
  assert(fragment_shader);

  auto program =
      std::make_shared<render_tree::Program>(vertex_shader, fragment_shader);

  render_tree::Pipeline::Params params;
  params.blend.src_color = FromProtoBlendCoefficient(
      pipeline.blend_parameters().src_color());
  params.blend.dst_color = FromProtoBlendCoefficient(
      pipeline.blend_parameters().dst_color());
  params.blend.src_alpha = FromProtoBlendCoefficient(
      pipeline.blend_parameters().src_alpha());
  params.blend.dst_alpha = FromProtoBlendCoefficient(
      pipeline.blend_parameters().dst_alpha());

  return std::make_shared<render_tree::Pipeline>(program, params);
}

std::shared_ptr<render_tree::DrawCall> ParseDrawCall(
    const entify_renderer::DrawCall& draw_call,
    const ExternalReferenceLookup& reference_lookup) {
  auto pipeline = LookupNode<render_tree::Pipeline>(
      reference_lookup, draw_call.pipeline_id());
  assert(pipeline);

  auto vertex_buffer = LookupNode<render_tree::VertexBuffer>(
      reference_lookup, draw_call.vertex_buffer_id());
  assert(vertex_buffer);

  auto vertex_uniform_values = LookupNode<render_tree::UniformValues>(
      reference_lookup, draw_call.vertex_uniform_values_id());

  auto fragment_uniform_values = LookupNode<render_tree::UniformValues>(
      reference_lookup, draw_call.fragment_uniform_values_id());

  return std::make_shared<render_tree::DrawCall>(
      pipeline, vertex_buffer, vertex_uniform_values, fragment_uniform_values);
}

std::vector<std::shared_ptr<render_tree::DrawTree>> ParseRepeatedDrawTreeField(
    const ::google::protobuf::RepeatedField<::google::protobuf::int64>&
        repeated_field,
    const ExternalReferenceLookup& reference_lookup) {
  std::vector<std::shared_ptr<render_tree::DrawTree>> draw_tree_sequence;
  draw_tree_sequence.reserve(repeated_field.size());
  for (const auto& draw_sub_tree_id : repeated_field) {
    auto draw_tree = LookupNode<render_tree::DrawTree>(
        reference_lookup, draw_sub_tree_id);
    assert(draw_tree);
    draw_tree_sequence.push_back(draw_tree);
  }
  return draw_tree_sequence;
}

std::shared_ptr<render_tree::DrawSequence> ParseDrawSequence(
    const entify_renderer::DrawSequence& draw_sequence,
    const ExternalReferenceLookup& reference_lookup) {
  return std::make_shared<render_tree::DrawSequence>(
      ParseRepeatedDrawTreeField(
          draw_sequence.draw_tree_ids(), reference_lookup));
}

std::shared_ptr<render_tree::DrawSequence> ParseDrawSet(
    const entify_renderer::DrawSet& draw_set,
    const ExternalReferenceLookup& reference_lookup) {
  return std::make_shared<render_tree::DrawSequence>(
      ParseRepeatedDrawTreeField(
          draw_set.draw_tree_ids(), reference_lookup));
}

namespace {
GLenum FromProtoSamplerWrapType(int32_t in) {
  switch (in) {
    case entify_renderer::SamplerWrapTypeRepeat: return GL_REPEAT;
    case entify_renderer::SamplerWrapTypeClamp: return GL_CLAMP_TO_EDGE;
  }
  assert(false);
  return 0;
}

GLenum FromProtoSamplerFilterType(int32_t in) {
  switch (in) {
    case entify_renderer::SamplerFilterTypeLinear: return GL_LINEAR;
    case entify_renderer::SamplerFilterTypeNearest: return GL_NEAREST;
  }
  assert(false);
  return 0;
}
}  // namespace

std::shared_ptr<render_tree::Sampler> ParseSampler(
    const entify_renderer::Sampler& sampler,
    const ExternalReferenceLookup& reference_lookup) {
  auto texture = LookupNode<render_tree::Texture>(
      reference_lookup, sampler.texture_id());
  assert(texture);
  return std::make_shared<render_tree::Sampler>(
      texture,
      FromProtoSamplerWrapType(sampler.wrap_s()),
      FromProtoSamplerWrapType(sampler.wrap_t()),
      FromProtoSamplerFilterType(sampler.min_filter()),
      FromProtoSamplerFilterType(sampler.mag_filter()));
}

std::shared_ptr<render_tree::DrawTree> ParseDrawTree(
    const entify_renderer::DrawTree& draw_tree,
    const ExternalReferenceLookup& reference_lookup) {
  switch (draw_tree.DerivedType_case()) {
    case entify_renderer::DrawTree::kDrawCall:
      return ParseDrawCall(draw_tree.draw_call(), reference_lookup);
    case entify_renderer::DrawTree::kDrawSequence:
      return ParseDrawSequence(draw_tree.draw_sequence(), reference_lookup);
    case entify_renderer::DrawTree::kDrawSet:
      return ParseDrawSet(draw_tree.draw_set(), reference_lookup);
    default:
      assert(false);
  }

  return nullptr;
}

std::shared_ptr<render_tree::RenderTarget> ParseRenderTarget(
    const entify_renderer::RenderTarget& render_target,
    const ExternalReferenceLookup& reference_lookup) {
  auto draw_tree = LookupNode<render_tree::DrawTree>(
      reference_lookup, render_target.draw_tree_id());
  assert(draw_tree);

  return std::make_shared<render_tree::RenderTarget>(
      render_target.width_in_pixels(), render_target.height_in_pixels(),
      draw_tree);
}

std::shared_ptr<render_tree::PixelData> ParsePixelData(
    const entify_renderer::PixelData& pixel_data) {
  std::vector<char> data(
      pixel_data.data().begin(), pixel_data.data().end());
  return std::make_shared<render_tree::PixelData>(
      pixel_data.width_in_pixels(), pixel_data.height_in_pixels(),
      pixel_data.stride_in_bytes(), FromProtoPixelType(pixel_data.pixel_type()),
      std::move(data));
}


std::shared_ptr<render_tree::Texture> ParseTexture(
    const entify_renderer::Texture& texture,
    const ExternalReferenceLookup& reference_lookup) {
  switch (texture.DerivedType_case()) {
    case entify_renderer::Texture::kRenderTarget:
      return ParseRenderTarget(texture.render_target(), reference_lookup);
    case entify_renderer::Texture::kPixelData:
      return ParsePixelData(texture.pixel_data());
    default:
      assert(false);
  }

  return nullptr;
}
}   // namespace

ParseOutput ParseProtocolBuffer(
    const ExternalReferenceLookup& reference_lookup,
    const char* data, size_t data_size) {
  entify_renderer::RendererNode node;
  node.ParseFromArray(data, data_size);

  switch (node.DerivedType_case()) {
    case entify_renderer::RendererNode::kVertexBuffer: {
      return ParseOutput(
          ParseVertexBuffer(node.vertex_buffer()));
    } break;
    case entify_renderer::RendererNode::kUniformValues: {
      return ParseOutput(
          ParseUniformValues(node.uniform_values(), reference_lookup));
    } break;
    case entify_renderer::RendererNode::kGlslVertexShader: {
      return ParseOutput(
          ParseGLSLVertexShader(node.glsl_vertex_shader()));
    } break;
    case entify_renderer::RendererNode::kGlslFragmentShader: {
      return ParseOutput(
          ParseGLSLFragmentShader(node.glsl_fragment_shader()));
    } break;
    case entify_renderer::RendererNode::kPipeline: {
      return ParseOutput(
          ParsePipeline(node.pipeline(), reference_lookup));
    } break;
    case entify_renderer::RendererNode::kDrawTree: {
      return ParseOutput(
          ParseDrawTree(node.draw_tree(), reference_lookup));
    } break;
    case entify_renderer::RendererNode::kSampler: {
      return ParseOutput(
          ParseSampler(node.sampler(), reference_lookup));
    } break;
    case entify_renderer::RendererNode::kTexture: {
      return ParseOutput(
          ParseTexture(node.texture(), reference_lookup));
    } break;
    default:
      assert(false);
  };

  assert(false);
  return std::unique_ptr<ExternalReference>();
}

}  // namespace gles2
}  // namespace renderer
}  // namespace entify
