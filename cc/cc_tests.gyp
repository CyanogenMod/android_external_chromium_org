# Copyright (c) 2012 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

{
  'variables': {
    'chromium_code': 1,
    'cc_unit_tests_source_files': [
      'animation/animation_unittest.cc',
      'animation/keyframed_animation_curve_unittest.cc',
      'animation/layer_animation_controller_unittest.cc',
      'animation/scrollbar_animation_controller_linear_fade_unittest.cc',
      'animation/timing_function_unittest.cc',
      'animation/transform_operations_unittest.cc',
      'base/float_quad_unittest.cc',
      'base/hash_pair_unittest.cc',
      'base/math_util_unittest.cc',
      'base/region_unittest.cc',
      'base/scoped_ptr_vector_unittest.cc',
      'base/tiling_data_unittest.cc',
      'base/util_unittest.cc',
      'input/top_controls_manager_unittest.cc',
      'layers/content_layer_unittest.cc',
      'layers/contents_scaling_layer_unittest.cc',
      'layers/delegated_renderer_layer_impl_unittest.cc',
      'layers/heads_up_display_unittest.cc',
      'layers/layer_impl_unittest.cc',
      'layers/layer_iterator_unittest.cc',
      'layers/layer_position_constraint_unittest.cc',
      'layers/layer_unittest.cc',
      'layers/nine_patch_layer_impl_unittest.cc',
      'layers/nine_patch_layer_unittest.cc',
      'layers/picture_image_layer_impl_unittest.cc',
      'layers/picture_layer_impl_unittest.cc',
      'layers/render_surface_unittest.cc',
      'layers/scrollbar_layer_unittest.cc',
      'layers/solid_color_layer_impl_unittest.cc',
      'layers/texture_layer_unittest.cc',
      'layers/tiled_layer_impl_unittest.cc',
      'layers/tiled_layer_unittest.cc',
      'output/delegating_renderer_unittest.cc',
      'output/gl_renderer_unittest.cc',
      'output/output_surface_unittest.cc',
      'output/renderer_pixeltest.cc',
      'output/render_surface_filters_unittest.cc',
      'output/shader_unittest.cc',
      'output/software_renderer_unittest.cc',
      'output/texture_copier_unittest.cc',
      'quads/draw_quad_unittest.cc',
      'quads/render_pass_unittest.cc',
      'resources/layer_quad_unittest.cc',
      'resources/picture_layer_tiling_set_unittest.cc',
      'resources/picture_layer_tiling_unittest.cc',
      'resources/picture_pile_impl_unittest.cc',
      'resources/picture_pile_unittest.cc',
      'resources/picture_unittest.cc',
      'resources/prioritized_resource_unittest.cc',
      'resources/raster_worker_pool_unittest.cc',
      'resources/resource_provider_unittest.cc',
      'resources/resource_update_controller_unittest.cc',
      'resources/scoped_resource_unittest.cc',
      'resources/tile_manager_unittest.cc',
      'resources/tile_priority_unittest.cc',
      'resources/worker_pool_unittest.cc',
      'scheduler/delay_based_time_source_unittest.cc',
      'scheduler/frame_rate_controller_unittest.cc',
      'scheduler/rolling_time_delta_history_unittest.cc',
      'scheduler/scheduler_state_machine_unittest.cc',
      'scheduler/scheduler_unittest.cc',
      'scheduler/texture_uploader_unittest.cc',
      'test/fake_web_graphics_context_3d_unittest.cc',
      'trees/damage_tracker_unittest.cc',
      'trees/layer_sorter_unittest.cc',
      'trees/layer_tree_host_common_unittest.cc',
      'trees/layer_tree_host_impl_unittest.cc',
      'trees/layer_tree_host_pixeltest_filters.cc',
      'trees/layer_tree_host_pixeltest_masks.cc',
      'trees/layer_tree_host_pixeltest_on_demand_raster.cc',
      'trees/layer_tree_host_pixeltest_readback.cc',
      'trees/layer_tree_host_unittest_animation.cc',
      'trees/layer_tree_host_unittest.cc',
      'trees/layer_tree_host_unittest_context.cc',
      'trees/layer_tree_host_unittest_damage.cc',
      'trees/layer_tree_host_unittest_delegated.cc',
      'trees/layer_tree_host_unittest_occlusion.cc',
      'trees/layer_tree_host_unittest_picture.cc',
      'trees/layer_tree_host_unittest_scroll.cc',
      'trees/layer_tree_host_unittest_video.cc',
      'trees/occlusion_tracker_unittest.cc',
      'trees/quad_culler_unittest.cc',
      'trees/tree_synchronizer_unittest.cc',
    ],
    'cc_tests_support_files': [
      'test/animation_test_common.cc',
      'test/animation_test_common.h',
      'test/fake_content_layer.cc',
      'test/fake_content_layer.h',
      'test/fake_content_layer_client.cc',
      'test/fake_content_layer_client.h',
      'test/fake_content_layer_impl.cc',
      'test/fake_content_layer_impl.h',
      'test/fake_context_provider.cc',
      'test/fake_context_provider.h',
      'test/fake_delegated_renderer_layer.cc',
      'test/fake_delegated_renderer_layer.h',
      'test/fake_delegated_renderer_layer_impl.cc',
      'test/fake_delegated_renderer_layer_impl.h',
      'test/fake_impl_proxy.h',
      'test/fake_output_surface.h',
      'test/fake_layer_tree_host_client.cc',
      'test/fake_layer_tree_host_client.h',
      'test/fake_layer_tree_host_impl.cc',
      'test/fake_layer_tree_host_impl_client.cc',
      'test/fake_layer_tree_host_impl_client.h',
      'test/fake_layer_tree_host_impl.h',
      'test/fake_picture_layer.cc',
      'test/fake_picture_layer.h',
      'test/fake_picture_layer_impl.cc',
      'test/fake_picture_layer_impl.h',
      'test/fake_picture_layer_tiling_client.cc',
      'test/fake_picture_layer_tiling_client.h',
      'test/fake_picture_pile_impl.cc',
      'test/fake_picture_pile_impl.h',
      'test/fake_proxy.cc',
      'test/fake_proxy.h',
      'test/fake_rendering_stats_instrumentation.h',
      'test/fake_scrollbar.cc',
      'test/fake_scrollbar.h',
      'test/fake_scrollbar_layer.cc',
      'test/fake_scrollbar_layer.h',
      'test/fake_tile_manager.cc',
      'test/fake_tile_manager.h',
      'test/fake_tile_manager_client.h',
      'test/fake_tile_manager_client.cc',
      'test/fake_output_surface.cc',
      'test/fake_output_surface.h',
      'test/fake_video_frame_provider.cc',
      'test/fake_video_frame_provider.h',
      'test/geometry_test_utils.cc',
      'test/geometry_test_utils.h',
      'test/impl_side_painting_settings.h',
      'test/layer_test_common.cc',
      'test/layer_test_common.h',
      'test/layer_tree_pixel_test.cc',
      'test/layer_tree_pixel_test.h',
      'test/layer_tree_test.cc',
      'test/layer_tree_test.h',
      'test/layer_tree_json_parser.cc',
      'test/layer_tree_json_parser.h',
      'test/mock_quad_culler.cc',
      'test/mock_quad_culler.h',
      'test/occlusion_tracker_test_common.h',
      'test/paths.cc',
      'test/paths.h',
      'test/pixel_test.cc',
      'test/pixel_test.h',
      'test/render_pass_test_common.cc',
      'test/render_pass_test_common.h',
      'test/render_pass_test_utils.cc',
      'test/render_pass_test_utils.h',
      'test/scheduler_test_common.cc',
      'test/scheduler_test_common.h',
      'test/skia_common.cc',
      'test/skia_common.h',
      'test/test_web_graphics_context_3d.cc',
      'test/test_web_graphics_context_3d.h',
      'test/tiled_layer_test_common.cc',
      'test/tiled_layer_test_common.h',
    ],
  },
  'targets': [
    {
      'target_name': 'cc_unittests',
      'type': '<(gtest_target_type)',
      'dependencies': [
        '../base/base.gyp:test_support_base',
        '../gpu/gpu.gyp:gpu',
        '../media/media.gyp:media',
        '../skia/skia.gyp:skia',
        '../testing/gmock.gyp:gmock',
        '../testing/gtest.gyp:gtest',
        '../ui/ui.gyp:ui',
        '../webkit/common/gpu/webkit_gpu.gyp:webkit_gpu',
        'cc.gyp:cc',
        'cc_test_support',
        'cc_test_utils',
      ],
      'sources': [
        'test/run_all_unittests.cc',
        'test/cc_test_suite.cc',
        '<@(cc_unit_tests_source_files)',
      ],
      'include_dirs': [
        'test',
        '.',
      ],
      'conditions': [
        ['OS == "android" and gtest_target_type == "shared_library"', {
          'dependencies': [
            '../testing/android/native_test.gyp:native_test_native_code',
          ],
        }],
        [ 'os_posix == 1 and OS != "mac" and OS != "android" and OS != "ios"', {
          'conditions': [
            [ 'linux_use_tcmalloc==1', {
              'dependencies': [
                '../base/allocator/allocator.gyp:allocator',
              ],
            }],
          ],
        }],
      ],
      # TODO(jschuh): crbug.com/167187 fix size_t to int truncations.
      'msvs_disabled_warnings': [ 4267, ],
    },
    {
      'target_name': 'cc_perftests',
      'type': '<(gtest_target_type)',
      'dependencies': [
        '../base/base.gyp:test_support_base',
        '../media/media.gyp:media',
        '../skia/skia.gyp:skia',
        '../testing/gmock.gyp:gmock',
        '../testing/gtest.gyp:gtest',
        '../ui/ui.gyp:ui',
        'cc.gyp:cc',
        'cc_test_support',
      ],
      'sources': [
        'resources/worker_pool_perftest.cc',
        'test/cc_test_suite.cc',
        'test/run_all_unittests.cc',
        'trees/layer_tree_host_perftest.cc',
      ],
      'include_dirs': [
        'test',
        '.',
      ],
      'conditions': [
        ['OS == "android" and gtest_target_type == "shared_library"', {
          'dependencies': [
            '../testing/android/native_test.gyp:native_test_native_code',
          ],
        }],
        # See http://crbug.com/162998#c4 for why this is needed.
        ['OS=="linux" and linux_use_tcmalloc==1', {
          'dependencies': [
            '../base/allocator/allocator.gyp:allocator',
          ],
        }],
      ],
      # TODO(jschuh): crbug.com/167187 fix size_t to int truncations.
      'msvs_disabled_warnings': [ 4267, ],
    },
    {
      'target_name': 'cc_test_support',
      'type': 'static_library',
      'include_dirs': [
        'test',
        '.',
        '..',
      ],
      'dependencies': [
        '../skia/skia.gyp:skia',
        '../testing/gmock.gyp:gmock',
        '../testing/gtest.gyp:gtest',
        '../third_party/WebKit/public/blink.gyp:blink',
        '../third_party/mesa/mesa.gyp:osmesa',
        '../ui/gl/gl.gyp:gl',
        '../ui/ui.gyp:ui',
      ],
      'sources': [
        '<@(cc_tests_support_files)',
      ],
      # TODO(jschuh): crbug.com/167187 fix size_t to int truncations.
      'msvs_disabled_warnings': [ 4267, ],
    },
    {
      'target_name': 'cc_test_utils',
      'type': 'static_library',
      'include_dirs': [
        '..'
      ],
      'sources': [
        'test/pixel_comparator.cc',
        'test/pixel_comparator.h',
        'test/pixel_test_utils.cc',
        'test/pixel_test_utils.h',
      ],
      'dependencies': [
        '../skia/skia.gyp:skia',
        '../ui/ui.gyp:ui',  # for png_codec
      ],
    },
  ],
  'conditions': [
    # Special target to wrap a gtest_target_type==shared_library
    # cc_unittests into an android apk for execution.
    ['OS == "android" and gtest_target_type == "shared_library"', {
      'targets': [
        {
          'target_name': 'cc_unittests_apk',
          'type': 'none',
          'dependencies': [
            'cc_unittests',
          ],
          'variables': {
            'test_suite_name': 'cc_unittests',
            'input_shlib_path': '<(SHARED_LIB_DIR)/<(SHARED_LIB_PREFIX)cc_unittests<(SHARED_LIB_SUFFIX)',
          },
          'includes': [ '../build/apk_test.gypi' ],
        },
        {
          'target_name': 'cc_perftests_apk',
          'type': 'none',
          'dependencies': [
            'cc_perftests',
          ],
          'variables': {
            'test_suite_name': 'cc_perftests',
            'input_shlib_path': '<(SHARED_LIB_DIR)/<(SHARED_LIB_PREFIX)cc_perftests<(SHARED_LIB_SUFFIX)',
          },
          'includes': [ '../build/apk_test.gypi' ],
        },
      ],
    }]
  ],
}
