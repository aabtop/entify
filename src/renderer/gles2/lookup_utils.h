#ifndef _SRC_ENTIFY_RENDERER_GLES2_LOOKUP_UTILS_H_
#define _SRC_ENTIFY_RENDERER_GLES2_LOOKUP_UTILS_H_

#include "src/external_reference.h"

namespace entify {
namespace renderer {
namespace gles2 {

template <typename T>
std::shared_ptr<T> ExternalReferenceToRenderTree(
    const ExternalReference& external_reference) {
  if (external_reference.type_id() != stdext::GetTypeId<T>()) {
    return nullptr;
  }

  return std::static_pointer_cast<T>(external_reference.object());
}

template <typename T>
std::shared_ptr<T> LookupNode(
    const ExternalReferenceLookup& reference_lookup, EntifyId id) {
  auto entry_found = reference_lookup.find(id);
  if (entry_found == reference_lookup.end()) {
    return nullptr;
  }
  return ExternalReferenceToRenderTree<T>(*entry_found->second);
}

}  // namespace gles2
}  // namespace renderer
}  // namespace entify

#endif  // _SRC_ENTIFY_RENDERER_GLES2_PROTOBUF_UTILS_H_
