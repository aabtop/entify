#include "entifypp/entifypp.h"
#include "submit_to_protobuf.h"

namespace entifypp {

std::shared_ptr<ReferenceTexture> Context::SubmitTexture(
    const std::shared_ptr<Texture>& texture) {
  EntifyReference reference = SubmitTextureToProtobuf(context(), texture);
  return std::shared_ptr<ReferenceTexture>(new ReferenceTexture(
      context(), reference, texture->hash()));
}

void Context::Submit(
    RenderTarget* render_target, const std::shared_ptr<DrawTree>& draw_tree) {
  SubmitDrawTreeToProtobuf(context(), render_target, draw_tree);  
}

}  // namespace entifypp
