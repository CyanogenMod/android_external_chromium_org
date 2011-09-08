# Copyright (c) 2011 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

{
  'targets': [
    {
      'target_name': 'ppapi_c<(nacl_ppapi_library_suffix)',
      'type': 'none',
      'all_dependent_settings': {
        'include_dirs+': [
          '..',
        ],
      },
      'sources': [
        'c/pp_bool.h',
        'c/pp_completion_callback.h',
        'c/pp_errors.h',
        'c/pp_file_info.h',
        'c/pp_graphics_3d.h',
        'c/pp_input_event.h',
        'c/pp_instance.h',
        'c/pp_macros.h',
        'c/pp_module.h',
        'c/pp_point.h',
        'c/pp_rect.h',
        'c/pp_resource.h',
        'c/pp_size.h',
        'c/pp_stdint.h',
        'c/pp_time.h',
        'c/pp_var.h',
        'c/ppb.h',
        'c/ppb_audio.h',
        'c/ppb_audio_config.h',
        'c/ppb_core.h',
        'c/ppb_file_io.h',
        'c/ppb_file_ref.h',
        'c/ppb_file_system.h',
        'c/ppb_graphics_2d.h',
        'c/ppb_graphics_3d.h',
        'c/ppb_image_data.h',
        'c/ppb_input_event.h',
        'c/ppb_instance.h',
        'c/ppb_messaging.h',
        'c/ppb_opengles.h',
        'c/ppb_url_loader.h',
        'c/ppb_url_request_info.h',
        'c/ppb_url_response_info.h',
        'c/ppb_var.h',
        'c/ppp.h',
        'c/ppp_graphics_3d.h',
        'c/ppp_input_event.h',
        'c/ppp_instance.h',
        'c/ppp_messaging.h',

        # Dev interfaces.
        'c/dev/pp_cursor_type_dev.h',
        'c/dev/pp_video_dev.h',
        'c/dev/ppb_buffer_dev.h',
        'c/dev/ppb_char_set_dev.h',
        'c/dev/ppb_context_3d_dev.h',
        'c/dev/ppb_context_3d_trusted_dev.h',
        'c/dev/ppb_console_dev.h',
        'c/dev/ppb_cursor_control_dev.h',
        'c/dev/ppb_directory_reader_dev.h',
        'c/dev/ppb_file_chooser_dev.h',
        'c/dev/ppb_find_dev.h',
        'c/dev/ppb_font_dev.h',
        'c/dev/ppb_fullscreen_dev.h',
        'c/dev/ppb_memory_dev.h',
        'c/dev/ppb_mouse_lock_dev.h',
        'c/dev/ppb_query_policy_dev.h',
        'c/dev/ppb_scrollbar_dev.h',
        'c/dev/ppb_surface_3d_dev.h',
        'c/dev/ppb_testing_dev.h',
        'c/dev/ppb_url_util_dev.h',
        'c/dev/ppb_video_decoder_dev.h',
        'c/dev/ppb_widget_dev.h',
        'c/dev/ppb_zoom_dev.h',
        'c/dev/ppp_cursor_control_dev.h',
        'c/dev/ppp_find_dev.h',
        'c/dev/ppp_mouse_lock_dev.h',
        'c/dev/ppp_network_state_dev.h',
        'c/dev/ppp_policy_update_dev.h',
        'c/dev/ppp_printing_dev.h',
        'c/dev/ppp_scrollbar_dev.h',
        'c/dev/ppp_selection_dev.h',
        'c/dev/ppp_video_decoder_dev.h',
        'c/dev/ppp_widget_dev.h',
        'c/dev/ppp_zoom_dev.h',

        # Private interfaces.
        'c/private/ppb_flash.h',
        'c/private/ppb_flash_clipboard.h',
        'c/private/ppb_flash_file.h',
        'c/private/ppb_flash_menu.h',
        'c/private/ppb_flash_net_connector.h',
        'c/private/ppb_flash_tcp_socket.h',
        'c/private/ppb_gpu_blacklist_private.h',
        'c/private/ppb_instance_private.h',
        'c/private/ppb_nacl_private.h',
        'c/private/ppb_pdf.h',
        'c/private/ppb_proxy_private.h',
        'c/private/ppp_instance_private.h',

        # Deprecated interfaces.
        'c/dev/deprecated_bool.h',
        'c/dev/ppb_var_deprecated.h',
        'c/dev/ppp_class_deprecated.h',

        # Trusted interfaces.
        'c/trusted/ppb_audio_trusted.h',
        'c/trusted/ppb_broker_trusted.h',
        'c/trusted/ppb_buffer_trusted.h',
        'c/trusted/ppb_file_io_trusted.h',
        'c/trusted/ppb_graphics_3d_trusted.h',
        'c/trusted/ppb_image_data_trusted.h',
        'c/trusted/ppb_url_loader_trusted.h',
        'c/trusted/ppp_broker.h',
      ],
      'conditions': [
        ['p2p_apis==1', {
          'sources': [
            'c/dev/ppb_transport_dev.h',
          ],
        }],
      ],
    },
    {
      'target_name': 'ppapi_cpp_objects<(nacl_ppapi_library_suffix)',
      'type': 'static_library',
      'dependencies': [
        'ppapi_c<(nacl_ppapi_library_suffix)'
      ],
      'include_dirs+': [
        '..',
      ],
      'sources': [
        'cpp/audio.cc',
        'cpp/audio.h',
        'cpp/audio_config.cc',
        'cpp/audio_config.h',
        'cpp/completion_callback.cc',
        'cpp/completion_callback.h',
        'cpp/core.cc',
        'cpp/core.h',
        'cpp/file_io.cc',
        'cpp/file_io.h',
        'cpp/file_ref.cc',
        'cpp/file_ref.h',
        'cpp/file_system.cc',
        'cpp/file_system.h',
        'cpp/graphics_2d.cc',
        'cpp/graphics_2d.h',
        'cpp/graphics_3d.cc',
        'cpp/graphics_3d.h',
        'cpp/graphics_3d_client.cc',
        'cpp/graphics_3d_client.h',
        'cpp/image_data.cc',
        'cpp/image_data.h',
        'cpp/input_event.cc',
        'cpp/input_event.h',
        'cpp/instance.cc',
        'cpp/instance.h',
        'cpp/logging.h',
        'cpp/module.cc',
        'cpp/module.h',
        'cpp/module_impl.h',
        'cpp/non_thread_safe_ref_count.h',
        'cpp/paint_aggregator.cc',
        'cpp/paint_aggregator.h',
        'cpp/paint_manager.cc',
        'cpp/paint_manager.h',
        'cpp/point.h',
        'cpp/rect.cc',
        'cpp/rect.h',
        'cpp/resource.cc',
        'cpp/resource.h',
        'cpp/size.h',
        'cpp/url_loader.cc',
        'cpp/url_loader.h',
        'cpp/url_request_info.cc',
        'cpp/url_request_info.h',
        'cpp/url_response_info.cc',
        'cpp/url_response_info.h',
        'cpp/var.cc',
        'cpp/var.h',

        # Dev interfaces.
        'cpp/dev/buffer_dev.cc',
        'cpp/dev/buffer_dev.h',
        'cpp/dev/context_3d_dev.cc',
        'cpp/dev/context_3d_dev.h',
        'cpp/dev/directory_entry_dev.cc',
        'cpp/dev/directory_entry_dev.h',
        'cpp/dev/directory_reader_dev.cc',
        'cpp/dev/directory_reader_dev.h',
        'cpp/dev/file_chooser_dev.cc',
        'cpp/dev/file_chooser_dev.h',
        'cpp/dev/find_dev.cc',
        'cpp/dev/find_dev.h',
        'cpp/dev/font_dev.cc',
        'cpp/dev/font_dev.h',
        'cpp/dev/fullscreen_dev.cc',
        'cpp/dev/fullscreen_dev.h',
        'cpp/dev/memory_dev.cc',
        'cpp/dev/memory_dev.h',
        'cpp/dev/mouse_lock_dev.cc',
        'cpp/dev/mouse_lock_dev.h',
        'cpp/dev/printing_dev.cc',
        'cpp/dev/printing_dev.h',
        'cpp/dev/scrollbar_dev.cc',
        'cpp/dev/scrollbar_dev.h',
        'cpp/dev/selection_dev.cc',
        'cpp/dev/selection_dev.h',
        'cpp/dev/surface_3d_dev.cc',
        'cpp/dev/surface_3d_dev.h',
        'cpp/dev/url_util_dev.cc',
        'cpp/dev/url_util_dev.h',
        'cpp/dev/video_capture_client_dev.cc',
        'cpp/dev/video_capture_client_dev.h',
        'cpp/dev/video_capture_dev.cc',
        'cpp/dev/video_capture_dev.h',
        'cpp/dev/video_decoder_client_dev.cc',
        'cpp/dev/video_decoder_client_dev.h',
        'cpp/dev/video_decoder_dev.cc',
        'cpp/dev/video_decoder_dev.h',
        'cpp/dev/widget_client_dev.cc',
        'cpp/dev/widget_client_dev.h',
        'cpp/dev/widget_dev.cc',
        'cpp/dev/widget_dev.h',
        'cpp/dev/zoom_dev.cc',
        'cpp/dev/zoom_dev.h',

        # Deprecated interfaces.
        'cpp/dev/scriptable_object_deprecated.h',
        'cpp/dev/scriptable_object_deprecated.cc',

        # Private interfaces.
        'cpp/private/instance_private.cc',
        'cpp/private/instance_private.h',
        'cpp/private/var_private.cc',
        'cpp/private/var_private.h',
      ],
      'conditions': [
        ['OS=="win"', {
          'msvs_settings': {
            'VCCLCompilerTool': {
              'AdditionalOptions': ['/we4244'],  # implicit conversion, possible loss of data
            },
          },
        }],
        ['OS=="linux"', {
          'cflags': ['-Wextra', '-pedantic'],
        }],
        ['OS=="mac"', {
          'xcode_settings': {
            'WARNING_CFLAGS': ['-Wextra', '-pedantic'],
           },
        }],
        ['p2p_apis==1', {
          'sources': [
            'cpp/dev/transport_dev.cc',
            'cpp/dev/transport_dev.h',
          ],
        }],
      ],
    },
    {
      'target_name': 'ppapi_cpp<(nacl_ppapi_library_suffix)',
      'type': 'static_library',
      'dependencies': [
        'ppapi_c<(nacl_ppapi_library_suffix)',
        'ppapi_cpp_objects<(nacl_ppapi_library_suffix)',
      ],
      'include_dirs+': [
        '..',
      ],
      'sources': [
        'cpp/module_embedder.h',
        'cpp/ppp_entrypoints.cc',
      ],
      'conditions': [
        ['OS=="linux"', {
          'cflags': ['-Wextra', '-pedantic'],
        }],
        ['OS=="mac"', {
          'xcode_settings': {
            'WARNING_CFLAGS': ['-Wextra', '-pedantic'],
           },
        }]
      ],
    },
  ],
}
