import copy
import os
import sys

import respire.buildlib.cc as cc
import respire.buildlib.cc_toolchains.raspi as raspi
import respire.buildlib.cc_toolchains.discovery as cc_discovery
import respire.buildlib.modules as modules
import respire.buildlib.run_with_timestamp as run_with_timestamp


def EntryPoint(registry, out_dir, config, platform):
  if not os.path.exists(out_dir):
    os.makedirs(out_dir)

  if platform == 'host':
    platform = sys.platform

  host_toolchain = cc_discovery.DiscoverHostToolchain()

  if platform == 'raspi':
    toolchain = raspi.RaspberryPiToolchain()
  else:
    toolchain = host_toolchain

  if config == 'debug':
    out_dir = os.path.join(out_dir, 'debug')
    configuration = cc.Configuration(optimize='None', include_symbols=True)
  elif config == 'release':
    out_dir = os.path.join(out_dir, 'release')
    configuration = cc.Configuration(optimize='Maximum', include_symbols=False)

  configured_toolchain = cc.ToolchainWithConfiguration(toolchain, configuration)

  host_configuration = cc.Configuration(
      optimize='Maximum', include_symbols=False)
  host_configured_toolchain = cc.ToolchainWithConfiguration(
      host_toolchain, host_configuration)

  stdext_modules = registry.SubRespireExternal(
      'stdext/src/build.respire.py', 'Build',
      out_dir=os.path.join(out_dir, 'stdext'),
      configured_toolchain=configured_toolchain,
      platform=platform)

  entify_modules = registry.SubRespire(
      Build, out_dir=os.path.join(out_dir, 'entify'), platform=platform,
      configured_toolchain=configured_toolchain,
      host_configured_toolchain=host_configured_toolchain,
      stdext_modules=stdext_modules)

  registry.SubRespire(StartBuilds, build_modules=entify_modules)
  registry.SubRespire(
      StartPackageBuild, out_dir=os.path.join(out_dir, 'package'),
      build_modules=entify_modules)


def Build(registry, out_dir, platform, configured_toolchain,
          host_configured_toolchain, stdext_modules):
  if not os.path.exists(out_dir):
    os.makedirs(out_dir)

  protobuf_modules = registry.SubRespireExternal(
      'third_party/protobuf/build.respire.py', 'Build',
      out_dir=os.path.join(out_dir, 'protobuf'),
      configured_toolchain=configured_toolchain,
      host_configured_toolchain=host_configured_toolchain)

  flatbuffers_modules = registry.SubRespireExternal(
      'third_party/flatbuffers/build.respire.py', 'Build',
      out_dir=os.path.join(out_dir, 'flatbuffers'),
      configured_toolchain=configured_toolchain,
      host_configured_toolchain=host_configured_toolchain)

  platform_window_modules = registry.SubRespireExternal(
      'platform_window/build.respire.py', 'Build',
      out_dir=os.path.join(out_dir, 'platform_window'),
      platform=platform,
      configured_toolchain=configured_toolchain,
      stdext_modules=stdext_modules)

  # Setup the entify root include directory for all subsequent build targets.
  configured_toolchain.configuration.include_directories += [
    os.path.join(os.path.dirname(os.path.realpath(__file__)), os.pardir)
  ]

  renderer_modules = registry.SubRespireExternal(
      'renderer/build.respire.py', 'Build',
      out_dir=os.path.join(out_dir, 'renderer'),
      platform=platform,
      configured_toolchain=configured_toolchain,
      protobuf_modules=protobuf_modules,
      flatbuffers_modules=flatbuffers_modules,
      stdext_module=stdext_modules['stdext_lib'],
      third_party_directory=os.path.abspath('third_party'))

  entify_modules = registry.SubRespire(
      BuildLibWithDeps, out_dir=out_dir,
      configured_toolchain=configured_toolchain,
      platform_window_modules=platform_window_modules,
      renderer_modules=renderer_modules,
      stdext_module=stdext_modules['stdext_lib'])

  entify_modules = registry.SubRespire(
      AddEntifyDemoToModules, out_dir=out_dir,
      configured_toolchain=configured_toolchain,
      entify_modules=entify_modules)

  entify_modules = registry.SubRespire(
      AddEntifyppDemoToModules, out_dir=out_dir,
      configured_toolchain=configured_toolchain,
      entify_modules=entify_modules)

  return entify_modules


def BuildLibWithDeps(
    registry, out_dir, configured_toolchain, platform_window_modules,
    renderer_modules, stdext_module):
  entify_modules = {}

  export_configured_toolchain = copy.deepcopy(configured_toolchain)
  export_configured_toolchain.configuration.defines += ['EXPORT_ENTIFY']

  libb2_module = registry.SubRespireExternal(
      'third_party/libb2/build.respire.py', 'Build',
      out_dir=os.path.join(out_dir, 'libb2'),
      configured_toolchain=configured_toolchain)

  entify_modules['entify'] = registry.SubRespire(
      BuildEntify, out_dir=out_dir,
      configured_toolchain=export_configured_toolchain,
      platform_window_module=platform_window_modules['platform_window'],
      renderer_module=renderer_modules['renderer'],
      stdext_module=stdext_module, libb2_module=libb2_module)

  entify_modules['entifypp'] = registry.SubRespireExternal(
      'entifypp/build.respire.py', 'Build',
      out_dir=os.path.join(out_dir, 'entifypp'),
      configured_toolchain=configured_toolchain,
      entify_module=entify_modules['entify'],
      libb2_module=libb2_module,
      protobuf_module=renderer_modules['protobuf_c_lib'])

  entify_modules['renderer_protobuf_defs'] = renderer_modules['protobuf_c_lib']

  return entify_modules


def BuildEntify(
    registry, out_dir, configured_toolchain, platform_window_module,
    renderer_module, stdext_module, libb2_module):
  entify_module = modules.SharedLibraryModule(
      'entify', registry, out_dir, configured_toolchain,
      sources=[
          'entify.cc',
          'include/entify/entify.h',
          'include/entify/registry.h',
          'registry.cc',
          'context.cc',
          'context.h',
          'backend.h',
          'external_reference.h',
      ],
      public_include_paths=[
          'include',
      ],
      module_dependencies=[
          platform_window_module,
          renderer_module,
          stdext_module,
          libb2_module
      ])

  entify_module.AttachPackageFile('julia/')

  return entify_module


def AddEntifyDemoToModules(
    registry, out_dir, configured_toolchain, entify_modules):
  out_dir = os.path.join(out_dir, 'entify_demo')
  if not os.path.exists(out_dir):
    os.makedirs(out_dir)

  entify_demo_module = modules.ExecutableModule(
      'entify_demo', registry, out_dir, configured_toolchain,
      sources = [
        'demo/main.cc',
      ],
      module_dependencies=[
        entify_modules['entify'],
        entify_modules['renderer_protobuf_defs']
      ])

  entify_modules['entify_demo'] = entify_demo_module

  return entify_modules


def AddEntifyppDemoToModules(
    registry, out_dir, configured_toolchain, entify_modules):
  out_dir = os.path.join(out_dir, 'entifypp_demo')
  if not os.path.exists(out_dir):
    os.makedirs(out_dir)
  entify_demo_module = modules.ExecutableModule(
      'entifypp_demo', registry, out_dir, configured_toolchain,
      sources = [
        'demo/entifypp_main.cc',
      ],
      module_dependencies=[entify_modules['entifypp']])

  entify_modules['entifypp_demo'] = entify_demo_module

  return entify_modules


def StartBuilds(registry, build_modules):
  for build_module in build_modules.values():
    for output_file in build_module.GetOutputFiles():
      registry.Build(output_file)

def GetEntifySharedModule(registry, entify_modules):
  return entify_modules['entify']


def Package(registry, out_dir, module):
  if not os.path.exists(out_dir):
    os.makedirs(out_dir)

  return module.CopyPackageFiles(out_dir, registry)

def StartPackageBuild(registry, out_dir, build_modules):
  entify_package_stamp = registry.SubRespire(
      Package, out_dir=out_dir, module=build_modules['entify'])
  registry.SubRespire(BuildPackage, output_file=entify_package_stamp)


def BuildPackage(registry, output_file):
  registry.Build(output_file)
