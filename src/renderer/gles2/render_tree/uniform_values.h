#ifndef _SRC_ENTIFY_RENDERER_GLES2_RENDER_TREE_UNIFORM_VALUES_H_
#define _SRC_ENTIFY_RENDERER_GLES2_RENDER_TREE_UNIFORM_VALUES_H_

#include <GLES2/gl2.h>

#include "src/renderer/gles2/render_tree/sampler.h"
#include "src/renderer/gles2/render_tree/types.h"

namespace entify {
namespace renderer {
namespace gles2 {
namespace render_tree {

class UniformValues {
 public:
  UniformValues(TypeTuple&& types, std::vector<char>&& data,
                std::vector<std::shared_ptr<Sampler>>&& samplers)
      : types_(types), data_(data), samplers_(std::move(samplers)) {}
  ~UniformValues() {}

  const TypeTuple& types() const { return types_; }
  const std::vector<char>& data() const { return data_; }
  const std::vector<std::shared_ptr<Sampler>>& samplers() const {
    return samplers_;
  }

 private:
  TypeTuple types_;
  std::vector<char> data_;
  std::vector<std::shared_ptr<Sampler>> samplers_;
};

}  // namespace render_tree
}  // namespace gles2
}  // namespace renderer
}  // namespace entify

#endif  // _SRC_ENTIFY_RENDERER_GLES2_RENDER_TREE_UNIFORM_VALUES_H_
