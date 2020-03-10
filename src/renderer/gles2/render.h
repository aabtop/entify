#ifndef _SRC_ENTIFY_RENDERER_GLES2_RENDER_H_
#define _SRC_ENTIFY_RENDERER_GLES2_RENDER_H_

#include <memory>

#include "src/renderer/gles2/render_tree/draw_tree.h"

namespace entify {
namespace renderer {
namespace gles2 {

void Render(int width, int height,
            const std::shared_ptr<render_tree::DrawTree>& draw_tree);

}  // namespace gles2
}  // namespace renderer
}  // namespace entify

#endif  // _SRC_ENTIFY_RENDERER_GLES2_RENDER_H_
