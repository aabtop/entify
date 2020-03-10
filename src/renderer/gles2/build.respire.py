import glob
import os

import respire.buildlib.cc as cc
import respire.buildlib.copy_files as copy_files
import respire.buildlib.modules as modules


def MakeRaspiRendererModule(
    module_title, registry, out_dir, configured_toolchain, sources,
    module_dependencies):
  return modules.StaticLibraryModule(
      module_title, registry, out_dir, configured_toolchain,
      sources=sources,
      system_libraries=[
        'brcmEGL',
        'brcmGLESv2',
        'bcm_host',
        'vcos',
        'vchiq_arm',
      ],
      module_dependencies=module_dependencies)


def MakeLinuxRendererModule(
    module_title, registry, out_dir, configured_toolchain, sources,
    module_dependencies, use_egldevice):
  if use_egldevice:
    configured_toolchain.configuration.defines += ['USE_EGLDEVICE']
    sources += [
      'egl_device_interface.cc',
      'egl_device_interface.h',
    ]

  return modules.StaticLibraryModule(
      module_title, registry, out_dir, configured_toolchain,
      sources=sources,
      system_libraries=[
        'EGL',
        'GLESv2',
      ],
      module_dependencies=module_dependencies)


def MakeWin32RendererModule(
    module_title, registry, out_dir, configured_toolchain, sources,
    module_dependencies, third_party_directory):

  configured_toolchain.configuration.include_directories += [
    os.path.join(third_party_directory, 'angle/include'),
  ]

  renderer_module = modules.StaticLibraryModule(
      module_title, registry, out_dir, configured_toolchain,
      sources = sources,
      static_libraries=[
        os.path.join(third_party_directory, 'angle/lib/libEGL.lib'),
        os.path.join(third_party_directory, 'angle/lib/libGLESv2.lib'),
      ],
      system_libraries=[
        'User32',
        'Gdi32',
      ],
      module_dependencies=module_dependencies)

  for dll_file in glob.glob(third_party_directory + '/angle/lib/*.dll'):
    renderer_module.AttachPackageFile(dll_file)

  return renderer_module


def Build(registry, out_dir, platform, configured_toolchain,
          entify_protobuf_definitions_module,
          entify_flatbuffers_definitions_module,
          stdext_module, third_party_directory):
  if not os.path.exists(out_dir):
    os.makedirs(out_dir)

  MODULE_TITLE = 'renderer_gles2'
  COMMON_SOURCES = [
    'backend.cc',
    'backend.h',
    'parse_protobuf.cc',
    'parse_protobuf.h',
    'parse_flatbuffer.cc',
    'parse_flatbuffer.h',
    'lookup_utils.h',
    'render.cc',
    'render.h',
    'render_tree/draw_call.cc',
    'render_tree/draw_call.h',
    'render_tree/draw_sequence.h',
    'render_tree/draw_tree.h',
    'render_tree/fragment_shader.cc',
    'render_tree/fragment_shader.h',
    'render_tree/program.cc',
    'render_tree/program.h',
    'render_tree/sampler.h',
    'render_tree/texture.cc',
    'render_tree/texture.h',
    'render_tree/types.h',
    'render_tree/uniform_values.h',
    'render_tree/vertex_buffer.cc',
    'render_tree/vertex_buffer.h',
    'render_tree/vertex_shader.cc',
    'render_tree/vertex_shader.h',
    'utils.cc',
    'utils.h',
    'window_render_target.cc',
    'window_render_target.h',
  ]
  MODULE_DEPENDENCIES = [
    entify_protobuf_definitions_module,
    entify_flatbuffers_definitions_module,
    stdext_module,
  ]

  if platform == 'win32':
    return MakeWin32RendererModule(
        MODULE_TITLE, registry, out_dir, configured_toolchain, COMMON_SOURCES,
        MODULE_DEPENDENCIES, third_party_directory)
  elif platform == 'raspi':
    return MakeRaspiRendererModule(
        MODULE_TITLE, registry, out_dir, configured_toolchain, COMMON_SOURCES,
        MODULE_DEPENDENCIES)
  elif platform == 'jetson' or platform == 'linux':
    use_egldevice = (platform == 'jetson')
    return MakeLinuxRendererModule(
        MODULE_TITLE, registry, out_dir, configured_toolchain, COMMON_SOURCES,
        MODULE_DEPENDENCIES, use_egldevice)
  else:
    raise Exception('Unsupported platform.')
