#ifndef _SRC_ENTIFY_RENDERER_RENDER_TARGET_H_
#define _SRC_ENTIFY_RENDERER_RENDER_TARGET_H_

namespace entify {
namespace renderer {

class RenderTarget {
 public:
  virtual ~RenderTarget() {}

  virtual int GetWidth() = 0;
  virtual int GetHeight() = 0;
};

}  // namespace renderer
}  // namespace entify

#endif  // _SRC_ENTIFY_RENDERER_RENDER_TARGET_H_
