import os

import respire.buildlib.cc as cc
import respire.buildlib.modules as modules

def Build(registry, out_dir, configured_toolchain, entify_module, libb2_module,
          protobuf_module):
  if not os.path.exists(out_dir):
    os.makedirs(out_dir)

  glm_module = registry.SubRespireExternal(
      '../third_party/glm/build.respire.py', 'Build',
      out_dir = os.path.join(out_dir, 'glm'),
      configured_toolchain=configured_toolchain)

  return registry.SubRespire(
      BuildWithDeps, out_dir=out_dir, configured_toolchain=configured_toolchain,
      entify_module=entify_module, libb2_module=libb2_module,
      glm_module=glm_module, protobuf_module=protobuf_module)

def BuildWithDeps(
    registry, out_dir, configured_toolchain, entify_module, libb2_module,
    glm_module, protobuf_module):

  configured_toolchain.configuration.include_directories += [
      os.path.dirname(os.path.realpath(__file__)),
  ]

  entifypp_module = modules.StaticLibraryModule(
      'entifypp', registry, out_dir, configured_toolchain,
      sources = [
        'entifypp.cc',
        'include/entifypp/draw_call.h',
        'include/entifypp/draw_sequence.h',
        'include/entifypp/draw_set.h',
        'include/entifypp/draw_tree.h',
        'include/entifypp/entifypp.h',
        'include/entifypp/glsl_fragment_shader.h',
        'include/entifypp/glsl_vertex_shader.h',
        'include/entifypp/hash.h',
        'include/entifypp/pipeline.h',
        'include/entifypp/types.h',
        'include/entifypp/uniform_values.h',
        'include/entifypp/vertex_buffer.h',
        'submit_to_protobuf.cc',
        'submit_to_protobuf.h',
      ],
      public_include_paths=[
        'include',
      ],
      module_dependencies=[
        entify_module,
        libb2_module,
        glm_module,
        protobuf_module,
      ])

  return entifypp_module
