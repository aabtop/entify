#ifndef _SRC_PLATFORM_WINDOW_PLATFORM_WINDOW_H_
#define _SRC_PLATFORM_WINDOW_PLATFORM_WINDOW_H_

#include <cstdint>

#if defined(EXPORT_PLATFORM_WINDOW)
#define SHARED_LIBRARY_EXPORT
#endif
#include "stdext/shared_library.h"

#ifdef __cplusplus  
extern "C" { 
#endif

typedef void* PlatformWindow;
typedef void* NativeWindow;

enum PlatformWindowEvent {
  kPlatformWindowEventQuitRequest,
};

typedef void (*PlatformWindowEventCallback)(
              void*,  // Context data passed in by the caller.
              PlatformWindowEvent,  // The type of newly fired event.
              void*  // Event data.
              );

// The |event_callback| may be called from an arbitrary thread.
PUBLIC_API PlatformWindow PlatformWindowMakeDefaultWindow(
    const char* title, PlatformWindowEventCallback event_callback,
    void* context);
PUBLIC_API void PlatformWindowDestroyWindow(
    PlatformWindow platform_window);

PUBLIC_API NativeWindow PlatformWindowGetNativeWindow(
    PlatformWindow platform_window);

PUBLIC_API int32_t PlatformWindowGetWidth(PlatformWindow window);
PUBLIC_API int32_t PlatformWindowGetHeight(PlatformWindow window);

#ifdef __cplusplus  
} 
#endif

#endif  // #ifndef _SRC_PLATFORM_WINDOW_PLATFORM_WINDOW_H_
