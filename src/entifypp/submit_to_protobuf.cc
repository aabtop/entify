#include "entify/entify.h"
#include "entify/renderer_definitions.pb.h"
#include "entifypp/draw_tree_visitor.h"
#include "entifypp/draw_call.h"
#include "entifypp/draw_sequence.h"
#include "entifypp/draw_set.h"
#include "submit_to_protobuf.h"

namespace entifypp {

class ScopedReference {
 public:
  ScopedReference(EntifyContext context, EntifyReference reference)
      : context_(context), reference_(reference) {}
  ScopedReference(EntifyContext context)
      : ScopedReference(context, kEntifyInvalidReference) {}
  ScopedReference(ScopedReference&& rhs)
      : context_(rhs.context_), reference_(rhs.reference_) {
    rhs.reference_ = kEntifyInvalidReference;
  }

  ScopedReference(const ScopedReference&) = delete;
  ScopedReference& operator=(const ScopedReference&) = delete;

  ~ScopedReference() {
    if (reference_ != kEntifyInvalidReference) {
      EntifyReleaseReference(context_, reference_);
    }
  }

  ScopedReference& operator=(EntifyReference rhs) {
    assert(reference_ == kEntifyInvalidReference);
    reference_ = rhs;
    return *this;
  }

  ScopedReference& operator=(ScopedReference&& rhs) {
    assert(reference_ == kEntifyInvalidReference);
    assert(context_ == rhs.context_);
    reference_ = rhs.reference_;
    rhs.reference_ = kEntifyInvalidReference;
    return *this;
  }


  EntifyReference reference() const { return reference_; }

  EntifyReference take() {
    EntifyReference result = reference_;
    reference_ = kEntifyInvalidReference;
    return result;
  }

  EntifyContext context() const { return context_; }

 private:
  EntifyContext context_;
  EntifyReference reference_;
};

template <typename SetMethod, typename ReleaseMethod, typename T>
std::string SerializeAsRendererNode(
    SetMethod set_method, ReleaseMethod release_method,
    T* derived_node) {
  entify_renderer::RendererNode node;
  (node.*set_method)(derived_node);

  std::string serialized;
  bool result = node.SerializeToString(&serialized);
  assert(result);

  (node.*release_method)();

  return std::move(serialized);
}

class DrawTreeProtobufVisitor : public DrawTreeVisitor {
 public:
  DrawTreeProtobufVisitor(EntifyContext context)
      : context_(context), reference_(context_, kEntifyInvalidReference) {}
  ~DrawTreeProtobufVisitor() {
    // You should do something with the reference that is produced by the visit.
    assert(reference_.reference() == kEntifyInvalidReference);
  }

  void Visit(const DrawCall* draw_call) override;
  void Visit(const DrawSequence* draw_sequence) override;
  void Visit(const DrawSet* draw_set) override;

  ScopedReference&& take_reference() { return std::move(reference_); }

 private:
  EntifyContext context_;
  ScopedReference reference_;
};

entify_renderer::PrimitiveType ConvertTypeToProtobuf(Type type) {
  switch (type) {
    case TypeFloat32V1: return entify_renderer::PrimitiveTypeFloat32V1;
    case TypeFloat32V2: return entify_renderer::PrimitiveTypeFloat32V2;
    case TypeFloat32V3: return entify_renderer::PrimitiveTypeFloat32V3;
    case TypeFloat32V4: return entify_renderer::PrimitiveTypeFloat32V4;
    case TypeFloat32M44: return entify_renderer::PrimitiveTypeFloat32M44;
    case TypeUInt8V1: return entify_renderer::PrimitiveTypeUInt8V1;
    case TypeUInt8V2: return entify_renderer::PrimitiveTypeUInt8V2;
    case TypeUInt8V3: return entify_renderer::PrimitiveTypeUInt8V3;
    case TypeUInt8V4: return entify_renderer::PrimitiveTypeUInt8V4;
    case TypeSampler: return entify_renderer::PrimitiveTypeSampler;
    default:
      assert(false);
      return static_cast<entify_renderer::PrimitiveType>(0);
  }
}

entify_renderer::PixelType ConvertPixelTypeToProtobuf(PixelType type) {
  switch (type) {
    case kPixelTypeRGB: return entify_renderer::PixelTypeRGB;
    case kPixelTypeRGBA: return entify_renderer::PixelTypeRGBA;
  }

  assert(false);
  return static_cast<entify_renderer::PixelType>(0);
}

void PopulateProtobufTypeTuple(
    const std::vector<Type>& types,
    entify_renderer::PrimitiveTypeTuple* output_pb) {
  for (auto type : types) {
    output_pb->add_types(ConvertTypeToProtobuf(type));
  }
}

void PopulateProtobufNamedTypeTuple(
    const std::vector<std::string>& names,
    const std::vector<Type>& types,
    entify_renderer::NamedPrimitiveTypeTuple* output_pb) {
  assert(names.size() == types.size());
  for (size_t i = 0; i < names.size(); ++i) {
    entify_renderer::NamedPrimitiveType* named_type_pb =
        output_pb->add_named_types();
    named_type_pb->set_name(names[i]);
    named_type_pb->set_type(ConvertTypeToProtobuf(types[i]));
  }
}

ScopedReference SubmitGLSLVertexShader(
    EntifyContext context, const GLSLVertexShaderBase* shader) {
  ScopedReference result(context);

  result = EntifyTryGetReferenceFromId(context, shader->hash());
  if (result.reference() != kEntifyInvalidReference) {
    return std::move(result);
  }

  entify_renderer::NamedPrimitiveTypeTuple input_types_pb;
  PopulateProtobufNamedTypeTuple(
      shader->vertex_attribute_names(), shader->GetInputTypes(),
      &input_types_pb);

  entify_renderer::PrimitiveTypeTuple output_types_pb;
  PopulateProtobufTypeTuple(shader->GetOutputTypes(), &output_types_pb);

  entify_renderer::NamedPrimitiveTypeTuple uniform_types_pb;
  PopulateProtobufNamedTypeTuple(
      shader->uniform_names(), shader->GetUniformTypes(),
      &uniform_types_pb);

  entify_renderer::GLSLVertexShader shader_pb;
  shader_pb.set_allocated_input_types(&input_types_pb);
  shader_pb.set_allocated_output_types(&output_types_pb);
  shader_pb.set_allocated_uniform_types(&uniform_types_pb);
  shader_pb.set_source(shader->source());

  std::string serialized = SerializeAsRendererNode(
      &entify_renderer::RendererNode::set_allocated_glsl_vertex_shader,
      &entify_renderer::RendererNode::release_glsl_vertex_shader,
      &shader_pb);

  result = EntifyCreateReferenceFromProtocolBuffer(
      context, shader->hash(), serialized.data(), serialized.size());
  assert(result.reference() != kEntifyInvalidReference);

  shader_pb.release_input_types();
  shader_pb.release_output_types();
  shader_pb.release_uniform_types();

  return std::move(result);
}

ScopedReference SubmitGLSLFragmentShader(
    EntifyContext context, const GLSLFragmentShaderBase* shader) {
  ScopedReference result(context);

  result = EntifyTryGetReferenceFromId(context, shader->hash());
  if (result.reference() != kEntifyInvalidReference) {
    return std::move(result);
  }

  entify_renderer::PrimitiveTypeTuple input_types_pb;
  PopulateProtobufTypeTuple(shader->GetInputTypes(), &input_types_pb);

  entify_renderer::NamedPrimitiveTypeTuple uniform_types_pb;
  PopulateProtobufNamedTypeTuple(
      shader->uniform_names(), shader->GetUniformTypes(),
      &uniform_types_pb);

  entify_renderer::GLSLFragmentShader shader_pb;
  shader_pb.set_allocated_input_types(&input_types_pb);
  shader_pb.set_allocated_uniform_types(&uniform_types_pb);
  shader_pb.set_source(shader->source());

  std::string serialized = SerializeAsRendererNode(
      &entify_renderer::RendererNode::set_allocated_glsl_fragment_shader,
      &entify_renderer::RendererNode::release_glsl_fragment_shader,
      &shader_pb);

  result = EntifyCreateReferenceFromProtocolBuffer(
      context, shader->hash(), serialized.data(), serialized.size());
  assert(result.reference() != kEntifyInvalidReference);

  shader_pb.release_input_types();
  shader_pb.release_uniform_types();

  return std::move(result);
}

namespace {
entify_renderer::BlendCoefficient ConvertPipelineBlendCoefficient(
    PipelineOptions::BlendOptions::Coefficient coefficient) {
  switch (coefficient) {
    case PipelineOptions::BlendOptions::Coefficient::kZero:
        return entify_renderer::BlendCoefficientZero;
    case PipelineOptions::BlendOptions::Coefficient::kOne:
        return entify_renderer::BlendCoefficientOne;
    case PipelineOptions::BlendOptions::Coefficient::kSrcAlpha:
        return entify_renderer::BlendCoefficientSrcAlpha;
    case PipelineOptions::BlendOptions::Coefficient::kOneMinusSrcAlpha:
        return entify_renderer::BlendCoefficientOneMinusSrcAlpha;
  }
  assert(false);
  return entify_renderer::BlendCoefficientOne;
}
}  // namespace

ScopedReference SubmitPipeline(
    EntifyContext context, const PipelineBase* pipeline) {
  ScopedReference result(context);

  result = EntifyTryGetReferenceFromId(context, pipeline->hash());
  if (result.reference() != kEntifyInvalidReference) {
    return std::move(result);
  }

  ScopedReference vertex_shader =
      SubmitGLSLVertexShader(context, pipeline->vertex_shader_base().get());
  ScopedReference fragment_shader =
      SubmitGLSLFragmentShader(context, pipeline->fragment_shader_base().get());

  entify_renderer::BlendParameters blend_params_pb;
  blend_params_pb.set_src_color(ConvertPipelineBlendCoefficient(
      pipeline->options().blend.src_color));
  blend_params_pb.set_dst_color(ConvertPipelineBlendCoefficient(
      pipeline->options().blend.dst_color));
  blend_params_pb.set_src_alpha(ConvertPipelineBlendCoefficient(
      pipeline->options().blend.src_alpha));
  blend_params_pb.set_dst_alpha(ConvertPipelineBlendCoefficient(
      pipeline->options().blend.dst_alpha));

  entify_renderer::Pipeline pipeline_pb;
  pipeline_pb.set_vertex_shader_id(pipeline->vertex_shader_base()->hash());
  pipeline_pb.set_fragment_shader_id(pipeline->fragment_shader_base()->hash());
  pipeline_pb.set_allocated_blend_parameters(&blend_params_pb);

  std::string serialized = SerializeAsRendererNode(
      &entify_renderer::RendererNode::set_allocated_pipeline,
      &entify_renderer::RendererNode::release_pipeline,
      &pipeline_pb);

  result = EntifyCreateReferenceFromProtocolBuffer(
      context, pipeline->hash(), serialized.data(), serialized.size());
  assert(result.reference() != kEntifyInvalidReference);

  pipeline_pb.release_blend_parameters();

  return std::move(result);
}

class TextureProtobufVisitor : public TextureVisitor {
 public:
  TextureProtobufVisitor(EntifyContext context)
      : context_(context), reference_(context_, kEntifyInvalidReference) {}
  ~TextureProtobufVisitor() {
    // You should do something with the reference that is produced by the visit.
    assert(reference_.reference() == kEntifyInvalidReference);
  }

  void Visit(const ReferenceTexture* reference) override;
  void Visit(const RenderTargetTexture* render_target) override;
  void Visit(const PixelDataTexture* pixel_data) override;

  ScopedReference&& take_reference() { return std::move(reference_); }

 private:
  EntifyContext context_;
  ScopedReference reference_;
};

ScopedReference SubmitTexture(
    EntifyContext context, const Texture* texture) {
  ScopedReference result(context);

  result = EntifyTryGetReferenceFromId(context, texture->hash());
  if (result.reference() != kEntifyInvalidReference) {
    return std::move(result);
  }

  TextureProtobufVisitor visitor(context);
  texture->Accept(&visitor);

  return visitor.take_reference();
}

namespace {
entify_renderer::SamplerWrapType ConvertSamplerWrapTypeToProtobuf(
    Sampler::WrapType type) {
  switch (type) {
    case Sampler::WrapType::kRepeat:
        return entify_renderer::SamplerWrapTypeRepeat;
    case Sampler::WrapType::kClamp:
        return entify_renderer::SamplerWrapTypeClamp;
  }
  assert(false);
  return entify_renderer::SamplerWrapTypeRepeat;
}

entify_renderer::SamplerFilterType ConvertSamplerFilterTypeToProtobuf(
    Sampler::FilterType type) {
  switch (type) {
    case Sampler::FilterType::kLinear:
        return entify_renderer::SamplerFilterTypeLinear;
    case Sampler::FilterType::kNearest:
        return entify_renderer::SamplerFilterTypeNearest;
  }
  assert(false);
  return entify_renderer::SamplerFilterTypeLinear;
}
}  // namespace

ScopedReference SubmitSampler(EntifyContext context, const Sampler* sampler) {
  ScopedReference result(context);

  result = EntifyTryGetReferenceFromId(context, sampler->hash());
  if (result.reference() != kEntifyInvalidReference) {
    return std::move(result);
  }

  ScopedReference texture = SubmitTexture(context, sampler->texture().get());

  entify_renderer::Sampler sampler_pb;
  sampler_pb.set_texture_id(sampler->texture()->hash());

  sampler_pb.set_wrap_s(ConvertSamplerWrapTypeToProtobuf(
      sampler->options().wrap_s));
  sampler_pb.set_wrap_t(ConvertSamplerWrapTypeToProtobuf(
      sampler->options().wrap_t));
  sampler_pb.set_min_filter(ConvertSamplerFilterTypeToProtobuf(
      sampler->options().min_filter));
  sampler_pb.set_mag_filter(ConvertSamplerFilterTypeToProtobuf(
      sampler->options().mag_filter));

  std::string serialized = SerializeAsRendererNode(
      &entify_renderer::RendererNode::set_allocated_sampler,
      &entify_renderer::RendererNode::release_sampler,
      &sampler_pb);

  result = EntifyCreateReferenceFromProtocolBuffer(
      context, sampler->hash(), serialized.data(), serialized.size());
  assert(result.reference() != kEntifyInvalidReference);

  return result;
}

ScopedReference SubmitUniformValues(
    EntifyContext context, const UniformValuesBase* uniform_values) {
  ScopedReference result(context);

  result = EntifyTryGetReferenceFromId(context, uniform_values->hash());
  if (result.reference() != kEntifyInvalidReference) {
    return std::move(result);
  }

  entify_renderer::PrimitiveTypeTuple types_pb;
  PopulateProtobufTypeTuple(uniform_values->GetTypes(), &types_pb);

  entify_renderer::UniformValues uniform_values_pb;
  uniform_values_pb.set_allocated_types(&types_pb);

  auto data_info = uniform_values->GetDataInfo();
  uniform_values_pb.set_data(data_info.data, data_info.size);

  auto samplers = uniform_values->GetSamplers();
  std::vector<ScopedReference> sampler_references;
  sampler_references.reserve(samplers.size());
  for (const auto& sampler : samplers) {
    sampler_references.emplace_back(SubmitSampler(context, sampler.get()));
    uniform_values_pb.add_sampler_ids(sampler->hash());
  }

  std::string serialized = SerializeAsRendererNode(
      &entify_renderer::RendererNode::set_allocated_uniform_values,
      &entify_renderer::RendererNode::release_uniform_values,
      &uniform_values_pb);

  result = EntifyCreateReferenceFromProtocolBuffer(
      context, uniform_values->hash(), serialized.data(), serialized.size());
  assert(result.reference() != kEntifyInvalidReference);

  uniform_values_pb.release_types();

  return std::move(result);
}

ScopedReference SubmitVertexBuffer(
    EntifyContext context, const VertexBufferBase* vertex_buffer) {
  ScopedReference result(context);

  result = EntifyTryGetReferenceFromId(context, vertex_buffer->hash());
  if (result.reference() != kEntifyInvalidReference) {
    return std::move(result);
  }

  entify_renderer::PrimitiveTypeTuple types_pb;
  PopulateProtobufTypeTuple(vertex_buffer->GetTypes(), &types_pb);

  entify_renderer::VertexBuffer vertex_buffer_pb;
  vertex_buffer_pb.set_allocated_types(&types_pb);

  auto data_info = vertex_buffer->GetDataInfo();
  vertex_buffer_pb.set_data(
      data_info.data, data_info.num_vertices * data_info.stride_in_bytes);
  vertex_buffer_pb.set_stride_in_bytes(data_info.stride_in_bytes);

  auto offsets = vertex_buffer->GetDataOffsets();
  for (auto offset : offsets) {
    vertex_buffer_pb.add_offsets(offset);
  }

  std::string serialized = SerializeAsRendererNode(
      &entify_renderer::RendererNode::set_allocated_vertex_buffer,
      &entify_renderer::RendererNode::release_vertex_buffer,
      &vertex_buffer_pb);

  result = EntifyCreateReferenceFromProtocolBuffer(
      context, vertex_buffer->hash(), serialized.data(), serialized.size());
  assert(result.reference() != kEntifyInvalidReference);

  vertex_buffer_pb.release_types();

  return std::move(result);
}

void DrawTreeProtobufVisitor::Visit(const DrawCall* draw_call) {
  ScopedReference pipeline =
      SubmitPipeline(context_, draw_call->pipeline().get());
  ScopedReference vertex_buffer =
      SubmitVertexBuffer(context_, draw_call->vertex_buffer().get());
  ScopedReference vertex_uniforms =
      SubmitUniformValues(context_, draw_call->vertex_uniforms().get());
  ScopedReference fragment_uniforms =
      SubmitUniformValues(context_, draw_call->fragment_uniforms().get());

  entify_renderer::DrawCall draw_call_pb;
  draw_call_pb.set_pipeline_id(draw_call->pipeline()->hash());
  draw_call_pb.set_vertex_buffer_id(draw_call->vertex_buffer()->hash());
  draw_call_pb.set_vertex_uniform_values_id(
      draw_call->vertex_uniforms()->hash());
  draw_call_pb.set_fragment_uniform_values_id(
      draw_call->fragment_uniforms()->hash());
  entify_renderer::DrawTree draw_tree_pb;
  draw_tree_pb.set_allocated_draw_call(&draw_call_pb);

  std::string serialized = SerializeAsRendererNode(
      &entify_renderer::RendererNode::set_allocated_draw_tree,
      &entify_renderer::RendererNode::release_draw_tree,
      &draw_tree_pb);

  reference_ = EntifyCreateReferenceFromProtocolBuffer(
      context_, draw_call->hash(), serialized.data(), serialized.size());
  assert(reference_.reference() != kEntifyInvalidReference);

  draw_tree_pb.release_draw_call();
}

ScopedReference SubmitDrawTree(
    EntifyContext context, const DrawTree* draw_tree) {
  ScopedReference result(context);

  result = EntifyTryGetReferenceFromId(context, draw_tree->hash());
  if (result.reference() != kEntifyInvalidReference) {
    return std::move(result);
  }

  DrawTreeProtobufVisitor visitor(context);
  draw_tree->Accept(&visitor);

  result = visitor.take_reference();

  return result;
}

ScopedReference SubmitSetOrSequence(
    EntifyContext context,
    EntifyId hash,
    const std::vector<std::shared_ptr<DrawTree>>& set_or_sequence) {
  ScopedReference result(context);

  entify_renderer::DrawSequence draw_sequence_pb;

  std::vector<ScopedReference> child_draw_trees;
  child_draw_trees.reserve(set_or_sequence.size());

  for (const auto& draw_tree : set_or_sequence) {
    child_draw_trees.emplace_back(SubmitDrawTree(context, draw_tree.get()));

    draw_sequence_pb.add_draw_tree_ids(draw_tree->hash());
  }

  entify_renderer::DrawTree draw_tree_pb;
  draw_tree_pb.set_allocated_draw_sequence(&draw_sequence_pb);

  std::string serialized = SerializeAsRendererNode(
      &entify_renderer::RendererNode::set_allocated_draw_tree,
      &entify_renderer::RendererNode::release_draw_tree,
      &draw_tree_pb);

  result = EntifyCreateReferenceFromProtocolBuffer(
      context, hash, serialized.data(), serialized.size());
  assert(result.reference() != kEntifyInvalidReference);

  draw_tree_pb.release_draw_sequence();

  return std::move(result);
}

void DrawTreeProtobufVisitor::Visit(const DrawSet* draw_set) {
  reference_ = SubmitSetOrSequence(context_, draw_set->hash(), draw_set->set());
}

void DrawTreeProtobufVisitor::Visit(const DrawSequence* draw_sequence) {
  reference_ = SubmitSetOrSequence(
      context_, draw_sequence->hash(), draw_sequence->sequence());
}

void TextureProtobufVisitor::Visit(const ReferenceTexture* reference) {
  EntifyReference entify_reference = reference->reference();
  EntifyAddReference(reference_.context(), entify_reference);
  reference_ = entify_reference;
}

void TextureProtobufVisitor::Visit(const RenderTargetTexture* render_target) {
  ScopedReference draw_tree =
      SubmitDrawTree(context_, render_target->draw_tree().get());

  entify_renderer::RenderTarget render_target_pb;
  render_target_pb.set_width_in_pixels(render_target->width_in_pixels());
  render_target_pb.set_height_in_pixels(render_target->height_in_pixels());
  render_target_pb.set_draw_tree_id(render_target->draw_tree()->hash());

  entify_renderer::Texture texture_pb;
  texture_pb.set_allocated_render_target(&render_target_pb);

  std::string serialized = SerializeAsRendererNode(
      &entify_renderer::RendererNode::set_allocated_texture,
      &entify_renderer::RendererNode::release_texture,
      &texture_pb);

  reference_ = EntifyCreateReferenceFromProtocolBuffer(
      context_, render_target->hash(), serialized.data(), serialized.size());
  assert(reference_.reference() != kEntifyInvalidReference);

  texture_pb.release_render_target();
}

void TextureProtobufVisitor::Visit(const PixelDataTexture* pixel_data) {
  entify_renderer::PixelData pixel_data_pb;
  pixel_data_pb.set_width_in_pixels(pixel_data->width_in_pixels());
  pixel_data_pb.set_height_in_pixels(pixel_data->height_in_pixels());
  pixel_data_pb.set_stride_in_bytes(pixel_data->stride_in_bytes());
  pixel_data_pb.set_pixel_type(ConvertPixelTypeToProtobuf(pixel_data->pixel_type()));

  pixel_data_pb.set_data(
      pixel_data->pixel_data().data(), pixel_data->pixel_data().size());

  entify_renderer::Texture texture_pb;
  texture_pb.set_allocated_pixel_data(&pixel_data_pb);

  std::string serialized = SerializeAsRendererNode(
      &entify_renderer::RendererNode::set_allocated_texture,
      &entify_renderer::RendererNode::release_texture,
      &texture_pb);

  reference_ = EntifyCreateReferenceFromProtocolBuffer(
      context_, pixel_data->hash(), serialized.data(), serialized.size());
  assert(reference_.reference() != kEntifyInvalidReference);

  texture_pb.release_pixel_data();
}

EntifyReference SubmitTextureToProtobuf(
    EntifyContext context, const std::shared_ptr<Texture>& texture) {
  ScopedReference reference(SubmitTexture(context, texture.get()));
  return reference.take();
}

void SubmitDrawTreeToProtobuf(
    EntifyContext context, RenderTarget* render_target,
    const std::shared_ptr<DrawTree>& draw_tree) {
  ScopedReference reference(SubmitDrawTree(context, draw_tree.get()));
  EntifySubmit(context, reference.reference(), render_target->render_target());
}

}  // namespace entifypp
