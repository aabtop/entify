#ifndef _SRC_ENTIFY_ENTIFYPP_RENDER_TREE_DRAW_TREE_H_
#define _SRC_ENTIFY_ENTIFYPP_RENDER_TREE_DRAW_TREE_H_

#include "entifypp/draw_tree_visitor.h"
#include "entifypp/hash.h"

namespace entifypp {

class DrawTree {
 public:
  EntifyId hash() const { return hash_; }

  virtual void Accept(DrawTreeVisitor* visitor) const = 0;

 protected:
  EntifyId hash_;
};


}  // namespace entifypp

#endif  // _SRC_ENTIFY_ENTIFYPP_RENDER_TREE_DRAW_TREE_H_
