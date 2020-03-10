#include "src/renderer/gles2/utils.h"

#include <cassert>
#include <iomanip>
#include <sstream>
#include <vector>

#include <GLES2/gl2.h>

namespace entify {
namespace renderer {
namespace gles2 {

std::string AddLineNumbers(const std::string& source) {
  std::ostringstream oss;

  size_t current_position = 0;
  int line_number = 0;
  while (current_position < source.length()) {
    size_t next_newline_pos = source.find("\n", current_position);
    if (next_newline_pos == std::string::npos) {
      next_newline_pos = source.length();
    }
    ++line_number;

    oss << std::setw(5) << line_number << ": ";
    oss << source.substr(current_position, next_newline_pos - current_position)
        << "\n";

    current_position = next_newline_pos + 1;
  }
  return oss.str();
}

std::string CheckForShaderCompileErrors(
    GLuint shader_handle, const std::string& source) {
  GLint compile_result;
  int info_log_length;
  GL_CALL(glGetShaderiv(shader_handle, GL_COMPILE_STATUS, &compile_result));
  if (!compile_result) {
    GL_CALL(glGetShaderiv(shader_handle, GL_INFO_LOG_LENGTH, &info_log_length));
    if (info_log_length > 0) {
      std::vector<char> compile_error_message(info_log_length + 1);
      GL_CALL(glGetShaderInfoLog(
          shader_handle, info_log_length, NULL, compile_error_message.data()));
      std::ostringstream oss;
      oss << "Error compiling shader: " << std::endl;
      oss << AddLineNumbers(source) << std::endl << std::endl;
      oss << "Shader compiler error: " << compile_error_message.data()
                << std::endl;
      return oss.str();
    }
  }

  return std::string();
}

}  // namespace gles2
}  // namespace renderer
}  // namespace entify
