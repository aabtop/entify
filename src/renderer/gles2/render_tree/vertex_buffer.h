#ifndef _SRC_ENTIFY_RENDERER_GLES2_RENDER_TREE_VERTEX_BUFFER_H_
#define _SRC_ENTIFY_RENDERER_GLES2_RENDER_TREE_VERTEX_BUFFER_H_

#include <GLES2/gl2.h>

#include "src/renderer/gles2/render_tree/types.h"

namespace entify {
namespace renderer {
namespace gles2 {
namespace render_tree {

class VertexBuffer {
 public:
  VertexBuffer(const char* data, int32_t num_bytes, int32_t stride_in_bytes,
               std::vector<int32_t>&& data_offsets, TypeTuple&& types);
  ~VertexBuffer() {
    glDeleteBuffers(1, &handle_);
  }

  GLuint handle() const { return handle_; }
  int32_t num_vertices() const { return num_vertices_; }
  int32_t stride_in_bytes() const { return stride_in_bytes_; }
  const TypeTuple& types() const { return types_; }
  const std::vector<int32_t>& data_offsets() const { return data_offsets_; }

 private:
  GLuint handle_;
  int32_t num_vertices_;
  int32_t stride_in_bytes_;
  TypeTuple types_;
  std::vector<int32_t> data_offsets_;
};

}  // namespace render_tree
}  // namespace gles2
}  // namespace renderer
}  // namespace entify

#endif  // _SRC_ENTIFY_RENDERER_GLES2_RENDER_TREE_VERTEX_BUFFER_H_
