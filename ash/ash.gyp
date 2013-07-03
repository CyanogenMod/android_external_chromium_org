# Copyright (c) 2012 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

{
  'variables': {
    'chromium_code': 1,
    'grit_out_dir': '<(SHARED_INTERMEDIATE_DIR)/chrome',
  },
  'includes': [
    'ash_resources.gypi',
  ],
  'targets': [
    {
      'target_name': 'ash',
      'type': '<(component)',
      'dependencies': [
        '../base/base.gyp:base',
        '../base/base.gyp:base_i18n',
        '../base/third_party/dynamic_annotations/dynamic_annotations.gyp:dynamic_annotations',
        '../cc/cc.gyp:cc',
        '../content/content.gyp:content',
        '../content/content.gyp:content_browser',
        '../ipc/ipc.gyp:ipc',
        '../net/net.gyp:net',
        '../skia/skia.gyp:skia',
        '../third_party/icu/icu.gyp:icui18n',
        '../third_party/icu/icu.gyp:icuuc',
        '../ui/app_list/app_list.gyp:app_list',
        '../ui/aura/aura.gyp:aura',
        '../ui/base/strings/ui_strings.gyp:ui_strings',
        '../ui/compositor/compositor.gyp:compositor',
        '../ui/keyboard/keyboard.gyp:keyboard',
        '../ui/message_center/message_center.gyp:message_center',
        '../ui/oak/oak.gyp:oak',
        '../ui/ui.gyp:ui',
        '../ui/ui.gyp:ui_resources',
        '../ui/views/controls/webview/webview.gyp:webview',
        '../ui/views/views.gyp:views',
        '../ui/web_dialogs/web_dialogs.gyp:web_dialogs',
        '../url/url.gyp:url_lib',
        'ash_strings.gyp:ash_strings',
        'ash_resources',
        'ash_wallpaper_resources',
      ],
      'defines': [
        'ASH_IMPLEMENTATION',
      ],
      'sources': [
        # All .cc, .h under ash, except unittests
        'accelerators/accelerator_controller.cc',
        'accelerators/accelerator_controller.h',
        'accelerators/accelerator_dispatcher.cc',
        'accelerators/accelerator_dispatcher.h',
        'accelerators/accelerator_filter.cc',
        'accelerators/accelerator_filter.h',
        'accelerators/accelerator_table.cc',
        'accelerators/accelerator_table.h',
        'accelerators/exit_warning_handler.cc',
        'accelerators/exit_warning_handler.h',
        'accelerators/focus_manager_factory.cc',
        'accelerators/focus_manager_factory.h',
        'accelerators/nested_dispatcher_controller.cc',
        'accelerators/nested_dispatcher_controller.h',
        'ash_constants.cc',
        'ash_constants.h',
        'ash_switches.cc',
        'ash_switches.h',
        'cancel_mode.cc',
        'cancel_mode.h',
        'caps_lock_delegate.h',
        'caps_lock_delegate_stub.cc',
        'caps_lock_delegate_stub.h',
        'debug.cc',
        'debug.h',
        'desktop_background/desktop_background_controller.cc',
        'desktop_background/desktop_background_controller.h',
        'desktop_background/desktop_background_controller_observer.h',
        'desktop_background/desktop_background_view.cc',
        'desktop_background/desktop_background_view.h',
        'desktop_background/desktop_background_widget_controller.cc',
        'desktop_background/desktop_background_widget_controller.h',
        'desktop_background/user_wallpaper_delegate.h',
        'desktop_background/wallpaper_resizer.cc',
        'desktop_background/wallpaper_resizer.h',
        'desktop_background/wallpaper_resizer_observer.h',
        'display/display_change_observer_x11.cc',
        'display/display_change_observer_x11.h',
        'display/display_controller.cc',
        'display/display_controller.h',
        'display/display_error_dialog.cc',
        'display/display_error_dialog.h',
        'display/display_info.h',
        'display/display_info.cc',
        'display/display_layout.h',
        'display/display_layout.cc',
        'display/display_manager.cc',
        'display/display_manager.h',
        'display/display_pref_util.h',
        'display/event_transformation_handler.cc',
        'display/event_transformation_handler.h',
        'display/mirror_window_controller.cc',
        'display/mirror_window_controller.h',
        'display/mouse_cursor_event_filter.cc',
        'display/mouse_cursor_event_filter.h',
        'display/output_configurator_animation.cc',
        'display/output_configurator_animation.h',
        'display/root_window_transformers.cc',
        'display/root_window_transformers.h',
        'display/screen_position_controller.cc',
        'display/screen_position_controller.h',
        'display/shared_display_edge_indicator.cc',
        'display/shared_display_edge_indicator.h',
        'drag_drop/drag_drop_controller.cc',
        'drag_drop/drag_drop_controller.h',
        'drag_drop/drag_drop_tracker.cc',
        'drag_drop/drag_drop_tracker.h',
        'drag_drop/drag_image_view.cc',
        'drag_drop/drag_image_view.h',
        'event_rewriter_delegate.h',
        'focus_cycler.cc',
        'focus_cycler.h',
        'high_contrast/high_contrast_controller.cc',
        'high_contrast/high_contrast_controller.h',
        'host/root_window_host_factory.cc',
        'host/root_window_host_factory.h',
        'host/root_window_host_factory_win.cc',
        'keyboard_overlay/keyboard_overlay_delegate.cc',
        'keyboard_overlay/keyboard_overlay_delegate.h',
        'keyboard_overlay/keyboard_overlay_view.cc',
        'keyboard_overlay/keyboard_overlay_view.h',
        'launcher/alternate_app_list_button.cc',
        'launcher/alternate_app_list_button.h',
        'launcher/app_list_button.cc',
        'launcher/app_list_button.h',
        'launcher/launcher.cc',
        'launcher/launcher.h',
        'launcher/launcher_alignment_menu.cc',
        'launcher/launcher_alignment_menu.h',
        'launcher/launcher_button.cc',
        'launcher/launcher_button.h',
        'launcher/launcher_delegate.h',
        'launcher/launcher_icon_observer.h',
        'launcher/launcher_model.cc',
        'launcher/launcher_model.h',
        'launcher/launcher_model_observer.h',
        'launcher/launcher_navigator.cc',
        'launcher/launcher_navigator.h',
        'launcher/launcher_tooltip_manager.cc',
        'launcher/launcher_tooltip_manager.h',
        'launcher/launcher_types.cc',
        'launcher/launcher_types.h',
        'launcher/launcher_util.cc',
        'launcher/launcher_util.h',
        'launcher/launcher_view.cc',
        'launcher/launcher_view.h',
        'launcher/overflow_bubble.cc',
        'launcher/overflow_bubble.h',
        'launcher/overflow_button.cc',
        'launcher/overflow_button.h',
        'launcher/tabbed_launcher_button.cc',
        'launcher/tabbed_launcher_button.h',
        'magnifier/magnification_controller.cc',
        'magnifier/magnification_controller.h',
        'magnifier/magnifier_constants.h',
        'magnifier/partial_magnification_controller.cc',
        'magnifier/partial_magnification_controller.h',
        'popup_message.cc',
        'popup_message.h',
        'root_window_controller.cc',
        'root_window_controller.h',
        'rotator/screen_rotation.cc',
        'rotator/screen_rotation.h',
        'screen_ash.cc',
        'screen_ash.h',
        'screensaver/screensaver_view.cc',
        'screensaver/screensaver_view.h',
        'screenshot_delegate.h',
        'session_state_delegate.h',
        'session_state_observer.h',
        'shelf/background_animator.cc',
        'shelf/background_animator.h',
        'shelf/shelf_bezel_event_filter.cc',
        'shelf/shelf_bezel_event_filter.h',
        'shelf/shelf_layout_manager.cc',
        'shelf/shelf_layout_manager.h',
        'shelf/shelf_layout_manager_observer.h',
        'shelf/shelf_types.h',
        'shelf/shelf_widget.cc',
        'shelf/shelf_widget.h',
        'shell.cc',
        'shell.h',
        'shell_delegate.h',
        'shell_factory.h',
        'shell_window_ids.h',
        'system/bluetooth/bluetooth_observer.h',
        'system/bluetooth/tray_bluetooth.cc',
        'system/bluetooth/tray_bluetooth.h',
        'system/brightness/brightness_observer.h',
        'system/brightness/brightness_control_delegate.h',
        'system/brightness/tray_brightness.cc',
        'system/brightness/tray_brightness.h',
        'system/chromeos/audio/audio_observer.h',
        'system/chromeos/audio/tray_audio.cc',
        'system/chromeos/audio/tray_audio.h',
        'system/chromeos/enterprise/enterprise_domain_observer.h',
        'system/chromeos/enterprise/tray_enterprise.h',
        'system/chromeos/enterprise/tray_enterprise.cc',
        'system/chromeos/keyboard_brightness_controller.cc',
        'system/chromeos/keyboard_brightness_controller.h',
        'system/chromeos/label_tray_view.h',
        'system/chromeos/label_tray_view.cc',
        'system/chromeos/managed/tray_locally_managed_user.h',
        'system/chromeos/managed/tray_locally_managed_user.cc',
        'system/chromeos/network/network_connect.cc',
        'system/chromeos/network/network_connect.h',
        'system/chromeos/network/network_detailed_view.h',
        'system/chromeos/network/network_icon.cc',
        'system/chromeos/network/network_icon.h',
        'system/chromeos/network/network_icon_animation.cc',
        'system/chromeos/network/network_icon_animation.h',
        'system/chromeos/network/network_icon_animation_observer.h',
        'system/chromeos/network/network_observer.cc',
        'system/chromeos/network/network_observer.h',
        'system/chromeos/network/network_state_list_detailed_view.cc',
        'system/chromeos/network/network_state_list_detailed_view.h',
        'system/chromeos/network/network_state_notifier.cc',
        'system/chromeos/network/network_state_notifier.h',
        'system/chromeos/network/network_tray_delegate.h',
        'system/chromeos/network/tray_network.cc',
        'system/chromeos/network/tray_network.h',
        'system/chromeos/network/tray_network_state_observer.cc',
        'system/chromeos/network/tray_network_state_observer.h',
        'system/chromeos/network/tray_sms.cc',
        'system/chromeos/network/tray_sms.h',
        'system/chromeos/network/tray_vpn.cc',
        'system/chromeos/network/tray_vpn.h',
        'system/chromeos/power/power_status.cc',
        'system/chromeos/power/power_status.h',
        'system/chromeos/power/power_status_view.cc',
        'system/chromeos/power/power_status_view.h',
        'system/chromeos/power/tray_power.cc',
        'system/chromeos/power/tray_power.h',
        'system/chromeos/screen_security/screen_capture_observer.h',
        'system/chromeos/screen_security/screen_capture_tray_item.cc',
        'system/chromeos/screen_security/screen_capture_tray_item.h',
        'system/chromeos/screen_security/screen_share_observer.h',
        'system/chromeos/screen_security/screen_share_tray_item.cc',
        'system/chromeos/screen_security/screen_share_tray_item.h',
        'system/chromeos/screen_security/screen_tray_item.cc',
        'system/chromeos/screen_security/screen_tray_item.h',
        'system/chromeos/settings/tray_settings.cc',
        'system/chromeos/settings/tray_settings.h',
        'system/chromeos/tray_display.cc',
        'system/chromeos/tray_display.h',
        'system/date/clock_observer.h',
        'system/date/date_view.cc',
        'system/date/date_view.h',
        'system/date/tray_date.cc',
        'system/date/tray_date.h',
        'system/drive/drive_observer.h',
        'system/drive/tray_drive.cc',
        'system/drive/tray_drive.h',
        'system/ime/ime_observer.h',
        'system/ime/tray_ime.cc',
        'system/ime/tray_ime.h',
        'system/keyboard_brightness/keyboard_brightness_control_delegate.h',
        'system/locale/tray_locale.cc',
        'system/locale/tray_locale.h',
        'system/logout_button/logout_button_observer.h',
        'system/logout_button/tray_logout_button.cc',
        'system/logout_button/tray_logout_button.h',
        'system/monitor/tray_monitor.cc',
        'system/monitor/tray_monitor.h',
        'system/session_length_limit/session_length_limit_observer.h',
        'system/session_length_limit/tray_session_length_limit.cc',
        'system/session_length_limit/tray_session_length_limit.h',
        'system/status_area_widget.cc',
        'system/status_area_widget.h',
        'system/status_area_widget_delegate.cc',
        'system/status_area_widget_delegate.h',
        'system/tray/actionable_view.cc',
        'system/tray/actionable_view.h',
        'system/tray/fixed_sized_image_view.cc',
        'system/tray/fixed_sized_image_view.h',
        'system/tray/fixed_sized_scroll_view.cc',
        'system/tray/fixed_sized_scroll_view.h',
        'system/tray/hover_highlight_view.cc',
        'system/tray/hover_highlight_view.h',
        'system/tray/special_popup_row.cc',
        'system/tray/special_popup_row.h',
        'system/tray/system_tray.cc',
        'system/tray/system_tray.h',
        'system/tray/system_tray_bubble.cc',
        'system/tray/system_tray_bubble.h',
        'system/tray/system_tray_delegate.cc',
        'system/tray/system_tray_delegate.h',
        'system/tray/system_tray_item.cc',
        'system/tray/system_tray_item.h',
        'system/tray/system_tray_notifier.cc',
        'system/tray/system_tray_notifier.h',
        'system/tray/test_system_tray_delegate.cc',
        'system/tray/test_system_tray_delegate.h',
        'system/tray/throbber_view.cc',
        'system/tray/throbber_view.h',
        'system/tray/tray_background_view.cc',
        'system/tray/tray_background_view.h',
        'system/tray/tray_bar_button_with_title.cc',
        'system/tray/tray_bar_button_with_title.h',
        'system/tray/tray_bubble_wrapper.cc',
        'system/tray/tray_bubble_wrapper.h',
        'system/tray/tray_constants.cc',
        'system/tray/tray_constants.h',
        'system/tray/tray_details_view.cc',
        'system/tray/tray_details_view.h',
        'system/tray/tray_empty.cc',
        'system/tray/tray_empty.h',
        'system/tray/tray_event_filter.cc',
        'system/tray/tray_event_filter.h',
        'system/tray/tray_image_item.cc',
        'system/tray/tray_image_item.h',
        'system/tray/tray_item_more.cc',
        'system/tray/tray_item_more.h',
        'system/tray/tray_item_view.cc',
        'system/tray/tray_item_view.h',
        'system/tray/tray_notification_view.cc',
        'system/tray/tray_notification_view.h',
        'system/tray/tray_popup_header_button.cc',
        'system/tray/tray_popup_header_button.h',
        'system/tray/tray_popup_label_button.cc',
        'system/tray/tray_popup_label_button.cc',
        'system/tray/tray_popup_label_button.h',
        'system/tray/tray_popup_label_button_border.cc',
        'system/tray/tray_popup_label_button_border.h',
        'system/tray/tray_utils.cc',
        'system/tray/tray_utils.h',
        'system/tray/view_click_listener.h',
        'system/tray_accessibility.cc',
        'system/tray_accessibility.h',
        'system/tray_caps_lock.cc',
        'system/tray_caps_lock.h',
        'system/tray_update.cc',
        'system/tray_update.h',
        'system/user/login_status.cc',
        'system/user/login_status.h',
        'system/user/tray_user.cc',
        'system/user/tray_user.h',
        'system/user/update_observer.h',
        'system/user/user_observer.h',
        'system/web_notification/web_notification_tray.cc',
        'system/web_notification/web_notification_tray.h',
        'touch/touch_hud_debug.cc',
        'touch/touch_hud_debug.h',
        'touch/touch_hud_projection.cc',
        'touch/touch_hud_projection.h',
        'touch/touch_observer_hud.cc',
        'touch/touch_observer_hud.h',
        'touch/touch_uma.cc',
        'touch/touch_uma.h',
        'volume_control_delegate.h',
        'wm/app_list_controller.cc',
        'wm/app_list_controller.h',
        'wm/activation_controller.cc',
        'wm/activation_controller.h',
        'wm/activation_controller_delegate.h',
        'wm/always_on_top_controller.cc',
        'wm/always_on_top_controller.h',
        'wm/ash_activation_controller.cc',
        'wm/ash_activation_controller.h',
        'wm/ash_native_cursor_manager.cc',
        'wm/ash_native_cursor_manager.h',
        'wm/ash_focus_rules.cc',
        'wm/ash_focus_rules.h',
        'wm/base_layout_manager.cc',
        'wm/base_layout_manager.h',
        'wm/boot_splash_screen.cc',
        'wm/boot_splash_screen.h',
        'wm/capture_controller.cc',
        'wm/capture_controller.h',
        'wm/coordinate_conversion.cc',
        'wm/coordinate_conversion.h',
        'wm/custom_frame_view_ash.cc',
        'wm/custom_frame_view_ash.h',
        'wm/default_window_resizer.cc',
        'wm/default_window_resizer.h',
        'wm/dock/docked_window_layout_manager.cc',
        'wm/dock/docked_window_layout_manager.h',
        'wm/dock/docked_window_resizer.cc',
        'wm/dock/docked_window_resizer.h',
        'wm/drag_window_controller.cc',
        'wm/drag_window_controller.h',
        'wm/drag_window_resizer.cc',
        'wm/drag_window_resizer.h',
        'wm/event_client_impl.cc',
        'wm/event_client_impl.h',
        'wm/event_rewriter_event_filter.cc',
        'wm/event_rewriter_event_filter.h',
        'wm/frame_painter.cc',
        'wm/frame_painter.h',
        'wm/gestures/long_press_affordance_handler.cc',
        'wm/gestures/long_press_affordance_handler.h',
        'wm/gestures/shelf_gesture_handler.cc',
        'wm/gestures/shelf_gesture_handler.h',
        'wm/gestures/system_pinch_handler.cc',
        'wm/gestures/system_pinch_handler.h',
        'wm/gestures/tray_gesture_handler.cc',
        'wm/gestures/tray_gesture_handler.h',
        'wm/gestures/two_finger_drag_handler.cc',
        'wm/gestures/two_finger_drag_handler.h',
        'wm/image_cursors.cc',
        'wm/image_cursors.h',
        'wm/lock_state_controller.cc',
        'wm/lock_state_controller.h',
        'wm/lock_state_controller_impl2.cc',
        'wm/lock_state_controller_impl2.h',
        'wm/lock_state_observer.h',
        'wm/maximize_bubble_controller.cc',
        'wm/maximize_bubble_controller.h',
        'wm/overlay_event_filter.cc',
        'wm/overlay_event_filter.h',
        'wm/panels/panel_frame_view.cc',
        'wm/panels/panel_frame_view.h',
        'wm/panels/panel_layout_manager.cc',
        'wm/panels/panel_layout_manager.h',
        'wm/panels/panel_window_event_handler.cc',
        'wm/panels/panel_window_event_handler.h',
        'wm/panels/panel_window_resizer.cc',
        'wm/panels/panel_window_resizer.h',
        'wm/partial_screenshot_view.cc',
        'wm/partial_screenshot_view.h',
        'wm/power_button_controller.cc',
        'wm/power_button_controller.h',
        'wm/property_util.cc',
        'wm/property_util.h',
        'wm/resize_shadow.cc',
        'wm/resize_shadow.h',
        'wm/resize_shadow_controller.cc',
        'wm/resize_shadow_controller.h',
        'wm/root_window_layout_manager.cc',
        'wm/root_window_layout_manager.h',
        'wm/screen_dimmer.cc',
        'wm/screen_dimmer.h',
        'wm/session_state_animator.cc',
        'wm/session_state_animator.h',
        'wm/session_state_controller_impl.cc',
        'wm/session_state_controller_impl.h',
        'wm/stacking_controller.cc',
        'wm/stacking_controller.h',
        'wm/status_area_layout_manager.cc',
        'wm/status_area_layout_manager.h',
        'wm/sticky_keys.cc',
        'wm/sticky_keys.h',
        'wm/system_background_controller.cc',
        'wm/system_background_controller.h',
        'wm/system_gesture_event_filter.cc',
        'wm/system_gesture_event_filter.h',
        'wm/system_modal_container_event_filter.cc',
        'wm/system_modal_container_event_filter.h',
        'wm/system_modal_container_event_filter_delegate.h',
        'wm/system_modal_container_layout_manager.cc',
        'wm/system_modal_container_layout_manager.h',
        'wm/toplevel_window_event_handler.cc',
        'wm/toplevel_window_event_handler.h',
        'wm/user_activity_detector.cc',
        'wm/user_activity_detector.h',
        'wm/user_activity_observer.h',
        'wm/video_detector.cc',
        'wm/video_detector.h',
        'wm/window_animations.cc',
        'wm/window_animations.h',
        'wm/window_cycle_controller.cc',
        'wm/window_cycle_controller.h',
        'wm/window_cycle_list.cc',
        'wm/window_cycle_list.h',
        'wm/window_properties.cc',
        'wm/window_properties.h',
        'wm/window_resizer.cc',
        'wm/window_resizer.h',
        'wm/window_util.cc',
        'wm/window_util.h',
        'wm/workspace_controller.cc',
        'wm/workspace_controller.h',
        'wm/workspace/auto_window_management.cc',
        'wm/workspace/auto_window_management.h',
        'wm/workspace/colored_window_controller.cc',
        'wm/workspace/colored_window_controller.h',
        'wm/workspace/desktop_background_fade_controller.cc',
        'wm/workspace/desktop_background_fade_controller.h',
        'wm/workspace/frame_maximize_button.cc',
        'wm/workspace/frame_maximize_button.h',
        'wm/workspace/magnetism_matcher.cc',
        'wm/workspace/magnetism_matcher.h',
        'wm/workspace/maximize_bubble_frame_state.h',
        'wm/workspace/multi_window_resize_controller.cc',
        'wm/workspace/multi_window_resize_controller.h',
        'wm/workspace/phantom_window_controller.cc',
        'wm/workspace/phantom_window_controller.h',
        'wm/workspace/snap_sizer.cc',
        'wm/workspace/snap_sizer.h',
        'wm/workspace/snap_types.h',
        'wm/workspace/workspace.cc',
        'wm/workspace/workspace.h',
        'wm/workspace/workspace_animations.cc',
        'wm/workspace/workspace_animations.h',
        'wm/workspace/workspace_cycler.cc',
        'wm/workspace/workspace_cycler.h',
        'wm/workspace/workspace_cycler_animator.cc',
        'wm/workspace/workspace_cycler_animator.h',
        'wm/workspace/workspace_cycler_configuration.cc',
        'wm/workspace/workspace_cycler_configuration.h',
        'wm/workspace/workspace_event_handler.cc',
        'wm/workspace/workspace_event_handler.h',
        'wm/workspace/workspace_layout_manager.cc',
        'wm/workspace/workspace_layout_manager.h',
        'wm/workspace/workspace_manager.cc',
        'wm/workspace/workspace_manager.h',
        'wm/workspace/workspace_types.h',
        'wm/workspace/workspace_window_resizer.cc',
        'wm/workspace/workspace_window_resizer.h',
      ],
      'conditions': [
        ['OS=="mac"', {
          'sources/': [
            ['exclude', 'accelerators/accelerator_controller.cc'],
            ['exclude', 'accelerators/accelerator_controller.h'],
            ['exclude', 'accelerators/accelerator_dispatcher.cc'],
            ['exclude', 'accelerators/accelerator_dispatcher.h'],
            ['exclude', 'accelerators/accelerator_filter.cc'],
            ['exclude', 'accelerators/accelerator_filter.h'],
            ['exclude', 'accelerators/exit_warning_handler.cc'],
            ['exclude', 'accelerators/exit_warning_handler.h'],
            ['exclude', 'accelerators/nested_dispatcher_controller.cc'],
            ['exclude', 'accelerators/nested_dispatcher_controller.h'],
          ],
        }],
        ['OS=="win"', {
          'sources/': [
            ['exclude', 'host/root_window_host_factory.cc'],
            ['exclude', 'wm/sticky_keys.cc'],
            ['exclude', 'wm/sticky_keys.h'],
          ],
          # TODO(jschuh): crbug.com/167187 fix size_t to int truncations.
          'msvs_disabled_warnings': [ 4267, ],
        }],
        ['OS!="linux"', {
          'sources/': [
            ['exclude', 'system/monitor/tray_monitor.cc'],
            ['exclude', 'system/monitor/tray_monitor.h'],
          ],
        }],
        ['chromeos==1', {
          'dependencies': [
            '../chromeos/chromeos.gyp:chromeos',
            # Ash #includes power_supply_properties.pb.h directly.
            '../chromeos/chromeos.gyp:power_manager_proto',
          ],
        }, { # else: chromeos!=1
          'sources/': [
            ['exclude', '/chromeos/'],
            ['exclude', 'display/display_error_dialog.cc'],
            ['exclude', 'display/display_error_dialog.h'],
            ['exclude', 'display/output_configurator_animation.cc'],
            ['exclude', 'display/output_configurator_animation.h'],
          ],
        }],
      ],
    },
    {
      'target_name': 'ash_test_support',
      'type': 'static_library',
      'dependencies': [
        '../skia/skia.gyp:skia',
        '../testing/gtest.gyp:gtest',
        'ash',
        'ash_resources',
      ],
      'sources': [
        'test/ash_test_base.cc',
        'test/ash_test_base.h',
        'test/ash_test_helper.cc',
        'test/ash_test_helper.h',
        'test/cursor_manager_test_api.cc',
        'test/cursor_manager_test_api.h',
        'test/launcher_view_test_api.cc',
        'test/launcher_view_test_api.h',
        'test/display_manager_test_api.cc',
        'test/display_manager_test_api.h',
        'test/mirror_window_test_api.cc',
        'test/mirror_window_test_api.h',
        'test/shell_test_api.cc',
        'test/shell_test_api.h',
        'test/test_activation_delegate.cc',
        'test/test_activation_delegate.h',
        'test/test_launcher_delegate.cc',
        'test/test_launcher_delegate.h',
        'test/test_session_state_delegate.cc',
        'test/test_session_state_delegate.cc',
        'test/test_shell_delegate.cc',
        'test/test_shell_delegate.h',
        'test/test_suite.cc',
        'test/test_suite.h',
        'test/test_suite_init.h',
        'test/test_suite_init.mm',
        'test/ui_controls_factory_ash.cc',
        'test/ui_controls_factory_ash.h',
      ],
      'conditions': [
        ['OS=="win"', {
          'dependencies': [
            '../ipc/ipc.gyp:ipc',
            '../ui/metro_viewer/metro_viewer.gyp:metro_viewer_messages',
            '../win8/win8.gyp:metro_viewer',
            '../win8/win8.gyp:test_support_win8',
            '../win8/win8_tests.gyp:test_registrar',
          ],
          'sources': [
            'test/test_metro_viewer_process_host.cc',
            'test/test_metro_viewer_process_host.h',
          ],
        }],
      ],
    },
    {
      'target_name': 'ash_unittests',
      'type': 'executable',
      'dependencies': [
        '../base/base.gyp:base',
        '../base/base.gyp:test_support_base',
        '../chrome/chrome_resources.gyp:packed_resources',
        '../content/content.gyp:content_browser',
        '../content/content.gyp:test_support_content',
        '../skia/skia.gyp:skia',
        '../testing/gtest.gyp:gtest',
        '../third_party/icu/icu.gyp:icui18n',
        '../third_party/icu/icu.gyp:icuuc',
        '../ui/app_list/app_list.gyp:app_list',
        '../ui/aura/aura.gyp:aura',
        '../ui/aura/aura.gyp:aura_test_support',
        '../ui/compositor/compositor.gyp:compositor',
        '../ui/keyboard/keyboard.gyp:keyboard',
        '../ui/message_center/message_center.gyp:message_center',
        '../ui/message_center/message_center.gyp:message_center_test_support',
        '../ui/ui.gyp:ui',
        '../ui/ui.gyp:ui_resources',
        '../ui/ui.gyp:ui_test_support',
        '../ui/views/views.gyp:views',
        '../ui/views/views.gyp:views_examples_with_content_lib',
        '../ui/views/views.gyp:views_test_support',
        '../ui/views/views.gyp:views_with_content_test_support',
        '../ui/web_dialogs/web_dialogs.gyp:web_dialogs_test_support',
        '../url/url.gyp:url_lib',
        'ash_strings.gyp:ash_strings',
        'ash',
        'ash_resources',
        'ash_test_support',
      ],
      'sources': [
        '../ui/compositor/test/layer_animator_test_controller.cc',
        '../ui/compositor/test/layer_animator_test_controller.h',
        '../ui/views/test/test_views_delegate.cc',
        '../ui/views/test/test_views_delegate.h',
        'accelerators/accelerator_controller_unittest.cc',
        'accelerators/accelerator_filter_unittest.cc',
        'accelerators/accelerator_table_unittest.cc',
        'accelerators/nested_dispatcher_controller_unittest.cc',
        'desktop_background/desktop_background_controller_unittest.cc',
        'desktop_background/wallpaper_resizer_unittest.cc',
        'dip_unittest.cc',
        'display/display_controller_unittest.cc',
        'display/display_error_dialog_unittest.cc',
        'display/display_info_unittest.cc',
        'display/display_manager_unittest.cc',
        'display/mirror_window_controller_unittest.cc',
        'display/mouse_cursor_event_filter_unittest.cc',
        'display/root_window_transformers_unittest.cc',
        'display/screen_position_controller_unittest.cc',
        'drag_drop/drag_drop_controller_unittest.cc',
        'drag_drop/drag_drop_tracker_unittest.cc',
        'extended_desktop_unittest.cc',
        'focus_cycler_unittest.cc',
        'keyboard_overlay/keyboard_overlay_delegate_unittest.cc',
        'keyboard_overlay/keyboard_overlay_view_unittest.cc',
        'launcher/launcher_model_unittest.cc',
        'launcher/launcher_navigator_unittest.cc',
        'launcher/launcher_tooltip_manager_unittest.cc',
        'launcher/launcher_unittest.cc',
        'launcher/launcher_view_unittest.cc',
        'magnifier/magnification_controller_unittest.cc',
        'root_window_controller_unittest.cc',
        'screen_ash_unittest.cc',
        'screensaver/screensaver_view_unittest.cc',
        'session_state_delegate_stub.cc',
        'session_state_delegate_stub.h',
        'shelf/shelf_layout_manager_unittest.cc',
        'shelf/shelf_widget_unittest.cc',
        'shell_unittest.cc',
        'shell/app_list.cc',
        'shell/bubble.cc',
        'shell/context_menu.cc',
        'shell/context_menu.h',
        'shell/launcher_delegate_impl.cc',
        'shell/lock_view.cc',
        'shell/panel_window.cc',
        'shell/shell_delegate_impl.cc',
        'shell/shell_delegate_impl.h',
        'shell/toplevel_window.cc',
        'shell/widgets.cc',
        'shell/window_type_launcher.cc',
        'shell/window_watcher.cc',
        'shell/window_watcher_unittest.cc',
        'system/chromeos/network/network_state_notifier_unittest.cc',
        'system/chromeos/power/power_status_unittest.cc',
        'system/chromeos/power/tray_power_unittest.cc',
        'system/chromeos/screen_security/screen_tray_item_unittest.cc',
        'system/chromeos/tray_display_unittest.cc',
        'system/tray/system_tray_unittest.cc',
        'system/user/tray_user_unittest.cc',
        'system/web_notification/web_notification_tray_unittest.cc',
        'test/ash_test_helper_unittest.cc',
        'test/ash_unittests.cc',
        'tooltips/tooltip_controller_unittest.cc',
        'touch/touch_observer_hud_unittest.cc',
        'wm/activation_controller_unittest.cc',
        'wm/ash_activation_controller_unittest.cc',
        'wm/ash_native_cursor_manager_unittest.cc',
        'wm/base_layout_manager_unittest.cc',
        'wm/custom_frame_view_ash_unittest.cc',
        'wm/dock/docked_window_layout_manager_unittest.cc',
        'wm/dock/docked_window_resizer_unittest.cc',
        'wm/drag_window_resizer_unittest.cc',
        'wm/frame_painter_unittest.cc',
        'wm/lock_state_controller_impl2_unittest.cc',
        'wm/panels/panel_layout_manager_unittest.cc',
        'wm/panels/panel_window_resizer_unittest.cc',
        'wm/partial_screenshot_view_unittest.cc',
        'wm/power_button_controller_unittest.cc',
        'wm/screen_dimmer_unittest.cc',
        'wm/stacking_controller_unittest.cc',
        'wm/sticky_keys_unittest.cc',
        'wm/system_gesture_event_filter_unittest.cc',
        'wm/system_modal_container_layout_manager_unittest.cc',
        'wm/toplevel_window_event_handler_unittest.cc',
        'wm/user_activity_detector_unittest.cc',
        'wm/video_detector_unittest.cc',
        'wm/window_animations_unittest.cc',
        'wm/window_cycle_controller_unittest.cc',
        'wm/window_manager_unittest.cc',
        'wm/window_modality_controller_unittest.cc',
        'wm/window_util_unittest.cc',
        'wm/workspace_controller_test_helper.cc',
        'wm/workspace_controller_test_helper.h',
        'wm/workspace/magnetism_matcher_unittest.cc',
        'wm/workspace/multi_window_resize_controller_unittest.cc',
        'wm/workspace/workspace_event_handler_test_helper.cc',
        'wm/workspace/workspace_event_handler_test_helper.h',
        'wm/workspace/workspace_event_handler_unittest.cc',
        'wm/workspace/workspace_layout_manager_unittest.cc',
        'wm/workspace/workspace_manager_unittest.cc',
        'wm/workspace/workspace_window_resizer_unittest.cc',
        '<(SHARED_INTERMEDIATE_DIR)/ash/ash_resources/ash_wallpaper_resources.rc',
      ],
      'conditions': [
        ['OS=="win"', {
          'sources/': [
            # TODO(zork): fix this test to build on Windows. See: crosbug.com/26906
            ['exclude', 'focus_cycler_unittest.cc'],
            # All tests for multiple displays: not supported on Windows Ash.
            ['exclude', 'accelerators/nested_dispatcher_controller_unittest.cc'],
            ['exclude', 'wm/drag_window_resizer_unittest.cc'],
            # Can't resize on Windows Ash. http://crbug.com/165962
            ['exclude', 'ash_root_window_transformer_unittest.cc'],
            ['exclude', 'magnifier/magnification_controller_unittest.cc'],
            ['exclude', 'wm/workspace/workspace_window_resizer_unittest.cc'],
            ['exclude', 'wm/sticky_keys_unittest.cc'],
          ],
          'sources': [
            '<(SHARED_INTERMEDIATE_DIR)/ui/ui_resources/ui_unscaled_resources.rc',
          ],
          # TODO(jschuh): crbug.com/167187 fix size_t to int truncations.
          'msvs_disabled_warnings': [ 4267, ],
        }],
        ['OS=="mac"', {
          'sources/': [
            ['exclude', 'accelerators/accelerator_controller_unittest.cc'],
            ['exclude', 'accelerators/accelerator_filter_unittest.cc'],
            ['exclude', 'accelerators/nested_dispatcher_controller_unittest.cc'],
            ['exclude', 'drag_drop/drag_drop_controller_unittest.cc'],
            ['exclude', 'tooltips/tooltip_controller_unittest.cc'],
          ],
          'dependencies': [
            # Mac tests access resources via the 'AuraShell.app' directory.
            'ash_shell',
          ],
          # Special linker instructions that avoids stripping Obj-C classes that
          # are not referenced in code, but are referenced in nibs.
          'xcode_settings': {'OTHER_LDFLAGS': ['-Wl,-ObjC']},
        }],
        ['chromeos!=1', {
          'sources/': [
            ['exclude', 'display/display_error_dialog_unittest.cc'],
          ],
        }, {  # chromeos==1
          'dependencies': [
            '../chromeos/chromeos.gyp:power_manager_proto',
          ],
        }],
      ],
    },
    {
      'target_name': 'ash_shell',
      'type': 'executable',
      'dependencies': [
        'ash_strings.gyp:ash_strings',
        '../base/base.gyp:base',
        '../base/base.gyp:base_i18n',
        '../chrome/chrome_resources.gyp:packed_resources',
        '../content/content.gyp:content_shell_lib',
        '../content/content.gyp:content',
        '../skia/skia.gyp:skia',
        '../third_party/icu/icu.gyp:icui18n',
        '../third_party/icu/icu.gyp:icuuc',
        '../ui/app_list/app_list.gyp:app_list',
        '../ui/aura/aura.gyp:aura',
        '../ui/compositor/compositor.gyp:compositor',
        '../ui/keyboard/keyboard.gyp:keyboard',
        '../ui/message_center/message_center.gyp:message_center',
        '../ui/ui.gyp:ui',
        '../ui/ui.gyp:ui_resources',
        '../ui/views/views.gyp:views',
        '../ui/views/views.gyp:views_examples_lib',
        '../ui/views/views.gyp:views_examples_with_content_lib',
        '../ui/views/views.gyp:views_test_support',
        'ash',
        'ash_resources',
      ],
      'sources': [
        'session_state_delegate_stub.cc',
        'session_state_delegate_stub.h',
        'shell/app_list.cc',
        'shell/bubble.cc',
        'shell/content_client/shell_browser_main_parts.cc',
        'shell/content_client/shell_browser_main_parts.h',
        'shell/content_client/shell_content_browser_client.cc',
        'shell/content_client/shell_content_browser_client.h',
        'shell/content_client/shell_main_delegate.cc',
        'shell/content_client/shell_main_delegate.h',
        'shell/context_menu.cc',
        'shell/context_menu.h',
        'shell/example_factory.h',
        'shell/launcher_delegate_impl.cc',
        'shell/launcher_delegate_impl.h',
        'shell/lock_view.cc',
        'shell/panel_window.cc',
        'shell/panel_window.h',
        'shell/shell_delegate_impl.cc',
        'shell/shell_delegate_impl.h',
        'shell/shell_main.cc',
        'shell/shell_main_parts.cc',
        'shell/shell_main_parts.h',
        'shell/shell_main_parts_mac.mm',
        'shell/toplevel_window.cc',
        'shell/toplevel_window.h',
        'shell/widgets.cc',
        'shell/window_type_launcher.cc',
        'shell/window_type_launcher.h',
        'shell/window_watcher.cc',
        'shell/window_watcher.h',
        '../content/app/startup_helper_win.cc',
        '../ui/views/test/test_views_delegate.cc',
        '<(SHARED_INTERMEDIATE_DIR)/ash/ash_resources/ash_wallpaper_resources.rc',
      ],
      'conditions': [
        ['OS=="win"', {
          'msvs_settings': {
            'VCLinkerTool': {
              'SubSystem': '2',  # Set /SUBSYSTEM:WINDOWS
            },
          },
          'dependencies': [
            '../sandbox/sandbox.gyp:sandbox',
          ],
        }],
        ['OS=="mac"', {
          'product_name': 'AuraShell',
          'mac_bundle': 1,
          'sources/': [
            ['exclude', 'shell/shell_main_parts.cc'],
          ],
          'mac_bundle_resources': [
            'shell/cocoa/app.icns',
            'shell/cocoa/app-Info.plist',
            'shell/cocoa/nibs/MainMenu.xib',
            'shell/cocoa/nibs/RootWindow.xib',
            '<(SHARED_INTERMEDIATE_DIR)/repack/chrome.pak',
            '<!@pymod_do_main(repack_locales -o -p <(OS) -g <(grit_out_dir) -s <(SHARED_INTERMEDIATE_DIR) -x <(SHARED_INTERMEDIATE_DIR) <(locales))',
          ],
          'mac_bundle_resources!': [
            'shell/cocoa/app-Info.plist',
          ],
          'xcode_settings': {
            'INFOPLIST_FILE': 'shell/cocoa/app-Info.plist',
          },
        }],
      ],
    },
  ],
}
