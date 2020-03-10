#ifndef _SRC_ENTIFY_RENDERER_GLES2_PARSE_OUTPUT_H_
#define _SRC_ENTIFY_RENDERER_GLES2_PARSE_OUTPUT_H_

#include <string>

#include "src/external_reference.h"

namespace entify {
namespace renderer {

struct ParseOutput {
  std::unique_ptr<ExternalReference> value;
  std::string error_message;

  ParseOutput(std::unique_ptr<ExternalReference>&& value)
      : value(std::move(value)) {}
  template <typename T>
  ParseOutput(const std::shared_ptr<T>& object)
      : value(new ExternalReference(object)) {}

  ParseOutput(const std::string& error)
      : error_message(error) {}
};

}  // namespace renderer
}  // namespace entify

#endif  // _SRC_ENTIFY_RENDERER_GLES2_PARSE_OUTPUT_H_
