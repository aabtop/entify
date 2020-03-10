#ifndef _SRC_ENTIFY_RENDERER_GLES2_RENDER_TREE_PIPELINE_H_
#define _SRC_ENTIFY_RENDERER_GLES2_RENDER_TREE_PIPELINE_H_

#include <memory>

#include <GLES2/gl2.h>

#include "src/renderer/gles2/render_tree/program.h"

namespace entify {
namespace renderer {
namespace gles2 {
namespace render_tree {

class Pipeline {
 public:
  struct Params {
    struct Blend {
      bool operator==(const Blend& rhs) const {
        return src_color == rhs.src_color &&
               dst_color == rhs.dst_color &&
               src_alpha == rhs.src_alpha &&
               dst_alpha == rhs.dst_alpha;
      }
      bool operator!=(const Blend& rhs) const { return !operator==(rhs); }

      GLenum src_color;
      GLenum dst_color;
      GLenum src_alpha;
      GLenum dst_alpha;
    };

    Blend blend;
  };

  Pipeline(const std::shared_ptr<Program>& program,
           const Params& params) : program_(program), params_(params) {}

  const std::shared_ptr<Program>& program() const { return program_; }

  const Params& params() const { return params_; }

 private:
  std::shared_ptr<Program> program_;
  Params params_;
};

}  // namespace render_tree
}  // namespace gles2
}  // namespace renderer
}  // namespace entify

#endif  // _SRC_ENTIFY_RENDERER_GLES2_RENDER_TREE_PIPELINE_H_
