#ifndef _SRC_ENTIFY_RENDERER_GLES2_RENDER_TREE_PROGRAM_H_
#define _SRC_ENTIFY_RENDERER_GLES2_RENDER_TREE_PROGRAM_H_

#include <memory>

#include <GLES2/gl2.h>

#include "src/renderer/gles2/render_tree/fragment_shader.h"
#include "src/renderer/gles2/render_tree/vertex_shader.h"

namespace entify {
namespace renderer {
namespace gles2 {
namespace render_tree {

class Program {
 public:
  Program(const std::shared_ptr<VertexShader>& vertex_shader,
          const std::shared_ptr<FragmentShader>& fragment_shader);
  ~Program() {
    glDeleteProgram(handle_);
  }

  const std::shared_ptr<VertexShader>& vertex_shader() const {
    return vertex_shader_;
  }
  const std::shared_ptr<FragmentShader>& fragment_shader() const {
    return fragment_shader_;
  }

  GLuint handle() const { return handle_; }

  const std::vector<GLint>& vertex_attribute_indices() const {
    return vertex_attribute_indices_;
  }

  const std::vector<GLint>& vertex_uniform_locations() const {
    return vertex_uniform_locations_;
  }

  const std::vector<GLint>& fragment_uniform_locations() const {
    return fragment_uniform_locations_;
  }

  const std::string error() const { return error_; }

 private:
  std::shared_ptr<VertexShader> vertex_shader_;
  std::shared_ptr<FragmentShader> fragment_shader_;

  std::vector<GLint> vertex_attribute_indices_;
  std::vector<GLint> vertex_uniform_locations_;
  std::vector<GLint> fragment_uniform_locations_;

  GLuint handle_;

  std::string error_;
};

}  // namespace render_tree
}  // namespace gles2
}  // namespace renderer
}  // namespace entify

#endif  // _SRC_ENTIFY_RENDERER_GLES2_RENDER_TREE_PROGRAM_H_
