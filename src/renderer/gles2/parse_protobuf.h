#ifndef _SRC_ENTIFY_RENDERER_GLES2_PARSE_PROTOBUF_H_
#define _SRC_ENTIFY_RENDERER_GLES2_PARSE_PROTOBUF_H_

#include <memory>

#include "src/external_reference.h"
#include "src/renderer/parse_output.h"

namespace entify {
namespace renderer {
namespace gles2 {

// This function assumes that it is called while a context is current.
ParseOutput ParseProtocolBuffer(
    const ExternalReferenceLookup& reference_lookup,
    const char* data, size_t data_size);

}  // namespace gles2
}  // namespace renderer
}  // namespace entify

#endif  // _SRC_ENTIFY_RENDERER_GLES2_PARSE_PROTOBUF_H_
