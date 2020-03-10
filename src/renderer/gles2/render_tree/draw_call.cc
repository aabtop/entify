#include "src/renderer/gles2/render_tree/draw_call.h"

#include <cassert>

#include "src/renderer/gles2/render_tree/types.h"

namespace entify {
namespace renderer {
namespace gles2 {
namespace render_tree {

DrawCall::DrawCall(
    const std::shared_ptr<Pipeline>& pipeline,
    const std::shared_ptr<VertexBuffer>& vertex_buffer,
    const std::shared_ptr<UniformValues>& vertex_uniform_values,
    const std::shared_ptr<UniformValues>& fragment_uniform_values)
    : DrawTree(DrawTree::kTypeDrawCall), pipeline_(pipeline),
      vertex_buffer_(vertex_buffer),
      vertex_uniform_values_(vertex_uniform_values),
      fragment_uniform_values_(fragment_uniform_values) {
  // Do type checking.
  assert(pipeline->program()->vertex_shader()->input_types() ==
             vertex_buffer->types());
  if (vertex_uniform_values_) {
    assert(pipeline->program()->vertex_shader()->uniform_types() ==
               vertex_uniform_values->types());
  } else {
    assert(pipeline->program()->vertex_shader()->uniform_types().empty());
  }
  if (fragment_uniform_values_) {
    assert(pipeline->program()->fragment_shader()->uniform_types() ==
               fragment_uniform_values->types());
  } else {
    assert(pipeline->program()->fragment_shader()->uniform_types().empty());
  }
}

}  // namespace render_tree
}  // namespace gles2
}  // namespace renderer
}  // namespace entify
