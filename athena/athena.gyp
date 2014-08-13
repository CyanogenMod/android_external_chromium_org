# Copyright 2014 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

{
  'variables': {
    'chromium_code': 1,
  },
  'targets': [
    {
      'target_name': 'athena_lib',
      'type': '<(component)',
      'dependencies': [
        '../skia/skia.gyp:skia',
        '../ui/accessibility/accessibility.gyp:ax_gen',
        '../ui/app_list/app_list.gyp:app_list',
        '../ui/aura/aura.gyp:aura',
        '../ui/events/events.gyp:events_base',
        '../ui/strings/ui_strings.gyp:ui_strings',
        '../ui/views/views.gyp:views',
      ],
      'defines': [
        'ATHENA_IMPLEMENTATION',
      ],
      'sources': [
        # All .cc, .h under athena, except unittests
        'activity/activity.cc',
        'activity/activity_factory.cc',
        'activity/activity_manager_impl.cc',
        'activity/activity_view_manager_impl.cc',
        'activity/activity_frame_view.cc',
        'activity/activity_frame_view.h',
        'activity/activity_widget_delegate.cc',
        'activity/activity_widget_delegate.h',
        'activity/public/activity.h',
        'activity/public/activity_factory.h',
        'activity/public/activity_manager.h',
        'activity/public/activity_view_manager.h',
        'activity/public/activity_view_model.h',
        'athena_export.h',
        'common/closure_animation_observer.cc',
        'common/closure_animation_observer.h',
        'common/container_priorities.h',
        'common/fill_layout_manager.cc',
        'common/fill_layout_manager.h',
        'common/switches.cc',
        'common/switches.h',
        'home/app_list_view_delegate.cc',
        'home/app_list_view_delegate.h',
        'home/bottom_home_view.cc',
        'home/bottom_home_view.h',
        'home/home_card_impl.cc',
        'home/minimized_home.cc',
        'home/minimized_home.h',
        'home/public/app_model_builder.h',
        'home/public/home_card.h',
        'input/accelerator_manager_impl.cc',
        'input/accelerator_manager_impl.h',
        'input/input_manager_impl.cc',
        'input/public/accelerator_manager.h',
        'input/public/input_manager.h',
        'screen/background_controller.cc',
        'screen/background_controller.h',
        'screen/public/screen_manager.h',
        'screen/screen_accelerator_handler.cc',
        'screen/screen_accelerator_handler.h',
        'screen/screen_manager_impl.cc',
        'system/power_button_controller.cc',
        'system/power_button_controller.h',
        'system/public/system_ui.h', 
        'system/system_ui_impl.cc',
        'wm/public/window_manager.h',
        'wm/public/window_manager_observer.h',
        'wm/bezel_controller.cc',
        'wm/bezel_controller.h',
        'wm/split_view_controller.cc',
        'wm/split_view_controller.h',
        'wm/title_drag_controller.cc',
        'wm/title_drag_controller.h',
        'wm/window_manager_impl.cc',
        'wm/window_overview_mode.cc',
        'wm/window_overview_mode.h',
      ],
    },
    {
      'target_name': 'athena_content_lib',
      'type': 'static_library',
      'dependencies': [
        'athena_lib',
        '../base/third_party/dynamic_annotations/dynamic_annotations.gyp:dynamic_annotations',
        '../components/components.gyp:renderer_context_menu',
        '../components/components.gyp:web_modal',
        '../content/content.gyp:content_browser',
        '../ui/app_list/app_list.gyp:app_list',
        '../ui/keyboard/keyboard.gyp:keyboard',
        '../ui/keyboard/keyboard.gyp:keyboard_resources',
        '../third_party/WebKit/public/blink.gyp:blink',
        '../ui/views/controls/webview/webview.gyp:webview',
        '../skia/skia.gyp:skia',
      ],
      'defines': [
        'ATHENA_IMPLEMENTATION',
      ],
      'sources': [
        'content/public/content_activity_factory.h',
        'content/public/content_app_model_builder.h',
        'content/public/web_contents_view_delegate_creator.h',
        'content/content_activity_factory.cc',
        'content/content_app_model_builder.cc',
        'content/app_activity.h',
        'content/app_activity.cc',
        'content/render_view_context_menu_impl.cc',
        'content/render_view_context_menu_impl.h',
        'content/web_activity.h',
        'content/web_activity.cc',
        'content/web_contents_view_delegate_factory_impl.cc',
        'virtual_keyboard/public/virtual_keyboard_manager.h',
        'virtual_keyboard/virtual_keyboard_manager_impl.cc',
      ],
    },
    {
      'target_name': 'athena_test_support',
      'type': 'static_library',
      'dependencies': [
        '../base/base.gyp:test_support_base',
        '../chromeos/chromeos.gyp:chromeos',
        '../skia/skia.gyp:skia',
        '../testing/gtest.gyp:gtest',
        '../ui/accessibility/accessibility.gyp:ax_gen',
        '../ui/app_list/app_list.gyp:app_list',
        '../ui/aura/aura.gyp:aura_test_support',
        '../ui/base/ui_base.gyp:ui_base_test_support',
        '../ui/compositor/compositor.gyp:compositor_test_support',
        '../ui/views/views.gyp:views',
        '../ui/wm/wm.gyp:wm',
        '../url/url.gyp:url_lib',
        'athena_lib',
        'resources/athena_resources.gyp:athena_resources',
      ],
      'sources': [
        'main/athena_launcher.cc',
        'main/athena_launcher.h',
        'main/placeholder.cc',
        'main/placeholder.h',
        'test/athena_test_base.cc',
        'test/athena_test_base.h',
        'test/athena_test_helper.cc',
        'test/athena_test_helper.h',
        'test/sample_activity.cc',
        'test/sample_activity.h',
        'test/sample_activity_factory.cc',
        'test/sample_activity_factory.h',
        'test/test_app_model_builder.cc',
        'test/test_app_model_builder.h',
      ],
    },
    {
      'target_name': 'athena_unittests',
      'type': 'executable',
      'dependencies': [
        '../testing/gtest.gyp:gtest',
        '../skia/skia.gyp:skia',
        'athena_lib',
        'athena_test_support',
        'resources/athena_resources.gyp:athena_pak',
      ],
      'sources': [
        'test/athena_unittests.cc',
        'activity/activity_manager_unittest.cc',
        'home/home_card_unittest.cc',
        'input/accelerator_manager_unittest.cc',
        'screen/screen_manager_unittest.cc',
        'wm/window_manager_unittest.cc',
      ],
    }
  ],
}

