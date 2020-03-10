#ifndef _SRC_ENTIFY_RENDERER_GLES2_RENDER_TREE_DRAW_CALL_H_
#define _SRC_ENTIFY_RENDERER_GLES2_RENDER_TREE_DRAW_CALL_H_

#include <memory>

#include <GLES2/gl2.h>

#include "src/renderer/gles2/render_tree/draw_tree.h"
#include "src/renderer/gles2/render_tree/pipeline.h"
#include "src/renderer/gles2/render_tree/vertex_buffer.h"
#include "src/renderer/gles2/render_tree/uniform_values.h"

namespace entify {
namespace renderer {
namespace gles2 {
namespace render_tree {

class DrawCall : public DrawTree {
 public:
  DrawCall(const std::shared_ptr<Pipeline>& pipeline,
           const std::shared_ptr<VertexBuffer>& vertex_buffer,
           const std::shared_ptr<UniformValues>& vertex_uniform_values,
           const std::shared_ptr<UniformValues>& fragment_uniform_values);

  ~DrawCall() {}

  const std::shared_ptr<Pipeline>& pipeline() const {
    return pipeline_;
  }
  const std::shared_ptr<VertexBuffer>& vertex_buffer() const {
    return vertex_buffer_;
  }
  const std::shared_ptr<UniformValues>& vertex_uniform_values() const {
    return vertex_uniform_values_;
  }
  const std::shared_ptr<UniformValues>& fragment_uniform_values() const {
    return fragment_uniform_values_;
  }

 private:
  std::shared_ptr<Pipeline> pipeline_;
  std::shared_ptr<VertexBuffer> vertex_buffer_;
  std::shared_ptr<UniformValues> vertex_uniform_values_;
  std::shared_ptr<UniformValues> fragment_uniform_values_;
};

}  // namespace render_tree
}  // namespace gles2
}  // namespace renderer
}  // namespace entify

#endif  // _SRC_ENTIFY_RENDERER_GLES2_RENDER_TREE_DRAW_CALL_H_
