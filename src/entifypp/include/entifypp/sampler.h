#ifndef _SRC_ENTIFY_ENTIFYPP_RENDER_TREE_SAMPLER_H_
#define _SRC_ENTIFY_ENTIFYPP_RENDER_TREE_SAMPLER_H_

#include "entifypp/texture.h"
#include "entifypp/hash.h"
#include "entifypp/internal/type_id.h"

namespace entifypp {

class Sampler {
 public:
  enum class WrapType {
    kRepeat,
    kClamp,
  };

  enum class FilterType {
    kLinear,
    kNearest
  };

  struct Options {
    WrapType wrap_s = WrapType::kRepeat;
    WrapType wrap_t = WrapType::kRepeat;
    FilterType min_filter = FilterType::kLinear;
    FilterType mag_filter = FilterType::kLinear;

    Options() {}

    Options(WrapType wrap_type)
        : wrap_s(wrap_type), wrap_t(wrap_type) {}

    Options(FilterType filter_type)
        : min_filter(filter_type), mag_filter(filter_type) {}

    Options(WrapType wrap_type, FilterType filter_type)
        : wrap_s(wrap_type), wrap_t(wrap_type), min_filter(filter_type),
          mag_filter(filter_type) {}

    Options& set_wrap_s(WrapType wrap_type) {
      wrap_s = wrap_type;
      return *this;
    }

    Options& set_wrap_t(WrapType wrap_type) {
      wrap_t = wrap_type;
      return *this;
    }

    Options& set_min_filter(FilterType filter_type) {
      min_filter = filter_type;
      return *this;
    }

    Options& set_max_filter(FilterType filter_type) {
      mag_filter = filter_type;
      return *this;
    }
  };

  Sampler(const std::shared_ptr<Texture>& texture,
          const Options& options = Options())
      : texture_(texture), options_(options) {
    Hasher hasher;
    hasher.Add(internal::GetTypeId<Sampler>());
    hasher.Add(texture_->hash());
    hasher.Add(options_.wrap_s);
    hasher.Add(options_.wrap_t);
    hasher.Add(options_.min_filter);
    hasher.Add(options_.mag_filter);
    hash_ = hasher.Get();
  }

  const std::shared_ptr<Texture>& texture() const { return texture_; }
  const Options& options() const { return options_; }

  EntifyId hash() const { return hash_; }

 private:
  std::shared_ptr<Texture> texture_;
  Options options_;

  EntifyId hash_;
};

}  // namespace entifypp

#endif  // _SRC_ENTIFY_ENTIFYPP_RENDER_TREE_SAMPLER_H_
