#ifndef _SRC_ENTIFY_RENDERER_GLES2_RENDER_TREE_VERTEX_SHADER_H_
#define _SRC_ENTIFY_RENDERER_GLES2_RENDER_TREE_VERTEX_SHADER_H_

#include <string>
#include <utility>
#include <vector>

#include <GLES2/gl2.h>

#include "src/renderer/gles2/render_tree/types.h"

namespace entify {
namespace renderer {
namespace gles2 {
namespace render_tree {

class VertexShader {
 public:
  VertexShader(
      const std::string& source,
      std::pair<std::vector<std::string>, TypeTuple>&& input_types,
      TypeTuple&& output_types,
      std::pair<std::vector<std::string>, TypeTuple>&& uniform_types);
  ~VertexShader() {
    glDeleteShader(handle_);
  }

  GLuint handle() const { return handle_; }

  const TypeTuple& input_types() const { return input_types_; }
  const std::vector<std::string>& vertex_attribute_names() const {
    return vertex_attribute_names_;
  }
  const TypeTuple& output_types() const { return output_types_; }
  const TypeTuple& uniform_types() const { return uniform_types_; }
  const std::vector<std::string>& uniform_names() const {
    return uniform_names_;
  }

  const std::string& error() const { return error_; }

 private:
  GLuint handle_;
  TypeTuple input_types_;
  std::vector<std::string> vertex_attribute_names_;
  TypeTuple output_types_;
  TypeTuple uniform_types_;
  std::vector<std::string> uniform_names_;
  std::string error_;
};

}  // namespace render_tree
}  // namespace gles2
}  // namespace renderer
}  // namespace entify

#endif  // _SRC_ENTIFY_RENDERER_GLES2_RENDER_TREE_VERTEX_SHADER_H_
