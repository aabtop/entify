#ifndef _SRC_ENTIFY_RENDERER_GLES2_RENDER_TREE_TEXTURE_H_
#define _SRC_ENTIFY_RENDERER_GLES2_RENDER_TREE_TEXTURE_H_

#include <memory>
#include <vector>

#include <GLES2/gl2.h>

#include "src/renderer/gles2/render_tree/draw_tree.h"
#include "src/renderer/gles2/render_tree/types.h"

namespace entify {
namespace renderer {
namespace gles2 {
namespace render_tree {

enum PixelType {
  kPixelTypeRGB,
  kPixelTypeRGBA,
};

class Texture {
 public:
  Texture(int width_in_pixels, int height_in_pixels)
      : width_in_pixels_(width_in_pixels),
        height_in_pixels_(height_in_pixels) {}
  virtual ~Texture() {}

  int width_in_pixels() const { return width_in_pixels_; }
  int height_in_pixels() const { return height_in_pixels_; }

  virtual GLuint handle() const = 0;

 private:
  int width_in_pixels_;
  int height_in_pixels_;
};

class RenderTarget : public Texture {
 public:
  RenderTarget(int width_in_pixels, int height_in_pixels,
               const std::shared_ptr<DrawTree>& draw_tree);
  ~RenderTarget();

  GLuint handle() const override { return texture_handle_; }

 private:
  GLuint texture_handle_;
};

class PixelData : public Texture {
 public:
  PixelData(int width_in_pixels, int height_in_pixels, int stride_in_bytes,
          PixelType pixel_type, std::vector<char>&& data);
  ~PixelData();

  int stride_in_bytes() const { return stride_in_bytes_; }
  PixelType pixel_type() const { return pixel_type_; }
  GLuint handle() const override { return handle_; }

 private:
  int stride_in_bytes_;
  PixelType pixel_type_;
  GLuint handle_;
};

}  // namespace render_tree
}  // namespace gles2
}  // namespace renderer
}  // namespace entify

#endif  // _SRC_ENTIFY_RENDERER_GLES2_RENDER_TREE_TEXTURE_H_
