#ifndef _SRC_ENTIFY_RENDERER_GLES2_RENDER_TREE_SAMPLER_H_
#define _SRC_ENTIFY_RENDERER_GLES2_RENDER_TREE_SAMPLER_H_

#include <memory>

#include "src/renderer/gles2/render_tree/texture.h"
#include "src/renderer/gles2/render_tree/types.h"

namespace entify {
namespace renderer {
namespace gles2 {
namespace render_tree {

class Sampler {
 public:
  Sampler(const std::shared_ptr<Texture>& texture,
          GLenum wrap_s, GLenum wrap_t,
          GLenum min_filter, GLenum mag_filter)
      : texture_(texture), wrap_s_(wrap_s), wrap_t_(wrap_t),
        min_filter_(min_filter), mag_filter_(mag_filter) {}
  ~Sampler() {}

  const std::shared_ptr<Texture>& texture() const { return texture_; }
  GLenum wrap_s() const { return wrap_s_; }
  GLenum wrap_t() const { return wrap_t_; }
  GLenum min_filter() const { return min_filter_; }
  GLenum mag_filter() const { return mag_filter_; }

 private:
  std::shared_ptr<Texture> texture_;
  GLenum wrap_s_;
  GLenum wrap_t_;
  GLenum min_filter_;
  GLenum mag_filter_;
};

}  // namespace render_tree
}  // namespace gles2
}  // namespace renderer
}  // namespace entify

#endif  // _SRC_ENTIFY_RENDERER_GLES2_RENDER_TREE_TEXTURE_H_
