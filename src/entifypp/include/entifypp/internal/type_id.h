#ifndef _SRC_ENTIFY_ENTIFYPP_RENDER_TREE_INTERNAL_TYPE_ID_H_
#define _SRC_ENTIFY_ENTIFYPP_RENDER_TREE_INTERNAL_TYPE_ID_H_

namespace entifypp {
namespace internal {

namespace detail {

template <typename T>
class TypeIdHelper {
 public:
  static bool dummy_;
};

template <typename T>
bool TypeIdHelper<T>::dummy_ = false;

}  // namespace detail

using TypeId = intptr_t;

template <typename T>
TypeId GetTypeId() {
  return reinterpret_cast<TypeId>(&(detail::TypeIdHelper<T>::dummy_));
}

} // namespace internal
} // namespace entifypp

#endif  // _SRC_ENTIFY_ENTIFYPP_RENDER_TREE_INTERNAL_TYPE_ID_H_
