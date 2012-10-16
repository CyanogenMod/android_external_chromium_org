# Copyright (c) 2012 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

{
  'variables': {
    'cc_source_files': [
      'hash_pair.h',
      'scoped_ptr_hash_map.h',
      'scoped_ptr_vector.h',
      'bitmap_canvas_layer_texture_updater.cc',
      'BitmapCanvasLayerTextureUpdater.h',
      'bitmap_skpicture_canvas_layer_texture_updater.cc',
      'BitmapSkPictureCanvasLayerTextureUpdater.h',
      'caching_bitmap_canvas_layer_texture_updater.cc',
      'caching_bitmap_canvas_layer_texture_updater.h',
      'active_animation.cc',
      'CCActiveAnimation.h',
      'CCAppendQuadsData.h',
      'animation_curve.cc',
      'CCAnimationCurve.h',
      'CCAnimationEvents.h',
      'checkerboard_draw_quad.cc',
      'CCCheckerboardDrawQuad.h',
      'CCCompletionEvent.h',
      'damage_tracker.cc',
      'CCDamageTracker.h',
      'debug_border_draw_quad.cc',
      'CCDebugBorderDrawQuad.h',
      'debug_rect_history.cc',
      'CCDebugRectHistory.h',
      'delay_based_time_source.cc',
      'CCDelayBasedTimeSource.h',
      'delegated_renderer_layer_impl.cc',
      'CCDelegatedRendererLayerImpl.h',
      'direct_renderer.cc',
      'CCDirectRenderer.h',
      'draw_quad.cc',
      'CCDrawQuad.h',
      'font_atlas.cc',
      'CCFontAtlas.h',
      'frame_rate_controller.cc',
      'CCFrameRateController.h',
      'frame_rate_counter.cc',
      'CCFrameRateCounter.h',
      'CCGraphicsContext.h',
      'heads_up_display_layer_impl.cc',
      'CCHeadsUpDisplayLayerImpl.h',
      'io_surface_draw_quad.cc',
      'CCIOSurfaceDrawQuad.h',
      'io_surface_layer_impl.cc',
      'CCIOSurfaceLayerImpl.h',
      'CCInputHandler.h',
      'keyframed_animation_curve.cc',
      'CCKeyframedAnimationCurve.h',
      'layer_animation_controller.cc',
      'CCLayerAnimationController.h',
      'layer_impl.cc',
      'CCLayerImpl.h',
      'layer_iterator.cc',
      'CCLayerIterator.h',
      'layer_quad.cc',
      'CCLayerQuad.h',
      'layer_sorter.cc',
      'CCLayerSorter.h',
      'layer_tiling_data.cc',
      'CCLayerTilingData.h',
      'layer_tree_host.cc',
      'CCLayerTreeHost.h',
      'CCLayerTreeHostClient.h',
      'layer_tree_host_common.cc',
      'CCLayerTreeHostCommon.h',
      'layer_tree_host_impl.cc',
      'CCLayerTreeHostImpl.h',
      'math_util.cc',
      'CCMathUtil.h',
      'occlusion_tracker.cc',
      'CCOcclusionTracker.h',
      'overdraw_metrics.cc',
      'CCOverdrawMetrics.h',
      'page_scale_animation.cc',
      'CCPageScaleAnimation.h',
      'prioritized_texture.cc',
      'CCPrioritizedTexture.h',
      'prioritized_texture_manager.cc',
      'CCPrioritizedTextureManager.h',
      'priority_calculator.cc',
      'CCPriorityCalculator.h',
      'proxy.cc',
      'CCProxy.h',
      'quad_culler.cc',
      'CCQuadCuller.h',
      'CCQuadSink.h',
      'render_pass.cc',
      'CCRenderPass.h',
      'render_pass_draw_quad.cc',
      'CCRenderPassDrawQuad.h',
      'CCRenderPassSink.h',
      'render_surface_impl.cc',
      'CCRenderSurface.h',
      'render_surface_filters.cc',
      'CCRenderSurfaceFilters.h',
      'renderer.cc',
      'CCRenderer.h',
      'gl_renderer.cc',
      'CCRendererGL.h',
      'software_renderer.cc',
      'CCRendererSoftware.h',
      'CCRenderingStats.h',
      'resource_provider.cc',
      'CCResourceProvider.h',
      'scheduler.cc',
      'CCScheduler.h',
      'scheduler_state_machine.cc',
      'CCSchedulerStateMachine.h',
      'scoped_texture.cc',
      'CCScopedTexture.h',
      'scoped_thread_proxy.cc',
      'CCScopedThreadProxy.h',
      'scrollbar_animation_controller.cc',
      'CCScrollbarAnimationController.h',
      'scrollbar_animation_controller_linear_fade.cc',
      'CCScrollbarAnimationControllerLinearFade.h',
      'scrollbar_layer_impl.cc',
      'CCScrollbarLayerImpl.h',
      'scrollbar_geometry_fixed_thumb.cc',
      'CCScrollbarGeometryFixedThumb.h',
      'scrollbar_geometry_stub.cc',
      'CCScrollbarGeometryStub.h',
      'settings.cc',
      'CCSettings.h',
      'shared_quad_state.cc',
      'CCSharedQuadState.h',
      'single_thread_proxy.cc',
      'CCSingleThreadProxy.h',
      'solid_color_draw_quad.cc',
      'CCSolidColorDrawQuad.h',
      'solid_color_layer_impl.cc',
      'CCSolidColorLayerImpl.h',
      'stream_video_draw_quad.cc',
      'CCStreamVideoDrawQuad.h',
      'texture.cc',
      'CCTexture.h',
      'texture_draw_quad.cc',
      'CCTextureDrawQuad.h',
      'texture_layer_impl.cc',
      'CCTextureLayerImpl.h',
      'texture_update_controller.cc',
      'CCTextureUpdateController.h',
      'texture_update_queue.cc',
      'CCTextureUpdateQueue.h',
      'CCThread.h',
      'thread_proxy.cc',
      'CCThreadProxy.h',
      'CCThreadTask.h',
      'tile_draw_quad.cc',
      'CCTileDrawQuad.h',
      'tiled_layer_impl.cc',
      'CCTiledLayerImpl.h',
      'CCTimeSource.h',
      'timer.cc',
      'CCTimer.h',
      'timing_function.cc',
      'CCTimingFunction.h',
      'video_layer_impl.cc',
      'CCVideoLayerImpl.h',
      'yuv_video_draw_quad.cc',
      'CCYUVVideoDrawQuad.h',
      'canvas_layer_texture_updater.cc',
      'CanvasLayerTextureUpdater.h',
      'content_layer.cc',
      'ContentLayerChromium.h',
      'ContentLayerChromiumClient.h',
      'delegated_renderer_layer.cc',
      'DelegatedRendererLayerChromium.h',
      'frame_buffer_skpicture_canvas_layer_texture_updater.cc',
      'FrameBufferSkPictureCanvasLayerTextureUpdater.h',
      'geometry_binding.cc',
      'GeometryBinding.h',
      'heads_up_display_layer.cc',
      'HeadsUpDisplayLayerChromium.h',
      'io_surface_layer.cc',
      'IOSurfaceLayerChromium.h',
      'image_layer.cc',
      'ImageLayerChromium.h',
      'layer.cc',
      'LayerChromium.h',
      'LayerPainterChromium.h',
      'layer_texture_sub_image.cc',
      'LayerTextureSubImage.h',
      'layer_texture_updater.cc',
      'LayerTextureUpdater.h',
      'PlatformColor.h',
      'program_binding.cc',
      'ProgramBinding.h',
      'rate_limiter.cc',
      'RateLimiter.h',
      'render_surface.cc',
      'RenderSurfaceChromium.h',
      'scrollbar_layer.cc',
      'ScrollbarLayerChromium.h',
      'shader.cc',
      'ShaderChromium.h',
      'skpicture_canvas_layer_texture_updater.cc',
      'SkPictureCanvasLayerTextureUpdater.h',
      'solid_color_layer.cc',
      'SolidColorLayerChromium.h',
      'switches.cc',
      'switches.h',
      'texture_copier.cc',
      'TextureCopier.h',
      'texture_layer.cc',
      'TextureLayerChromium.h',
      'TextureLayerChromiumClient.h',
      'TextureUploader.h',
      'throttled_texture_uploader.cc',
      'ThrottledTextureUploader.h',
      'unthrottled_texture_uploader.cc',
      'UnthrottledTextureUploader.h',
      'tiled_layer.cc',
      'TiledLayerChromium.h',
      'tree_synchronizer.cc',
      'TreeSynchronizer.h',
      'video_layer.cc',
      'VideoLayerChromium.h',

      'bitmap_canvas_layer_texture_updater.h',
      'bitmap_skpicture_canvas_layer_texture_updater.h',
      'canvas_layer_texture_updater.h',
      'active_animation.h',
      'animation_curve.h',
      'animation_events.h',
      'append_quads_data.h',
      'checkerboard_draw_quad.h',
      'completion_event.h',
      'damage_tracker.h',
      'debug_border_draw_quad.h',
      'debug_rect_history.h',
      'delay_based_time_source.h',
      'delegated_renderer_layer_impl.h',
      'direct_renderer.h',
      'draw_quad.h',
      'font_atlas.h',
      'frame_rate_controller.h',
      'frame_rate_counter.h',
      'graphics_context.h',
      'heads_up_display_layer_impl.h',
      'input_handler.h',
      'io_surface_draw_quad.h',
      'io_surface_layer_impl.h',
      'keyframed_animation_curve.h',
      'layer_animation_controller.h',
      'layer_impl.h',
      'layer_iterator.h',
      'layer_quad.h',
      'layer_sorter.h',
      'layer_tiling_data.h',
      'layer_tree_host_client.h',
      'layer_tree_host_common.h',
      'layer_tree_host.h',
      'layer_tree_host_impl.h',
      'math_util.h',
      'occlusion_tracker.h',
      'overdraw_metrics.h',
      'page_scale_animation.h',
      'prioritized_texture.h',
      'prioritized_texture_manager.h',
      'priority_calculator.h',
      'proxy.h',
      'quad_culler.h',
      'quad_sink.h',
      'gl_renderer.h',
      'renderer.h',
      'software_renderer.h',
      'rendering_stats.h',
      'render_pass_draw_quad.h',
      'render_pass.h',
      'render_pass_sink.h',
      'render_surface_filters.h',
      'render_surface_impl.h',
      'resource_provider.h',
      'scheduler.h',
      'scheduler_state_machine.h',
      'scoped_texture.h',
      'scoped_thread_proxy.h',
      'scrollbar_animation_controller.h',
      'scrollbar_animation_controller_linear_fade.h',
      'scrollbar_geometry_fixed_thumb.h',
      'scrollbar_geometry_stub.h',
      'scrollbar_layer_impl.h',
      'settings.h',
      'shared_quad_state.h',
      'single_thread_proxy.h',
      'solid_color_draw_quad.h',
      'solid_color_layer_impl.h',
      'stream_video_draw_quad.h',
      'texture_draw_quad.h',
      'texture.h',
      'texture_layer_impl.h',
      'texture_update_controller.h',
      'texture_update_queue.h',
      'threaded_test.h',
      'thread.h',
      'thread_proxy.h',
      'thread_task.h',
      'tiled_layer_impl.h',
      'tile_draw_quad.h',
      'timer.h',
      'time_source.h',
      'timing_function.h',
      'video_layer_impl.h',
      'yuv_video_draw_quad.h',
      'content_layer_client.h',
      'content_layer.h',
      'delegated_renderer_layer.h',
      'frame_buffer_skpicture_canvas_layer_texture_updater.h',
      'geometry_binding.h',
      'heads_up_display_layer.h',
      'image_layer.h',
      'io_surface_layer.h',
      'layer.h',
      'layer_painter.h',
      'layer_texture_sub_image.h',
      'layer_texture_updater.h',
      'platform_color.h',
      'program_binding.h',
      'rate_limiter.h',
      'render_surface.h',
      'scrollbar_layer.h',
      'shader.h',
      'skpicture_canvas_layer_texture_updater.h',
      'solid_color_layer.h',
      'texture_copier.h',
      'texture_layer_client.h',
      'texture_layer.h',
      'texture_uploader.h',
      'throttled_texture_uploader.h',
      'tiled_layer.h',
      'tree_synchronizer.h',
      'unthrottled_texture_uploader.h',
      'video_layer.h',
    ],
  },
  'targets': [
    {
      'target_name': 'cc',
      'type': 'static_library',
      'includes': [
        'cc.gypi',
      ],
      'dependencies': [
        '<(DEPTH)/base/base.gyp:base',
        '<(DEPTH)/base/third_party/dynamic_annotations/dynamic_annotations.gyp:dynamic_annotations',
        '<(DEPTH)/skia/skia.gyp:skia',
        '<(DEPTH)/ui/gl/gl.gyp:gl',
        '<(DEPTH)/ui/ui.gyp:ui',
        '<(webkit_src_dir)/Source/WTF/WTF.gyp/WTF.gyp:wtf',
        '<(webkit_src_dir)/Source/WebCore/WebCore.gyp/WebCore.gyp:webcore_platform_geometry',
        '<(webkit_src_dir)/Source/WebKit/chromium/WebKit.gyp:webkit_wtf_support',
      ],
      'defines': [
        # http://crbug.com/154052
        'WEBKIT_GLUE_IMPLEMENTATION=1',
      ],
      'include_dirs': [
        '<(webkit_src_dir)/Source/Platform/chromium',
        '<@(cc_stubs_dirs)',
      ],
      'sources': [
        '<@(cc_source_files)',
        'stubs/Extensions3D.h',
        'stubs/Extensions3DChromium.h',
        'stubs/FloatPoint.h',
        'stubs/FloatPoint3D.h',
        'stubs/FloatQuad.h',
        'stubs/FloatRect.h',
        'stubs/FloatSize.h',
        'stubs/GraphicsContext3D.h',
        'stubs/GraphicsTypes3D.h',
        'stubs/IntPoint.h',
        'stubs/IntRect.h',
        'stubs/IntSize.h',
        'stubs/NotImplemented.h',
        'stubs/Region.h',
        'stubs/SkiaUtils.h',
        'stubs/TilingData.h',
        'stubs/TraceEvent.h',
        'stubs/UnitBezier.h',

        'stubs/extensions_3d_chromium.h',
        'stubs/extensions_3d.h',
        'stubs/float_point_3d.h',
        'stubs/float_point.h',
        'stubs/float_quad.h',
        'stubs/float_rect.h',
        'stubs/float_size.h',
        'stubs/graphics_context_3d.h',
        'stubs/graphics_types_3d.h',
        'stubs/int_point.h',
        'stubs/int_rect.h',
        'stubs/int_size.h',
        'stubs/not_implemented.h',
        'stubs/skia_utils.h',
        'stubs/tiling_data.h',
        'stubs/trace_event.h',
        'stubs/unit_bezier.h',
      ],
    },
  ],
}
