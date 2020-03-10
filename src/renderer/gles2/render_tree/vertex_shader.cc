#include "src/renderer/gles2/render_tree/vertex_shader.h"
#include "src/renderer/gles2/utils.h"

namespace entify {
namespace renderer {
namespace gles2 {
namespace render_tree {

VertexShader::VertexShader(
      const std::string& source,
      std::pair<std::vector<std::string>, TypeTuple>&& input_types,
      TypeTuple&& output_types,
      std::pair<std::vector<std::string>, TypeTuple>&& uniform_types)
      : input_types_(std::move(input_types.second)),
        vertex_attribute_names_(std::move(input_types.first)),
        output_types_(std::move(output_types)),
        uniform_types_(std::move(uniform_types.second)),
        uniform_names_(std::move(uniform_types.first)) {
  assert(uniform_types_.size() == uniform_names_.size());
  handle_ = glCreateShader(GL_VERTEX_SHADER);
  const char* source_c_str = source.c_str();
  GL_CALL(glShaderSource(handle_, 1, &source_c_str, NULL));
  GL_CALL(glCompileShader(handle_));
  error_ = CheckForShaderCompileErrors(handle_, source_c_str);
}

}  // namespace render_tree
}  // namespace gles2
}  // namespace renderer
}  // namespace entify
