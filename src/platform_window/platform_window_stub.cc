#include "platform_window/platform_window.h"

PlatformWindow PlatformWindowMakeDefaultWindow(
    const char* title, PlatformWindowEventCallback event_callback,
    void* context) {
  // Return any non-zero value to indicate that it is not null/invalid.
  return reinterpret_cast<PlatformWindow>(sizeof(PlatformWindow));
}

void PlatformWindowDestroyWindow(PlatformWindow platform_window) {}

NativeWindow PlatformWindowGetNativeWindow(PlatformWindow platform_window) {
  // Return any non-zero value to indicate that it is not null/invalid.
  return reinterpret_cast<NativeWindow>(sizeof(NativeWindow));
}

int32_t PlatformWindowGetWidth(PlatformWindow window) {
  return 1920;
}

int32_t PlatformWindowGetHeight(PlatformWindow window) {
  return 1080;
}
