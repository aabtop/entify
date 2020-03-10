#include "src/renderer/gles2/window_render_target.h"

#include <EGL/egl.h>
#include <EGL/eglext.h>

#if defined(USE_EGLDEVICE)
#include "src/renderer/gles2/egl_device_interface.h"
#endif

#include "src/renderer/gles2/utils.h"

namespace entify {
namespace renderer {
namespace gles2 {

WindowRenderTarget::WindowRenderTarget(
    NativeWindowType egl_native_window, int width, int height,
    EGLDisplay display, const EGLConfig& config)
    : display_(display), width_(width), height_(height) {
#if defined(USE_EGLDEVICE)
  EglDeviceInterface* egl_device_interface = GetEglDeviceInterface();

  EGLAttrib layer_attr[] = { EGL_NONE };
  EGLOutputLayerEXT egl_layer;
  EGLint num_layers;
  EGL_CALL(egl_device_interface->peglGetOutputLayersEXT(
      display_, layer_attr, &egl_layer, 1, &num_layers));

  EGLStreamKHR egl_stream;
  // Set EGL_STREAM_FIFO_LENGTH_KHR to 1 so that we enter FIFO mode and so
  // that calls to eglSwapBuffers() will block on the GPU.
  EGLint stream_attr[] = { EGL_STREAM_FIFO_LENGTH_KHR, 1, EGL_NONE };
  egl_stream = egl_device_interface->peglCreateStreamKHR(display_, stream_attr);
  ASSERT_NO_EGL_ERROR;
  assert(egl_stream != EGL_NO_STREAM_KHR);

  EGL_CALL(egl_device_interface->peglStreamConsumerOutputEXT(
      display_, egl_stream, egl_layer));

  // Create a surface to feed the stream
  EGLint surface_attr[] = { EGL_WIDTH, width, EGL_HEIGHT, height, EGL_NONE };
  surface_ = egl_device_interface->peglCreateStreamProducerSurfaceKHR(
      display_, config, egl_stream, surface_attr);
  ASSERT_NO_EGL_ERROR;
  assert(surface_ != EGL_NO_SURFACE);
#else
  EGLint kSurfaceAttributes[] = { EGL_NONE };
  surface_ = eglCreateWindowSurface(
      display_, config, egl_native_window, kSurfaceAttributes);
  ASSERT_NO_EGL_ERROR;
#endif
}

WindowRenderTarget::~WindowRenderTarget() {
  eglDestroySurface(display_, surface_);
  // TODO: Destroy stream also if USE_EGLDEVICE.
}

}  // namespace gles2
}  // namespace renderer
}  // namespace entify
