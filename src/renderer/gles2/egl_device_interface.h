#ifndef _SRC_ENTIFY_RENDERER_GLES2_EGL_DEVICE_INTERFACE_H_
#define _SRC_ENTIFY_RENDERER_GLES2_EGL_DEVICE_INTERFACE_H_

#include <EGL/egl.h>
#include <EGL/eglext.h>

namespace entify {
namespace renderer {
namespace gles2 {

struct EglDeviceInterface {
  PFNEGLQUERYDEVICESEXTPROC peglQueryDevicesEXT;
  PFNEGLGETPLATFORMDISPLAYEXTPROC peglGetPlatformDisplayEXT;
  PFNEGLGETOUTPUTLAYERSEXTPROC peglGetOutputLayersEXT;
  PFNEGLCREATESTREAMKHRPROC peglCreateStreamKHR;
  PFNEGLDESTROYSTREAMKHRPROC peglDestroyStreamKHR;
  PFNEGLSTREAMCONSUMEROUTPUTEXTPROC peglStreamConsumerOutputEXT;
  PFNEGLCREATESTREAMPRODUCERSURFACEKHRPROC peglCreateStreamProducerSurfaceKHR;
};

EglDeviceInterface* GetEglDeviceInterface();

}  // namespace gles2
}  // namespace renderer
}  // namespace entify

#endif  // _SRC_ENTIFY_RENDERER_GLES2_EGL_DEVICE_INTERFACE_H_
