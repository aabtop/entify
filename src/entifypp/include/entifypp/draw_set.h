#ifndef _SRC_ENTIFY_ENTIFYPP_RENDER_TREE_DRAW_SET_H_
#define _SRC_ENTIFY_ENTIFYPP_RENDER_TREE_DRAW_SET_H_

#include <memory>

#include "entifypp/draw_tree.h"
#include "entifypp/hash.h"

namespace entifypp {

class DrawSet : public DrawTree {
 public:
  DrawSet(std::vector<std::shared_ptr<DrawTree>>&& set)
      : set_(set) {
    Hasher hasher;
    for (const auto& draw_tree : set_) {
      hasher.Add(draw_tree->hash());  
    }
    hash_ = hasher.Get();
  }

  const std::vector<std::shared_ptr<DrawTree>>& set() const {
    return set_;
  }

  void Accept(DrawTreeVisitor* visitor) const override {
    visitor->Visit(this);
  }

 private:
  std::vector<std::shared_ptr<DrawTree>> set_;
};

}  // namespace entifypp

#endif  // _SRC_ENTIFY_ENTIFYPP_RENDER_TREE_DRAW_SET_H_
