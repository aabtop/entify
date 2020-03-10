#include "src/renderer/gles2/render_tree/vertex_buffer.h"

#include <GLES2/gl2.h>

#include "src/renderer/gles2/utils.h"

namespace entify {
namespace renderer {
namespace gles2 {
namespace render_tree {

VertexBuffer::VertexBuffer(const char* data, int32_t num_bytes,
                           int32_t stride_in_bytes,
                           std::vector<int32_t>&& data_offsets,
                           TypeTuple&& types)
    : stride_in_bytes_(stride_in_bytes),
      num_vertices_(num_bytes / stride_in_bytes),
      types_(std::move(types)),
      data_offsets_(std::move(data_offsets)) {
  int components_size_sum = 0;
  for (const auto& type : types_) {
    components_size_sum += TypeToSize(type);
  }
  assert(components_size_sum == stride_in_bytes_);

  GL_CALL(glGenBuffers(1, &handle_));
  GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, handle_));
  GL_CALL(glBufferData(GL_ARRAY_BUFFER, num_bytes, data, GL_STATIC_DRAW));
}

}  // namespace render_tree
}  // namespace gles2
}  // namespace renderer
}  // namespace entify
