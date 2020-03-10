// We put this at the top because sometimes the egl.h include transitively
// includes the dreaded X11 headers which define "Display" which of course
// conflict with stuff in another header, in this case the protobuf generated
// file.
#include "src/renderer/gles2/backend.h"

#include <memory>

#include <GLES2/gl2.h>
#include <EGL/eglext.h>

#if defined(USE_EGLDEVICE)
#include "src/renderer/gles2/egl_device_interface.h"
#endif

#include "src/renderer/gles2/lookup_utils.h"
#include "src/renderer/gles2/parse_protobuf.h"
#include "src/renderer/gles2/parse_flatbuffer.h"
#include "src/renderer/gles2/render.h"
#include "src/renderer/gles2/render_tree/draw_tree.h"
#include "src/renderer/gles2/utils.h"
#include "src/renderer/gles2/window_render_target.h"

namespace entify {
namespace renderer {
namespace gles2 {


Backend::Backend() {
#if defined(USE_EGLDEVICE)
  EglDeviceInterface* egl_device_interface = GetEglDeviceInterface();

  EGLDeviceEXT device;
  EGLint device_count;
  EGL_CALL(egl_device_interface->peglQueryDevicesEXT(
      1, &device, &device_count));

  display_ = EGL_CALL(egl_device_interface->peglGetPlatformDisplayEXT(
      EGL_PLATFORM_DEVICE_EXT, device, 0));
  assert(display_ != EGL_NO_DISPLAY);
#else
  display_ = EGL_CALL(eglGetDisplay(EGL_DEFAULT_DISPLAY));
  ASSERT_NO_EGL_ERROR;
#endif

  EGLint egl_major_version, egl_minor_version;
  EGL_CALL(eglInitialize(display_, &egl_major_version, &egl_minor_version));

  EGL_CALL(eglBindAPI(EGL_OPENGL_ES_API));

  EGLint surface_type = EGL_PBUFFER_BIT;
#if !defined(USE_EGLDEVICE)
  surface_type = EGL_WINDOW_BIT;
#endif

  EGLint kConfigAttributes[] = {
    EGL_SURFACE_TYPE, surface_type,
    EGL_RED_SIZE, 8,
    EGL_GREEN_SIZE, 8,
    EGL_BLUE_SIZE, 8,
    EGL_ALPHA_SIZE, 8,
    EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
    EGL_NONE
  };


  EGLint num_configs;
  EGL_CALL(eglChooseConfig(
      display_, kConfigAttributes, &config_, 1, &num_configs));
  assert(num_configs > 0);

  EGLint kContextAttributes[] = { EGL_CONTEXT_CLIENT_VERSION, 2, EGL_NONE };
  context_ = EGL_CALL(eglCreateContext(
      display_, config_, NULL, kContextAttributes));
  ASSERT_NO_EGL_ERROR;

  InitializeDummySurface();
}

void Backend::InitializeDummySurface() {
  const EGLint kDummySurfaceAttribList[] = {
      EGL_WIDTH, 1,
      EGL_HEIGHT, 1,
      EGL_NONE,
  };

  dummy_surface_ = EGL_CALL(eglCreatePbufferSurface(
      display_, config_, kDummySurfaceAttribList));
}

Backend::~Backend() {
  EGL_CALL(eglDestroySurface(display_, dummy_surface_));

  EGL_CALL(eglDestroyContext(display_, context_));
  EGL_CALL(eglTerminate(display_));
}

std::unique_ptr<RenderTarget> Backend::CreateRenderTargetFromPlatformWindow(
    PlatformWindow platform_window, int width, int height) {
  return std::unique_ptr<WindowRenderTarget>(new WindowRenderTarget(
      reinterpret_cast<NativeWindowType>(platform_window), width, height,
      display_, config_));
}

void Backend::ReleaseReferences(
    std::vector<std::shared_ptr<void>>&& references) {
  WithCurrent current_context(this);
  references.clear();
}

ParseOutput Backend::ParseProtocolBuffer(
    const ExternalReferenceLookup& reference_lookup,
    const char* data, size_t data_size) {
  WithCurrent current_context(this);

  return entify::renderer::gles2::ParseProtocolBuffer(
      reference_lookup, data, data_size);
}

ParseOutput Backend::ParseFlatBuffer(
    const ExternalReferenceLookup& reference_lookup,
    const char* data, size_t data_size) {
  WithCurrent current_context(this);

  return entify::renderer::gles2::ParseFlatBuffer(
      reference_lookup, data, data_size);
}

void Backend::Submit(
    ExternalReference* render_tree, RenderTarget* render_target) {
  assert(render_tree);
  auto draw_tree =
      ExternalReferenceToRenderTree<render_tree::DrawTree>(
          *render_tree);
  assert(draw_tree);

  WindowRenderTarget* egl_surface_render_target =
      static_cast<WindowRenderTarget*>(render_target);
  EGLSurface egl_surface = egl_surface_render_target->egl_surface();

  int width = egl_surface_render_target->GetWidth();
  int height = egl_surface_render_target->GetHeight();

  WithCurrent current_context(this, egl_surface);

  Render(width, height, draw_tree);

  EGL_CALL(eglSwapBuffers(display_, egl_surface));
}

Backend::WithCurrent::WithCurrent(Backend* backend, EGLSurface surface)
    : backend_(backend), surface_(surface) {
  assert(backend_->context_ != EGL_NO_CONTEXT);
  assert(!backend_->context_is_current_);
  EGL_CALL(eglMakeCurrent(
      backend_->display_, surface, surface, backend_->context_));
  backend_->context_is_current_ = true;
}

Backend::WithCurrent::WithCurrent(Backend* backend)
    : WithCurrent(backend, backend->dummy_surface_) {}

Backend::WithCurrent::~WithCurrent() {
  assert(backend_->context_is_current_);
  backend_->context_is_current_ = false;
  EGL_CALL(eglMakeCurrent(
      backend_->display_, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT));
}

}  // namespace gles2

std::unique_ptr<entify::renderer::Backend> MakeDefaultRenderer() {
  return std::unique_ptr<gles2::Backend>(new gles2::Backend());
}

}  // namespace renderer
}  // namespace entify
