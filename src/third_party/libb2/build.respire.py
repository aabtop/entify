import os

import respire.buildlib.cc as cc
import respire.buildlib.modules as modules


def Build(registry, out_dir, configured_toolchain):
  if not os.path.exists(out_dir):
    os.makedirs(out_dir)

#  configured_toolchain.configuration.include_directories += [
#      os.path.dirname(os.path.realpath(__file__)),
#  ]

  configured_toolchain.configuration.defines += [
    'HAVE_SSE2',
    'SUFFIX=',
    'BLAKE2_DLL_EXPORTS',
    'BLAKE2_DLL',
  ]

  libb2_module = modules.SharedLibraryModule(
      'blake2', registry, out_dir, configured_toolchain,
      sources=[
        'src/blake2s-ref.c',
        'src/blake2b-ref.c',
        'src/blake2.h',
        'src/blake2-impl.h',
        'src/blake2sp.c',
        'src/blake2bp.c',
        'src/blake2-kat.h',
      ],
      public_include_paths=[
        'src',
      ])

  return libb2_module
