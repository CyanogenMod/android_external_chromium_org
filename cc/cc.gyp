# Copyright (c) 2012 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

{
  'variables': {
    'chromium_code': 1,
    'cc_source_files': [
      'animation/animation.cc',
      'animation/animation.h',
      'animation/animation_curve.cc',
      'animation/animation_curve.h',
      'animation/animation_events.cc',
      'animation/animation_events.h',
      'animation/animation_id_provider.cc',
      'animation/animation_id_provider.h',
      'animation/animation_registrar.cc',
      'animation/animation_registrar.h',
      'animation/keyframed_animation_curve.cc',
      'animation/keyframed_animation_curve.h',
      'animation/layer_animation_controller.cc',
      'animation/layer_animation_controller.h',
      'animation/layer_animation_event_observer.h',
      'animation/layer_animation_value_observer.h',
      'animation/scrollbar_animation_controller.h',
      'animation/scrollbar_animation_controller_linear_fade.cc',
      'animation/scrollbar_animation_controller_linear_fade.h',
      'animation/timing_function.cc',
      'animation/timing_function.h',
      'animation/transform_operation.cc',
      'animation/transform_operation.h',
      'animation/transform_operations.cc',
      'animation/transform_operations.h',
      'base/completion_event.h',
      'base/hash_pair.h',
      'base/math_util.cc',
      'base/math_util.h',
      'base/region.cc',
      'base/region.h',
      'base/scoped_ptr_algorithm.h',
      'base/scoped_ptr_deque.h',
      'base/scoped_ptr_hash_map.h',
      'base/scoped_ptr_vector.h',
      'base/switches.cc',
      'base/switches.h',
      'base/thread.h',
      'base/thread_impl.cc',
      'base/thread_impl.h',
      'base/tiling_data.cc',
      'base/tiling_data.h',
      'base/util.h',
      'base/worker_pool.cc',
      'base/worker_pool.h',
      'debug/debug_colors.cc',
      'debug/debug_colors.h',
      'debug/debug_rect_history.cc',
      'debug/debug_rect_history.h',
      'debug/devtools_instrumentation.h',
      'debug/fake_web_graphics_context_3d.cc',
      'debug/fake_web_graphics_context_3d.h',
      'debug/frame_rate_counter.cc',
      'debug/frame_rate_counter.h',
      'debug/layer_tree_debug_state.cc',
      'debug/layer_tree_debug_state.h',
      'debug/overdraw_metrics.cc',
      'debug/overdraw_metrics.h',
      'debug/paint_time_counter.cc',
      'debug/paint_time_counter.h',
      'debug/rendering_stats.cc',
      'debug/rendering_stats.h',
      'debug/rendering_stats_instrumentation.cc',
      'debug/rendering_stats_instrumentation.h',
      'debug/ring_buffer.h',
      'debug/traced_picture.cc',
      'debug/traced_picture.h',
      'debug/traced_value.cc',
      'debug/traced_value.h',
      'input/input_handler.h',
      'input/page_scale_animation.cc',
      'input/page_scale_animation.h',
      'input/top_controls_manager.cc',
      'input/top_controls_manager.h',
      'input/top_controls_manager_client.h',
      'layers/append_quads_data.h',
      'layers/compositing_reasons.h',
      'layers/content_layer.cc',
      'layers/content_layer.h',
      'layers/content_layer_client.h',
      'layers/contents_scaling_layer.cc',
      'layers/contents_scaling_layer.h',
      'layers/delegated_renderer_layer.cc',
      'layers/delegated_renderer_layer.h',
      'layers/delegated_renderer_layer_client.h',
      'layers/delegated_renderer_layer_impl.cc',
      'layers/delegated_renderer_layer_impl.h',
      'layers/draw_properties.h',
      'layers/heads_up_display_layer.cc',
      'layers/heads_up_display_layer.h',
      'layers/heads_up_display_layer_impl.cc',
      'layers/heads_up_display_layer_impl.h',
      'layers/image_layer.cc',
      'layers/image_layer.h',
      'layers/io_surface_layer.cc',
      'layers/io_surface_layer.h',
      'layers/io_surface_layer_impl.cc',
      'layers/io_surface_layer_impl.h',
      'layers/layer.cc',
      'layers/layer.h',
      'layers/layer_impl.cc',
      'layers/layer_impl.h',
      'layers/layer_iterator.cc',
      'layers/layer_iterator.h',
      'layers/layer_lists.h',
      'layers/layer_position_constraint.cc',
      'layers/layer_position_constraint.h',
      'layers/nine_patch_layer.cc',
      'layers/nine_patch_layer.h',
      'layers/nine_patch_layer_impl.cc',
      'layers/nine_patch_layer_impl.h',
      'layers/paint_properties.h',
      'layers/picture_image_layer.cc',
      'layers/picture_image_layer.h',
      'layers/picture_image_layer_impl.cc',
      'layers/picture_image_layer_impl.h',
      'layers/picture_layer.cc',
      'layers/picture_layer.h',
      'layers/picture_layer_impl.cc',
      'layers/picture_layer_impl.h',
      'layers/quad_sink.h',
      'layers/render_pass_sink.h',
      'layers/render_surface.cc',
      'layers/render_surface.h',
      'layers/render_surface_impl.cc',
      'layers/render_surface_impl.h',
      'layers/scrollbar_layer.cc',
      'layers/scrollbar_layer.h',
      'layers/scrollbar_layer_impl.cc',
      'layers/scrollbar_layer_impl.h',
      'layers/solid_color_layer.cc',
      'layers/solid_color_layer.h',
      'layers/solid_color_layer_impl.cc',
      'layers/solid_color_layer_impl.h',
      'layers/texture_layer.cc',
      'layers/texture_layer.h',
      'layers/texture_layer_client.h',
      'layers/texture_layer_impl.cc',
      'layers/texture_layer_impl.h',
      'layers/tiled_layer.cc',
      'layers/tiled_layer.h',
      'layers/tiled_layer_impl.cc',
      'layers/tiled_layer_impl.h',
      'layers/video_frame_provider.h',
      'layers/video_frame_provider_client_impl.cc',
      'layers/video_frame_provider_client_impl.h',
      'layers/video_layer.cc',
      'layers/video_layer.h',
      'layers/video_layer_impl.cc',
      'layers/video_layer_impl.h',
      'output/compositor_frame.cc',
      'output/compositor_frame.h',
      'output/compositor_frame_ack.cc',
      'output/compositor_frame_ack.h',
      'output/compositor_frame_metadata.cc',
      'output/compositor_frame_metadata.h',
      'output/context_provider.h',
      'output/copy_output_request.cc',
      'output/copy_output_request.h',
      'output/delegated_frame_data.h',
      'output/delegated_frame_data.cc',
      'output/delegating_renderer.cc',
      'output/delegating_renderer.h',
      'output/direct_renderer.cc',
      'output/direct_renderer.h',
      'output/geometry_binding.cc',
      'output/geometry_binding.h',
      'output/gl_frame_data.h',
      'output/gl_frame_data.cc',
      'output/gl_renderer.cc',
      'output/gl_renderer.h',
      'output/gl_renderer_draw_cache.cc',
      'output/gl_renderer_draw_cache.h',
      'output/output_surface.cc',
      'output/output_surface.h',
      'output/output_surface_client.h',
      'output/program_binding.cc',
      'output/program_binding.h',
      'output/render_surface_filters.cc',
      'output/render_surface_filters.h',
      'output/renderer.cc',
      'output/renderer.h',
      'output/shader.cc',
      'output/shader.h',
      'output/software_frame_data.cc',
      'output/software_frame_data.h',
      'output/software_output_device.cc',
      'output/software_output_device.h',
      'output/software_renderer.cc',
      'output/software_renderer.h',
      'output/texture_copier.cc',
      'output/texture_copier.h',
      'quads/checkerboard_draw_quad.cc',
      'quads/checkerboard_draw_quad.h',
      'quads/content_draw_quad_base.cc',
      'quads/content_draw_quad_base.h',
      'quads/debug_border_draw_quad.cc',
      'quads/debug_border_draw_quad.h',
      'quads/draw_quad.cc',
      'quads/draw_quad.h',
      'quads/io_surface_draw_quad.cc',
      'quads/io_surface_draw_quad.h',
      'quads/picture_draw_quad.cc',
      'quads/picture_draw_quad.h',
      'quads/render_pass.cc',
      'quads/render_pass.h',
      'quads/render_pass_draw_quad.cc',
      'quads/render_pass_draw_quad.h',
      'quads/shared_quad_state.cc',
      'quads/shared_quad_state.h',
      'quads/solid_color_draw_quad.cc',
      'quads/solid_color_draw_quad.h',
      'quads/stream_video_draw_quad.cc',
      'quads/stream_video_draw_quad.h',
      'quads/texture_draw_quad.cc',
      'quads/texture_draw_quad.h',
      'quads/tile_draw_quad.cc',
      'quads/tile_draw_quad.h',
      'quads/yuv_video_draw_quad.cc',
      'quads/yuv_video_draw_quad.h',
      'resources/bitmap_content_layer_updater.cc',
      'resources/bitmap_content_layer_updater.h',
      'resources/bitmap_skpicture_content_layer_updater.cc',
      'resources/bitmap_skpicture_content_layer_updater.h',
      'resources/caching_bitmap_content_layer_updater.cc',
      'resources/caching_bitmap_content_layer_updater.h',
      'resources/content_layer_updater.cc',
      'resources/content_layer_updater.h',
      'resources/image_layer_updater.cc',
      'resources/image_layer_updater.h',
      'resources/image_raster_worker_pool.cc',
      'resources/image_raster_worker_pool.h',
      'resources/layer_painter.h',
      'resources/layer_quad.cc',
      'resources/layer_quad.h',
      'resources/layer_tiling_data.cc',
      'resources/layer_tiling_data.h',
      'resources/layer_updater.cc',
      'resources/layer_updater.h',
      'resources/managed_memory_policy.cc',
      'resources/managed_memory_policy.h',
      'resources/managed_tile_state.cc',
      'resources/managed_tile_state.h',
      'resources/memory_history.cc',
      'resources/memory_history.h',
      'resources/picture.cc',
      'resources/picture.h',
      'resources/picture_layer_tiling.cc',
      'resources/picture_layer_tiling.h',
      'resources/picture_layer_tiling_set.cc',
      'resources/picture_layer_tiling_set.h',
      'resources/picture_pile.cc',
      'resources/picture_pile.h',
      'resources/picture_pile_base.cc',
      'resources/picture_pile_base.h',
      'resources/picture_pile_impl.cc',
      'resources/picture_pile_impl.h',
      'resources/pixel_buffer_raster_worker_pool.cc',
      'resources/pixel_buffer_raster_worker_pool.h',
      'resources/platform_color.h',
      'resources/prioritized_resource.cc',
      'resources/prioritized_resource.h',
      'resources/prioritized_resource_manager.cc',
      'resources/prioritized_resource_manager.h',
      'resources/priority_calculator.cc',
      'resources/priority_calculator.h',
      'resources/raster_worker_pool.cc',
      'resources/raster_worker_pool.h',
      'resources/resource.cc',
      'resources/resource.h',
      'resources/resource_pool.cc',
      'resources/resource_pool.h',
      'resources/resource_provider.cc',
      'resources/resource_provider.h',
      'resources/resource_update.cc',
      'resources/resource_update.h',
      'resources/resource_update_controller.cc',
      'resources/resource_update_controller.h',
      'resources/resource_update_queue.cc',
      'resources/resource_update_queue.h',
      'resources/scoped_resource.cc',
      'resources/scoped_resource.h',
      'resources/skpicture_content_layer_updater.cc',
      'resources/skpicture_content_layer_updater.h',
      'resources/sync_point_helper.cc',
      'resources/sync_point_helper.h',
      'resources/texture_mailbox.cc',
      'resources/texture_mailbox.h',
      'resources/tile.cc',
      'resources/tile.h',
      'resources/tile_manager.cc',
      'resources/tile_manager.h',
      'resources/tile_priority.cc',
      'resources/tile_priority.h',
      'resources/transferable_resource.cc',
      'resources/transferable_resource.h',
      'resources/video_resource_updater.cc',
      'resources/video_resource_updater.h',
      'scheduler/delay_based_time_source.cc',
      'scheduler/delay_based_time_source.h',
      'scheduler/frame_rate_controller.cc',
      'scheduler/frame_rate_controller.h',
      'scheduler/rate_limiter.cc',
      'scheduler/rate_limiter.h',
      'scheduler/rolling_time_delta_history.cc',
      'scheduler/rolling_time_delta_history.h',
      'scheduler/scheduler.cc',
      'scheduler/scheduler.h',
      'scheduler/scheduler_settings.cc',
      'scheduler/scheduler_settings.h',
      'scheduler/scheduler_state_machine.cc',
      'scheduler/scheduler_state_machine.h',
      'scheduler/texture_uploader.cc',
      'scheduler/texture_uploader.h',
      'scheduler/time_source.h',
      'scheduler/vsync_time_source.cc',
      'scheduler/vsync_time_source.h',
      'trees/damage_tracker.cc',
      'trees/damage_tracker.h',
      'trees/layer_sorter.cc',
      'trees/layer_sorter.h',
      'trees/layer_tree_host.cc',
      'trees/layer_tree_host.h',
      'trees/layer_tree_host_client.h',
      'trees/layer_tree_host_common.cc',
      'trees/layer_tree_host_common.h',
      'trees/layer_tree_host_impl.cc',
      'trees/layer_tree_host_impl.h',
      'trees/layer_tree_impl.cc',
      'trees/layer_tree_impl.h',
      'trees/layer_tree_settings.cc',
      'trees/layer_tree_settings.h',
      'trees/occlusion_tracker.cc',
      'trees/occlusion_tracker.h',
      'trees/proxy.cc',
      'trees/proxy.h',
      'trees/quad_culler.cc',
      'trees/quad_culler.h',
      'trees/single_thread_proxy.cc',
      'trees/single_thread_proxy.h',
      'trees/thread_proxy.cc',
      'trees/thread_proxy.h',
      'trees/tree_synchronizer.cc',
      'trees/tree_synchronizer.h',
    ],
  },
  'targets': [
    {
      'target_name': 'cc',
      'type': '<(component)',
      'dependencies': [
        '<(DEPTH)/base/base.gyp:base',
        '<(DEPTH)/base/third_party/dynamic_annotations/dynamic_annotations.gyp:dynamic_annotations',
        '<(DEPTH)/gpu/gpu.gyp:gpu',
        '<(DEPTH)/skia/skia.gyp:skia',
        '<(DEPTH)/media/media.gyp:media',
        '<(DEPTH)/ui/gl/gl.gyp:gl',
        '<(DEPTH)/ui/surface/surface.gyp:surface',
        '<(DEPTH)/ui/ui.gyp:ui',
        '<(DEPTH)/third_party/WebKit/Source/WebKit/chromium/WebKit.gyp:webkit',
      ],
      'defines': [
        'CC_IMPLEMENTATION=1',
      ],
      'sources': [
        '<@(cc_source_files)',
      ],
      # TODO(jschuh): crbug.com/167187 fix size_t to int truncations.
      'msvs_disabled_warnings': [ 4267, ],
    },
  ],
}
