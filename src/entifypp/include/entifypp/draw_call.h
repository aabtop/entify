#ifndef _SRC_ENTIFY_ENTIFYPP_RENDER_TREE_DRAW_CALL_H_
#define _SRC_ENTIFY_ENTIFYPP_RENDER_TREE_DRAW_CALL_H_

#include <memory>

#include "entifypp/draw_tree.h"
#include "entifypp/pipeline.h"
#include "entifypp/vertex_buffer.h"
#include "entifypp/uniform_values.h"
#include "entifypp/hash.h"

namespace entifypp {

class DrawCall : public DrawTree {
 public:
  template <typename VertexInputType,
            typename FragmentInputType,
            typename VertexUniformTypes,
            typename FragmentUniformTypes>
  DrawCall(const std::shared_ptr<Pipeline<VertexInputType,
                                          FragmentInputType,
                                          VertexUniformTypes,
                                          FragmentUniformTypes>>& pipeline,
           const std::shared_ptr<VertexBuffer<VertexInputType>>& vertex_buffer,
           const std::shared_ptr<UniformValues<VertexUniformTypes>>&
               vertex_uniforms,
           const std::shared_ptr<UniformValues<FragmentUniformTypes>>&
               fragment_uniforms)
      : pipeline_(pipeline),
        vertex_buffer_(vertex_buffer),
        vertex_uniforms_(vertex_uniforms),
        fragment_uniforms_(fragment_uniforms) {
    Hasher hasher;
    hasher.Add(pipeline_->hash());
    hasher.Add(vertex_buffer_->hash());
    hasher.Add(vertex_uniforms_->hash());
    hasher.Add(fragment_uniforms_->hash());
    hash_ = hasher.Get();
  }

  const std::shared_ptr<PipelineBase>& pipeline() const {
    return pipeline_;
  }
  const std::shared_ptr<VertexBufferBase>& vertex_buffer() const {
    return vertex_buffer_;
  }
  const std::shared_ptr<UniformValuesBase>& vertex_uniforms() const {
    return vertex_uniforms_;
  }
  const std::shared_ptr<UniformValuesBase>& fragment_uniforms() const {
    return fragment_uniforms_;
  }

  void Accept(DrawTreeVisitor* visitor) const override {
    visitor->Visit(this);
  }

 private:
  std::shared_ptr<PipelineBase> pipeline_;
  std::shared_ptr<VertexBufferBase> vertex_buffer_;
  std::shared_ptr<UniformValuesBase> vertex_uniforms_;
  std::shared_ptr<UniformValuesBase> fragment_uniforms_;
};

}  // namespace entifypp

#endif  // _SRC_ENTIFY_ENTIFYPP_RENDER_TREE_DRAW_CALL_H_
