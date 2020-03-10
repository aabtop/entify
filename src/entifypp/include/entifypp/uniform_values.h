#ifndef _SRC_ENTIFY_ENTIFYPP_RENDER_TREE_UNIFORM_VALUES_H_
#define _SRC_ENTIFY_ENTIFYPP_RENDER_TREE_UNIFORM_VALUES_H_

#include <tuple>
#include <vector>

#include "entifypp/sampler.h"
#include "entifypp/hash.h"
#include "entifypp/types.h"

namespace entifypp {

namespace detail {

template<typename T>
typename std::enable_if<!std::is_same<
    T, std::shared_ptr<Sampler>>::value, void>::type
MaybePushBackSampler(
    const T& element, std::vector<std::shared_ptr<Sampler>>* samplers) {}

template<typename T>
typename std::enable_if<std::is_same<
    T, std::shared_ptr<Sampler>>::value, void>::type
MaybePushBackSampler(
    const T& element, std::vector<std::shared_ptr<Sampler>>* samplers) {
  samplers->push_back(element);
}


// Utility function to extract samplers only from types tuple.
template<std::size_t Index = 0, typename TupleType>
typename std::enable_if<Index == std::tuple_size<TupleType>::value, void>::type
AddSamplersToVector(const TupleType& type_tuple,
                    std::vector<std::shared_ptr<Sampler>>* samplers) {}

template<std::size_t Index = 0, typename TupleType>
typename std::enable_if<(Index < std::tuple_size<TupleType>::value), void>::type
AddSamplersToVector(const TupleType& type_tuple,
                    std::vector<std::shared_ptr<Sampler>>* samplers) {
  MaybePushBackSampler(std::get<Index>(type_tuple), samplers);
  AddSamplersToVector<Index + 1, TupleType>(type_tuple, samplers);
}

}  // namespace detail

class UniformValuesBase {
 public:
  virtual ~UniformValuesBase() {}
  
  virtual std::vector<Type> GetTypes() const = 0;

  virtual std::vector<std::shared_ptr<Sampler>> GetSamplers() const = 0;

  EntifyId hash() const { return hash_; }

  struct DataInfo {
    const void* data;
    size_t size;
  };
  virtual DataInfo GetDataInfo() const = 0;

 protected:
  EntifyId hash_;
};

namespace detail {

template<typename T>
typename std::enable_if<std::is_same<
    T, std::shared_ptr<Sampler>>::value, size_t>::type
MaybeWriteData(uint8_t* out_data, const T& element) {
  return 0;
}

template<typename T>
typename std::enable_if<!std::is_same<
    T, std::shared_ptr<Sampler>>::value, size_t>::type
MaybeWriteData(uint8_t* out_data, const T& element) {
  *reinterpret_cast<T*>(out_data) = element;
  return sizeof(T);
}

template<std::size_t Index = 0, typename TupleType>
typename std::enable_if<Index == std::tuple_size<TupleType>::value,
                        size_t>::type
PushBackData(uint8_t* out_data, const TupleType& values) {
  return 0;
}

template<std::size_t Index = 0, typename TupleType>
typename std::enable_if<(Index < std::tuple_size<TupleType>::value),
                        size_t>::type
PushBackData(uint8_t* out_data, const TupleType& values) {
  size_t size = MaybeWriteData(out_data, std::get<Index>(values));
  return size + PushBackData<Index + 1, TupleType>(out_data + size, values);
}

}  // namespace detail

template <typename TypeTuple>
class UniformValues : public UniformValuesBase {
 public:
  UniformValues(TypeTuple&& values)
      : values_(std::move(values)) {
    data_size_ = detail::PushBackData(data_, values_);
    Hasher hasher;
    hasher.Add(internal::GetTypeId<UniformValues>());
    hasher.Add(values_);
    hash_ = hasher.Get();
  }

  template <typename... U>
  UniformValues(U... params)
      : UniformValues(TypeTuple(params...)) {}

  const TypeTuple& values() const { return values_; }

  std::vector<Type> GetTypes() const override {
    return TupleToTypeVector<TypeTuple>();
  }

  std::vector<std::shared_ptr<Sampler>> GetSamplers() const override {
    std::vector<std::shared_ptr<Sampler>> samplers;
    detail::AddSamplersToVector<0, TypeTuple>(values_, &samplers);
    return samplers;
  }

  DataInfo GetDataInfo() const override {
    return {data_, data_size_};
  }

 private:
  TypeTuple values_;

  // Data layed out in memory in order (tuple does not guarantee this).
  uint8_t data_[sizeof(TypeTuple)];
  size_t data_size_;
};

}  // namespace entifypp

#endif  // _SRC_ENTIFY_ENTIFYPP_RENDER_TREE_UNIFORM_VALUES_H_
