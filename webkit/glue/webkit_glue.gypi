# Copyright (c) 2011 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

{
  'variables': {
    'conditions': [
      ['inside_chromium_build==0', {
        'webkit_src_dir': '../../../../..',
      },{
        'webkit_src_dir': '../../third_party/WebKit',
      }],
    ],
  },
  'targets': [
    {
      'target_name': 'webkit_resources',
      'type': 'none',
      'msvs_guid': '0B469837-3D46-484A-AFB3-C5A6C68730B9',
      'variables': {
        'grit_out_dir': '<(SHARED_INTERMEDIATE_DIR)/webkit',
      },
      'actions': [
        {
          'action_name': 'webkit_resources',
          'variables': {
            'grit_grd_file': 'webkit_resources.grd',
          },
          'includes': [ '../../build/grit_action.gypi' ],
        },
        {
          'action_name': 'webkit_chromium_resources',
          'variables': {
            'grit_grd_file': '<(webkit_src_dir)/Source/WebKit/chromium/WebKit.grd',
          },
          'includes': [ '../../build/grit_action.gypi' ],
        },
      ],
      'includes': [ '../../build/grit_target.gypi' ],
    },
    {
      'target_name': 'webkit_strings',
      'type': 'none',
      'msvs_guid': '60B43839-95E6-4526-A661-209F16335E0E',
      'variables': {
        'grit_out_dir': '<(SHARED_INTERMEDIATE_DIR)/webkit',
      },
      'actions': [
        {
          'action_name': 'webkit_strings',
          'variables': {
            'grit_grd_file': 'webkit_strings.grd',
          },
          'includes': [ '../../build/grit_action.gypi' ],
        },
      ],
      'includes': [ '../../build/grit_target.gypi' ],
    },
    {
      'target_name': 'webkit_user_agent',
      'type': 'static_library',
      'msvs_guid': 'DB162DE1-7D56-4C4A-8A9F-80D396CD7AA8',
      'dependencies': [
        '<(DEPTH)/app/app.gyp:app_base',
        '<(DEPTH)/base/base.gyp:base_i18n',
      ],
      'actions': [
        {
          'action_name': 'webkit_version',
          'inputs': [
            '<(script)',
            '<(webkit_src_dir)<(version_file)',
            '../../build/util/lastchange.py',  # Used by the script.
          ],
          'outputs': [
            '<(INTERMEDIATE_DIR)/webkit_version.h',
          ],
          'action': ['python', '<(script)', '<(webkit_src_dir)',
                     '<(version_file)', '<(INTERMEDIATE_DIR)'],
          'variables': {
            'script': '../build/webkit_version.py',
            # version_file is a relative path from |webkit_src_dir| to
            # the version file.  But gyp will eat the variable unless
            # it looks like an absolute path, so write it like one and
            # then use it carefully above.
            'version_file': '/Source/WebCore/Configurations/Version.xcconfig',
          },
        },
      ],
      'include_dirs': [
        '<(INTERMEDIATE_DIR)',
      ],
      'sources': [
        'user_agent.cc',
        'user_agent.h',
      ],
      # Dependents may rely on files generated by this target or one of its
      # own hard dependencies.
      'hard_dependency': 1,
      'conditions': [
      ],
    },
    {
      'target_name': 'glue',
      'type': 'static_library',
      'msvs_guid': 'C66B126D-0ECE-4CA2-B6DC-FA780AFBBF09',
      #TODO(dmichael): Remove this #define once all plugins are ported from
      #                PPP_Instance and PPB_Instance scripting functions.
      'defines': [
        'PPAPI_INSTANCE_REMOVE_SCRIPTING',
      ],
      'dependencies': [
        '<(DEPTH)/app/app.gyp:app_base',
        '<(DEPTH)/base/base.gyp:base_i18n',
        '<(DEPTH)/gpu/gpu.gyp:gles2_implementation',
        '<(DEPTH)/net/net.gyp:net',
        '<(DEPTH)/ppapi/ppapi.gyp:ppapi_c',
        '<(DEPTH)/ppapi/ppapi_internal.gyp:ppapi_shared',
        '<(DEPTH)/printing/printing.gyp:printing',
        '<(DEPTH)/skia/skia.gyp:skia',
        '<(DEPTH)/third_party/icu/icu.gyp:icui18n',
        '<(DEPTH)/third_party/icu/icu.gyp:icuuc',
        '<(DEPTH)/third_party/npapi/npapi.gyp:npapi',
        'webkit_resources',
        'webkit_strings',
        'webkit_user_agent',
      ],
      'actions': [
      ],
      'include_dirs': [
        '<(INTERMEDIATE_DIR)',
        '<(SHARED_INTERMEDIATE_DIR)/webkit',
      ],
      'sources': [
        # This list contains all .h, .cc, and .mm files in glue except for
        # those in the test subdirectory and those with unittest in in their
        # names.
        '../plugins/npapi/carbon_plugin_window_tracker_mac.cc',
        '../plugins/npapi/carbon_plugin_window_tracker_mac.h',
        '../plugins/npapi/coregraphics_private_symbols_mac.h',
        '../plugins/npapi/default_plugin_shared.h',
        '../plugins/npapi/gtk_plugin_container.cc',
        '../plugins/npapi/gtk_plugin_container.h',
        '../plugins/npapi/gtk_plugin_container_manager.cc',
        '../plugins/npapi/gtk_plugin_container_manager.h',
        '../plugins/npapi/npapi_extension_thunk.cc',
        '../plugins/npapi/npapi_extension_thunk.h',
        '../plugins/npapi/plugin_constants_win.cc',
        '../plugins/npapi/plugin_constants_win.h',
        '../plugins/npapi/plugin_group.cc',
        '../plugins/npapi/plugin_group.h',
        '../plugins/npapi/plugin_host.cc',
        '../plugins/npapi/plugin_host.h',
        '../plugins/npapi/plugin_instance.cc',
        '../plugins/npapi/plugin_instance.h',
        '../plugins/npapi/plugin_instance_mac.mm',
        '../plugins/npapi/plugin_lib.cc',
        '../plugins/npapi/plugin_lib.h',
        '../plugins/npapi/plugin_lib_mac.mm',
        '../plugins/npapi/plugin_lib_posix.cc',
        '../plugins/npapi/plugin_lib_win.cc',
        '../plugins/npapi/plugin_list.cc',
        '../plugins/npapi/plugin_list.h',
        '../plugins/npapi/plugin_list_mac.mm',
        '../plugins/npapi/plugin_list_posix.cc',
        '../plugins/npapi/plugin_list_win.cc',
        '../plugins/npapi/plugin_stream.cc',
        '../plugins/npapi/plugin_stream.h',
        '../plugins/npapi/plugin_stream_posix.cc',
        '../plugins/npapi/plugin_stream_url.cc',
        '../plugins/npapi/plugin_stream_url.h',
        '../plugins/npapi/plugin_stream_win.cc',
        '../plugins/npapi/plugin_string_stream.cc',
        '../plugins/npapi/plugin_string_stream.h',
        '../plugins/npapi/plugin_web_event_converter_mac.h',
        '../plugins/npapi/plugin_web_event_converter_mac.mm',
        '../plugins/npapi/quickdraw_drawing_manager_mac.cc',
        '../plugins/npapi/quickdraw_drawing_manager_mac.h',
        '../plugins/npapi/webplugin.cc',
        '../plugins/npapi/webplugin.h',
        '../plugins/npapi/webplugin_2d_device_delegate.cc',
        '../plugins/npapi/webplugin_2d_device_delegate.h',
        '../plugins/npapi/webplugin_3d_device_delegate.cc',
        '../plugins/npapi/webplugin_3d_device_delegate.h',
        '../plugins/npapi/webplugin_accelerated_surface_mac.h',
        '../plugins/npapi/webplugin_audio_device_delegate.cc',
        '../plugins/npapi/webplugin_audio_device_delegate.h',
        '../plugins/npapi/webplugin_delegate.cc',
        '../plugins/npapi/webplugin_delegate.h',
        '../plugins/npapi/webplugin_delegate_impl.cc',
        '../plugins/npapi/webplugin_delegate_impl.h',
        '../plugins/npapi/webplugin_delegate_impl_gtk.cc',
        '../plugins/npapi/webplugin_delegate_impl_mac.mm',
        '../plugins/npapi/webplugin_delegate_impl_win.cc',
        '../plugins/npapi/webplugin_file_delegate.cc',
        '../plugins/npapi/webplugin_file_delegate.h',
        '../plugins/npapi/webplugin_impl.cc',
        '../plugins/npapi/webplugin_impl.h',
        '../plugins/npapi/webplugin_print_delegate.cc',
        '../plugins/npapi/webplugin_print_delegate.h',
        '../plugins/npapi/webplugininfo.cc',
        '../plugins/npapi/webplugininfo.h',
        '../plugins/npapi/webview_plugin.cc',
        '../plugins/npapi/webview_plugin.h',
        '../plugins/plugin_switches.cc',
        '../plugins/plugin_switches.h',
        '../plugins/ppapi/callbacks.cc',
        '../plugins/ppapi/callbacks.h',
        '../plugins/ppapi/common.h',
        '../plugins/ppapi/dir_contents.h',
        '../plugins/ppapi/event_conversion.cc',
        '../plugins/ppapi/event_conversion.h',
        '../plugins/ppapi/file_callbacks.cc',
        '../plugins/ppapi/file_callbacks.h',
        '../plugins/ppapi/file_path.cc',
        '../plugins/ppapi/file_path.h',
        '../plugins/ppapi/file_type_conversions.cc',
        '../plugins/ppapi/file_type_conversions.h',
        '../plugins/ppapi/fullscreen_container.h',
        '../plugins/ppapi/message_channel.cc',
        '../plugins/ppapi/message_channel.h',
        '../plugins/ppapi/npapi_glue.cc',
        '../plugins/ppapi/npapi_glue.h',
        '../plugins/ppapi/plugin_delegate.h',
        '../plugins/ppapi/plugin_module.cc',
        '../plugins/ppapi/plugin_module.h',
        '../plugins/ppapi/plugin_object.cc',
        '../plugins/ppapi/plugin_object.h',
        '../plugins/ppapi/ppapi_interface_factory.cc',
        '../plugins/ppapi/ppapi_interface_factory.h',
        '../plugins/ppapi/ppapi_plugin_instance.cc',
        '../plugins/ppapi/ppapi_plugin_instance.h',
        '../plugins/ppapi/ppapi_webplugin_impl.cc',
        '../plugins/ppapi/ppapi_webplugin_impl.h',
        '../plugins/ppapi/ppb_audio_impl.cc',
        '../plugins/ppapi/ppb_audio_impl.h',
        '../plugins/ppapi/ppb_broker_impl.cc',
        '../plugins/ppapi/ppb_broker_impl.h',
        '../plugins/ppapi/ppb_buffer_impl.cc',
        '../plugins/ppapi/ppb_buffer_impl.h',
        '../plugins/ppapi/ppb_char_set_impl.cc',
        '../plugins/ppapi/ppb_char_set_impl.h',
        '../plugins/ppapi/ppb_console_impl.cc',
        '../plugins/ppapi/ppb_console_impl.h',
        '../plugins/ppapi/ppb_context_3d_impl.cc',
        '../plugins/ppapi/ppb_context_3d_impl.h',
        '../plugins/ppapi/ppb_crypto_impl.cc',
        '../plugins/ppapi/ppb_crypto_impl.h',
        '../plugins/ppapi/ppb_cursor_control_impl.cc',
        '../plugins/ppapi/ppb_cursor_control_impl.h',
        '../plugins/ppapi/ppb_directory_reader_impl.cc',
        '../plugins/ppapi/ppb_directory_reader_impl.h',
        '../plugins/ppapi/ppb_file_chooser_impl.cc',
        '../plugins/ppapi/ppb_file_chooser_impl.h',
        '../plugins/ppapi/ppb_file_io_impl.cc',
        '../plugins/ppapi/ppb_file_io_impl.h',
        '../plugins/ppapi/ppb_file_ref_impl.cc',
        '../plugins/ppapi/ppb_file_ref_impl.h',
        '../plugins/ppapi/ppb_file_system_impl.cc',
        '../plugins/ppapi/ppb_file_system_impl.h',
        '../plugins/ppapi/ppb_find_impl.cc',
        '../plugins/ppapi/ppb_find_impl.h',
        '../plugins/ppapi/ppb_flash_clipboard_impl.cc',
        '../plugins/ppapi/ppb_flash_clipboard_impl.h',
        '../plugins/ppapi/ppb_flash_file_impl.cc',
        '../plugins/ppapi/ppb_flash_file_impl.h',
        '../plugins/ppapi/ppb_flash_impl.cc',
        '../plugins/ppapi/ppb_flash_impl.h',
        '../plugins/ppapi/ppb_flash_impl_linux.cc',
        '../plugins/ppapi/ppb_flash_menu_impl.cc',
        '../plugins/ppapi/ppb_flash_menu_impl.h',
        '../plugins/ppapi/ppb_flash_net_connector_impl.cc',
        '../plugins/ppapi/ppb_flash_net_connector_impl.h',
        '../plugins/ppapi/ppb_font_impl.cc',
        '../plugins/ppapi/ppb_font_impl.h',
        '../plugins/ppapi/ppb_gles_chromium_texture_mapping_impl.cc',
        '../plugins/ppapi/ppb_gles_chromium_texture_mapping_impl.h',
        '../plugins/ppapi/ppb_graphics_2d_impl.cc',
        '../plugins/ppapi/ppb_graphics_2d_impl.h',
        '../plugins/ppapi/ppb_graphics_3d_impl.cc',
        '../plugins/ppapi/ppb_graphics_3d_impl.h',
        '../plugins/ppapi/ppb_image_data_impl.cc',
        '../plugins/ppapi/ppb_image_data_impl.h',
        '../plugins/ppapi/ppb_layer_compositor_impl.cc',
        '../plugins/ppapi/ppb_layer_compositor_impl.h',
        '../plugins/ppapi/ppb_opengles_impl.cc',
        '../plugins/ppapi/ppb_opengles_impl.h',
        '../plugins/ppapi/ppb_pdf_impl.cc',
        '../plugins/ppapi/ppb_pdf_impl.h',
        '../plugins/ppapi/ppb_proxy_impl.cc',
        '../plugins/ppapi/ppb_proxy_impl.h',
        '../plugins/ppapi/ppb_scrollbar_impl.cc',
        '../plugins/ppapi/ppb_scrollbar_impl.h',
        '../plugins/ppapi/ppb_surface_3d_impl.cc',
        '../plugins/ppapi/ppb_surface_3d_impl.h',
        '../plugins/ppapi/ppb_uma_private_impl.cc',
        '../plugins/ppapi/ppb_uma_private_impl.h',
        '../plugins/ppapi/ppb_url_loader_impl.cc',
        '../plugins/ppapi/ppb_url_loader_impl.h',
        '../plugins/ppapi/ppb_url_request_info_impl.cc',
        '../plugins/ppapi/ppb_url_request_info_impl.h',
        '../plugins/ppapi/ppb_url_response_info_impl.cc',
        '../plugins/ppapi/ppb_url_response_info_impl.h',
        '../plugins/ppapi/ppb_url_util_impl.cc',
        '../plugins/ppapi/ppb_url_util_impl.h',
        '../plugins/ppapi/ppb_video_decoder_impl.cc',
        '../plugins/ppapi/ppb_video_decoder_impl.h',
        '../plugins/ppapi/ppb_video_layer_impl.cc',
        '../plugins/ppapi/ppb_video_layer_impl.h',
        '../plugins/ppapi/ppb_video_layer_software.cc',
        '../plugins/ppapi/ppb_video_layer_software.h',
        '../plugins/ppapi/ppb_widget_impl.cc',
        '../plugins/ppapi/ppb_widget_impl.h',
        '../plugins/ppapi/resource.cc',
        '../plugins/ppapi/resource.h',
        '../plugins/ppapi/resource_creation_impl.cc',
        '../plugins/ppapi/resource_creation_impl.h',
        '../plugins/ppapi/resource_tracker.cc',
        '../plugins/ppapi/resource_tracker.h',
        '../plugins/ppapi/string.cc',
        '../plugins/ppapi/string.h',
        '../plugins/ppapi/var.cc',
        '../plugins/ppapi/var.h',
        '../plugins/ppapi/webkit_forwarding_impl.cc',
        '../plugins/ppapi/webkit_forwarding_impl.h',
        '../plugins/sad_plugin.cc',
        '../plugins/sad_plugin.h',
        'media/audio_decoder.cc',
        'media/audio_decoder.h',
        'media/buffered_data_source.cc',
        'media/buffered_data_source.h',
        'media/buffered_resource_loader.cc',
        'media/buffered_resource_loader.h',
        'media/simple_data_source.cc',
        'media/simple_data_source.h',
        'media/video_renderer_impl.cc',
        'media/video_renderer_impl.h',
        'media/web_data_source.cc',
        'media/web_data_source.h',
        'media/web_data_source_factory.cc',
        'media/web_data_source_factory.h',
        'media/web_video_renderer.h',
        'alt_error_page_resource_fetcher.cc',
        'alt_error_page_resource_fetcher.h',
        'context_menu.cc',
        'context_menu.h',
        'cpp_binding_example.cc',
        'cpp_binding_example.h',
        'cpp_bound_class.cc',
        'cpp_bound_class.h',
        'cpp_variant.cc',
        'cpp_variant.h',
        'dom_operations.cc',
        'dom_operations.h',
        'form_data.cc',
        'form_data.h',
        'form_field.cc',
        'form_field.h',
        'ftp_directory_listing_response_delegate.cc',
        'ftp_directory_listing_response_delegate.h',
        'gl_bindings_skia_cmd_buffer.cc',
        'gl_bindings_skia_cmd_buffer.h',
        'glue_serialize.cc',
        'glue_serialize.h',
        'idb_bindings.cc',
        'idb_bindings.h',
        'image_decoder.cc',
        'image_decoder.h',
        'image_resource_fetcher.cc',
        'image_resource_fetcher.h',
        'multipart_response_delegate.cc',
        'multipart_response_delegate.h',
        'npruntime_util.cc',
        'npruntime_util.h',
        'p2p_transport.h',
        'password_form.cc',
        'password_form.h',
        'password_form_dom_manager.cc',
        'password_form_dom_manager.h',
        'resource_fetcher.cc',
        'resource_fetcher.h',
        'resource_loader_bridge.cc',
        'resource_loader_bridge.h',
        'resource_type.h',
        'scoped_clipboard_writer_glue.h',
        'simple_webmimeregistry_impl.cc',
        'simple_webmimeregistry_impl.h',
        'site_isolation_metrics.cc',
        'site_isolation_metrics.h',
        'webaccessibility.cc',
        'webaccessibility.h',
        'webclipboard_impl.cc',
        'webclipboard_impl.h',
        'web_io_operators.cc',
        'web_io_operators.h',
        'webcookie.cc',
        'webcookie.h',
        'webcursor.cc',
        'webcursor.h',
        'webcursor_gtk.cc',
        'webcursor_gtk_data.h',
        'webcursor_mac.mm',
        'webcursor_win.cc',
        'webdropdata.cc',
        'webdropdata_win.cc',
        'webdropdata.h',
        'webfileutilities_impl.cc',
        'webfileutilities_impl.h',
        'webkit_constants.h',
        'webkit_glue.cc',
        'webkit_glue.h',
        'webkitclient_impl.cc',
        'webkitclient_impl.h',
        'webmediaplayer_impl.h',
        'webmediaplayer_impl.cc',
        'webmenuitem.cc',
        'webmenuitem.h',
        'webmenurunner_mac.h',
        'webmenurunner_mac.mm',
        'webpreferences.cc',
        'webpreferences.h',
        'websocketstreamhandle_bridge.h',
        'websocketstreamhandle_delegate.h',
        'websocketstreamhandle_impl.cc',
        'websocketstreamhandle_impl.h',
        'webthemeengine_impl_linux.cc',
        'webthemeengine_impl_mac.cc',
        'webthemeengine_impl_win.cc',
        'weburlloader_impl.cc',
        'weburlloader_impl.h',
        'webvideoframe_impl.cc',
        'webvideoframe_impl.h',
        'window_open_disposition.h',
        'window_open_disposition.cc',

        # These files used to be built in the webcore target, but moved here
        # since part of glue.
        '../extensions/v8/benchmarking_extension.cc',
        '../extensions/v8/benchmarking_extension.h',
        '../extensions/v8/gc_extension.cc',
        '../extensions/v8/gc_extension.h',
        '../extensions/v8/heap_profiler_extension.cc',
        '../extensions/v8/heap_profiler_extension.h',
        '../extensions/v8/playback_extension.cc',
        '../extensions/v8/playback_extension.h',
        '../extensions/v8/profiler_extension.cc',
        '../extensions/v8/profiler_extension.h',

      ],
      # When glue is a dependency, it needs to be a hard dependency.
      # Dependents may rely on files generated by this target or one of its
      # own hard dependencies.
      'hard_dependency': 1,
      'conditions': [
        ['toolkit_uses_gtk == 1', {
          'dependencies': [
            '<(DEPTH)/build/linux/system.gyp:gtk',
          ],
          'sources!': [
            'plugins/plugin_stubs.cc',
          ],
        }, { # else: toolkit_uses_gtk != 1
          'sources/': [['exclude', '_(linux|gtk)(_data)?\\.cc$'],
                       ['exclude', r'/gtk_']],
        }],
        ['OS!="mac"', {
          'sources/': [['exclude', '_mac\\.(cc|mm)$']],
          'sources!': [
            'webthemeengine_impl_mac.cc',
          ],
        }, {  # else: OS=="mac"
          'sources/': [['exclude', 'plugin_(lib|list)_posix\\.cc$']],
          'link_settings': {
            'libraries': [
              '$(SDKROOT)/System/Library/Frameworks/QuartzCore.framework',
            ],
          },
        }],
        ['enable_gpu!=1', {
          'sources!': [
            '../plugins/ppapi/ppb_graphics_3d_impl.cc',
            '../plugins/ppapi/ppb_graphics_3d_impl.h',
            '../plugins/ppapi/ppb_open_gl_es_impl.cc',
          ],
        }],
        ['OS!="win"', {
          'sources/': [['exclude', '_win\\.cc$']],
          'sources!': [
            'webthemeengine_impl_win.cc',
          ],
        }, {  # else: OS=="win"
          'sources/': [['exclude', '_posix\\.cc$']],
          'include_dirs': [
            '<(DEPTH)/third_party/wtl/include',
          ],
          'dependencies': [
            '<(DEPTH)/build/win/system.gyp:cygwin',
          ],
          'sources!': [
            'plugins/plugin_stubs.cc',
          ],
          'conditions': [
            ['inside_chromium_build==1 and component=="shared_library"', {
              'dependencies': [
                '<(DEPTH)/third_party/WebKit/Source/WebKit/chromium/WebKit.gyp:webkit',
                '<(DEPTH)/v8/tools/gyp/v8.gyp:v8',
               ],
               'export_dependent_settings': [
                 '<(DEPTH)/third_party/WebKit/Source/WebKit/chromium/WebKit.gyp:webkit',
                 '<(DEPTH)/v8/tools/gyp/v8.gyp:v8',
               ],
            }],
          ],
        }],
        ['inside_chromium_build==0', {
          'dependencies': [
            '<(DEPTH)/webkit/support/setup_third_party.gyp:third_party_headers',
          ],
        }],
        ['p2p_apis==1', {
          'sources': [
            '../plugins/ppapi/ppb_transport_impl.cc',
            '../plugins/ppapi/ppb_transport_impl.h',
          ],
        }],
      ],
    },
  ],
  'conditions': [
    ['use_third_party_translations==1', {
      'targets': [
        {
          'target_name': 'inspector_strings',
          'type': 'none',
          'variables': {
            'grit_out_dir': '<(PRODUCT_DIR)/resources/inspector/l10n',
          },
          'actions': [
            {
              'action_name': 'inspector_strings',
              'variables': {
                'grit_grd_file': 'inspector_strings.grd',
              },
              'includes': [ '../../build/grit_action.gypi' ],
            },
          ],
          'includes': [ '../../build/grit_target.gypi' ],
        },
      ],
    }],
  ],
}
