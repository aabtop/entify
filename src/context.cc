#include "src/context.h"

#include <cassert>
#include <vector>

namespace entify {

EntifyReference Context::TryGetReferenceFromId(EntifyId id) {
  auto found = id_lookup_.find(id);
  if (found != id_lookup_.end()) {
    found->second->increment_external_reference_count();
    return found->second.get();
  }
  return kEntifyInvalidReference;
}

EntifyReference Context::CreateReferenceFromProtocolBuffer(
    EntifyId id, const char* data, size_t data_size) {
  last_error_.clear();
  renderer::ParseOutput result = backend_->ParseProtocolBuffer(
      id_lookup_, data, data_size);
  if (result.value.get() == nullptr) {
    last_error_ = result.error_message;
    assert(!last_error_.empty());
    return kEntifyInvalidReference;
  }

  result.value->increment_external_reference_count();

  auto insert_results =
      id_lookup_.insert(std::make_pair(id, std::move(result.value)));
  assert(insert_results.second);

  return insert_results.first->second.get();
}

EntifyReference Context::CreateReferenceFromFlatBuffer(
    EntifyId id, const char* data, size_t data_size) {
  last_error_.clear();
  renderer::ParseOutput result = backend_->ParseFlatBuffer(
      id_lookup_, data, data_size);
  if (result.value.get() == nullptr) {
    last_error_ = result.error_message;
    assert(!last_error_.empty());
    return kEntifyInvalidReference;
  }

  result.value->increment_external_reference_count();

  auto insert_results =
      id_lookup_.insert(std::make_pair(id, std::move(result.value)));
  assert(insert_results.second);

  return insert_results.first->second.get();
}

int Context::GetLastError(const char** message) {
  if (last_error_.empty()) {
    return 0;
  } else {
    *message = last_error_.c_str();
    return 1;
  }
}

void Context::AddReference(EntifyReference reference) {
  ExternalReference* external_reference =
      static_cast<ExternalReference*>(reference);
  external_reference->increment_external_reference_count();
}

void Context::ReleaseReference(EntifyReference reference) {
  ExternalReference* external_reference =
      static_cast<ExternalReference*>(reference);
  external_reference->decrement_external_reference_count();
}

std::unique_ptr<Context::RenderTarget>
Context::CreateRenderTargetFromPlatformWindow(
    PlatformWindow platform_window, int width, int height) {
  return backend_->CreateRenderTargetFromPlatformWindow(
      platform_window, width, height);
}

void Context::DoGarbageCollection() {
  std::vector<EntifyId> to_erase;
  for (const auto& entry : id_lookup_) {
    const ExternalReference& reference = *entry.second;
    if (!reference.is_referenced()) {
      to_erase.push_back(entry.first);
    }
  }

  std::vector<std::shared_ptr<void>> references_to_release;
  references_to_release.reserve(to_erase.size());
  for (const auto& id_to_erase : to_erase) {
    auto found = id_lookup_.find(id_to_erase);
    references_to_release.push_back(found->second->object());
    id_lookup_.erase(found);
  }

  backend_->ReleaseReferences(std::move(references_to_release));
}

void Context::Submit(EntifyReference render_tree, RenderTarget* render_target) {
  backend_->Submit(static_cast<ExternalReference*>(render_tree), render_target);
  DoGarbageCollection();
}

}  // namespace entify
