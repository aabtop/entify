#include "entify/entify.h"
#include "src/context.h"

EntifyRenderTarget EntifyCreateRenderTargetFromPlatformWindow(
    EntifyContext context, EntifyPlatformNativeWindow window,
    int32_t width, int32_t height) {
  std::unique_ptr<entify::Context::RenderTarget> render_target =
      static_cast<entify::Context*>(context)->
          CreateRenderTargetFromPlatformWindow(window, width, height);

  return render_target.release();
}

void EntifyReleaseRenderTarget(
    EntifyContext context, EntifyRenderTarget render_target) {
  delete static_cast<entify::Context::RenderTarget*>(render_target);
}

void EntifySubmit(
    EntifyContext context, EntifyReference render_tree,
    EntifyRenderTarget render_target) {
  static_cast<entify::Context*>(context)->Submit(
      render_tree, static_cast<entify::Context::RenderTarget*>(render_target));
}
