#ifndef _SRC_ENTIFY_RENDERER_GLES2_WINDOW_RENDER_TARGET_H_
#define _SRC_ENTIFY_RENDERER_GLES2_WINDOW_RENDER_TARGET_H_

#include "src/renderer/render_target.h"

#include <EGL/egl.h>

namespace entify {
namespace renderer {
namespace gles2 {

class WindowRenderTarget : public entify::renderer::RenderTarget {
 public:
  WindowRenderTarget(
      NativeWindowType egl_native_window, int width, int height,
      EGLDisplay display, const EGLConfig& window_config);
  ~WindowRenderTarget();

  EGLSurface egl_surface() const { return surface_; }

  int GetWidth() override { return width_; }
  int GetHeight() override { return height_; }

 private:
  EGLDisplay display_;
  const int width_;
  const int height_;

  EGLSurface surface_;
};

}  // namespace gles2
}  // namespace renderer
}  // namespace entify

#endif  // _SRC_ENTIFY_RENDERER_GLES2_WINDOW_RENDER_TARGET_H_
