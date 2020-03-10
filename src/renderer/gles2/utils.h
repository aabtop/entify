#ifndef _SRC_ENTIFY_RENDERER_GLES2_UTILS_H_
#define _SRC_ENTIFY_RENDERER_GLES2_UTILS_H_

#include <cassert>
#include <iostream>
#include <string>

#include <EGL/egl.h>
#include <GLES2/gl2.h>

#define ASSERT_NO_EGL_ERROR \
  do { \
    EGLint error = eglGetError(); \
    if (error != EGL_SUCCESS) { \
      std::cerr << "EGL Error: " << error << std::endl; \
      assert(false); \
    } \
  } while (false)

#define EGL_CALL(x) \
  x; \
  ASSERT_NO_EGL_ERROR

#define ASSERT_NO_GL_ERROR \
  do { \
    EGLint error = glGetError(); \
    if (error != GL_NO_ERROR) { \
      std::cerr << "GL Error: " << error << std::endl; \
      assert(false); \
    } \
  } while (false)

#define GL_CALL(x) \
  x; \
  ASSERT_NO_GL_ERROR

namespace entify {
namespace renderer {
namespace gles2 {

std::string CheckForShaderCompileErrors(
    GLuint shader_handle, const std::string& source);

}  // namespace gles2
}  // namespace renderer
}  // namespace entify

#endif  // _SRC_ENTIFY_RENDERER_GLES2_UTILS_H_
