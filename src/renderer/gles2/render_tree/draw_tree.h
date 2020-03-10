#ifndef _SRC_ENTIFY_RENDERER_GLES2_RENDER_TREE_DRAW_TREE_H_
#define _SRC_ENTIFY_RENDERER_GLES2_RENDER_TREE_DRAW_TREE_H_

namespace entify {
namespace renderer {
namespace gles2 {
namespace render_tree {

class DrawTree {
 public:
  enum Type {
    kTypeDrawCall,
    kTypeDrawSequence,
    kTypeDrawSet,
  };

  DrawTree(Type type) : type_(type) {}

  virtual ~DrawTree() {}

  const Type type() const { return type_; }

 private:
  Type type_;
};

}  // namespace render_tree
}  // namespace gles2
}  // namespace renderer
}  // namespace entify

#endif  // _SRC_ENTIFY_RENDERER_GLES2_RENDER_TREE_DRAW_TREE_H_
