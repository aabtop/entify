#ifndef _SRC_ENTIFY_ENTIFYPP_RENDER_TREE_GLSL_FRAGMENT_SHADER_H_
#define _SRC_ENTIFY_ENTIFYPP_RENDER_TREE_GLSL_FRAGMENT_SHADER_H_

#include <string>

#include "entifypp/hash.h"
#include "entifypp/types.h"

namespace entifypp {

class GLSLFragmentShaderBase {
 public:
  GLSLFragmentShaderBase(std::vector<std::string>&& varying_names,
                         std::vector<std::string>&& uniform_names,
                         std::string&& source)
      : varying_names_(std::move(varying_names)),
        uniform_names_(std::move(uniform_names)),
        source_(std::move(source)) {}

  virtual ~GLSLFragmentShaderBase() {}

  const std::vector<std::string>& varying_names() const {
    return varying_names_;
  }

  const std::vector<std::string>& uniform_names() const {
    return uniform_names_;
  }

  virtual std::vector<Type> GetInputTypes() const = 0;
  virtual std::vector<Type> GetUniformTypes() const = 0;

  const std::string& source() const { return source_; }

  EntifyId hash() const { return hash_; }

 private:
  std::vector<std::string> varying_names_;
  std::vector<std::string> uniform_names_;
  std::string source_;

 protected:
  EntifyId hash_;
};

template <typename InputType,
          typename UniformTypes>
class GLSLFragmentShader : public GLSLFragmentShaderBase {
 public:
  GLSLFragmentShader(std::vector<std::string>&& varying_names,
                     std::vector<std::string>&& uniform_names,
                     std::string&& source)
      : GLSLFragmentShaderBase(
            std::move(varying_names),
            std::move(uniform_names),
            std::move(source)) {
    assert(GLSLFragmentShaderBase::varying_names().size() ==
               std::tuple_size<InputType>::value);
    assert(GLSLFragmentShaderBase::uniform_names().size() ==
               std::tuple_size<UniformTypes>::value);
    Hasher hasher;
    hasher.Add(internal::GetTypeId<GLSLFragmentShader>());
    hasher.Add(GLSLFragmentShaderBase::varying_names());
    hasher.Add(GLSLFragmentShaderBase::uniform_names());
    hasher.Add(GLSLFragmentShaderBase::source());
    hash_ = hasher.Get();
  }

  std::vector<Type> GetInputTypes() const override {
    return TupleToTypeVector<InputType>();
  }
  std::vector<Type> GetUniformTypes() const override {
    return TupleToTypeVector<UniformTypes>();
  }
};

}  // namespace entifypp

#endif  // _SRC_ENTIFY_ENTIFYPP_RENDER_TREE_GLSL_FRAGMENT_SHADER_H_
