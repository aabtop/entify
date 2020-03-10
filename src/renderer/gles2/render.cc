#include "src/renderer/gles2/render.h"

#include <cassert>

#include "src/renderer/gles2/render_tree/draw_call.h"
#include "src/renderer/gles2/render_tree/draw_sequence.h"
#include "src/renderer/gles2/render_tree/fragment_shader.h"
#include "src/renderer/gles2/render_tree/program.h"
#include "src/renderer/gles2/render_tree/types.h"
#include "src/renderer/gles2/render_tree/uniform_values.h"
#include "src/renderer/gles2/render_tree/vertex_buffer.h"
#include "src/renderer/gles2/render_tree/vertex_shader.h"
#include "src/renderer/gles2/utils.h"

namespace entify {
namespace renderer {
namespace gles2 {

namespace {

void SetBlendOptions(const render_tree::Pipeline::Params::Blend& blend) {
  if (blend.src_color == GL_ONE && blend.dst_color == GL_ZERO
      && blend.src_alpha == GL_ONE && blend.dst_alpha == GL_ZERO) {
    GL_CALL(glDisable(GL_BLEND));  
  } else {
    GL_CALL(glEnable(GL_BLEND));  
    GL_CALL(glBlendFuncSeparate(GL_ONE, GL_ONE_MINUS_SRC_ALPHA,
                                GL_ONE, GL_ONE_MINUS_SRC_ALPHA));
  }
}

void UseProgram(const std::shared_ptr<render_tree::Program>& program) {
  GL_CALL(glUseProgram(program->handle()));
}

void WriteUniformData(render_tree::Type type, GLint location, const char* data) {
  switch(type) {
    case render_tree::TypeFloat32V1: {
      GL_CALL(glUniform1fv(
          location, 1, reinterpret_cast<const GLfloat*>(data)));
    } break;
    case render_tree::TypeFloat32V2: {
      GL_CALL(glUniform2fv(
          location, 1, reinterpret_cast<const GLfloat*>(data)));
    } break;
    case render_tree::TypeFloat32V3: {
      GL_CALL(glUniform3fv(
          location, 1, reinterpret_cast<const GLfloat*>(data)));
    } break;
    case render_tree::TypeFloat32V4: {
      GL_CALL(glUniform4fv(
          location, 1, reinterpret_cast<const GLfloat*>(data)));
    } break;
    case render_tree::TypeFloat32M44: {
      GL_CALL(glUniformMatrix4fv(
          location, 1, GL_FALSE, reinterpret_cast<const GLfloat*>(data)));
    } break;
    default: {
      assert(false);
    }
  }
}

void SetSampler(
    GLint location, int sampler_index,
    const std::shared_ptr<render_tree::Sampler>& sampler) {
  GL_CALL(glActiveTexture(GL_TEXTURE0 + sampler_index));
  GL_CALL(glBindTexture(GL_TEXTURE_2D, sampler->texture()->handle()));

  GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, sampler->wrap_s()));
  GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, sampler->wrap_t()));
  GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
                          sampler->min_filter()));
  GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER,
                          sampler->mag_filter()));

  GL_CALL(glUniform1i(location, sampler_index));
}

void WriteUniformsData(
    const render_tree::TypeTuple& types,
    const std::vector<GLint>& locations, const char* data,
    const std::vector<std::shared_ptr<render_tree::Sampler>>& samplers) {
  int data_offset = 0;
  int sampler_index = 0;
  for (size_t i = 0; i < types.size(); ++i) {
    const render_tree::Type& type = types[i];

    if (type == render_tree::TypeSampler) {
      SetSampler(locations[i], sampler_index, samplers[sampler_index]);
      ++sampler_index;
    } else {
      WriteUniformData(type, locations[i], data + data_offset);
      data_offset += TypeToSize(type);
    }
  }
}

void SetVertexShaderUniforms(
    const std::vector<GLint>& locations,
    const std::shared_ptr<render_tree::UniformValues>& vertex_shader_uniforms) {
  if (!vertex_shader_uniforms) {
    return;
  }

  WriteUniformsData(vertex_shader_uniforms->types(), 
                    locations,
                    vertex_shader_uniforms->data().data(),
                    vertex_shader_uniforms->samplers());
}

void SetFragmentShaderUniforms(
    const std::vector<GLint>& locations,
    const std::shared_ptr<render_tree::UniformValues>&
        fragment_shader_uniforms) {
  if (!fragment_shader_uniforms) {
    return;
  }

  WriteUniformsData(fragment_shader_uniforms->types(), 
                    locations,
                    fragment_shader_uniforms->data().data(),
                    fragment_shader_uniforms->samplers());
}

void SetVertexBuffer(
    const std::vector<GLint>& indices,
    const std::shared_ptr<render_tree::VertexBuffer>& vertex_buffer) {
  GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer->handle()));

  const render_tree::TypeTuple& types = vertex_buffer->types();
  const std::vector<int32_t>& data_offsets = vertex_buffer->data_offsets();
  for (size_t i = 0; i < types.size(); ++i) {
    GL_CALL(glEnableVertexAttribArray(indices[i]));
    GL_CALL(glVertexAttribPointer(
        indices[i], TypeToComponentCount(types[i]), TypeToGLType(types[i]),
        GL_FALSE, vertex_buffer->stride_in_bytes(),
        reinterpret_cast<void*>(data_offsets[i])));
  }
}

void TransitionToGLState(
    const render_tree::DrawCall* previous_draw_call,
    const render_tree::DrawCall* draw_call) {
  bool pipeline_dirty = !previous_draw_call ||
                        previous_draw_call->pipeline() != draw_call->pipeline();

  bool blend_options_dirty =
      pipeline_dirty && (!previous_draw_call
          || previous_draw_call->pipeline()->params().blend
                 != draw_call->pipeline()->params().blend);

  if (blend_options_dirty) {
    SetBlendOptions(draw_call->pipeline()->params().blend);
  }

  bool program_dirty =
      pipeline_dirty && (!previous_draw_call
          || previous_draw_call->pipeline()->program()->vertex_shader()
                  != draw_call->pipeline()->program()->vertex_shader()
          || previous_draw_call->pipeline()->program()->fragment_shader()
                  != draw_call->pipeline()->program()->fragment_shader());

  if (program_dirty) {
    UseProgram(draw_call->pipeline()->program());
  }

  if (program_dirty ||
      previous_draw_call->vertex_uniform_values() !=
          draw_call->vertex_uniform_values()) {
    SetVertexShaderUniforms(
        draw_call->pipeline()->program()->vertex_uniform_locations(),
        draw_call->vertex_uniform_values());
  }

  if (program_dirty ||
      previous_draw_call->fragment_uniform_values() !=
      draw_call->fragment_uniform_values()) {
    SetFragmentShaderUniforms(
        draw_call->pipeline()->program()->fragment_uniform_locations(),
        draw_call->fragment_uniform_values());
  }

  if (!previous_draw_call ||
      previous_draw_call->vertex_buffer() != draw_call->vertex_buffer()) {
    SetVertexBuffer(
        draw_call->pipeline()->program()->vertex_attribute_indices(),
        draw_call->vertex_buffer());
  }
}

const render_tree::DrawCall* RenderDrawTree(
    const render_tree::DrawCall* previous_draw_call,
    const render_tree::DrawTree* draw_tree);

const render_tree::DrawCall* RenderDrawCall(
    const render_tree::DrawCall* previous_draw_call,
    const render_tree::DrawCall* draw_call) {
  TransitionToGLState(previous_draw_call, draw_call);

  GL_CALL(glDrawArrays(
      GL_TRIANGLES, 0, draw_call->vertex_buffer()->num_vertices()));

  return draw_call;
}

const render_tree::DrawCall* RenderDrawSequence(
    const render_tree::DrawCall* previous_draw_call,
    const render_tree::DrawSequence* draw_sequence) {
  const render_tree::DrawCall* last_draw_call = previous_draw_call;

  for (const auto& draw_tree : draw_sequence->sequence()) {
    last_draw_call = RenderDrawTree(last_draw_call, draw_tree.get());
  }

  return last_draw_call;
}

const render_tree::DrawCall* RenderDrawTree(
    const render_tree::DrawCall* previous_draw_call,
    const render_tree::DrawTree* draw_tree) {
  switch (draw_tree->type()) {
    case render_tree::DrawTree::kTypeDrawCall: {
      return RenderDrawCall(
          previous_draw_call,
          static_cast<const render_tree::DrawCall*>(draw_tree));
    } break;
    case render_tree::DrawTree::kTypeDrawSequence: {
      return RenderDrawSequence(
          previous_draw_call,
          static_cast<const render_tree::DrawSequence*>(draw_tree));
    } break;
    case render_tree::DrawTree::kTypeDrawSet: {
      assert(false);  // Not implemented.
    } break;
  }

  return nullptr;
}
}  // namespace

void Render(int width, int height,
            const std::shared_ptr<render_tree::DrawTree>& draw_tree) {
  GL_CALL(glViewport(0, 0, width, height));
  GL_CALL(glScissor(0, 0, width, height));
  GL_CALL(glClearColor(0.0f, 0.0f, 0.0f, 0.0f));
  GL_CALL(glClear(GL_COLOR_BUFFER_BIT));

  RenderDrawTree(nullptr, draw_tree.get());
}

}  // namespace gles2
}  // namespace renderer
}  // namespace entify
