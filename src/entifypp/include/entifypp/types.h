#ifndef _SRC_ENTIFY_ENTIFYPP_RENDER_TREE_TYPES_H_
#define _SRC_ENTIFY_ENTIFYPP_RENDER_TREE_TYPES_H_

#include <memory>

#include "entifypp/sampler.h"
#include "glm.hpp"

namespace entifypp {

enum Type {
  TypeInvalid,

  TypeFloat32V1,
  TypeFloat32V2,
  TypeFloat32V3,
  TypeFloat32V4,
  TypeFloat32M44,
  TypeUInt8V1,
  TypeUInt8V2,
  TypeUInt8V3,
  TypeUInt8V4,
  TypeSampler,
};

template <typename T>
struct ToTypeEnum {};

template <>
struct ToTypeEnum<float> {
  enum { value = TypeFloat32V1 };
};

template <>
struct ToTypeEnum<glm::vec2> {
  enum { value = TypeFloat32V2 };
};

template <>
struct ToTypeEnum<glm::vec3> {
  enum { value = TypeFloat32V3 };
};

template <>
struct ToTypeEnum<glm::vec4> {
  enum { value = TypeFloat32V4 };
};

template <>
struct ToTypeEnum<glm::mat4> {
  enum { value = TypeFloat32M44 };
};

template <>
struct ToTypeEnum<uint8_t> {
  enum { value = TypeUInt8V1 };
};

template <>
struct ToTypeEnum<glm::tvec2<uint8_t>> {
  enum { value = TypeUInt8V2 };
};

template <>
struct ToTypeEnum<glm::tvec3<uint8_t>> {
  enum { value = TypeUInt8V3 };
};

template <>
struct ToTypeEnum<glm::tvec4<uint8_t>> {
  enum { value = TypeUInt8V4 };
};

template <>
struct ToTypeEnum<std::shared_ptr<Sampler>> {
  enum { value = TypeSampler };
};

namespace detail {

template <typename... Args>
struct PushTypesToVector {};

template <typename Head, typename... Rest>
struct PushTypesToVector<Head, Rest...> {
  static void Push(std::vector<Type>* result) {
    result->push_back(static_cast<Type>(ToTypeEnum<Head>::value));
    PushTypesToVector<Rest...>::Push(result);
  }
};

template <>
struct PushTypesToVector<> {
  static void Push(std::vector<Type>* result) {}
};

template<typename T>
struct ToTypeVector {};

template<typename... TupleTypes>
struct ToTypeVector<std::tuple<TupleTypes...>> {
  static std::vector<Type> value() {
    std::vector<Type> result;
    result.reserve(sizeof...(TupleTypes));
    PushTypesToVector<TupleTypes...>::Push(&result);
    return result;
  }
};

}  // namespace detail

template<typename T>
std::vector<Type> TupleToTypeVector() {
  return detail::ToTypeVector<T>::value();
}

}  // namespace entifypp

#endif  // _SRC_ENTIFY_ENTIFYPP_RENDER_TREE_TYPES_H_
