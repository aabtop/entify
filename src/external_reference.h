#ifndef _SRC_ENTIFY_REFERENCE_H_
#define _SRC_ENTIFY_REFERENCE_H_

#include <memory>
#include <unordered_map>

#include "src/include/entify/registry.h"
#include "stdext/type_id.h"

namespace entify {

class ExternalReference {
 public:
  ExternalReference(
      stdext::TypeId type_id, const std::shared_ptr<void>& object)
      : external_reference_count_(0), type_id_(type_id), object_(object) {}

  template <typename T>
  ExternalReference(const std::shared_ptr<T>& object)
      : ExternalReference(stdext::GetTypeId<T>(), object) {}

  stdext::TypeId type_id() const {
    return type_id_;
  }

  const std::shared_ptr<void>& object() const { return object_; }

  bool is_referenced() const {
    return external_reference_count() > 0 || object().use_count() > 1;
  }

  int32_t external_reference_count() const { return external_reference_count_; }
  void increment_external_reference_count() { ++external_reference_count_; }
  void decrement_external_reference_count() { --external_reference_count_; }

 private:
  int32_t external_reference_count_;
  stdext::TypeId type_id_;

  // The "internal reference" to the object.
  std::shared_ptr<void> object_;
};

using ExternalReferenceLookup =
    std::unordered_map<EntifyId, std::unique_ptr<ExternalReference>>;

}  // namespace entify

#endif  // _SRC_ENTIFY_REFERENCE_H_
