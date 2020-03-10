#include "platform_window/platform_window.h"

#include <cassert>
#include <chrono>
#include <thread>

#include <EGL/egl.h>

#include <bcm_host.h>

// Thanks to iffy@google.com and following code most of this implementation:
//   https://cobalt.googlesource.com/cobalt/+/master/src/starboard/raspi/shared/

namespace {
class DispmanxAutoUpdate {
 public:
  DispmanxAutoUpdate() {
    handle_ = vc_dispmanx_update_start(0 /*screen*/);
    assert(handle_ != DISPMANX_NO_HANDLE);
  }
  ~DispmanxAutoUpdate() {
    if (handle_ != DISPMANX_NO_HANDLE) {
      Update();
    }
  }
  DISPMANX_UPDATE_HANDLE_T handle() const { return handle_; }
  void Update() {
    assert(handle_ != DISPMANX_NO_HANDLE);
    int32_t result = vc_dispmanx_update_submit_sync(handle_);
    assert(result == 0);
    handle_ = DISPMANX_NO_HANDLE;
  }
 private:
  DISPMANX_UPDATE_HANDLE_T handle_;
};

class ScopedDispmanxDisplay {
 public:
  ScopedDispmanxDisplay() {
    bcm_host_init();
    display_ = vc_dispmanx_display_open(0);
    assert(display_ != DISPMANX_NO_HANDLE);
  }

  ~ScopedDispmanxDisplay() {
    int32_t result = vc_dispmanx_display_close(display_);
    assert(result == 0);
    bcm_host_deinit();
  }

  DISPMANX_DISPLAY_HANDLE_T display() const { return display_; }

 private:
  DISPMANX_DISPLAY_HANDLE_T display_;
};

ScopedDispmanxDisplay* g_dispmanx_display = nullptr;

}  // namespace

PlatformWindow PlatformWindowMakeDefaultWindow(
    const char* title, PlatformWindowEventCallback event_callback,
    void* context) {
  assert(!g_dispmanx_display);
  g_dispmanx_display = new ScopedDispmanxDisplay();

  uint32_t width, height;
  int32_t result = graphics_get_display_size(0, &width, &height);
  assert(result >= 0);


  VC_RECT_T destination_rect;
  vc_dispmanx_rect_set(&destination_rect, 0, 0, width, height);
  // The "<< 16"s are part of the examples, but unexplained. It appears to work.
  VC_RECT_T source_rect;
  vc_dispmanx_rect_set(&source_rect, 0, 0, width << 16, height << 16);

  // Creating a window (called an "element" here, created by adding it to the
  // display) must happen within an "update", which seems to represent a sort of
  // window manager transaction.
  DispmanxAutoUpdate update;
  const int kLayer = 0;
  DISPMANX_ELEMENT_HANDLE_T dispmanx_element = vc_dispmanx_element_add(
      update.handle(), g_dispmanx_display->display(), kLayer, &destination_rect,
      DISPMANX_NO_HANDLE, &source_rect, DISPMANX_PROTECTION_NONE,
      NULL /*alpha*/, NULL /*clamp*/, DISPMANX_NO_ROTATE);
  assert(dispmanx_element != DISPMANX_NO_HANDLE);

  EGL_DISPMANX_WINDOW_T* dispmanx_window = new EGL_DISPMANX_WINDOW_T();
  dispmanx_window->element = dispmanx_element;
  dispmanx_window->width = width;
  dispmanx_window->height = height;

  return dispmanx_window;
}

void PlatformWindowDestroyWindow(PlatformWindow platform_window) {
  while (true) {
    std::this_thread::sleep_for(std::chrono::seconds(1));
  }

  EGL_DISPMANX_WINDOW_T* dispmanx_window = static_cast<EGL_DISPMANX_WINDOW_T*>(
      platform_window);

  DispmanxAutoUpdate update;
  int32_t result = vc_dispmanx_element_remove(
      update.handle(), dispmanx_window->element);
  assert(result == 0);

  delete static_cast<EGL_DISPMANX_WINDOW_T*>(dispmanx_window);

  delete g_dispmanx_display;
}

NativeWindow PlatformWindowGetNativeWindow(PlatformWindow platform_window) {
  return platform_window;
}

int32_t PlatformWindowGetWidth(PlatformWindow window) {
  return static_cast<EGL_DISPMANX_WINDOW_T*>(window)->width;
}

int32_t PlatformWindowGetHeight(PlatformWindow window) {
  return static_cast<EGL_DISPMANX_WINDOW_T*>(window)->height;
}
