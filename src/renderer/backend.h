#ifndef _SRC_ENTIFY_RENDERER_BACKEND_H_
#define _SRC_ENTIFY_RENDERER_BACKEND_H_

#include <memory>
#include <vector>

#include "src/external_reference.h"
#include "src/renderer/render_target.h"
#include "src/renderer/parse_output.h"

namespace entify {
namespace renderer {

using PlatformWindow = void*;

class Backend {
 public:
  virtual ~Backend() {}

  virtual std::unique_ptr<RenderTarget> CreateRenderTargetFromPlatformWindow(
      PlatformWindow platform_window, int width, int height) = 0;

  virtual void ReleaseReferences(
      std::vector<std::shared_ptr<void>>&& references) = 0;

  virtual ParseOutput ParseProtocolBuffer(
      const ExternalReferenceLookup& reference_lookup,
      const char* data, size_t data_size) = 0;

  virtual ParseOutput ParseFlatBuffer(
      const ExternalReferenceLookup& reference_lookup,
      const char* data, size_t data_size) = 0;

  virtual void Submit(
      ExternalReference* render_tree, RenderTarget* render_target) = 0;
};

std::unique_ptr<Backend> MakeDefaultRenderer();

}  // namespace renderer
}  // namespace entify

#endif  // _SRC_ENTIFY_RENDERER_BACKEND_H_
