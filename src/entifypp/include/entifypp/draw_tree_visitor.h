#ifndef _SRC_ENTIFY_ENTIFYPP_RENDER_TREE_DRAW_TREE_VISITOR_H_
#define _SRC_ENTIFY_ENTIFYPP_RENDER_TREE_DRAW_TREE_VISITOR_H_

namespace entifypp {

class DrawCall;
class DrawSequence;
class DrawSet;

class DrawTreeVisitor {
 public:
  virtual void Visit(const DrawCall* draw_call) = 0;
  virtual void Visit(const DrawSequence* draw_sequence) = 0;
  virtual void Visit(const DrawSet* draw_set) = 0;
};

}  // namespace entifypp

#endif  // _SRC_ENTIFY_ENTIFYPP_RENDER_TREE_DRAW_TREE_VISITOR_H_
