# Copyright (c) 2012 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

# This GYP file defines untrusted (NaCl) targets.  All targets in this
# file should be conditionally depended upon via 'disable_nacl!=1' to avoid
# requiring NaCl sources for building.

{
  'includes': [
    '../native_client/build/common.gypi',
  ],
  'targets': [
    {
      'target_name': 'base_untrusted',
      'type': 'none',
      'dependencies': [
        '<(DEPTH)/native_client/tools.gyp:prep_toolchain',
        '<(DEPTH)/native_client/src/untrusted/pthread/pthread.gyp:pthread_lib',
        '<(DEPTH)/native_client/src/untrusted/nacl/nacl.gyp:nacl_lib_newlib',
      ],
      'variables': {
        'nlib_target': 'libbase_untrusted.a',
        'build_glibc': 0,
        'build_newlib': 1,
        'include_dirs': [
          '..',
        ],
        'sources': [
          '../base/at_exit.cc',
          '../base/atomicops_internals_x86_gcc.cc',
#          '../base/base_paths.cc',
#          '../base/base_paths_posix.cc',
          '../base/bind_helpers.cc',
          '../base/build_time.cc',
          '../base/callback_internal.cc',
#          '../base/command_line.cc',
#          '../base/cpu.cc',
          '../base/debug/alias.cc',
          '../base/debug/debugger_posix.cc',
#          '../base/debug/stack_trace.cc',
          '../base/debug/trace_event.cc',
          '../base/debug/trace_event_impl.cc',
#          '../base/environment.cc',
          '../base/file_path.cc',
#          '../base/file_util.cc',
#          '../base/file_util_posix.cc',
#          '../base/file_util_proxy.cc',
          '../base/files/file_path_watcher.cc',
#          '../base/files/file_path_watcher_kqueue.cc',
          '../base/files/file_path_watcher_stub.cc',
#          '../base/global_descriptors_posix.cc',
          '../base/json/json_reader.cc',
          '../base/json/json_string_value_serializer.cc',
          '../base/json/json_writer.cc',
          '../base/json/string_escape.cc',
          '../base/lazy_instance.cc',
          '../base/location.cc',
          '../base/logging.cc',
          '../base/memory/ref_counted.cc',
          '../base/memory/ref_counted_memory.cc',
          '../base/memory/singleton.cc',
          '../base/memory/weak_ptr.cc',
          '../base/message_loop.cc',
          '../base/message_loop_proxy.cc',
          '../base/message_loop_proxy_impl.cc',
          '../base/message_pump.cc',
          '../base/message_pump_default.cc',
          '../base/metrics/histogram.cc',
          '../base/metrics/stats_counters.cc',
          '../base/metrics/stats_table.cc',
#          '../base/native_library_posix.cc',
          '../base/os_compat_nacl.cc',
#          '../base/path_service.cc',
          '../base/pending_task.cc',
          '../base/pickle.cc',
          '../base/platform_file.cc',
#          '../base/platform_file_posix.cc',
#          '../base/process_posix.cc',
#          '../base/process_util.cc',
#          '../base/process_util_posix.cc',
          '../base/profiler/alternate_timer.cc',
          '../base/profiler/scoped_profile.cc',
          '../base/profiler/tracked_time.cc',
          '../base/property_bag.cc',
          '../base/rand_util.cc',
          '../base/rand_util_nacl.cc',
          '../base/safe_strerror_posix.cc',
#          '../base/scoped_native_library.cc',
#          '../base/scoped_temp_dir.cc',
          '../base/sha1_portable.cc',
          '../base/shared_memory_nacl.cc',
          '../base/string_number_conversions.cc',
          '../base/string_piece.cc',
          '../base/string_split.cc',
          '../base/string_util.cc',
          '../base/string16.cc',
          '../base/stringprintf.cc',
          '../base/sync_socket_nacl.cc',
          '../base/synchronization/cancellation_flag.cc',
          '../base/synchronization/condition_variable_posix.cc',
          '../base/synchronization/lock.cc',
          '../base/synchronization/lock_impl_posix.cc',
          '../base/synchronization/waitable_event_posix.cc',
          '../base/synchronization/waitable_event_watcher_posix.cc',
          '../base/system_monitor/system_monitor.cc',
          '../base/system_monitor/system_monitor_posix.cc',
#          '../base/sys_info_posix.cc',
          '../base/sys_string_conversions_posix.cc',
          '../base/task_runner.cc',
          '../base/thread_task_runner_handle.cc',
          '../base/threading/non_thread_safe_impl.cc',
          '../base/threading/platform_thread_posix.cc',
          '../base/threading/post_task_and_reply_impl.cc',
#          '../base/threading/sequenced_worker_pool.cc',
          '../base/threading/simple_thread.cc',
          '../base/threading/thread.cc',
          '../base/threading/thread_checker_impl.cc',
          '../base/threading/thread_collision_warner.cc',
          '../base/threading/thread_local_posix.cc',
          '../base/threading/thread_local_storage_posix.cc',
          '../base/threading/thread_restrictions.cc',
          '../base/threading/watchdog.cc',
          '../base/threading/worker_pool.cc',
          '../base/threading/worker_pool_posix.cc',
          '../base/third_party/dmg_fp/g_fmt.cc',
          '../base/third_party/dmg_fp/dtoa_wrapper.cc',
          '../base/third_party/dynamic_annotations/dynamic_annotations.c',
          '../base/third_party/icu/icu_utf.cc',
          '../base/third_party/nspr/prtime.cc',
          '../base/time_posix.cc',
          '../base/time.cc',
          '../base/timer.cc',
          '../base/tracked_objects.cc',
          '../base/tracking_info.cc',
          '../base/utf_offset_string_conversions.cc',
          '../base/utf_string_conversion_utils.cc',
          '../base/utf_string_conversions.cc',
          '../base/values.cc',
          '../base/value_conversions.cc',
          '../base/version.cc',
          '../base/vlog.cc',
#          '../base/nix/mime_util_xdg.cc',
#          '../base/nix/xdg_util.cc',
        ],
        'compile_flags': [
          '-pthread',
        ],
      },
    },
    {
      'target_name': 'gpu_untrusted',
      'type': 'none',
      'dependencies': [
        'base_untrusted',
      ],
      'variables': {
        'nlib_target': 'libgpu_untrusted.a',
        'build_glibc': 0,
        'build_newlib': 1,
        'include_dirs': [
          '../gpu/command_buffer',
          '..',
          '../third_party/khronos',
        ],
        'sources': [
          '../gpu/command_buffer/client/gles2_implementation.cc',
          '../gpu/command_buffer/client/program_info_manager.cc',
          '../gpu/command_buffer/common/gles2_cmd_utils.cc',
          '../gpu/command_buffer/common/logging.cc',
          '../gpu/command_buffer/common/cmd_buffer_common.cc',
          '../gpu/command_buffer/common/gles2_cmd_format.cc',
          '../gpu/command_buffer/common/id_allocator.cc',
          '../gpu/command_buffer/client/cmd_buffer_helper.cc',
          '../gpu/command_buffer/client/fenced_allocator.cc',
          '../gpu/command_buffer/client/mapped_memory.cc',
          '../gpu/command_buffer/client/ring_buffer.cc',
          '../gpu/command_buffer/client/transfer_buffer.cc',
          '../gpu/command_buffer/client/gles2_cmd_helper.cc',
          '../gpu/ipc/gpu_command_buffer_traits.cc',
        ],
        'compile_flags': [
          '-pthread',
        ],
        'link_flags': [
          '-pthread',
          '-lbase_untrusted',
        ],
      },
    },
    {
      'target_name': 'ipc_untrusted',
      'type': 'none',
      'dependencies': [
        'base_untrusted',
      ],
      'variables': {
        'nlib_target': 'libipc_untrusted.a',
        'build_glibc': 0,
        'build_newlib': 1,
        'include_dirs': [
          '..',
        ],
        'sources': [
          '../ipc/file_descriptor_set_posix.cc',
          '../ipc/ipc_channel.cc',
          '../ipc/ipc_channel_nacl.cc',
          '../ipc/ipc_channel_proxy.cc',
          '../ipc/ipc_channel_reader.cc',
          '../ipc/ipc_logging.cc',
          '../ipc/ipc_message.cc',
          '../ipc/ipc_message_utils.cc',
          '../ipc/ipc_platform_file.cc',
          '../ipc/ipc_sync_channel.cc',
          '../ipc/ipc_sync_message.cc',
          '../ipc/ipc_sync_message_filter.cc',
        ],
        'compile_flags': [
          '-pthread',
        ],
        'link_flags': [
          '-pthread',
          '-lbase_untrusted',
        ],
      },
    },
    {
      'target_name': 'ppapi_proxy_untrusted',
      'type': 'none',
      'dependencies': [
        'base_untrusted',
        'gpu_untrusted',
        'ipc_untrusted',
      ],
      'variables': {
        'nexe_target': 'libppapi_proxy_untrusted.nexe',
        'build_glibc': 0,
        'build_newlib': 1,
        'include_dirs': [
          '..',
          '../third_party/khronos',
          '../third_party/skia/include/config',
        ],
        'sources': [
          'shared_impl/callback_tracker.cc',
          'shared_impl/file_type_conversion.cc',
          'shared_impl/id_assignment.cc',
          'shared_impl/platform_file.cc',
          'shared_impl/ppapi_globals.cc',
          'shared_impl/ppapi_preferences.cc',
          'shared_impl/ppb_audio_config_shared.cc',
#          'shared_impl/ppb_audio_input_shared.cc',
          'shared_impl/ppb_audio_shared.cc',
          'shared_impl/ppb_crypto_shared.cc',
          'shared_impl/ppb_device_ref_shared.cc',
          'shared_impl/ppb_file_io_shared.cc',
          'shared_impl/ppb_file_ref_shared.cc',
#          'shared_impl/ppb_graphics_3d_shared.cc',
          'shared_impl/ppb_image_data_shared.cc',
          'shared_impl/ppb_input_event_shared.cc',
          'shared_impl/ppb_instance_shared.cc',
          'shared_impl/ppb_memory_shared.cc',
#          'shared_impl/ppb_opengles2_shared.cc',
          'shared_impl/ppb_resource_array_shared.cc',
          'shared_impl/ppb_url_request_info_shared.cc',
#          'shared_impl/ppb_url_util_shared.cc',
          'shared_impl/ppb_var_shared.cc',
#          'shared_impl/ppb_video_decoder_shared.cc',
#          'shared_impl/ppb_video_capture_shared.cc',
          'shared_impl/ppb_view_shared.cc',
          'shared_impl/ppp_instance_combined.cc',
          'shared_impl/proxy_lock.cc',
          'shared_impl/resource.cc',
          'shared_impl/resource_tracker.cc',
          'shared_impl/scoped_pp_resource.cc',
          'shared_impl/time_conversion.cc',
          'shared_impl/tracked_callback.cc',
          'shared_impl/var.cc',
          'shared_impl/var_tracker.cc',

          'thunk/enter.cc',
          'thunk/ppb_audio_config_thunk.cc',
#          'thunk/ppb_audio_input_thunk.cc',
#          'thunk/ppb_audio_input_trusted_thunk.cc',
          'thunk/ppb_audio_thunk.cc',
#          'thunk/ppb_audio_trusted_thunk.cc',
#          'thunk/ppb_broker_thunk.cc',
#          'thunk/ppb_browser_font_trusted_thunk.cc',
#          'thunk/ppb_buffer_thunk.cc',
#          'thunk/ppb_buffer_trusted_thunk.cc',
#          'thunk/ppb_char_set_thunk.cc',
          'thunk/ppb_console_thunk.cc',
          'thunk/ppb_cursor_control_thunk.cc',
          'thunk/ppb_device_ref_thunk.cc',
#          'thunk/ppb_directory_reader_thunk.cc',
          'thunk/ppb_input_event_thunk.cc',
#          'thunk/ppb_file_chooser_thunk.cc',
          'thunk/ppb_file_io_thunk.cc',
#          'thunk/ppb_file_io_trusted_thunk.cc',
          'thunk/ppb_file_ref_thunk.cc',
          'thunk/ppb_file_system_thunk.cc',
          'thunk/ppb_find_thunk.cc',
#          'thunk/ppb_flash_clipboard_thunk.cc',
#          'thunk/ppb_flash_fullscreen_thunk.cc',
#          'thunk/ppb_flash_menu_thunk.cc',
#          'thunk/ppb_flash_message_loop_thunk.cc',
          'thunk/ppb_fullscreen_thunk.cc',
          'thunk/ppb_gamepad_thunk.cc',
#          'thunk/ppb_gles_chromium_texture_mapping_thunk.cc',
#          'thunk/ppb_graphics_2d_thunk.cc',
#          'thunk/ppb_graphics_3d_thunk.cc',
#          'thunk/ppb_graphics_3d_trusted_thunk.cc',
#          'thunk/ppb_host_resolver_private_thunk.cc',
          'thunk/ppb_image_data_thunk.cc',
#          'thunk/ppb_image_data_trusted_thunk.cc',
          'thunk/ppb_instance_thunk.cc',
#          'thunk/ppb_layer_compositor_thunk.cc',
          'thunk/ppb_messaging_thunk.cc',
          'thunk/ppb_mouse_lock_thunk.cc',
          'thunk/ppb_resource_array_thunk.cc',
#          'thunk/ppb_scrollbar_thunk.cc',
#          'thunk/ppb_talk_private_thunk.cc',
#          'thunk/ppb_tcp_server_socket_private_thunk.cc',
#          'thunk/ppb_tcp_socket_private_thunk.cc',
          'thunk/ppb_text_input_thunk.cc',
#          'thunk/ppb_udp_socket_private_thunk.cc',
          'thunk/ppb_url_loader_thunk.cc',
          'thunk/ppb_url_request_info_thunk.cc',
          'thunk/ppb_url_response_info_thunk.cc',
#          'thunk/ppb_url_util_thunk.cc',
#          'thunk/ppb_video_capture_thunk.cc',
#          'thunk/ppb_video_decoder_thunk.cc',
#          'thunk/ppb_video_layer_thunk.cc',
          'thunk/ppb_view_thunk.cc',
#          'thunk/ppb_websocket_thunk.cc',
          'thunk/ppb_widget_thunk.cc',
#          'thunk/ppb_x509_certificate_private_thunk.cc',
          'thunk/ppb_zoom_thunk.cc',

#          'proxy/broker_dispatcher.cc',
          'proxy/dispatcher.cc',
          'proxy/host_dispatcher.cc',
          'proxy/host_var_serialization_rules.cc',
          'proxy/interface_list.cc',
          'proxy/interface_proxy.cc',
          'proxy/plugin_array_buffer_var.cc',
          'proxy/plugin_dispatcher.cc',
          'proxy/plugin_globals.cc',
          'proxy/plugin_message_filter.cc',
          'proxy/plugin_resource_tracker.cc',
          'proxy/plugin_var_serialization_rules.cc',
          'proxy/plugin_var_tracker.cc',
          'proxy/ppapi_messages.cc',
          'proxy/ppapi_param_traits.cc',
#          'proxy/ppb_audio_input_proxy.cc',
          'proxy/ppb_audio_proxy.cc',
#          'proxy/ppb_broker_proxy.cc',
#          'proxy/ppb_buffer_proxy.cc',
          'proxy/ppb_core_proxy.cc',
#          'proxy/ppb_file_chooser_proxy.cc',
          'proxy/ppb_file_io_proxy.cc',
          'proxy/ppb_file_ref_proxy.cc',
          'proxy/ppb_file_system_proxy.cc',
#          'proxy/ppb_graphics_2d_proxy.cc',
#          'proxy/ppb_graphics_3d_proxy.cc',
#          'proxy/ppb_host_resolver_private_proxy.cc',
          'proxy/ppb_image_data_proxy.cc',
          'proxy/ppb_instance_proxy.cc',
          'proxy/ppb_message_loop_proxy.cc',
#          'proxy/ppb_network_monitor_private_proxy.cc',
#          'proxy/ppb_pdf_proxy.cc',
#          'proxy/ppb_talk_private_proxy.cc',
#          'proxy/ppb_tcp_server_socket_private_proxy.cc',
#          'proxy/ppb_tcp_socket_private_proxy.cc',
#          'proxy/ppb_testing_proxy.cc',
#          'proxy/ppb_udp_socket_private_proxy.cc',
          'proxy/ppb_url_loader_proxy.cc',
          'proxy/ppb_url_response_info_proxy.cc',
          'proxy/ppb_var_deprecated_proxy.cc',
#          'proxy/ppb_video_capture_proxy.cc',
#          'proxy/ppb_video_decoder_proxy.cc',
          'proxy/ppp_class_proxy.cc',
#          'proxy/ppp_graphics_3d_proxy.cc',
          'proxy/ppp_input_event_proxy.cc',
#          'proxy/ppp_instance_private_proxy.cc',
          'proxy/ppp_instance_proxy.cc',
          'proxy/ppp_messaging_proxy.cc',
          'proxy/ppp_mouse_lock_proxy.cc',
          'proxy/ppp_printing_proxy.cc',
          'proxy/ppp_text_input_proxy.cc',
#          'proxy/ppp_video_decoder_proxy.cc',
          'proxy/proxy_channel.cc',
          'proxy/proxy_module.cc',
          'proxy/proxy_object_var.cc',
          'proxy/resource_creation_proxy.cc',
          'proxy/serialized_structs.cc',
          'proxy/serialized_var.cc',
        ],
        'compile_flags': [
          '-pthread',
        ],
        'link_flags': [
#          '-lppapi_cpp',
#          '-lppapi',
#          '-lppruntime',
          '-pthread',
          '-lgpu_untrusted',
          '-lipc_untrusted',
          '-lbase_untrusted',
        ],
      },
    },
  ],
}
