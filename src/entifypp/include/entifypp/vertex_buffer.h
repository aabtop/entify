#ifndef _SRC_ENTIFY_ENTIFYPP_RENDER_TREE_VERTEX_BUFFER_H_
#define _SRC_ENTIFY_ENTIFYPP_RENDER_TREE_VERTEX_BUFFER_H_

#include <tuple>
#include <vector>

#include "entifypp/hash.h"
#include "entifypp/types.h"

namespace entifypp {

namespace detail {

template<std::size_t Index = 0, typename TupleType>
typename std::enable_if<Index == std::tuple_size<TupleType>::value, void>::type
PushBackTupleElementOffset(std::vector<int32_t>* offsets) {}

template<std::size_t Index = 0, typename TupleType>
typename std::enable_if<(Index < std::tuple_size<TupleType>::value), void>::type
PushBackTupleElementOffset(std::vector<int32_t>* offsets) {
  TupleType* dummy = nullptr;
  offsets->push_back(static_cast<int32_t>(
      reinterpret_cast<intptr_t>(&std::get<Index>(*dummy))));
  PushBackTupleElementOffset<Index + 1, TupleType>(offsets);
}

template<typename T>
typename std::enable_if<std::is_same<
    T, std::shared_ptr<Sampler>>::value, void>::type
PushBackTupleElementOffset(
    const T& element, std::vector<std::shared_ptr<Sampler>>* samplers) {
  samplers->push_back(element);
}

}  // namespace detail

class VertexBufferBase {
 public:
  virtual ~VertexBufferBase() {}

  virtual std::vector<Type> GetTypes() const = 0;

  EntifyId hash() const { return hash_; }

  struct DataInfo {
    const void* data;
    size_t num_vertices;
    size_t stride_in_bytes;
  };
  virtual DataInfo GetDataInfo() const = 0;

  virtual std::vector<int32_t> GetDataOffsets() const = 0;

 protected:
  EntifyId hash_;
};

template <typename TypeTuple>
class VertexBuffer : public VertexBufferBase {
 public:
  VertexBuffer(std::vector<TypeTuple>&& vertices)
      : vertices_(std::move(vertices)) {
    Hasher hasher;
    hasher.Add(internal::GetTypeId<VertexBuffer>());
    hasher.Add(vertices_);
    hash_ = hasher.Get();
  }

  const std::vector<TypeTuple>& vertices() const { return vertices_; }

  std::vector<Type> GetTypes() const override {
    return TupleToTypeVector<TypeTuple>();
  }

  DataInfo GetDataInfo() const override {
    return {vertices_.data(), vertices_.size(), sizeof(TypeTuple)};
  }

  std::vector<int32_t> GetDataOffsets() const override {
    std::vector<int32_t> offsets;
    detail::PushBackTupleElementOffset<0, TypeTuple>(&offsets);
    return offsets;
  }

 private:
  std::vector<TypeTuple> vertices_;
  std::vector<int32_t> offsets_;
};

}  // namespace entifypp

#endif  // _SRC_ENTIFY_ENTIFYPP_RENDER_TREE_VERTEX_BUFFER_H_
