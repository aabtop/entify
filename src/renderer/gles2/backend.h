#ifndef _SRC_ENTIFY_RENDERER_GLES2_BACKEND_H_
#define _SRC_ENTIFY_RENDERER_GLES2_BACKEND_H_

#include <memory>
#include <vector>

#include "src/renderer/backend.h"
#include "src/external_reference.h"

#include <EGL/egl.h>

namespace entify {
namespace renderer {
namespace gles2 {

class Backend : public entify::renderer::Backend {
 public:
  Backend();
  ~Backend() override;

  std::unique_ptr<RenderTarget> CreateRenderTargetFromPlatformWindow(
      PlatformWindow platform_window, int width, int height) override;

  void ReleaseReferences(
      std::vector<std::shared_ptr<void>>&& references) override;

  ParseOutput ParseProtocolBuffer(
      const ExternalReferenceLookup& reference_lookup,
      const char* data, size_t data_size) override;

  ParseOutput ParseFlatBuffer(
      const ExternalReferenceLookup& reference_lookup,
      const char* data, size_t data_size) override;

  void Submit(
      ExternalReference* render_tree, RenderTarget* render_target) override;

  class WithCurrent {
   public:
    WithCurrent(Backend* backend, EGLSurface surface);
    WithCurrent(Backend* backend);
    ~WithCurrent();

   private:
    Backend* backend_;
    EGLSurface surface_;
  };

 private:
  // Create a dummy EGLSurface object to be assigned as the target surface
  // when we need to make OpenGL calls that do not depend on a surface (e.g.
  // creating a texture).
  void InitializeDummySurface();

  bool context_is_current_ = false;
  EGLContext context_;
  EGLDisplay display_;
  EGLConfig config_;
  EGLSurface dummy_surface_;
};

}  // namespace gles2
}  // namespace renderer
}  // namespace entify

#endif  // _SRC_ENTIFY_RENDERER_GLES2_BACKEND_H_
