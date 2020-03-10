import os
import sys

import respire.buildlib.cc as cc
import respire.buildlib.modules as modules


def MakeFlatCLibraryModule(registry, out_dir, configured_toolchain):
  return modules.StaticLibraryModule(
      'flatc_lib', registry, out_dir, configured_toolchain,
      sources = [
        'include/flatbuffers/code_generators.h',
        'include/flatbuffers/base.h',
        'include/flatbuffers/flatbuffers.h',
        'include/flatbuffers/hash.h',
        'include/flatbuffers/idl.h',
        'include/flatbuffers/util.h',
        'include/flatbuffers/reflection.h',
        'include/flatbuffers/reflection_generated.h',
        'include/flatbuffers/stl_emulation.h',
        'include/flatbuffers/flexbuffers.h',
        'include/flatbuffers/registry.h',
        'include/flatbuffers/minireflect.h',
        'src/code_generators.cpp',
        'src/idl_parser.cpp',
        'src/idl_gen_text.cpp',
        'src/reflection.cpp',
        'src/util.cpp',
      ],
      public_include_paths=['include'])


def MakeFlatCModule(registry, out_dir, configured_toolchain,
                    flatc_library_module):
  configured_toolchain.configuration.include_directories += [
    os.path.join(os.path.dirname(os.path.realpath(__file__)), 'grpc'),
  ]

  return modules.ExecutableModule(
      'flatc', registry, out_dir, configured_toolchain,
      sources = [
        'src/idl_gen_cpp.cpp',
        'src/idl_gen_dart.cpp',
        'src/idl_gen_general.cpp',
        'src/idl_gen_go.cpp',
        'src/idl_gen_js_ts.cpp',
        'src/idl_gen_julia.cpp',
        'src/idl_gen_php.cpp',
        'src/idl_gen_python.cpp',
        'src/idl_gen_lobster.cpp',
        'src/idl_gen_lua.cpp',
        'src/idl_gen_rust.cpp',
        'src/idl_gen_fbs.cpp',
        'src/idl_gen_grpc.cpp',
        'src/idl_gen_json_schema.cpp',
        'src/flatc.cpp',
        'src/flatc_main.cpp',
        'grpc/src/compiler/schema_interface.h',
        'grpc/src/compiler/cpp_generator.h',
        'grpc/src/compiler/cpp_generator.cc',
        'grpc/src/compiler/go_generator.h',
        'grpc/src/compiler/go_generator.cc',
        'grpc/src/compiler/java_generator.h',
        'grpc/src/compiler/java_generator.cc',
      ],
      module_dependencies=[flatc_library_module])


def ModifyConfigurationForFlatbuffers(configured_toolchain):
  return configured_toolchain
  WARNINGS_TO_DISABLE = [
    cc.Configuration.WARNING_SIGNED_UNSIGNED_MISMATCH,
    cc.Configuration.WARNING_POSSIBLE_LOSS_OF_DATA_WHEN_CONVERTING,
    cc.Configuration.WARNING_NO_DEFINITION_FOR_INLINE_FUNCTION,
    cc.Configuration.WARNING_UNARY_MINUS_APPLIED_TO_UNSIGNED_TYPE,
    cc.Configuration.WARNING_SWITCH_STATEMENT_CONTAINS_ONLY_DEFAULT,
    cc.Configuration.WARNING_FORCING_VALUE_TO_BOOL,
    cc.Configuration.WARNING_UNKNOWN_ATTRIBUTE,
    cc.Configuration.WARNING_IGNORE_MISSING_OVERRIDE,
  ]

  for warning in WARNINGS_TO_DISABLE:
    configured_toolchain.configuration.warning_toggles[warning] = False  

  # Only Windows and pthread platforms are supported currently, and all
  # HAVE_PTHREAD checks are done after Win32 checks.  
  configured_toolchain.configuration.defines += ['HAVE_PTHREAD']

  return configured_toolchain


def Build(registry, out_dir, configured_toolchain, host_configured_toolchain):
  if not os.path.exists(out_dir):
    os.makedirs(out_dir)

  host_out_dir = os.path.join(out_dir, 'host')
  if not os.path.exists(host_out_dir):
    os.makedirs(host_out_dir)

  flatbuffers_modules = {}

  configured_toolchain = ModifyConfigurationForFlatbuffers(configured_toolchain)
  host_configured_toolchain = ModifyConfigurationForFlatbuffers(
      host_configured_toolchain)

  flatc_library_module = MakeFlatCLibraryModule(
      registry, out_dir, host_configured_toolchain)
  flatbuffers_modules['flatc'] = registry.SubRespire(
      MakeFlatCModule, out_dir=out_dir,
      configured_toolchain=host_configured_toolchain,
      flatc_library_module=flatc_library_module)

  # Dummy empty module that will be used to setup include paths.  Flatbuffers
  # does not need to compile anything for runtime support, it's all in headers.
  flatbuffers_modules['flatbuffers'] = modules.StaticLibraryModule(
      'flatbuffers_lib', registry, out_dir, configured_toolchain,
      sources = [],
      public_include_paths=['include'])

  return flatbuffers_modules
