#ifndef _SRC_ENTIFY_ENTIFYPP_RENDER_TREE_HASH_H_
#define _SRC_ENTIFY_ENTIFYPP_RENDER_TREE_HASH_H_

#include <cassert>
#include <string>
#include <type_traits>
#include <vector>

#include "blake2.h"

#include "entify/registry.h"

namespace entifypp {

class Hasher {
 public:
  Hasher() {
    int return_value = blake2s_init(&state_, kHashLengthInBytes);
    assert(return_value != -1);
  }

  template<size_t Index = 0, typename... U>
  inline typename std::enable_if<Index == sizeof...(U), void>::type
  Add(const std::tuple<U...>& input) {}

  template<size_t Index = 0, typename... U>
  inline typename std::enable_if<Index < sizeof...(U), void>::type
  Add(const std::tuple<U...>& input) {
    Add(std::get<Index>(input));
    Add<Index + 1, U...>(input);
  }

  template <typename T>
  void Add(const std::shared_ptr<T>& input) {
    Add(input->hash());
  }

  void Add(const std::string& input) {
    int return_value = blake2s_update(&state_, reinterpret_cast<const uint8_t*>(input.data()), input.length());
    assert(return_value != -1);
  }

  template <typename T>
  void Add(const std::vector<T>& input) {
    for (const auto& element : input) {
      Add(element);
    }
  }

  template <typename T>
  void Add(const T& input) {
    static_assert(std::is_standard_layout<T>::value,
                  "Unsupported type for hasher.");
    int return_value = blake2s_update(
        &state_, reinterpret_cast<const uint8_t*>(&input), sizeof(input));
    assert(return_value != -1);
  }

  EntifyId Get() {
    EntifyId result;
    int return_value = blake2s_final(
        &state_, reinterpret_cast<uint8_t*>(&result), sizeof(EntifyId));
    assert(return_value != -1);
    return result;
  }

 private:
  const size_t kHashLengthInBytes = sizeof(EntifyId);
  // We use BLAKE2s since it's the minimum common denominator.
  blake2s_state state_;
};

// Convenience all-in-one function.
template <typename T>
EntifyId Hash(const T& input) {
  Hasher hasher;
  hasher.Add(input);
  return hasher.Get();
}

}  // namespace entifypp

#endif  // _SRC_ENTIFY_ENTIFYPP_RENDER_TREE_HASH_H_
