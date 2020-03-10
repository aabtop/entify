#include "entify/registry.h"

#include "context.h"
#include "renderer/backend.h"

EntifyContext EntifyCreateContext() {
  return new entify::Context(entify::renderer::MakeDefaultRenderer());
}

void EntifyDestroyContext(EntifyContext context) {
  delete static_cast<entify::Context*>(context);
}

EntifyReference EntifyTryGetReferenceFromId(
    EntifyContext context, EntifyId id) {
  return static_cast<entify::Context*>(context)->TryGetReferenceFromId(id);
}

EntifyReference EntifyCreateReferenceFromProtocolBuffer(
    EntifyContext context, EntifyId id, const char* data, size_t data_size) {
  return static_cast<entify::Context*>(context)
      ->CreateReferenceFromProtocolBuffer(id, data, data_size);
}

EntifyReference EntifyCreateReferenceFromFlatBuffer(
    EntifyContext context, EntifyId id, const char* data, size_t data_size) {
  return static_cast<entify::Context*>(context)
      ->CreateReferenceFromFlatBuffer(id, data, data_size);
}

int EntifyGetLastError(EntifyContext context, const char** message) {
  return static_cast<entify::Context*>(context)->GetLastError(message);
}

void EntifyAddReference(EntifyContext context, EntifyReference reference) {
  static_cast<entify::Context*>(context)->AddReference(reference);  
}

void EntifyReleaseReference(EntifyContext context, EntifyReference reference) {
  static_cast<entify::Context*>(context)->ReleaseReference(reference);
}
