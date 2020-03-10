import copy
import os

import respire.buildlib.cc as cc
import respire.buildlib.modules as modules


def Build(registry, out_dir, platform, configured_toolchain, stdext_modules):
  if not os.path.exists(out_dir):
    os.makedirs(out_dir)

  platform_window_modules = {}

  configured_toolchain.configuration.include_directories += [
      os.path.dirname(os.path.realpath(__file__)),
  ]

  if platform == 'win32':
    platform_window_build_kwargs = {
      'sources': [
        'platform_window_win32.cc',
        'include/platform_window/platform_window.h',
      ],
      'public_include_paths': [
        'include',
      ],
      'system_libraries': [
        'User32',
        'Gdi32',
      ]
    }
  elif platform == 'raspi':
    platform_window_build_kwargs = {
      'sources': [
        'platform_window_raspi.cc',
        'include/platform_window/platform_window.h',
      ],
      'public_include_paths': [
        'include',
      ],
      'system_libraries': [
        'brcmEGL',
        'brcmGLESv2',
        'bcm_host',
        'vcos',
        'vchiq_arm',
      ]
    }
  elif platform == 'linux':
    platform_window_build_kwargs = {
      'sources': [
        'platform_window_x11.cc',
        'include/platform_window/platform_window.h',
      ],
      'public_include_paths': [
        'include',
      ],
      'system_libraries': [
        'X11',
      ]
    }
  elif platform == 'jetson':
    # We use a stub window for Jetson because it follows the EGLDevice/EGLOutput
    # path, which doesn't have the concept of "window"s and doesn't go through
    # eglCreateWindowSurface.
    platform_window_build_kwargs = {
      'sources': [
        'platform_window_stub.cc',
        'include/platform_window/platform_window.h',
      ],
      'public_include_paths': [
        'include',
      ]
    }
  else:
    raise Exception('Unsupported platform: ' + str(platform))

  platform_window_build_kwargs['module_dependencies'] = [
    stdext_modules['stdext_lib'],
  ]

  export_configured_toolchain = copy.deepcopy(configured_toolchain)
  export_configured_toolchain.configuration.defines += (
      ['EXPORT_PLATFORM_WINDOW'])

  platform_window_module = modules.SharedLibraryModule(
      'platform_window', registry, out_dir, export_configured_toolchain,
      **platform_window_build_kwargs)

  platform_window_modules['platform_window'] = platform_window_module

  return platform_window_modules
