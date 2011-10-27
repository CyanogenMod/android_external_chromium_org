# Copyright (c) 2011 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

{
  'variables': {
    'chromium_code': 1,
  },
  'target_defaults': {
    'sources/': [
      ['exclude', '_(gl|win)\\.(cc?)$'],
    ],
    'conditions': [
      ['os_posix == 1 and OS != "mac"', {
        'sources/': [['include', '_(gl)\\.(h|cc)$'],]
      }],
      ['OS == "win"', {
        'sources/': [['include', '_(win)\\.(h|cc)$'],]
      }],
    ],
  },
  'targets': [
    {
      'target_name': 'compositor',
      'type': '<(component)',
      'dependencies': [
        '<(DEPTH)/base/base.gyp:base',
        '<(DEPTH)/base/third_party/dynamic_annotations/dynamic_annotations.gyp:dynamic_annotations',
        '<(DEPTH)/skia/skia.gyp:skia',
        '<(DEPTH)/ui/gfx/gl/gl.gyp:gl',
        '<(DEPTH)/ui/ui.gyp:ui',
        '<(DEPTH)/third_party/WebKit/Source/WebKit/chromium/WebKit.gyp:webkit',
      ],
      'defines': [
        'COMPOSITOR_IMPLEMENTATION',
      ],
      'sources': [
        'compositor.cc',
        'compositor.h',
        'compositor_export.h',
        'compositor_gl.cc',
        'compositor_gl.h',
        'compositor_cc.cc',
        'compositor_cc.h',
        'compositor_observer.h',
        'compositor_stub.cc',
        'compositor_win.cc',
        'layer.cc',
        'layer.h',
        'layer_animation_delegate.h',
        'layer_animation_element.cc',
        'layer_animation_element.h',
        'layer_animation_manager.cc',
        'layer_animation_manager.h',
        'layer_animation_sequence.cc',
        'layer_animation_sequence.h',
        'layer_animator.cc',
        'layer_animator.h',
      ],
      'conditions': [
        ['os_posix == 1 and OS != "mac"', {
          'sources!': [
            'compositor_stub.cc',
          ],
        }],
        ['OS == "win" and views_compositor == 1', {
          'sources!': [
            'compositor_stub.cc',
          ],
          # TODO(sky): before we make this real need to remove
          # IDR_BITMAP_BRUSH_IMAGE.
          'dependencies': [
            '<(DEPTH)/ui/ui.gyp:gfx_resources',
          ],
          'link_settings': {
            'libraries': [
              '-ld3d10.lib',
              '-ld3dx10d.lib',
              '-ldxerr.lib',
              '-ldxguid.lib',
            ]
          },
        }],
        ['OS == "win" and views_compositor == 0', {
          'sources/': [
            ['exclude', '^compositor_win.cc'],
          ],
        }],
        ['use_webkit_compositor == 1', {
            'sources/': [
              ['exclude', '^compositor_(gl|win|stub).(h|cc)$'],
            ],
            'dependencies': [
              '<(DEPTH)/webkit/support/webkit_support.gyp:webkit_gpu',
            ],
          }, {
            'sources!': [
              'compositor_cc.cc',
              'compositor_cc.h',
            ],
          }
        ],
      ],
    },
    {
      'target_name': 'compositor_test_support',
      'type': 'static_library',
      'dependencies': [
        '<(DEPTH)/base/base.gyp:base',
        '<(DEPTH)/skia/skia.gyp:skia',
      ],
      'sources': [
        'compositor_test_support.cc',
        'compositor_test_support.h',
      ],
      'conditions': [
        ['use_webkit_compositor == 1', {
            'dependencies': [
              '<(DEPTH)/webkit/support/webkit_support.gyp:webkit_support',
              '<(DEPTH)/third_party/WebKit/Source/WebKit/chromium/WebKit.gyp:webkit',
            ],
        }],
        ['views_compositor==1', {
          'sources': [
          'compositor.cc',
          'compositor.h',
          'layer.cc',
          'layer.h',
          'layer_animator.cc',
          'layer_animator.h',
          'layer_animator_delegate.h',
          'test_compositor.cc',
          'test_compositor.h',
          'test_texture.cc',
          'test_texture.h',
          ],
        }],
      ],
    },
    {
      'target_name': 'compositor_unittests',
      'type': 'executable',
      'dependencies': [
        '<(DEPTH)/base/base.gyp:base',
        '<(DEPTH)/base/base.gyp:test_support_base',
        '<(DEPTH)/skia/skia.gyp:skia',
        '<(DEPTH)/testing/gtest.gyp:gtest',
        '<(DEPTH)/ui/gfx/gl/gl.gyp:gl',
        '<(DEPTH)/ui/ui.gyp:gfx_resources',
        '<(DEPTH)/ui/ui.gyp:ui',
        '<(DEPTH)/ui/ui.gyp:ui_resources',
        'compositor',
        'compositor_test_support',
      ],
      'sources': [
        'layer_animation_element_unittest.cc',
        'layer_animation_sequence_unittest.cc',
        'layer_animator_unittest.cc',
        'layer_unittest.cc',
        'run_all_unittests.cc',
        'test_compositor_host.h',
        'test_compositor_host_linux.cc',
        'test_compositor_host_win.cc',
        'test_layer_animation_delegate.cc',
        'test_layer_animation_delegate.h',
        'test_suite.cc',
        'test_suite.h',
        'test_texture.cc',
        'test_texture.h',
        'test_utils.cc',
        'test_utils.h',
        '<(SHARED_INTERMEDIATE_DIR)/ui/gfx/gfx_resources.rc',
        '<(SHARED_INTERMEDIATE_DIR)/ui/ui_resources/ui_resources.rc',
      ],
      'conditions': [
        # osmesa GL implementation is used on linux.
        ['OS=="linux"', {
          'dependencies': [
            '<(DEPTH)/third_party/mesa/mesa.gyp:osmesa',
          ],
        }],
        ['OS!="mac"', {
          'dependencies': [
            '<(DEPTH)/chrome/chrome.gyp:packed_resources',
           ],
        }],
      ],
    },
  ],
}
