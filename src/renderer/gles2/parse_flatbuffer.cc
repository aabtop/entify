#include "src/renderer/gles2/parse_flatbuffer.h"

#include "entify/renderer_definitions_generated.h"
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

render_tree::Type FromProtoType(PrimitiveType in) {
  switch (in) {
    case PrimitiveType_Float32V1:
        return render_tree::TypeFloat32V1;
    case PrimitiveType_Float32V2:
        return render_tree::TypeFloat32V2;
    case PrimitiveType_Float32V3:
        return render_tree::TypeFloat32V3;
    case PrimitiveType_Float32V4:
        return render_tree::TypeFloat32V4;
    case PrimitiveType_Float32M44:
        return render_tree::TypeFloat32M44;
    case PrimitiveType_UInt8V1:
        return render_tree::TypeUInt8V1;
    case PrimitiveType_UInt8V2:
        return render_tree::TypeUInt8V2;
    case PrimitiveType_UInt8V3:
        return render_tree::TypeUInt8V3;
    case PrimitiveType_UInt8V4:
        return render_tree::TypeUInt8V4;
    case PrimitiveType_Sampler:
        return render_tree::TypeSampler;
  }

  return render_tree::TypeInvalid;
}

render_tree::Type FromProtoTypeUInt8(uint8_t in) {
  return FromProtoType(static_cast<PrimitiveType>(in));
}

render_tree::TypeTuple FromProtoTypeTuple(
    const flatbuffers::Vector<int8_t>* in) {
  render_tree::TypeTuple out;
  out.reserve(in->size());
  std::transform(in->begin(), in->end(), std::back_inserter(out),
                 &FromProtoTypeUInt8);
  return std::move(out);
}

std::pair<std::vector<std::string>, render_tree::TypeTuple>
FromNamedProtoTypeTuple(
    const flatbuffers::Vector<flatbuffers::Offset<NamedPrimitiveType>>* in) {
  std::pair<std::vector<std::string>, render_tree::TypeTuple> out;
  out.first.reserve(in->size());
  out.second.reserve(in->size());
  for (const auto& i : *in) {
    out.first.push_back(i->name()->str());
    out.second.push_back(FromProtoType(i->type()));        
  }
  return std::move(out);
}

ParseOutput ParseGLSLVertexShader(
    const GLSLVertexShader* glsl_vertex_shader) {
  auto shader = std::make_shared<render_tree::VertexShader>(
      glsl_vertex_shader->source()->str(),
      FromNamedProtoTypeTuple(glsl_vertex_shader->input_types()),
      FromProtoTypeTuple(glsl_vertex_shader->output_types()),
      FromNamedProtoTypeTuple(glsl_vertex_shader->uniform_types()));
  if (!shader->error().empty()) {
    return ParseOutput(shader->error());
  } else {
    return shader;
  }
}

ParseOutput ParseGLSLFragmentShader(
    const GLSLFragmentShader* glsl_fragment_shader) {
  auto shader = std::make_shared<render_tree::FragmentShader>(
      glsl_fragment_shader->source()->str(),
      FromProtoTypeTuple(glsl_fragment_shader->input_types()),
      FromNamedProtoTypeTuple(glsl_fragment_shader->uniform_types()));
  if (!shader->error().empty()) {
    return ParseOutput(shader->error());
  } else {
    return shader;
  }
}

ParseOutput ParseVertexBuffer(
    const VertexBuffer* vertex_buffer) {
  std::vector<int32_t> data_offsets;
  data_offsets.reserve(vertex_buffer->offsets()->size());
  std::copy(vertex_buffer->offsets()->begin(), vertex_buffer->offsets()->end(),
            std::back_inserter(data_offsets));

  return std::make_shared<render_tree::VertexBuffer>(
      reinterpret_cast<const char*>(vertex_buffer->data()->data()),
      vertex_buffer->data()->size(),
      vertex_buffer->stride_in_bytes(),
      std::move(data_offsets),
      FromProtoTypeTuple(vertex_buffer->types()));
}

std::vector<std::shared_ptr<render_tree::Sampler>> ParseRepeatedSamplerField(
    const flatbuffers::Vector<int64_t> *sampler_ids,
    const ExternalReferenceLookup& reference_lookup) {
  std::vector<std::shared_ptr<render_tree::Sampler>> samplers;
  if (!sampler_ids) {
    return samplers;
  }

  samplers.reserve(sampler_ids->size());
  for (const auto& sampler_id : *sampler_ids) {
    auto sampler = LookupNode<render_tree::Sampler>(
        reference_lookup, sampler_id);
    assert(sampler);
    samplers.push_back(sampler);
  }
  return samplers;
}

ParseOutput ParseUniformValues(
    const UniformValues* uniform_values,
    const ExternalReferenceLookup& reference_lookup) {
  std::vector<char> data(
      uniform_values->data()->begin(), uniform_values->data()->end());
  return std::make_shared<render_tree::UniformValues>(
      FromProtoTypeTuple(uniform_values->types()), std::move(data),
      ParseRepeatedSamplerField(uniform_values->sampler_ids(),
                                reference_lookup));
}

std::shared_ptr<render_tree::Texture> ParseRenderTarget(
    const RenderTarget* render_target,
    const ExternalReferenceLookup& reference_lookup) {
  auto draw_tree = LookupNode<render_tree::DrawTree>(
      reference_lookup, render_target->draw_tree_id());
  assert(draw_tree);

  return std::make_shared<render_tree::RenderTarget>(
      render_target->width_in_pixels(), render_target->height_in_pixels(),
      draw_tree);
}

render_tree::PixelType FromProtoPixelType(PixelType in) {
  switch (in) {
    case PixelType_RGB: return render_tree::kPixelTypeRGB;
    case PixelType_RGBA : return render_tree::kPixelTypeRGBA;
  }

  assert(false);
  return static_cast<render_tree::PixelType>(0);
}

std::shared_ptr<render_tree::Texture> ParsePixelData(
    const PixelData* pixel_data) {
  std::vector<char> data(
      pixel_data->data()->begin(), pixel_data->data()->end());
  return std::make_shared<render_tree::PixelData>(
      pixel_data->width_in_pixels(), pixel_data->height_in_pixels(),
      pixel_data->stride_in_bytes(),
      FromProtoPixelType(pixel_data->pixel_type()),
      std::move(data));
}

ParseOutput ParseTexture(
    const Texture* texture,
    const ExternalReferenceLookup& reference_lookup) {
  switch (texture->texture_type()) {
    case TextureUnion_pixel_data:
      return ParsePixelData(texture->texture_as_pixel_data());
    case TextureUnion_render_target:
      return ParseRenderTarget(
        texture->texture_as_render_target(), reference_lookup);
    default:
      assert(false);
  }

  return ParseOutput("Unknown Texture type.");
}

GLenum FromProtoSamplerWrapType(SamplerWrapType in) {
  switch (in) {
    case SamplerWrapType_TypeRepeat: return GL_REPEAT;
    case SamplerWrapType_TypeClamp: return GL_CLAMP_TO_EDGE;
  }
  assert(false);
  return 0;
}

GLenum FromProtoSamplerFilterType(SamplerFilterType in) {
  switch (in) {
    case SamplerFilterType_TypeLinear: return GL_LINEAR;
    case SamplerFilterType_TypeNearest: return GL_NEAREST;
  }
  assert(false);
  return 0;
}

ParseOutput ParseSampler(
    const Sampler* sampler,
    const ExternalReferenceLookup& reference_lookup) {
  auto texture = LookupNode<render_tree::Texture>(
      reference_lookup, sampler->texture_id());
  assert(texture);
  return std::make_shared<render_tree::Sampler>(
      texture,
      FromProtoSamplerWrapType(sampler->wrap_s()),
      FromProtoSamplerWrapType(sampler->wrap_t()),
      FromProtoSamplerFilterType(sampler->min_filter()),
      FromProtoSamplerFilterType(sampler->mag_filter()));
}

GLenum FromProtoBlendCoefficient(BlendCoefficient in) {
  switch (in) {
    case BlendCoefficient_Zero: return GL_ZERO;
    case BlendCoefficient_One: return GL_ONE;
    case BlendCoefficient_SrcAlpha: return GL_SRC_ALPHA;
    case BlendCoefficient_OneMinusSrcAlpha:
        return GL_ONE_MINUS_SRC_ALPHA;
  }
  assert(false);
  return 0;
}

ParseOutput ParsePipeline(
    const Pipeline* pipeline,
    const ExternalReferenceLookup& reference_lookup) {
  auto vertex_shader = LookupNode<render_tree::VertexShader>(
      reference_lookup, pipeline->vertex_shader_id());
  assert(vertex_shader);

  auto fragment_shader = LookupNode<render_tree::FragmentShader>(
      reference_lookup, pipeline->fragment_shader_id());
  assert(fragment_shader);

  auto program =
      std::make_shared<render_tree::Program>(vertex_shader, fragment_shader);

  if (!program->error().empty()) {
    return ParseOutput(program->error());
  }

  render_tree::Pipeline::Params params;
  params.blend.src_color = FromProtoBlendCoefficient(
      pipeline->blend_parameters()->src_color());
  params.blend.dst_color = FromProtoBlendCoefficient(
      pipeline->blend_parameters()->dst_color());
  params.blend.src_alpha = FromProtoBlendCoefficient(
      pipeline->blend_parameters()->src_alpha());
  params.blend.dst_alpha = FromProtoBlendCoefficient(
      pipeline->blend_parameters()->dst_alpha());

  return std::make_shared<render_tree::Pipeline>(program, params);
}

std::shared_ptr<render_tree::DrawTree> ParseDrawCall(
    const DrawCall* draw_call,
    const ExternalReferenceLookup& reference_lookup) {
  auto pipeline = LookupNode<render_tree::Pipeline>(
      reference_lookup, draw_call->pipeline_id());
  assert(pipeline);

  auto vertex_buffer = LookupNode<render_tree::VertexBuffer>(
      reference_lookup, draw_call->vertex_buffer_id());
  assert(vertex_buffer);

  auto vertex_uniform_values = LookupNode<render_tree::UniformValues>(
      reference_lookup, draw_call->vertex_uniform_values_id());

  auto fragment_uniform_values = LookupNode<render_tree::UniformValues>(
      reference_lookup, draw_call->fragment_uniform_values_id());

  return std::make_shared<render_tree::DrawCall>(
      pipeline, vertex_buffer, vertex_uniform_values, fragment_uniform_values);
}

std::vector<std::shared_ptr<render_tree::DrawTree>> MapTreeIdsToVector(
    const flatbuffers::Vector<int64_t>* in,
    const ExternalReferenceLookup& reference_lookup) {
  std::vector<std::shared_ptr<render_tree::DrawTree>> out;
  out.reserve(in->size());
  std::transform(in->begin(), in->end(),
                 std::back_inserter(out), [&reference_lookup](int64_t x) {
                  return LookupNode<render_tree::DrawTree>(reference_lookup, x);
                 });
  return out;
}

std::shared_ptr<render_tree::DrawTree> ParseDrawSequence(
    const DrawSequence* draw_sequence,
    const ExternalReferenceLookup& reference_lookup) {  
  return std::make_shared<render_tree::DrawSequence>(
      MapTreeIdsToVector(draw_sequence->draw_tree_ids(), reference_lookup));
}

std::shared_ptr<render_tree::DrawTree> ParseDrawSet(
    const DrawSet* draw_set,
    const ExternalReferenceLookup& reference_lookup) {
  return std::make_shared<render_tree::DrawSequence>(
      MapTreeIdsToVector(draw_set->draw_tree_ids(), reference_lookup));
}

ParseOutput ParseDrawTree(
    const DrawTree* draw_tree,
    const ExternalReferenceLookup& reference_lookup) {
  switch (draw_tree->draw_tree_type()) {
    case DrawTreeUnion_draw_call:
      return ParseDrawCall(
          draw_tree->draw_tree_as_draw_call(), reference_lookup);
    case DrawTreeUnion_draw_sequence:
      return ParseDrawSequence(
          draw_tree->draw_tree_as_draw_sequence(), reference_lookup);
    case DrawTreeUnion_draw_set:
      return ParseDrawSet(
          draw_tree->draw_tree_as_draw_set(), reference_lookup);
    default:
      assert(false);
  }

  return ParseOutput("Unknown DrawTree type.");
}

}  // namespace

ParseOutput ParseFlatBuffer(
    const ExternalReferenceLookup& reference_lookup,
    const char* data, size_t data_size) {
  const RendererNode* renderer_node =
      flatbuffers::GetRoot<entify::renderer::RendererNode>(data);

  switch(renderer_node->renderer_node_type()) {
    case RendererNodeUnion_glsl_vertex_shader: {
      return ParseGLSLVertexShader(
          renderer_node->renderer_node_as_glsl_vertex_shader());
    } break;
    case RendererNodeUnion_glsl_fragment_shader: {
      return ParseGLSLFragmentShader(
          renderer_node->renderer_node_as_glsl_fragment_shader());
    } break;
    case RendererNodeUnion_pipeline: {
      return ParsePipeline(
          renderer_node->renderer_node_as_pipeline(),
          reference_lookup);
    } break;
    case RendererNodeUnion_vertex_buffer: {
      return ParseVertexBuffer(
          renderer_node->renderer_node_as_vertex_buffer());
    } break;
    case RendererNodeUnion_uniform_values: {
      return ParseUniformValues(
          renderer_node->renderer_node_as_uniform_values(),
          reference_lookup);
    } break;
    case RendererNodeUnion_texture: {
      return ParseTexture(
          renderer_node->renderer_node_as_texture(),
          reference_lookup);
    } break;
    case RendererNodeUnion_sampler: {
      return ParseSampler(
          renderer_node->renderer_node_as_sampler(),
          reference_lookup);
    } break;
    case RendererNodeUnion_draw_tree: {
      return ParseDrawTree(
          renderer_node->renderer_node_as_draw_tree(),
          reference_lookup);
    } break;
  };

  assert(false);
  return std::unique_ptr<ExternalReference>();
}

}  // namespace gles2
}  // namespace renderer
}  // namespace entify
