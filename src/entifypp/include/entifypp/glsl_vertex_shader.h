#ifndef _SRC_ENTIFY_ENTIFYPP_RENDER_TREE_GLSL_VERTEX_SHADER_H_
#define _SRC_ENTIFY_ENTIFYPP_RENDER_TREE_GLSL_VERTEX_SHADER_H_

#include "entifypp/hash.h"
#include "entifypp/types.h"
#include "internal/type_id.h"

namespace entifypp {

class GLSLVertexShaderBase {
 public:
  GLSLVertexShaderBase(std::vector<std::string>&& vertex_attribute_names,
                       std::vector<std::string>&& varying_names,
                       std::vector<std::string>&& uniform_names,
                       std::string&& source)
      : vertex_attribute_names_(std::move(vertex_attribute_names)),
        varying_names_(std::move(varying_names)),
        uniform_names_(std::move(uniform_names)),
        source_(std::move(source)) {}

  const std::vector<std::string>& vertex_attribute_names() const {
    return vertex_attribute_names_;
  }
  const std::vector<std::string>& varying_names() const {
    return varying_names_;
  }
  const std::vector<std::string>& uniform_names() const {
    return uniform_names_;
  }

  virtual std::vector<Type> GetInputTypes() const = 0;
  virtual std::vector<Type> GetOutputTypes() const = 0;
  virtual std::vector<Type> GetUniformTypes() const = 0;

  const std::string& source() const { return source_; }

  EntifyId hash() const { return hash_; }

 private:
  std::vector<std::string> vertex_attribute_names_;
  std::vector<std::string> varying_names_;
  std::vector<std::string> uniform_names_;
  std::string source_;

 protected:
  EntifyId hash_;
};

template <typename InputType,
          typename OutputType,
          typename UniformTypes>
class GLSLVertexShader : public GLSLVertexShaderBase {
 public:
  GLSLVertexShader(std::vector<std::string>&& vertex_attribute_names,
                   std::vector<std::string>&& varying_names,
                   std::vector<std::string>&& uniform_names,
                   std::string&& source)
      : GLSLVertexShaderBase(
            std::move(vertex_attribute_names),
            std::move(varying_names),
            std::move(uniform_names),
            std::move(source)) {
    assert(GLSLVertexShaderBase::vertex_attribute_names().size() ==
               std::tuple_size<InputType>::value);
    assert(GLSLVertexShaderBase::varying_names().size() ==
               std::tuple_size<OutputType>::value);
    assert(GLSLVertexShaderBase::uniform_names().size() ==
               std::tuple_size<UniformTypes>::value);
    Hasher hasher;
    hasher.Add(internal::GetTypeId<GLSLVertexShader>());
    hasher.Add(GLSLVertexShaderBase::vertex_attribute_names());
    hasher.Add(GLSLVertexShaderBase::varying_names());
    hasher.Add(GLSLVertexShaderBase::uniform_names());
    hasher.Add(GLSLVertexShaderBase::source());
    hash_ = hasher.Get();
  }

  std::vector<Type> GetInputTypes() const override {
    return TupleToTypeVector<InputType>();
  }
  std::vector<Type> GetOutputTypes() const override {
    return TupleToTypeVector<OutputType>();
  }
  std::vector<Type> GetUniformTypes() const override {
    return TupleToTypeVector<UniformTypes>();
  }
};

}  // namespace entifypp

#endif  // _SRC_ENTIFY_ENTIFYPP_RENDER_TREE_GLSL_VERTEX_SHADER_H_
