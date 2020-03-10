#ifndef _SRC_ENTIFY_RENDERER_GLES2_RENDER_TREE_DRAW_SEQUENCE_H_
#define _SRC_ENTIFY_RENDERER_GLES2_RENDER_TREE_DRAW_SEQUENCE_H_

#include <memory>

#include <GLES2/gl2.h>

#include "src/renderer/gles2/render_tree/draw_tree.h"

namespace entify {
namespace renderer {
namespace gles2 {
namespace render_tree {

class DrawSequence : public DrawTree {
 public:
  DrawSequence(std::vector<std::shared_ptr<DrawTree>>&& sequence)
      : DrawTree(kTypeDrawSequence), sequence_(std::move(sequence)) {}
  ~DrawSequence() {}

  const std::vector<std::shared_ptr<DrawTree>>& sequence() const {
    return sequence_;
  }

 private:
  std::vector<std::shared_ptr<DrawTree>> sequence_;
};

}  // namespace render_tree
}  // namespace gles2
}  // namespace renderer
}  // namespace entify

#endif  // _SRC_ENTIFY_RENDERER_GLES2_RENDER_TREE_DRAW_SEQUENCE_H_
