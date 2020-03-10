#ifndef _SRC_ENTIFY_RENDERER_GLES2_RENDER_TREE_TYPES_H_
#define _SRC_ENTIFY_RENDERER_GLES2_RENDER_TREE_TYPES_H_

#include <cassert>
#include <vector>

namespace entify {
namespace renderer {
namespace gles2 {
namespace render_tree {

enum Type {
  TypeInvalid,

  TypeFloat32V1,
  TypeFloat32V2,
  TypeFloat32V3,
  TypeFloat32V4,
  TypeFloat32M44,
  TypeUInt8V1,
  TypeUInt8V2,
  TypeUInt8V3,
  TypeUInt8V4,
  TypeSampler,
};

inline GLenum TypeToGLType(Type type) {
  switch (type) {
    case TypeFloat32V1: return GL_FLOAT;
    case TypeFloat32V2: return GL_FLOAT;
    case TypeFloat32V3: return GL_FLOAT;
    case TypeFloat32V4: return GL_FLOAT;
    case TypeFloat32M44: return GL_FLOAT;
    case TypeUInt8V1: return GL_UNSIGNED_BYTE;
    case TypeUInt8V2: return GL_UNSIGNED_BYTE;
    case TypeUInt8V3: return GL_UNSIGNED_BYTE;
    case TypeUInt8V4: return GL_UNSIGNED_BYTE;
    default:
      assert(false);
      return 0;
  }
}

inline int GLTypeToComponentSize(GLenum gl_type) {
  switch (gl_type) {
    case GL_FLOAT: return 4;
    case GL_UNSIGNED_BYTE: return 1;
    default:
      assert(false);
      return -1;
  }
}

inline int TypeToComponentCount(Type type) {
  switch (type) {
    case TypeFloat32V1: return 1;
    case TypeFloat32V2: return 2;
    case TypeFloat32V3: return 3;
    case TypeFloat32V4: return 4;
    case TypeFloat32M44: return 16;
    case TypeUInt8V1: return 1;
    case TypeUInt8V2: return 2;
    case TypeUInt8V3: return 3;
    case TypeUInt8V4: return 4;
    default:
      assert(false);
      return -1;
  }
}

inline int TypeToSize(Type type) {
  return GLTypeToComponentSize(TypeToGLType(type)) * TypeToComponentCount(type);
}

using TypeTuple = std::vector<Type>;

}  // namespace render_tree
}  // namespace gles2
}  // namespace renderer
}  // namespace entify

#endif  // _SRC_ENTIFY_RENDERER_GLES2_RENDER_TREE_TYPES_H_
