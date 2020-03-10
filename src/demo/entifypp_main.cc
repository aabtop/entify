#include <atomic>

#include "entifypp/entifypp.h"
#include "glm.hpp"

namespace {

std::shared_ptr<entifypp::Sampler> GetSampler() {
  const int kWidthInPixels = 256;
  const int kHeightInPixels = 256;
  const int kPixelSizeInBytes = 4;

  const int kStrideInBytes = kWidthInPixels * kPixelSizeInBytes;
  const int kSizeInBytes = kStrideInBytes * kHeightInPixels;

  std::vector<uint8_t> data(kSizeInBytes);

  for (int i = 0; i < kHeightInPixels; ++i) {
    for (int j = 0; j < kWidthInPixels; ++j) {
      int byte_offset = i * kStrideInBytes + j * kPixelSizeInBytes;
      data[byte_offset + 0] = static_cast<uint8_t>(
          255.0f * static_cast<float>(i) / kHeightInPixels);
      data[byte_offset + 1] = static_cast<uint8_t>(
          255.0f * static_cast<float>(j) / kWidthInPixels);
      data[byte_offset + 2] = 0;
      data[byte_offset + 3] = 255;
    }
  }

  return std::make_shared<entifypp::Sampler>(
      std::make_shared<entifypp::Texture>(
          std::move(data), kWidthInPixels, kHeightInPixels, kStrideInBytes,
          entifypp::kPixelTypeRGBA));
}

std::shared_ptr<entifypp::DrawTree> GetTriangle(
    const glm::vec2& translation) {
  using VertexInputType = std::tuple<glm::vec3, glm::vec2>;
  using FragmentInputType = std::tuple<glm::vec2>;
  using VertexUniformType = std::tuple<glm::vec2>;
  using FragmentUniformType = std::tuple<std::shared_ptr<entifypp::Sampler>>;

  return std::make_shared<entifypp::DrawCall>(
      std::make_shared<entifypp::Pipeline<
          VertexInputType, FragmentInputType,
          VertexUniformType, FragmentUniformType>>(
          std::make_shared<entifypp::GLSLVertexShader<
              VertexInputType, FragmentInputType, VertexUniformType>>(
                  std::vector<std::string>{"a_position", "a_texcoords"},
                  std::vector<std::string>{"u_translation"},
                  "attribute vec3 a_position;"
                  "attribute vec2 a_texcoords;"
                  ""
                  "varying vec2 v_texcoords;"
                  ""
                  "uniform vec2 u_translation;"
                  ""
                  "void main() {  "
                  "  v_texcoords = a_texcoords;"
                  "  gl_Position ="
                  "      vec4(a_position.xy + u_translation, a_position.z, 1);"
                  "}"),
          std::make_shared<entifypp::GLSLFragmentShader<
              FragmentInputType, FragmentUniformType>>(
                  std::vector<std::string>{"u_texture"},
                  "precision highp float;"
                  "varying vec2 v_texcoords;"
                  ""
                  "uniform sampler2D u_texture;"
                  ""
                  "void main() {  "
                  "  gl_FragColor = texture2D(u_texture, v_texcoords);"
                  "}")),
      std::make_shared<entifypp::VertexBuffer<VertexInputType>>(
          std::vector<VertexInputType>{
              VertexInputType{{-0.5f, -0.5f, 0.0f}, {0.0f, 0.0f}},
              VertexInputType{{0.5f, -0.5f, 0.0f}, {1.0f, 0.0f}},
              VertexInputType{{0.0f, 0.5f, 0.0f}, {0.0f, 1.0f}},
          }),
      std::make_shared<entifypp::UniformValues<VertexUniformType>>(
          translation),
      std::make_shared<entifypp::UniformValues<FragmentUniformType>>(
          GetSampler()));

}

std::shared_ptr<entifypp::DrawTree> GetDrawTree() {
  return std::make_shared<entifypp::DrawSequence>(
      std::vector<std::shared_ptr<entifypp::DrawTree>>{
          GetTriangle(glm::vec2(-0.25f, 0.0f)),
          GetTriangle(glm::vec2(0.25f, 0.0f)),
          GetTriangle(glm::vec2(0.0f, 0.25f)),
  });
}

}  // namespace

int main(int argc, const char** args) {
  entifypp::Context context;

  std::atomic_bool quit_flag(false);
  entifypp::DefaultPlatformWindow window(
      "Entify Demo", [&quit_flag](PlatformWindowEvent event, void* data) {
        if (event == kPlatformWindowEventQuitRequest) {
          quit_flag.store(true);
        }
      });

  entifypp::RenderTarget render_target(context, window);

  while(!quit_flag.load()) {
    context.Submit(&render_target, GetDrawTree());
  }

  return 0;
}
