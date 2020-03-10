#ifndef _SRC_ENTIFY_ENTIFYPP_RENDER_TREE_TEXTURE_H_
#define _SRC_ENTIFY_ENTIFYPP_RENDER_TREE_TEXTURE_H_

#include <vector>

#include "entifypp/hash.h"
#include "entifypp/internal/type_id.h"

namespace entifypp {

enum PixelType {
  kPixelTypeRGB,
  kPixelTypeRGBA,
};

class ReferenceTexture;
class RenderTargetTexture;
class PixelDataTexture;

class TextureVisitor {
 public:
  virtual void Visit(const ReferenceTexture* reference_texture) = 0;
  virtual void Visit(const RenderTargetTexture* render_target) = 0;
  virtual void Visit(const PixelDataTexture* pixel_data) = 0;
};

class Texture {
 public:
  virtual ~Texture() {}

  EntifyId hash() const { return hash_; }

  virtual void Accept(TextureVisitor* visitor) const = 0;

 protected:
  EntifyId hash_;
};

class ReferenceTexture : public Texture {
 public:
  ~ReferenceTexture() {
    EntifyReleaseReference(context_, reference_);
  }

  void Accept(TextureVisitor* visitor) const override {
    visitor->Visit(this);
  }

  EntifyReference reference() const { return reference_; }

 private:
  ReferenceTexture(
      EntifyContext context, EntifyReference reference, EntifyId hash) {
    context_ = context;
    hash_ = hash;
    reference_ = reference;
  }

  EntifyContext context_;
  EntifyReference reference_;

  friend class Context;
};

class RenderTargetTexture : public Texture {
 public:
  RenderTargetTexture(int width_in_pixels, int height_in_pixels,
               std::shared_ptr<DrawTree> draw_tree)
      : width_in_pixels_(width_in_pixels), height_in_pixels_(height_in_pixels),
        draw_tree_(draw_tree) {
    Hasher hasher;
    hasher.Add(internal::GetTypeId<RenderTargetTexture>());
    hasher.Add(width_in_pixels_);
    hasher.Add(height_in_pixels_);
    hasher.Add(draw_tree_->hash());
    hash_ = hasher.Get();
  }

  int width_in_pixels() const { return width_in_pixels_; }
  int height_in_pixels() const { return height_in_pixels_; }
  const std::shared_ptr<DrawTree>& draw_tree() const { return draw_tree_; }

  void Accept(TextureVisitor* visitor) const override {
    visitor->Visit(this);
  }

 private:
  int width_in_pixels_;
  int height_in_pixels_;
  std::shared_ptr<DrawTree> draw_tree_;
};

class PixelDataTexture : public Texture {
 public:
  PixelDataTexture(
      std::vector<uint8_t>&& pixel_data, int width_in_pixels,
      int height_in_pixels, int stride_in_bytes, PixelType pixel_type)
      : pixel_data_(std::move(pixel_data)),
        width_in_pixels_(width_in_pixels), height_in_pixels_(height_in_pixels),
        stride_in_bytes_(stride_in_bytes), pixel_type_(pixel_type) {
    Hasher hasher;
    hasher.Add(internal::GetTypeId<PixelDataTexture>());
    hasher.Add(pixel_data_);
    hasher.Add(width_in_pixels_);
    hasher.Add(height_in_pixels_);
    hasher.Add(stride_in_bytes_);
    hasher.Add(pixel_type_);
    hash_ = hasher.Get();
  }

  const std::vector<uint8_t>& pixel_data() const { return pixel_data_; }
  int width_in_pixels() const { return width_in_pixels_; }
  int height_in_pixels() const { return height_in_pixels_; }
  int stride_in_bytes() const { return stride_in_bytes_; }
  PixelType pixel_type() const { return pixel_type_; }

  void Accept(TextureVisitor* visitor) const override {
    visitor->Visit(this);
  }

 private:
  std::vector<uint8_t> pixel_data_;
  int width_in_pixels_;
  int height_in_pixels_;
  int stride_in_bytes_;
  PixelType pixel_type_;
};

}  // namespace entifypp

#endif  // _SRC_ENTIFY_ENTIFYPP_RENDER_TREE_TEXTURE_H_
