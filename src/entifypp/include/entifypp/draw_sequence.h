#ifndef _SRC_ENTIFY_ENTIFYPP_RENDER_TREE_DRAW_SEQUENCE_H_
#define _SRC_ENTIFY_ENTIFYPP_RENDER_TREE_DRAW_SEQUENCE_H_

#include <memory>

#include "entifypp/draw_tree.h"
#include "entifypp/hash.h"

namespace entifypp {

class DrawSequence : public DrawTree {
 public:
  DrawSequence(std::vector<std::shared_ptr<DrawTree>>&& sequence)
      : sequence_(sequence) {
    Hasher hasher;
    for (const auto& draw_tree : sequence_) {
      hasher.Add(draw_tree->hash());  
    }
    hash_ = hasher.Get();
  }

  const std::vector<std::shared_ptr<DrawTree>>& sequence() const {
    return sequence_;
  }

  void Accept(DrawTreeVisitor* visitor) const override {
    visitor->Visit(this);
  }

 private:
  std::vector<std::shared_ptr<DrawTree>> sequence_;
};

}  // namespace entifypp

#endif  // _SRC_ENTIFY_ENTIFYPP_RENDER_TREE_DRAW_SEQUENCE_H_
