#include "src/renderer/gles2/render_tree/program.h"

#include "src/renderer/gles2/utils.h"

#include <iostream>

namespace entify {
namespace renderer {
namespace gles2 {
namespace render_tree {

Program::Program(
    const std::shared_ptr<VertexShader>& vertex_shader,
    const std::shared_ptr<FragmentShader>& fragment_shader)
    : vertex_shader_(vertex_shader), fragment_shader_(fragment_shader) {
  assert(vertex_shader_->output_types() == fragment_shader_->input_types());

  handle_ = glCreateProgram();
  GL_CALL(glAttachShader(handle_, vertex_shader_->handle()));
  GL_CALL(glAttachShader(handle_, fragment_shader_->handle()));

  GL_CALL(glLinkProgram(handle_));

  GLint link_result;
  int info_log_length;
  GL_CALL(glGetProgramiv(handle_, GL_LINK_STATUS, &link_result));
  if (!link_result) {
    GL_CALL(glGetProgramiv(handle_, GL_INFO_LOG_LENGTH, &info_log_length));
    if (info_log_length > 0) {
      std::vector<char> link_error_message(info_log_length + 1);
      GL_CALL(glGetProgramInfoLog(
          handle_, info_log_length, NULL, link_error_message.data()));
      error_ =
          "Shader link error: " + std::string(link_error_message.data());
      return;
    }
  }

  GL_CALL(glDetachShader(handle_, vertex_shader_->handle()));
  GL_CALL(glDetachShader(handle_, fragment_shader_->handle()));

  // Resolve vertex attribute indices.
  vertex_attribute_indices_.reserve(
      vertex_shader_->vertex_attribute_names().size());
  for (const auto& name : vertex_shader_->vertex_attribute_names()) {
    GLint index = glGetAttribLocation(handle_, name.c_str());
    if (index == -1) {
      error_ = "Could not find attribute '" + name + "'.";
      return;
    }
    vertex_attribute_indices_.push_back(index);
  }

  // Now resolve uniform locations.
  vertex_uniform_locations_.reserve(vertex_shader_->uniform_names().size());
  for (const auto& name : vertex_shader_->uniform_names()) {
    GLint location = glGetUniformLocation(handle_, name.c_str());
    if (location == -1) {
      error_ =  "Could not find vertex uniform '" + name + "'.";
      return;
    }
    vertex_uniform_locations_.push_back(location);
  }

  fragment_uniform_locations_.reserve(fragment_shader_->uniform_names().size());
  for (const auto& name : fragment_shader_->uniform_names()) {
    GLint location = glGetUniformLocation(handle_, name.c_str());
    if (location == -1) {
      error_ = "Could not find fragment uniform '" + name + "'.";
      return;
    }
    fragment_uniform_locations_.push_back(location);
  }
}

}  // namespace render_tree
}  // namespace gles2
}  // namespace renderer
}  // namespace entify
