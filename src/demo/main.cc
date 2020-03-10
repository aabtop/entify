#include "entify/entify.h"
#include "entify/renderer_definitions.pb.h"
#include "platform_window/platform_window.h"

namespace {
struct EntifyReferenceAndId {
  EntifyId id;
  EntifyReference reference;
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

EntifyReferenceAndId GetOrMakeVertexShader(EntifyContext context) {
  const EntifyId kId = 1;
  EntifyReference reference = EntifyTryGetReferenceFromId(context, kId);
  if (reference != kEntifyInvalidReference) {
    return {kId, reference};
  }

  entify_renderer::NamedPrimitiveTypeTuple input_types;
  entify_renderer::NamedPrimitiveType* position_attribute =
      input_types.add_named_types();
  position_attribute->set_name("a_position");
  position_attribute->set_type(entify_renderer::PrimitiveTypeFloat32V3);

  entify_renderer::PrimitiveTypeTuple output_types;
  output_types.add_types(entify_renderer::PrimitiveTypeFloat32V3);

  entify_renderer::NamedPrimitiveTypeTuple uniform_types;
  entify_renderer::NamedPrimitiveType* translation_uniform =
      uniform_types.add_named_types();
  translation_uniform->set_name("u_translation");
  translation_uniform->set_type(entify_renderer::PrimitiveTypeFloat32V3);

  entify_renderer::GLSLVertexShader shader;
  shader.set_allocated_input_types(&input_types);
  shader.set_allocated_output_types(&output_types);
  shader.set_allocated_uniform_types(&uniform_types);
  shader.set_source(""
      "attribute vec3 a_position;"
      ""
      "varying vec3 v_position;"
      ""
      "uniform vec3 u_translation;"
      ""
      "void main() {  "
      "  v_position = a_position;"
      "  gl_Position =  vec4(a_position + u_translation, 1);"
      "}"
    );

  std::string serialized = SerializeAsRendererNode(
      &entify_renderer::RendererNode::set_allocated_glsl_vertex_shader,
      &entify_renderer::RendererNode::release_glsl_vertex_shader,
      &shader);

  reference = EntifyCreateReferenceFromProtocolBuffer(
      context, kId, serialized.data(), serialized.size());
  assert(reference != kEntifyInvalidReference);

  shader.release_uniform_types();
  shader.release_output_types();
  shader.release_input_types();

  return {kId, reference};
}

EntifyReferenceAndId GetOrMakeFragmentShader(EntifyContext context) {
  const EntifyId kId = 2;
  EntifyReference reference = EntifyTryGetReferenceFromId(context, kId);
  if (reference != kEntifyInvalidReference) {
    return {kId, reference};
  }

  entify_renderer::PrimitiveTypeTuple input_types;
  input_types.add_types(entify_renderer::PrimitiveTypeFloat32V3);

  entify_renderer::NamedPrimitiveTypeTuple uniform_types;
  entify_renderer::NamedPrimitiveType* translation_uniform =
      uniform_types.add_named_types();
  translation_uniform->set_name("u_color");
  translation_uniform->set_type(entify_renderer::PrimitiveTypeFloat32V3);

  entify_renderer::GLSLFragmentShader shader;
  shader.set_allocated_input_types(&input_types);
  shader.set_allocated_uniform_types(&uniform_types);
  shader.set_source(""
      "precision highp float;"
      "varying vec3 v_position;"
      ""
      "uniform vec3 u_color;"
      ""
      "void main() {  "
      "  gl_FragColor =  vec4(u_color, 1.0);"
      "}"
    );

  std::string serialized = SerializeAsRendererNode(
      &entify_renderer::RendererNode::set_allocated_glsl_fragment_shader,
      &entify_renderer::RendererNode::release_glsl_fragment_shader,
      &shader);

  reference = EntifyCreateReferenceFromProtocolBuffer(
      context, kId, serialized.data(), serialized.size());
  assert(reference != kEntifyInvalidReference);

  shader.release_uniform_types();
  shader.release_input_types();

  return {kId, reference};
}

EntifyReferenceAndId GetOrMakePipeline(EntifyContext context) {
  const EntifyId kId = 3;
  EntifyReference reference = EntifyTryGetReferenceFromId(context, kId);
  if (reference != kEntifyInvalidReference) {
    return {kId, reference};
  }

  auto vertex_shader = GetOrMakeVertexShader(context);
  auto fragment_shader = GetOrMakeFragmentShader(context);
  entify_renderer::Pipeline pipeline;
  pipeline.set_vertex_shader_id(vertex_shader.id);
  pipeline.set_fragment_shader_id(fragment_shader.id);

  std::string serialized = SerializeAsRendererNode(
      &entify_renderer::RendererNode::set_allocated_pipeline,
      &entify_renderer::RendererNode::release_pipeline,
      &pipeline);

  reference = EntifyCreateReferenceFromProtocolBuffer(
      context, kId, serialized.data(), serialized.size());
  assert(reference != kEntifyInvalidReference);

  EntifyReleaseReference(context, vertex_shader.reference);
  EntifyReleaseReference(context, fragment_shader.reference);

  return {kId, reference};
}

EntifyReferenceAndId GetOrMakeVertexBuffer(EntifyContext context) {
  const EntifyId kId = 4;
  EntifyReference reference = EntifyTryGetReferenceFromId(context, kId);
  if (reference != kEntifyInvalidReference) {
    return {kId, reference};
  }

  entify_renderer::PrimitiveTypeTuple types;
  types.add_types(entify_renderer::PrimitiveTypeFloat32V3);

  entify_renderer::VertexBuffer vertex_buffer;
  vertex_buffer.set_allocated_types(&types);
  float kVertexData[] = {
    -0.5f, -0.5f, 0.0f,
    0.5f, -0.5f, 0.0f,
    0.0f, 0.5f, 0.0f,
  };
  vertex_buffer.set_stride_in_bytes(sizeof(float) * 3);
  vertex_buffer.set_data(kVertexData, sizeof(kVertexData));

  std::string serialized = SerializeAsRendererNode(
      &entify_renderer::RendererNode::set_allocated_vertex_buffer,
      &entify_renderer::RendererNode::release_vertex_buffer,
      &vertex_buffer);

  reference = EntifyCreateReferenceFromProtocolBuffer(
      context, kId, serialized.data(), serialized.size());
  assert(reference != kEntifyInvalidReference);

  vertex_buffer.release_types();

  return {kId, reference};
}

EntifyReferenceAndId GetOrMakeVertexUniformValues(
    EntifyContext context, EntifyId id, float translate_x, float translate_y) {
  EntifyReference reference = EntifyTryGetReferenceFromId(context, id);
  if (reference != kEntifyInvalidReference) {
    return {id, reference};
  }

  entify_renderer::PrimitiveTypeTuple types;
  types.add_types(entify_renderer::PrimitiveTypeFloat32V3);

  entify_renderer::UniformValues uniform_values;
  uniform_values.set_allocated_types(&types);

  float kUniformData[] = {translate_x, translate_y, 0.0f};
  uniform_values.set_data(kUniformData, sizeof(kUniformData));

  std::string serialized = SerializeAsRendererNode(
      &entify_renderer::RendererNode::set_allocated_uniform_values,
      &entify_renderer::RendererNode::release_uniform_values,
      &uniform_values);

  reference = EntifyCreateReferenceFromProtocolBuffer(
      context, id, serialized.data(), serialized.size());
  assert(reference != kEntifyInvalidReference);

  uniform_values.release_types();

  return {id, reference};  
}

EntifyReferenceAndId GetOrMakeFragmentUniformValues(
    EntifyContext context, EntifyId id, float r, float g, float b) {
  EntifyReference reference = EntifyTryGetReferenceFromId(context, id);
  if (reference != kEntifyInvalidReference) {
    return {id, reference};
  }

  entify_renderer::PrimitiveTypeTuple types;
  types.add_types(entify_renderer::PrimitiveTypeFloat32V3);

  entify_renderer::UniformValues uniform_values;
  uniform_values.set_allocated_types(&types);

  float kUniformData[] = {r, g, b};
  uniform_values.set_data(kUniformData, sizeof(kUniformData));

  std::string serialized = SerializeAsRendererNode(
      &entify_renderer::RendererNode::set_allocated_uniform_values,
      &entify_renderer::RendererNode::release_uniform_values,
      &uniform_values);

  reference = EntifyCreateReferenceFromProtocolBuffer(
      context, id, serialized.data(), serialized.size());
  assert(reference != kEntifyInvalidReference);

  uniform_values.release_types();

  return {id, reference};  
}

EntifyReferenceAndId GetOrMakeDrawCall(
    EntifyContext context, EntifyId id, float translate_x, float translate_y,
    float r, float g, float b) {
  EntifyReference reference = EntifyTryGetReferenceFromId(context, id);
  if (reference != kEntifyInvalidReference) {
    return {id, reference};
  }

  auto pipeline = GetOrMakePipeline(context);
  auto vertex_buffer = GetOrMakeVertexBuffer(context);
  auto vertex_uniform_values =
      GetOrMakeVertexUniformValues(context, id + 1, translate_x, translate_y);
  auto fragment_uniform_values =
      GetOrMakeFragmentUniformValues(context, id + 2, r, g, b);

  entify_renderer::DrawCall draw_call;
  draw_call.set_pipeline_id(pipeline.id);
  draw_call.set_vertex_buffer_id(vertex_buffer.id);
  draw_call.set_vertex_uniform_values_id(vertex_uniform_values.id);
  draw_call.set_fragment_uniform_values_id(fragment_uniform_values.id);
  entify_renderer::DrawTree draw_tree;
  draw_tree.set_allocated_draw_call(&draw_call);

  std::string serialized = SerializeAsRendererNode(
      &entify_renderer::RendererNode::set_allocated_draw_tree,
      &entify_renderer::RendererNode::release_draw_tree,
      &draw_tree);

  reference = EntifyCreateReferenceFromProtocolBuffer(
      context, id, serialized.data(), serialized.size());
  assert(reference != kEntifyInvalidReference);


  EntifyReleaseReference(context, fragment_uniform_values.reference);
  EntifyReleaseReference(context, vertex_uniform_values.reference);
  EntifyReleaseReference(context, vertex_buffer.reference);
  EntifyReleaseReference(context, pipeline.reference);

  draw_tree.release_draw_call();

  return {id, reference};
}

EntifyReferenceAndId GetOrMakeDrawTree(EntifyContext context) {
  const EntifyId kId = 6;
  EntifyReference reference = EntifyTryGetReferenceFromId(context, kId);
  if (reference != kEntifyInvalidReference) {
    return {kId, reference};
  }

  const EntifyId kDrawCall1Id = 50;
  const EntifyId kDrawCall2Id = 60;

  auto draw_call1 = GetOrMakeDrawCall(
      context, kDrawCall1Id, -0.25f, 0.0f, 1.0f, 0.3f, 0.3f);
  auto draw_call2 = GetOrMakeDrawCall(
      context, kDrawCall2Id, 0.25f, 0.0f, 0.2f, 0.2f, 1.0f);

  entify_renderer::DrawSequence draw_sequence;
  draw_sequence.add_draw_tree_ids(draw_call1.id);
  draw_sequence.add_draw_tree_ids(draw_call2.id);

  entify_renderer::DrawTree draw_tree;
  draw_tree.set_allocated_draw_sequence(&draw_sequence);

  std::string serialized = SerializeAsRendererNode(
      &entify_renderer::RendererNode::set_allocated_draw_tree,
      &entify_renderer::RendererNode::release_draw_tree,
      &draw_tree);

  reference = EntifyCreateReferenceFromProtocolBuffer(
      context, kId, serialized.data(), serialized.size());
  assert(reference != kEntifyInvalidReference);


  EntifyReleaseReference(context, draw_call2.reference);
  EntifyReleaseReference(context, draw_call1.reference);

  draw_tree.release_draw_sequence();

  return {kId, reference};
}

bool g_quit_flag = false;

void HandleWindowEvent(
    void* context, PlatformWindowEvent event, void* data) {
  if (event == kPlatformWindowEventQuitRequest) {
    g_quit_flag = true;
  }
}
}  // namespace

int main(int argc, const char** args) {
  EntifyContext context = EntifyCreateContext();
  assert(context != kEntifyInvalidContext);

  PlatformWindow window = PlatformWindowMakeDefaultWindow(
      "Entify Demo", &HandleWindowEvent, nullptr);
  EntifyRenderTarget render_target =
      EntifyCreateRenderTargetFromPlatformWindow(
          context, PlatformWindowGetNativeWindow(window),
          PlatformWindowGetWidth(window), PlatformWindowGetHeight(window));
  assert(render_target != kEntifyInvalidRenderTarget);

  const EntifyId kDrawCallId = 50;
  while(!g_quit_flag) {
    EntifyReference draw_tree = GetOrMakeDrawTree(context).reference;
    EntifySubmit(context, draw_tree, render_target);
    EntifyReleaseReference(context, draw_tree);
  }

  EntifyReleaseRenderTarget(context, render_target);
  PlatformWindowDestroyWindow(window);

  EntifyDestroyContext(context);

  return 0;
}