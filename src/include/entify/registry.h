#ifndef _SRC_ENTIFY_REGISTRY_H_
#define _SRC_ENTIFY_REGISTRY_H_

#include <cstdint>
#include <cstdlib>

#if defined(EXPORT_ENTIFY)
#define SHARED_LIBRARY_EXPORT
#endif
#include "stdext/shared_library.h"

#ifdef __cplusplus  
extern "C" { 
#endif

typedef void* EntifyContext;
typedef void* EntifyReference;
typedef int64_t EntifyId;

const EntifyContext kEntifyInvalidContext = 0;
const EntifyReference kEntifyInvalidReference = 0;

PUBLIC_API EntifyContext EntifyCreateContext();
PUBLIC_API void EntifyDestroyContext(EntifyContext context);

PUBLIC_API EntifyReference EntifyTryGetReferenceFromId(
    EntifyContext context, EntifyId id);

PUBLIC_API EntifyReference
    EntifyCreateReferenceFromProtocolBuffer(
        EntifyContext context, EntifyId id, const char* data, size_t data_size);

PUBLIC_API EntifyReference
    EntifyCreateReferenceFromFlatBuffer(
        EntifyContext context, EntifyId id, const char* data, size_t data_size);

// Returns 1 if there was an error from the previous
// EntifyCreateReferenceFromFlatBuffer() or
// EntifyCreateReferenceFromProtocolBuffer() call.  If 1 is returned,
// then |message| will be set to point to an error message.
PUBLIC_API int EntifyGetLastError(
    EntifyContext context, const char** message);

PUBLIC_API void EntifyAddReference(
    EntifyContext context, EntifyReference reference);

PUBLIC_API void EntifyReleaseReference(
    EntifyContext context, EntifyReference reference);

#ifdef __cplusplus
} 
#endif

#endif  // _SRC_ENTIFY_REGISTRY_H_
