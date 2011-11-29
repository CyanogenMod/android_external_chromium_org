# Copyright (c) 2011 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

{
  'variables': {
    'chromium_code': 1,
  },

  'targets': [
    {
      'target_name': 'aura_shell',
      'type': '<(component)',
      'dependencies': [
        '../../base/base.gyp:base',
        '../../base/base.gyp:base_i18n',
        '../../base/third_party/dynamic_annotations/dynamic_annotations.gyp:dynamic_annotations',
        '../../build/temp_gyp/googleurl.gyp:googleurl',
        '../../net/net.gyp:net',
        '../../skia/skia.gyp:skia',
        '../../third_party/icu/icu.gyp:icui18n',
        '../../third_party/icu/icu.gyp:icuuc',
        '../../views/views.gyp:views',
        '../aura/aura.gyp:aura',
        '../base/strings/ui_strings.gyp:ui_strings',
        '../gfx/compositor/compositor.gyp:compositor',
        '../ui.gyp:ui',
        '../ui.gyp:ui_resources',
        '../ui.gyp:ui_resources_standard',
      ],
      'defines': [
        'AURA_SHELL_IMPLEMENTATION',
      ],
      'sources': [
        # All .cc, .h under views, except unittests
        'always_on_top_controller.cc',
        'always_on_top_controller.h',
        'app_list.cc',
        'app_list.h',
        'default_container_event_filter.cc',
        'default_container_event_filter.h',
        'default_container_layout_manager.cc',
        'default_container_layout_manager.h',
        'desktop_background_view.cc',
        'desktop_background_view.h',
        'desktop_event_filter.cc',
        'desktop_event_filter.h',
        'desktop_layout_manager.cc',
        'desktop_layout_manager.h',
        'drag_drop_controller.cc',
        'drag_drop_controller.h',
        'drag_image_view.cc',
        'drag_image_view.h',
        'image_grid.cc',
        'image_grid.h',
        'launcher/app_launcher_button.cc',
        'launcher/app_launcher_button.h',
        'launcher/launcher.cc',
        'launcher/launcher.h',
        'launcher/launcher_model.cc',
        'launcher/launcher_model.h',
        'launcher/launcher_model_observer.h',
        'launcher/launcher_types.h',
        'launcher/launcher_view.cc',
        'launcher/launcher_view.h',
        'launcher/tabbed_launcher_button.cc',
        'launcher/tabbed_launcher_button.h',
        'launcher/view_model.cc',
        'launcher/view_model.h',
        'launcher/view_model_utils.cc',
        'launcher/view_model_utils.h',
        'modal_container_layout_manager.cc',
        'modal_container_layout_manager.h',
        'modality_event_filter.cc',
        'modality_event_filter.h',
        'modality_event_filter_delegate.h',
        'property_util.cc',
        'property_util.h',
        'shadow.cc',
        'shadow.h',
        'shadow_controller.cc',
        'shadow_controller.h',
        'shelf_layout_controller.cc',
        'shelf_layout_controller.h',
        'shell.cc',
        'shell.h',
        'shell_accelerator_controller.cc',
        'shell_accelerator_controller.h',
        'shell_accelerator_filter.cc',
        'shell_accelerator_filter.h',
        'shell_delegate.h',
        'shell_factory.h',
        'shell_window_ids.h',
        'show_state_controller.h',
        'show_state_controller.cc',
        'stacking_controller.cc',
        'stacking_controller.h',
        'status_area_view.cc',
        'status_area_view.h',
        'toplevel_frame_view.cc',
        'toplevel_frame_view.h',
        'toplevel_layout_manager.cc',
        'toplevel_layout_manager.h',
        'toplevel_window_event_filter.cc',
        'toplevel_window_event_filter.h',
        'window_frame.cc',
        'window_frame.h',
        'workspace_controller.cc',
        'workspace_controller.h',
        'workspace/workspace.cc',
        'workspace/workspace.h',
        'workspace/workspace_manager.cc',
        'workspace/workspace_manager.h',
        'workspace/workspace_observer.h',
      ],
    },
    {
      'target_name': 'aura_shell_unittests',
      'type': 'executable',
      'dependencies': [
        '../../base/base.gyp:base',
        '../../base/base.gyp:test_support_base',
        '../../chrome/chrome_resources.gyp:packed_resources',
        '../../build/temp_gyp/googleurl.gyp:googleurl',
        '../../skia/skia.gyp:skia',
        '../../testing/gtest.gyp:gtest',
        '../../third_party/icu/icu.gyp:icui18n',
        '../../third_party/icu/icu.gyp:icuuc',
        '../../views/views.gyp:views',
        '../gfx/compositor/compositor.gyp:compositor_test_support',
        '../gfx/compositor/compositor.gyp:test_compositor',
        '../ui.gyp:gfx_resources',
        '../ui.gyp:ui',
        '../ui.gyp:ui_resources',
        '../ui.gyp:ui_resources_standard',
        '../aura/aura.gyp:aura',
        '../aura/aura.gyp:test_support_aura',
        'aura_shell',
      ],
      'sources': [
        'default_container_layout_manager_unittest.cc',
        'desktop_event_filter_unittest.cc',
        'drag_drop_controller_unittest.cc',
        'image_grid_unittest.cc',
        'launcher/launcher_model_unittest.cc',
        'launcher/view_model_unittest.cc',
        'launcher/view_model_utils_unittest.cc',
        'modal_container_layout_manager_unittest.cc',
        'run_all_unittests.cc',
        'shadow_controller_unittest.cc',
        'shell_unittest.cc',
        'stacking_controller_unittest.cc',
        'test_suite.cc',
        'test_suite.h',
        'test/aura_shell_test_base.cc',
        'test/aura_shell_test_base.h',
        'toplevel_layout_manager_unittest.cc',
        'toplevel_window_event_filter_unittest.cc',
        'workspace_controller_unittest.cc',
        'workspace/workspace_manager_unittest.cc',

        '<(SHARED_INTERMEDIATE_DIR)/ui/gfx/gfx_resources.rc',
        '<(SHARED_INTERMEDIATE_DIR)/ui/ui_resources/ui_resources.rc',
        '<(SHARED_INTERMEDIATE_DIR)/ui/ui_resources_standard/ui_resources_standard.rc',
      ],
    },
    {
      'target_name': 'aura_shell_exe',
      'type': 'executable',
      'dependencies': [
        '../../base/base.gyp:base',
        '../../base/base.gyp:base_i18n',
        '../../chrome/chrome_resources.gyp:packed_resources',
        '../../skia/skia.gyp:skia',
        '../../third_party/icu/icu.gyp:icui18n',
        '../../third_party/icu/icu.gyp:icuuc',
        '../../views/views.gyp:views',
        '../aura/aura.gyp:aura',
        '../gfx/compositor/compositor.gyp:compositor',
        '../gfx/compositor/compositor.gyp:compositor_test_support',
        '../ui.gyp:gfx_resources',
        '../ui.gyp:ui',
        '../ui.gyp:ui_resources',
        '../ui.gyp:ui_resources_standard',
        '../../views/views.gyp:views_examples_lib',
        'aura_shell',
      ],
      'sources': [
        'examples/aura_shell_main.cc',
        'examples/bubble.cc',
        'examples/example_factory.h',
        'examples/lock_view.cc',
        'examples/toplevel_window.cc',
        'examples/toplevel_window.h',
        'examples/widgets.cc',
        'examples/window_type_launcher.cc',
        'examples/window_type_launcher.h',
        '<(SHARED_INTERMEDIATE_DIR)/ui/gfx/gfx_resources.rc',
        '<(SHARED_INTERMEDIATE_DIR)/ui/ui_resources/ui_resources.rc',
        '<(SHARED_INTERMEDIATE_DIR)/ui/ui_resources_standard/ui_resources_standard.rc',
      ],
    },
  ],
}
