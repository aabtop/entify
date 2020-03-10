#ifndef _SRC_ENTIFY_ENTIFYPP_ENTIFYPP_H_
#define _SRC_ENTIFY_ENTIFYPP_ENTIFYPP_H_

#include <cassert>
#include <functional>
#include <memory>

#include "entifypp/draw_sequence.h"
#include "entifypp/draw_set.h"
#include "entifypp/draw_call.h"
#include "entifypp/glsl_fragment_shader.h"
#include "entifypp/glsl_vertex_shader.h"
#include "entifypp/pipeline.h"
#include "entifypp/sampler.h"
#include "entifypp/texture.h"
#include "entifypp/uniform_values.h"
#include "entifypp/vertex_buffer.h"

#include "draw_tree.h"
#include "platform_window/platform_window.h"
#include "entify/entify.h"

namespace entifypp {

class DefaultPlatformWindow {
 public:
  using EventHandler = std::function<void(PlatformWindowEvent, void*)>;

  DefaultPlatformWindow(
      const std::string& title,
      EventHandler&& event_handler)
      : event_handler_(std::move(event_handler)) {
    window_ = PlatformWindowMakeDefaultWindow(
        title.c_str(), &RunEventHandler, this);
  }

  ~DefaultPlatformWindow() {
    PlatformWindowDestroyWindow(window_);
  }

  PlatformWindow window() const { return window_; }

 private:
  static void RunEventHandler(
      void* context, PlatformWindowEvent event, void* data) {
    static_cast<DefaultPlatformWindow*>(context)->event_handler_(event, data);
  }

  PlatformWindow window_;
  EventHandler event_handler_;
};

class RenderTarget;

class Context {
 public:
  Context() {
    context_ = EntifyCreateContext();
    assert(context_ != kEntifyInvalidContext);
  }

  ~Context() {
    EntifyDestroyContext(context_);
  }

  std::shared_ptr<ReferenceTexture> SubmitTexture(
      const std::shared_ptr<Texture>& texture);

  void Submit(
      RenderTarget* render_target, const std::shared_ptr<DrawTree>& draw_tree);

  EntifyContext context() const { return context_; }

 public:
  EntifyContext context_;
};

class RenderTarget {
 public:
  RenderTarget(const Context& context, PlatformWindow window)
      : context_(context) {
    render_target_ = EntifyCreateRenderTargetFromPlatformWindow(
        context_.context(), PlatformWindowGetNativeWindow(window),
        PlatformWindowGetWidth(window), PlatformWindowGetHeight(window));
    assert(render_target_ != kEntifyInvalidRenderTarget);
  }

  RenderTarget(const Context& context, const DefaultPlatformWindow& window)
      : RenderTarget(context, window.window()) {}

  ~RenderTarget() {
    EntifyReleaseRenderTarget(context_.context(), render_target_);
  }

  EntifyRenderTarget render_target() const { return render_target_; }

 private:
  const Context& context_;
  EntifyRenderTarget render_target_;
};

}  // namespace entifypp

#endif  // _SRC_ENTIFY_ENTIFYPP_ENTIFYPP_H_
