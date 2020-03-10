import os

import respire.buildlib.cc as cc
import respire.buildlib.modules as modules


def Build(registry, out_dir, platform, configured_toolchain, protobuf_modules,
          flatbuffers_modules, stdext_module, third_party_directory):
  if not os.path.exists(out_dir):
    os.makedirs(out_dir)

  renderer_modules = {}

  entify_protobuf_definitions_module = registry.SubRespire(
      BuildEntifyProtobufDefs,
      out_dir=os.path.join(out_dir, 'entify_protobuf_defs'),
      configured_toolchain=configured_toolchain,
      protobuf_modules=protobuf_modules)

  entify_flatbuffers_definitions_module = registry.SubRespire(
      BuildEntifyFlatbuffersDefs,
      out_dir=os.path.join(out_dir, 'entify_flatbuffers_defs'),
      configured_toolchain=configured_toolchain,
      flatbuffers_modules=flatbuffers_modules)

  return registry.SubRespire(
      BuildWithDeps, out_dir=out_dir, configured_toolchain=configured_toolchain,
      platform=platform,
      entify_protobuf_definitions_module=entify_protobuf_definitions_module,
      entify_flatbuffers_definitions_module=entify_flatbuffers_definitions_module,
      stdext_module=stdext_module,
      third_party_directory=third_party_directory)


def BuildWithDeps(registry, out_dir, configured_toolchain, platform,
                  entify_protobuf_definitions_module,
                  entify_flatbuffers_definitions_module,
                  stdext_module, third_party_directory):
  renderer_module = registry.SubRespireExternal(
      'gles2/build.respire.py', 'Build',
      out_dir=os.path.join(out_dir, 'gles2'),
      platform=platform,
      configured_toolchain=configured_toolchain,
      entify_protobuf_definitions_module=entify_protobuf_definitions_module,
      entify_flatbuffers_definitions_module=entify_flatbuffers_definitions_module,
      stdext_module=stdext_module,
      third_party_directory=third_party_directory)

  return {'protobuf_c_lib': entify_protobuf_definitions_module,
          'renderer': renderer_module}


def BuildEntifyProtobufDefs(
    registry, out_dir, configured_toolchain, protobuf_modules):
  # We want these files to go in to a 'entify' subdirectory so that the header
  # can be included prefixed with 'entify/'.
  include_dir = out_dir
  out_dir = os.path.join(include_dir, 'entify')
  if not os.path.exists(out_dir):
    os.makedirs(out_dir)

  protoc = protobuf_modules['protoc'].GetOutputFiles()[0]

  RENDERER_DEFINITIONS_PROTO_FILE = 'renderer_definitions.proto'
  abs_proto_path = os.path.abspath(RENDERER_DEFINITIONS_PROTO_FILE)
  abs_proto_dir = os.path.dirname(abs_proto_path)

  results = {
    'cc': os.path.join(out_dir, 'renderer_definitions.pb.cc'),
    'h': os.path.join(out_dir, 'renderer_definitions.pb.h'),
    'include_dir': include_dir,
  }

  registry.SystemCommand(
      inputs=[
        protoc,
        RENDERER_DEFINITIONS_PROTO_FILE
      ],
      outputs=[
        results['cc'],
        results['h'],
      ],
      command=[protoc, '--proto_path=' + abs_proto_dir, '--cpp_out=' + out_dir,
               RENDERER_DEFINITIONS_PROTO_FILE])

  return modules.StaticLibraryModule(
      'renderer_protobuf_definitions', registry, out_dir, configured_toolchain,
      sources=[
        results['cc'],
        results['h'],
      ],
      inherited_hard_dependencies=[results['h']],
      public_include_paths=[
        results['include_dir'],
      ],
      module_dependencies=[protobuf_modules['protobuf']])

def BuildEntifyFlatbuffersDefs(
    registry, out_dir, configured_toolchain, flatbuffers_modules):
  # We want these files to go in to a 'entify' subdirectory so that the header
  # can be included prefixed with 'entify/'.
  include_dir = out_dir
  out_dir = os.path.join(include_dir, 'entify')
  if not os.path.exists(out_dir):
    os.makedirs(out_dir)

  flatc = flatbuffers_modules['flatc'].GetOutputFiles()[0]

  RENDERER_DEFINITIONS_SCHEMA_FILE = 'renderer_definitions.fbs'
  abs_schema_path = os.path.abspath(RENDERER_DEFINITIONS_SCHEMA_FILE)
  abs_schema_dir = os.path.dirname(abs_schema_path)

  generated_header_file = os.path.join(
      out_dir, 'renderer_definitions_generated.h')

  registry.SystemCommand(
      inputs=[
        flatc,
        RENDERER_DEFINITIONS_SCHEMA_FILE
      ],
      outputs=[
        generated_header_file,
      ],
      command=[flatc, '-o', out_dir, '-c', abs_schema_path])

  julia_out_dir = os.path.join(out_dir, 'julia')
  if not os.path.exists(julia_out_dir):
    os.makedirs(julia_out_dir)

  julia_generated_file = (
      os.path.join(julia_out_dir, 'renderer_definitions_generated.jl'))
  julia_extra_files_dir = os.path.join(julia_out_dir, 'entify')

  registry.SystemCommand(
      inputs=[
        flatc,
        RENDERER_DEFINITIONS_SCHEMA_FILE
      ],
      outputs=[
        julia_generated_file,
      ],
      soft_outputs=[
        julia_extra_files_dir,
      ],
      command=[flatc, '-o', julia_out_dir, '--julia', abs_schema_path])

  flatbuffers_defs_module = modules.StaticLibraryModule(
      'renderer_flatbuffers_definitions', registry, out_dir,
      configured_toolchain,
      sources=[
        generated_header_file,
      ],
      inherited_hard_dependencies=[generated_header_file],
      public_include_paths=[
        include_dir,
      ],
      module_dependencies=[flatbuffers_modules['flatbuffers']])

  flatbuffers_defs_module.AttachPackageFile(julia_generated_file)
  flatbuffers_defs_module.AttachPackageGeneratedDirectory(julia_extra_files_dir)
  flatbuffers_defs_module.AttachPackageFile(
      '../third_party/flatbuffers/julia/src/')

  return flatbuffers_defs_module