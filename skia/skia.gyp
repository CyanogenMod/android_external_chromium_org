# Copyright (c) 2012 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

{
  'includes': ['../skia/sweskia.gypi'],
  'variables': {
    'skia_opts_ext%': '<!(python <(DEPTH)/build/dir_exists.py ../third_party/skia/src/opts/ext/)',
    'conditions': [
      ['clang==1', {
        # Do not use clang's integrated assembler.  It doesn't grok
        # some neon instructions.
        'clang_use_integrated_as': 0,
      }],
    ],
  },
  'conditions': [
    ['clang==0 or clang_use_integrated_as==0', {
      'cflags': [
        # The neon assembly contains conditional instructions which
        # aren't enclosed in an IT block. The GNU assembler complains
        # without this option.
        # See #86592.
        '-Wa,-mimplicit-it=always',
      ],
    }],
    # In component mode (shared_lib), we build all of skia as a single DLL.
    # However, in the static mode, we need to build skia as multiple targets
    # in order to support the use case where a platform (e.g. Android) may
    # already have a copy of skia as a system library.
    ['component=="static_library" and use_system_skia==0 and component_skia=="static_library"', {
      'targets': [
        {
          'target_name': 'skia_library',
          'type': 'static_library',
          'includes': [
            'skia_library.gypi',
            'skia_common.gypi',
          ],
        },
      ],
    }],
    ['component=="static_library" and use_system_skia==1 and component_skia=="static_library"', {
      'targets': [
        {
          'target_name': 'skia_library',
          'type': 'none',
          'includes': ['skia_system.gypi'],
        },
      ],
    }],
    ['component=="static_library" and component_skia=="static_library"', {
      'targets': [
        {
          'target_name': 'skia',
          'type': 'none',
          'dependencies': [
            'skia_library',
            'skia_chrome',
          ],
          'export_dependent_settings': [
            'skia_library',
            'skia_chrome',
          ],
        },
        {
          'target_name': 'skia_chrome',
          'type': 'static_library',
          'includes': [
            'skia_chrome.gypi',
            'skia_common.gypi',
          ],
        },
      ],
    },
    {  # component != static_library
      'targets': [
        {
          'target_name': 'skia',
          'type': '<(component_skia)',
          'product_name': '<(product_name)',
          'includes': [
            'skia_library.gypi',
            'skia_chrome.gypi',
            'skia_common.gypi',
          ],
          'link_settings': {
            'libraries!': [
              '-lstlport_static',
            ],
            'libraries' : [
              '-lstlport_sh_521'
            ],
          },
          'ldflags!': [
              '-Wl,--fatal-warnings',
          ],
          'dependencies': [
              '../build/android/setup.gyp:copy_system_libraries',
          ],
          'defines': [
            'SKIA_DLL',
            'SKIA_IMPLEMENTATION=1',
            'GR_GL_IGNORE_ES3_MSAA=0',
          ],
          'direct_dependent_settings': {
            'defines': [
              'SKIA_DLL',
              'GR_GL_IGNORE_ES3_MSAA=0',
            ],
          },
          'conditions': [
            [ 'component_skia== "none" and OS=="android"', {
              'link_settings': {
                'libraries' : [
                  '-L<(SHARED_LIB_DIR)/',
                  '-lsweskia',
                ],
              },

              'sources/': [
                  ['exclude', '\\.(cc|cpp)$'],
              ],

              'dependencies': [
                'sweskia',
              ],
              'dependencies!': [
                'skia_library_opts.gyp:skia_opts',
              ],
            }],
          ],
        },
        {
          'target_name': 'skia_library',
          'type': 'none',
        },
        {
          'target_name': 'skia_chrome',
          'type': 'none',
        },
      ],
    }],
    ['skia_opts_ext == "True"', {
      'targets': [
        {
          'target_name': 'D32_A8_Black_Neon_Test',
          'type': 'executable',
          'dependencies': [
            'skia',
          ],
          'include_dirs': [
            '..',
          ],
          'conditions': [
            [ '(arm_version >= 7 and (arm_neon == 1 or arm_neon_optional == 1)) or target_arch == "arm64"', {
              'defines': [
                '__ARM_HAVE_NEON',
              ],
            }],
          ],
          'sources': [
            '../third_party/skia/src/opts/ext/D32_A8_Black_unittest.cc',
          ],
          'ldflags': [
            '-llog',
          ],
        },
      ],
    }],
  ],

  # targets that are not dependent upon the component type
  'targets': [
    {
      'target_name': 'skia_chrome_opts',
      'type': 'static_library',
      'include_dirs': [
        '..',
        'config',
        '../third_party/skia/include/core',
      ],
      'conditions': [
        [ 'os_posix == 1 and OS != "mac" and OS != "android" and \
            target_arch != "arm" and target_arch != "mipsel" and \
            target_arch != "arm64" and target_arch != "mips64el"', {
          'cflags': [
            '-msse2',
          ],
        }],
        [ 'target_arch != "arm" and target_arch != "mipsel" and \
           target_arch != "arm64" and target_arch != "mips64el"', {
          'sources': [
            'ext/convolver_SSE2.cc',
          ],
        }],
        [ 'target_arch == "mipsel"',{
          'cflags': [
            '-fomit-frame-pointer',
          ],
          'sources': [
            'ext/convolver_mips_dspr2.cc',
          ],
        }],
      ],
    },
    {
      'target_name': 'image_operations_bench',
      'type': 'executable',
      'dependencies': [
        '../base/base.gyp:base',
        'skia',
      ],
      'include_dirs': [
        '..',
      ],
      'sources': [
        'ext/image_operations_bench.cc',
      ],
    },
    {
      'target_name': 'filter_fuzz_stub',
      'type': 'executable',
      'dependencies': [
        '../base/base.gyp:base',
        'skia.gyp:skia',
      ],
      'sources': [
        'tools/filter_fuzz_stub/filter_fuzz_stub.cc',
      ],
    },
  ],
}
