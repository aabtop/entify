import os

import respire.buildlib.cc as cc
import respire.buildlib.modules as modules

def Build(registry, out_dir, configured_toolchain):
  if not os.path.exists(out_dir):
    os.makedirs(out_dir)

  glm_module = modules.StaticLibraryModule(
      'glm', registry, out_dir, configured_toolchain,
      sources = [],
      public_include_paths=[
        'glm',
      ])

  return glm_module
