#ifndef _SRC_ENTIFY_ENTIFYPP_RENDER_TREE_PIPELINE_H_
#define _SRC_ENTIFY_ENTIFYPP_RENDER_TREE_PIPELINE_H_

#include <memory>

#include "entifypp/glsl_fragment_shader.h"
#include "entifypp/glsl_vertex_shader.h"
#include "entifypp/hash.h"
#include "internal/type_id.h"

namespace entifypp {

struct PipelineOptions {
  struct BlendOptions {
    enum class Coefficient {
      kZero,
      kOne,
      kSrcAlpha,
      kOneMinusSrcAlpha,
    };

    BlendOptions& set_src_color(Coefficient _src_color) {
      src_color = _src_color;
      return *this;
    }
    BlendOptions& set_dst_color(Coefficient _dst_color) {
      dst_color = _dst_color;
      return *this;
    }
    BlendOptions& set_src_alpha(Coefficient _src_alpha) {
      src_alpha = _src_alpha;
      return *this;
    }
    BlendOptions& set_dst_alpha(Coefficient _dst_alpha) {
      dst_alpha = _dst_alpha;
      return *this;
    }

    Coefficient src_color = Coefficient::kOne;
    Coefficient dst_color = Coefficient::kOneMinusSrcAlpha;
    Coefficient src_alpha = Coefficient::kOne;
    Coefficient dst_alpha = Coefficient::kOneMinusSrcAlpha;
  };

  PipelineOptions& set_blend(const BlendOptions& _blend) {
    blend = _blend;
    return *this;
  }

  BlendOptions blend;
};

inline PipelineOptions::BlendOptions NoBlending() {
  return PipelineOptions::BlendOptions()
             .set_src_color(PipelineOptions::BlendOptions::Coefficient::kOne)
             .set_dst_color(PipelineOptions::BlendOptions::Coefficient::kZero)
             .set_src_alpha(PipelineOptions::BlendOptions::Coefficient::kOne)
             .set_dst_alpha(PipelineOptions::BlendOptions::Coefficient::kZero);

}

class PipelineBase {
 public:
  PipelineBase(const std::shared_ptr<GLSLVertexShaderBase>& vertex_shader,
               const std::shared_ptr<GLSLFragmentShaderBase>& fragment_shader,
               const PipelineOptions& options)
      : vertex_shader_(vertex_shader), fragment_shader_(fragment_shader),
        options_(options) {}

  virtual ~PipelineBase() {}

  std::shared_ptr<GLSLVertexShaderBase> vertex_shader_base() const {
    return vertex_shader_;
  }

  std::shared_ptr<GLSLFragmentShaderBase> fragment_shader_base() const {
    return fragment_shader_;
  }

  const PipelineOptions& options() const { return options_; }

  EntifyId hash() const { return hash_; }

 protected:
  EntifyId hash_;

  std::shared_ptr<GLSLVertexShaderBase> vertex_shader_;
  std::shared_ptr<GLSLFragmentShaderBase> fragment_shader_;
  PipelineOptions options_;
};

template <typename VertexInputType,
          typename FragmentInputType,
          typename VertexUniformTypes,
          typename FragmentUniformTypes>
class Pipeline : public PipelineBase {
 public:
  using VertexShaderType =
      GLSLVertexShader<VertexInputType, FragmentInputType, VertexUniformTypes>;
  using FragmentShaderType =
      GLSLFragmentShader<FragmentInputType, FragmentUniformTypes>;

  Pipeline(const std::shared_ptr<VertexShaderType>& vertex_shader,
           const std::shared_ptr<FragmentShaderType>& fragment_shader,
           const PipelineOptions& options = PipelineOptions())
      : PipelineBase(vertex_shader, fragment_shader, options) {
    assert(vertex_shader_base()->varying_names() ==
               fragment_shader_base()->varying_names());
    Hasher hasher;
    hasher.Add(internal::GetTypeId<Pipeline>());
    hasher.Add(vertex_shader_->hash());
    hasher.Add(fragment_shader_->hash());
    hasher.Add(options_.blend.src_color);
    hasher.Add(options_.blend.dst_color);
    hasher.Add(options_.blend.src_alpha);
    hasher.Add(options_.blend.dst_alpha);
    hash_ = hasher.Get();
  }

  const std::shared_ptr<VertexShaderType>& vertex_shader() const {
    return std::static_pointer_cast<VertexShaderType>(vertex_shader_base());
  }
  const std::shared_ptr<FragmentShaderType>& fragment_shader() const {
    return std::static_pointer_cast<FragmentShaderType>(fragment_shader_base());
  }
};

}  // namespace entifypp

#endif  // _SRC_ENTIFY_ENTIFYPP_RENDER_TREE_PIPELINE_H_
