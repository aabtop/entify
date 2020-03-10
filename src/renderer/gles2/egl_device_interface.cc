#include "src/renderer/gles2/egl_device_interface.h"

#include <cassert>
#include <EGL/eglext.h>

#define EGL_GET_PROC_ADDR(name, type)                     \
  do {                                                    \
    interface.p##name = (type)eglGetProcAddress(#name);   \
    if (!interface.p##name) {                                       \
      assert(false);                                      \
    }                                                     \
  } while (0)

namespace entify {
namespace renderer {
namespace gles2 {

namespace {

EglDeviceInterface InitializeEglDeviceInterface() {
  EglDeviceInterface interface;

  EGL_GET_PROC_ADDR(eglQueryDevicesEXT, PFNEGLQUERYDEVICESEXTPROC);
  EGL_GET_PROC_ADDR(eglGetPlatformDisplayEXT, PFNEGLGETPLATFORMDISPLAYEXTPROC);

  EGL_GET_PROC_ADDR(eglGetOutputLayersEXT, PFNEGLGETOUTPUTLAYERSEXTPROC);
  EGL_GET_PROC_ADDR(eglCreateStreamKHR, PFNEGLCREATESTREAMKHRPROC);
  EGL_GET_PROC_ADDR(eglDestroyStreamKHR, PFNEGLDESTROYSTREAMKHRPROC);
  EGL_GET_PROC_ADDR(eglStreamConsumerOutputEXT,
                    PFNEGLSTREAMCONSUMEROUTPUTEXTPROC);
  EGL_GET_PROC_ADDR(eglCreateStreamProducerSurfaceKHR,
                    PFNEGLCREATESTREAMPRODUCERSURFACEKHRPROC);

  return interface;
}

}  // namespace

EglDeviceInterface* GetEglDeviceInterface() {
  static EglDeviceInterface s_interface = InitializeEglDeviceInterface();
  return &s_interface;
}

}  // namespace gles2
}  // namespace renderer
}  // namespace entify
