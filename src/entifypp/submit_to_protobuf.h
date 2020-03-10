#ifndef _SRC_ENTIFY_SUBMIT_RENDER_TREE_TO_PROTOBUF_H_
#define _SRC_ENTIFY_SUBMIT_RENDER_TREE_TO_PROTOBUF_H_

#include <memory>

#include "entify/entify.h"
#include "entifypp/entifypp.h"
#include "entifypp/draw_tree.h"

namespace entifypp {

EntifyReference SubmitTextureToProtobuf(
    EntifyContext context, const std::shared_ptr<Texture>& texture);

void SubmitDrawTreeToProtobuf(
    EntifyContext context, RenderTarget* render_target,
    const std::shared_ptr<DrawTree>& draw_tree);

}  // namespace entifypp

#endif  // _SRC_ENTIFY_SUBMIT_RENDER_TREE_TO_PROTOBUF_H_
