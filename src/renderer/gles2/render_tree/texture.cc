#include "src/renderer/gles2/render_tree/texture.h"

#include "src/renderer/gles2/render.h"
#include "src/renderer/gles2/utils.h"

namespace entify {
namespace renderer {
namespace gles2 {
namespace render_tree {

namespace {
int PixelTypeBytesPerPixel(PixelType pixel_type) {
  switch (pixel_type) {
    case kPixelTypeRGB: return 3;
    case kPixelTypeRGBA: return 4;
  }

  assert(false);
  return 0;
}

GLenum ConvertToGLPixelType(PixelType pixel_type) {
  switch (pixel_type) {
    case kPixelTypeRGB: return GL_RGB;
    case kPixelTypeRGBA: return GL_RGBA;
  }

  assert(false);
  return 0;
}
}  // namespace

RenderTarget::RenderTarget(
    int width_in_pixels, int height_in_pixels,
    const std::shared_ptr<DrawTree>& draw_tree)
    : Texture(width_in_pixels, height_in_pixels) {
  GL_CALL(glGenTextures(1, &texture_handle_));
  GL_CALL(glBindTexture(GL_TEXTURE_2D, texture_handle_));
  GL_CALL(glTexImage2D(
      GL_TEXTURE_2D, 0, GL_RGBA, width_in_pixels, height_in_pixels, 0,
      GL_RGBA, GL_UNSIGNED_BYTE, NULL));

  GLuint framebuffer_handle;
  GL_CALL(glGenFramebuffers(1, &framebuffer_handle));
  GL_CALL(glBindFramebuffer(GL_FRAMEBUFFER, framebuffer_handle));
  GL_CALL(glFramebufferTexture2D(
      GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
      texture_handle_, 0));

  GLenum status;
  status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
  assert(status == GL_FRAMEBUFFER_COMPLETE);

  Render(width_in_pixels, height_in_pixels, draw_tree);

  GL_CALL(glBindFramebuffer(GL_FRAMEBUFFER, 0));
  GL_CALL(glDeleteFramebuffers(1, &framebuffer_handle));
}

RenderTarget::~RenderTarget() {
  GL_CALL(glDeleteTextures(1, &texture_handle_));
}

PixelData::PixelData(
    int width_in_pixels, int height_in_pixels, int stride_in_bytes,
    PixelType pixel_type, std::vector<char>&& data)
    : Texture(width_in_pixels, height_in_pixels),
      stride_in_bytes_(stride_in_bytes), pixel_type_(pixel_type) {
  // Only tightly packed rows are supported right now.
  assert(stride_in_bytes_ ==
             width_in_pixels * PixelTypeBytesPerPixel(pixel_type_));

  GL_CALL(glGenTextures(1, &handle_));
  GL_CALL(glBindTexture(GL_TEXTURE_2D, handle_));

  GL_CALL(glTexImage2D(GL_TEXTURE_2D, 0, ConvertToGLPixelType(pixel_type_),
                       width_in_pixels, height_in_pixels, 0,
                       ConvertToGLPixelType(pixel_type_),
                       GL_UNSIGNED_BYTE, data.data()));

  GL_CALL(glBindTexture(GL_TEXTURE_2D, 0));
}

PixelData::~PixelData() {
  GL_CALL(glDeleteTextures(1, &handle_));
}

}  // namespace render_tree
}  // namespace gles2
}  // namespace renderer
}  // namespace entify
