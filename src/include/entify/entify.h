#ifndef _SRC_ENTIFY_ENTIFY_H_
#define _SRC_ENTIFY_ENTIFY_H_

#include "registry.h"

#if defined(EXPORT_ENTIFY)
#define SHARED_LIBRARY_EXPORT
#endif
#include "stdext/shared_library.h"

#ifdef __cplusplus  
extern "C" { 
#endif

typedef void* EntifyRenderTarget;

// Platform-defined type can be used to associate a render target with.
typedef void* EntifyPlatformNativeWindow;

const EntifyRenderTarget kEntifyInvalidRenderTarget = 0;

PUBLIC_API EntifyRenderTarget
    EntifyCreateRenderTargetFromPlatformWindow(
        EntifyContext context, EntifyPlatformNativeWindow window,
        int32_t width, int32_t height);
PUBLIC_API void EntifyReleaseRenderTarget(
    EntifyContext context, EntifyRenderTarget render_target);

PUBLIC_API void EntifySubmit(
    EntifyContext context, EntifyReference render_tree,
    EntifyRenderTarget render_target);

#ifdef __cplusplus  
} 
#endif

#endif  // _SRC_ENTIFY_ENTIFY_H_
