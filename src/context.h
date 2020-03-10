#ifndef _SRC_ENTIFY_CONTEXT_H_
#define _SRC_ENTIFY_CONTEXT_H_

#include "entify/registry.h"
#include "src/renderer/backend.h"
#include "src/external_reference.h"

namespace entify {

class Context {
 public:
  using RenderTarget = renderer::RenderTarget;
  using PlatformWindow = renderer::PlatformWindow;

  Context(std::unique_ptr<renderer::Backend> backend)
      : backend_(std::move(backend)) {}

  EntifyReference TryGetReferenceFromId(EntifyId id);
  EntifyReference CreateReferenceFromProtocolBuffer(
      EntifyId id, const char* data, size_t data_size);
  EntifyReference CreateReferenceFromFlatBuffer(
      EntifyId id, const char* data, size_t data_size);
  int GetLastError(const char** message);

  void AddReference(EntifyReference reference);
  void ReleaseReference(EntifyReference reference);

  std::unique_ptr<RenderTarget> CreateRenderTargetFromPlatformWindow(
      PlatformWindow platform_window, int width, int height);

  void Submit(EntifyReference render_tree, RenderTarget* render_target);

 private:
  // Looks through all of the external reference lookups and removes those who
  // are unreferenced (i.e. those whose only reference is the lookup entry
  // itself).
  void DoGarbageCollection();

  std::unique_ptr<renderer::Backend> backend_;
  ExternalReferenceLookup id_lookup_;
  std::string last_error_;
};

}  // namespace entify

#endif  // _SRC_ENTIFY_CONTEXT_H_
