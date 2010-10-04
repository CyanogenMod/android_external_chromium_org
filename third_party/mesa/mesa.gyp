# Copyright (c) 2010 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

{
  'variables': {
  },
  'target_defaults': {
    'conditions': [
      ['OS=="linux" or OS=="freebsd" or OS=="openbsd" or OS=="solaris"', {
        'cflags': [
          '-fPIC',
        ],
      }],
    ],
  },
  'targets': [
    {
      'target_name': 'mesa',
      'type': 'static_library',
      'include_dirs': [
        'MesaLib/include',
        'MesaLib/src/mesa',
      ],
      'sources': [
        'MesaLib/src/mesa/main/accum.c',
        'MesaLib/src/mesa/main/api_arrayelt.c',
        'MesaLib/src/mesa/main/api_exec.c',
        'MesaLib/src/mesa/main/api_loopback.c',
        'MesaLib/src/mesa/main/api_noop.c',
        'MesaLib/src/mesa/main/api_validate.c',
        'MesaLib/src/mesa/shader/arbprogparse.c',
        'MesaLib/src/mesa/shader/arbprogram.c',
        'MesaLib/src/mesa/main/arrayobj.c',
        'MesaLib/src/mesa/shader/atifragshader.c',
        'MesaLib/src/mesa/main/attrib.c',
        'MesaLib/src/mesa/main/blend.c',
        'MesaLib/src/mesa/main/bufferobj.c',
        'MesaLib/src/mesa/main/buffers.c',
        'MesaLib/src/mesa/main/clear.c',
        'MesaLib/src/mesa/main/clip.c',
        'MesaLib/src/mesa/main/colortab.c',
        'MesaLib/src/mesa/main/context.c',
        'MesaLib/src/mesa/main/convolve.c',
        'MesaLib/src/mesa/main/cpuinfo.c',
        'MesaLib/src/mesa/main/debug.c',
        'MesaLib/src/mesa/main/depth.c',
        'MesaLib/src/mesa/main/depthstencil.c',
        'MesaLib/src/mesa/main/dispatch.c',
        'MesaLib/src/mesa/main/dlist.c',
        'MesaLib/src/mesa/main/dlopen.c',
        'MesaLib/src/mesa/main/drawpix.c',
        'MesaLib/src/mesa/main/enable.c',
        'MesaLib/src/mesa/main/enums.c',
        'MesaLib/src/mesa/main/eval.c',
        'MesaLib/src/mesa/main/execmem.c',
        'MesaLib/src/mesa/main/extensions.c',
        'MesaLib/src/mesa/main/fbobject.c',
        'MesaLib/src/mesa/main/feedback.c',
        'MesaLib/src/mesa/main/ffvertex_prog.c',
        'MesaLib/src/mesa/main/fog.c',
        'MesaLib/src/mesa/main/formats.c',
        'MesaLib/src/mesa/main/framebuffer.c',
        'MesaLib/src/mesa/main/get.c',
        'MesaLib/src/mesa/main/getstring.c',
        'MesaLib/src/mesa/glapi/glapi.c',
        'MesaLib/src/mesa/glapi/glapi_getproc.c',
        'MesaLib/src/mesa/glapi/glthread.c',
        'MesaLib/src/mesa/shader/grammar/grammar_mesa.c',
        'MesaLib/src/mesa/main/hash.c',
        'MesaLib/src/mesa/shader/hash_table.c',
        'MesaLib/src/mesa/main/hint.c',
        'MesaLib/src/mesa/main/histogram.c',
        'MesaLib/src/mesa/main/image.c',
        'MesaLib/src/mesa/main/imports.c',
        'MesaLib/src/mesa/shader/lex.yy.c',
        'MesaLib/src/mesa/main/light.c',
        'MesaLib/src/mesa/main/lines.c',
        'MesaLib/src/mesa/math/m_debug_clip.c',
        'MesaLib/src/mesa/math/m_debug_norm.c',
        'MesaLib/src/mesa/math/m_debug_xform.c',
        'MesaLib/src/mesa/math/m_eval.c',
        'MesaLib/src/mesa/math/m_matrix.c',
        'MesaLib/src/mesa/math/m_translate.c',
        'MesaLib/src/mesa/math/m_vector.c',
        'MesaLib/src/mesa/math/m_xform.c',
        'MesaLib/src/mesa/main/matrix.c',
        'MesaLib/src/mesa/main/mipmap.c',
        'MesaLib/src/mesa/main/mm.c',
        'MesaLib/src/mesa/main/multisample.c',
        'MesaLib/src/mesa/shader/nvfragparse.c',
        'MesaLib/src/mesa/shader/nvprogram.c',
        'MesaLib/src/mesa/shader/nvvertparse.c',
        'MesaLib/src/mesa/main/pixel.c',
        'MesaLib/src/mesa/main/pixelstore.c',
        'MesaLib/src/mesa/main/points.c',
        'MesaLib/src/mesa/main/polygon.c',
        'MesaLib/src/mesa/shader/prog_cache.c',
        'MesaLib/src/mesa/shader/prog_execute.c',
        'MesaLib/src/mesa/shader/prog_instruction.c',
        'MesaLib/src/mesa/shader/prog_noise.c',
        'MesaLib/src/mesa/shader/prog_optimize.c',
        'MesaLib/src/mesa/shader/prog_parameter.c',
        'MesaLib/src/mesa/shader/prog_parameter_layout.c',
        'MesaLib/src/mesa/shader/prog_print.c',
        'MesaLib/src/mesa/shader/prog_statevars.c',
        'MesaLib/src/mesa/shader/prog_uniform.c',
        'MesaLib/src/mesa/shader/program.c',
        'MesaLib/src/mesa/shader/program_parse.tab.c',
        'MesaLib/src/mesa/shader/program_parse_extra.c',
        'MesaLib/src/mesa/shader/programopt.c',
        'MesaLib/src/mesa/main/queryobj.c',
        'MesaLib/src/mesa/main/rastpos.c',
        'MesaLib/src/mesa/main/rbadaptors.c',
        'MesaLib/src/mesa/main/readpix.c',
        'MesaLib/src/mesa/main/remap.c',
        'MesaLib/src/mesa/main/renderbuffer.c',
        'MesaLib/src/mesa/swrast/s_aaline.c',
        'MesaLib/src/mesa/swrast/s_aatriangle.c',
        'MesaLib/src/mesa/swrast/s_accum.c',
        'MesaLib/src/mesa/swrast/s_alpha.c',
        'MesaLib/src/mesa/swrast/s_atifragshader.c',
        'MesaLib/src/mesa/swrast/s_bitmap.c',
        'MesaLib/src/mesa/swrast/s_blend.c',
        'MesaLib/src/mesa/swrast/s_blit.c',
        'MesaLib/src/mesa/swrast/s_clear.c',
        'MesaLib/src/mesa/swrast/s_context.c',
        'MesaLib/src/mesa/swrast/s_copypix.c',
        'MesaLib/src/mesa/swrast/s_depth.c',
        'MesaLib/src/mesa/swrast/s_drawpix.c',
        'MesaLib/src/mesa/swrast/s_feedback.c',
        'MesaLib/src/mesa/swrast/s_fog.c',
        'MesaLib/src/mesa/swrast/s_fragprog.c',
        'MesaLib/src/mesa/swrast/s_lines.c',
        'MesaLib/src/mesa/swrast/s_logic.c',
        'MesaLib/src/mesa/swrast/s_masking.c',
        'MesaLib/src/mesa/swrast/s_points.c',
        'MesaLib/src/mesa/swrast/s_readpix.c',
        'MesaLib/src/mesa/swrast/s_span.c',
        'MesaLib/src/mesa/swrast/s_stencil.c',
        'MesaLib/src/mesa/swrast/s_texcombine.c',
        'MesaLib/src/mesa/swrast/s_texfilter.c',
        'MesaLib/src/mesa/swrast/s_triangle.c',
        'MesaLib/src/mesa/swrast/s_zoom.c',
        'MesaLib/src/mesa/main/scissor.c',
        'MesaLib/src/mesa/shader/shader_api.c',
        'MesaLib/src/mesa/main/shaders.c',
        'MesaLib/src/mesa/main/shared.c',
        'MesaLib/src/mesa/shader/slang/slang_builtin.c',
        'MesaLib/src/mesa/shader/slang/slang_codegen.c',
        'MesaLib/src/mesa/shader/slang/slang_compile.c',
        'MesaLib/src/mesa/shader/slang/slang_compile_function.c',
        'MesaLib/src/mesa/shader/slang/slang_compile_operation.c',
        'MesaLib/src/mesa/shader/slang/slang_compile_struct.c',
        'MesaLib/src/mesa/shader/slang/slang_compile_variable.c',
        'MesaLib/src/mesa/shader/slang/slang_emit.c',
        'MesaLib/src/mesa/shader/slang/slang_ir.c',
        'MesaLib/src/mesa/shader/slang/slang_label.c',
        'MesaLib/src/mesa/shader/slang/slang_link.c',
        'MesaLib/src/mesa/shader/slang/slang_log.c',
        'MesaLib/src/mesa/shader/slang/slang_mem.c',
        'MesaLib/src/mesa/shader/slang/slang_preprocess.c',
        'MesaLib/src/mesa/shader/slang/slang_print.c',
        'MesaLib/src/mesa/shader/slang/slang_simplify.c',
        'MesaLib/src/mesa/shader/slang/slang_storage.c',
        'MesaLib/src/mesa/shader/slang/slang_typeinfo.c',
        'MesaLib/src/mesa/shader/slang/slang_utility.c',
        'MesaLib/src/mesa/shader/slang/slang_vartable.c',
        'MesaLib/src/mesa/swrast_setup/ss_context.c',
        'MesaLib/src/mesa/swrast_setup/ss_triangle.c',
        'MesaLib/src/mesa/main/state.c',
        'MesaLib/src/mesa/main/stencil.c',
        'MesaLib/src/mesa/shader/symbol_table.c',
        'MesaLib/src/mesa/main/syncobj.c',
        'MesaLib/src/mesa/tnl/t_context.c',
        'MesaLib/src/mesa/tnl/t_draw.c',
        'MesaLib/src/mesa/tnl/t_pipeline.c',
        'MesaLib/src/mesa/tnl/t_rasterpos.c',
        'MesaLib/src/mesa/tnl/t_vb_cull.c',
        'MesaLib/src/mesa/tnl/t_vb_fog.c',
        'MesaLib/src/mesa/tnl/t_vb_light.c',
        'MesaLib/src/mesa/tnl/t_vb_normals.c',
        'MesaLib/src/mesa/tnl/t_vb_points.c',
        'MesaLib/src/mesa/tnl/t_vb_program.c',
        'MesaLib/src/mesa/tnl/t_vb_render.c',
        'MesaLib/src/mesa/tnl/t_vb_texgen.c',
        'MesaLib/src/mesa/tnl/t_vb_texmat.c',
        'MesaLib/src/mesa/tnl/t_vb_vertex.c',
        'MesaLib/src/mesa/tnl/t_vertex.c',
        'MesaLib/src/mesa/tnl/t_vertex_generic.c',
        'MesaLib/src/mesa/tnl/t_vp_build.c',
        'MesaLib/src/mesa/main/texcompress.c',
        'MesaLib/src/mesa/main/texcompress_fxt1.c',
        'MesaLib/src/mesa/main/texcompress_s3tc.c',
        'MesaLib/src/mesa/main/texenv.c',
        'MesaLib/src/mesa/main/texenvprogram.c',
        'MesaLib/src/mesa/main/texfetch.c',
        'MesaLib/src/mesa/main/texformat.c',
        'MesaLib/src/mesa/main/texgen.c',
        'MesaLib/src/mesa/main/texgetimage.c',
        'MesaLib/src/mesa/main/teximage.c',
        'MesaLib/src/mesa/main/texobj.c',
        'MesaLib/src/mesa/main/texparam.c',
        'MesaLib/src/mesa/main/texrender.c',
        'MesaLib/src/mesa/main/texstate.c',
        'MesaLib/src/mesa/main/texstore.c',
        'MesaLib/src/mesa/main/varray.c',
        'MesaLib/src/mesa/vbo/vbo_context.c',
        'MesaLib/src/mesa/vbo/vbo_exec.c',
        'MesaLib/src/mesa/vbo/vbo_exec_api.c',
        'MesaLib/src/mesa/vbo/vbo_exec_array.c',
        'MesaLib/src/mesa/vbo/vbo_exec_draw.c',
        'MesaLib/src/mesa/vbo/vbo_exec_eval.c',
        'MesaLib/src/mesa/vbo/vbo_rebase.c',
        'MesaLib/src/mesa/vbo/vbo_save.c',
        'MesaLib/src/mesa/vbo/vbo_save_api.c',
        'MesaLib/src/mesa/vbo/vbo_save_draw.c',
        'MesaLib/src/mesa/vbo/vbo_save_loopback.c',
        'MesaLib/src/mesa/vbo/vbo_split.c',
        'MesaLib/src/mesa/vbo/vbo_split_copy.c',
        'MesaLib/src/mesa/vbo/vbo_split_inplace.c',
        'MesaLib/src/mesa/main/viewport.c',
        'MesaLib/src/mesa/main/vtxfmt.c',
        'MesaLib/src/mesa/main/accum.h',
        'MesaLib/src/mesa/main/api_arrayelt.h',
        'MesaLib/src/mesa/main/api_eval.h',
        'MesaLib/src/mesa/main/api_exec.h',
        'MesaLib/src/mesa/main/api_loopback.h',
        'MesaLib/src/mesa/main/api_noop.h',
        'MesaLib/src/mesa/main/api_validate.h',
        'MesaLib/src/mesa/shader/arbprogparse.h',
        'MesaLib/src/mesa/shader/arbprogram.h',
        'MesaLib/src/mesa/shader/arbprogram_syn.h',
        'MesaLib/src/mesa/main/arrayobj.h',
        'MesaLib/src/mesa/shader/atifragshader.h',
        'MesaLib/src/mesa/main/attrib.h',
        'MesaLib/src/mesa/main/bitset.h',
        'MesaLib/src/mesa/main/blend.h',
        'MesaLib/src/mesa/main/bufferobj.h',
        'MesaLib/src/mesa/main/buffers.h',
        'MesaLib/src/mesa/main/clear.h',
        'MesaLib/src/mesa/main/clip.h',
        'MesaLib/src/mesa/main/colormac.h',
        'MesaLib/src/mesa/main/colortab.h',
        'MesaLib/src/mesa/main/config.h',
        'MesaLib/src/mesa/main/context.h',
        'MesaLib/src/mesa/main/convolve.h',
        'MesaLib/src/mesa/main/cpuinfo.h',
        'MesaLib/src/mesa/main/dd.h',
        'MesaLib/src/mesa/main/debug.h',
        'MesaLib/src/mesa/main/depth.h',
        'MesaLib/src/mesa/main/depthstencil.h',
        'MesaLib/src/mesa/main/dlist.h',
        'MesaLib/src/mesa/main/dlopen.h',
        'MesaLib/src/mesa/main/drawpix.h',
        'MesaLib/src/mesa/main/enable.h',
        'MesaLib/src/mesa/main/enums.h',
        'MesaLib/src/mesa/main/eval.h',
        'MesaLib/src/mesa/main/extensions.h',
        'MesaLib/src/mesa/main/fbobject.h',
        'MesaLib/src/mesa/main/feedback.h',
        'MesaLib/src/mesa/main/ffvertex_prog.h',
        'MesaLib/src/mesa/main/fog.h',
        'MesaLib/src/mesa/main/framebuffer.h',
        'MesaLib/src/mesa/main/get.h',
        'MesaLib/src/mesa/glapi/glapi.h',
        'MesaLib/src/mesa/glapi/glapioffsets.h',
        'MesaLib/src/mesa/glapi/glapitable.h',
        'MesaLib/src/mesa/glapi/glapitemp.h',
        'MesaLib/src/mesa/main/glheader.h',
        'MesaLib/src/mesa/glapi/glprocs.h',
        'MesaLib/src/mesa/glapi/glthread.h',
        'MesaLib/src/mesa/shader/grammar/grammar.h',
        'MesaLib/src/mesa/shader/grammar/grammar_crt.h',
        'MesaLib/src/mesa/shader/grammar/grammar_mesa.h',
        'MesaLib/src/mesa/shader/grammar/grammar_syn.h',
        'MesaLib/src/mesa/main/hash.h',
        'MesaLib/src/mesa/shader/hash_table.h',
        'MesaLib/src/mesa/main/hint.h',
        'MesaLib/src/mesa/main/histogram.h',
        'MesaLib/src/mesa/main/image.h',
        'MesaLib/src/mesa/main/imports.h',
        'MesaLib/src/mesa/main/light.h',
        'MesaLib/src/mesa/main/lines.h',
        'MesaLib/src/mesa/math/m_clip_tmp.h',
        'MesaLib/src/mesa/math/m_copy_tmp.h',
        'MesaLib/src/mesa/math/m_debug.h',
        'MesaLib/src/mesa/math/m_debug_util.h',
        'MesaLib/src/mesa/math/m_dotprod_tmp.h',
        'MesaLib/src/mesa/math/m_eval.h',
        'MesaLib/src/mesa/math/m_matrix.h',
        'MesaLib/src/mesa/math/m_norm_tmp.h',
        'MesaLib/src/mesa/math/m_trans_tmp.h',
        'MesaLib/src/mesa/math/m_translate.h',
        'MesaLib/src/mesa/math/m_vector.h',
        'MesaLib/src/mesa/math/m_xform.h',
        'MesaLib/src/mesa/math/m_xform_tmp.h',
        'MesaLib/src/mesa/main/macros.h',
        'MesaLib/src/mesa/math/mathmod.h',
        'MesaLib/src/mesa/main/matrix.h',
        'MesaLib/src/mesa/main/mcompiler.h',
        'MesaLib/src/mesa/main/mfeatures.h',
        'MesaLib/src/mesa/main/mipmap.h',
        'MesaLib/src/mesa/main/mm.h',
        'MesaLib/src/mesa/main/mtypes.h',
        'MesaLib/src/mesa/main/multisample.h',
        'MesaLib/src/mesa/shader/nvfragparse.h',
        'MesaLib/src/mesa/shader/nvprogram.h',
        'MesaLib/src/mesa/shader/nvvertparse.h',
        'MesaLib/src/mesa/main/pixel.h',
        'MesaLib/src/mesa/main/pixelstore.h',
        'MesaLib/src/mesa/main/points.h',
        'MesaLib/src/mesa/main/polygon.h',
        'MesaLib/src/mesa/shader/prog_execute.h',
        'MesaLib/src/mesa/shader/prog_instruction.h',
        'MesaLib/src/mesa/shader/prog_noise.h',
        'MesaLib/src/mesa/shader/prog_optimize.h',
        'MesaLib/src/mesa/shader/prog_parameter.h',
        'MesaLib/src/mesa/shader/prog_parameter_layout.h',
        'MesaLib/src/mesa/shader/prog_print.h',
        'MesaLib/src/mesa/shader/prog_statevars.h',
        'MesaLib/src/mesa/shader/prog_uniform.h',
        'MesaLib/src/mesa/shader/program.h',
        'MesaLib/src/mesa/shader/program_parse.tab.h',
        'MesaLib/src/mesa/shader/programopt.h',
        'MesaLib/src/mesa/main/queryobj.h',
        'MesaLib/src/mesa/main/rastpos.h',
        'MesaLib/src/mesa/main/rbadaptors.h',
        'MesaLib/src/mesa/main/readpix.h',
        'MesaLib/src/mesa/main/remap.h',
        'MesaLib/src/mesa/main/renderbuffer.h',
        'MesaLib/src/mesa/swrast/s_aaline.h',
        'MesaLib/src/mesa/swrast/s_aalinetemp.h',
        'MesaLib/src/mesa/swrast/s_aatriangle.h',
        'MesaLib/src/mesa/swrast/s_aatritemp.h',
        'MesaLib/src/mesa/swrast/s_accum.h',
        'MesaLib/src/mesa/swrast/s_alpha.h',
        'MesaLib/src/mesa/swrast/s_atifragshader.h',
        'MesaLib/src/mesa/swrast/s_blend.h',
        'MesaLib/src/mesa/swrast/s_context.h',
        'MesaLib/src/mesa/swrast/s_depth.h',
        'MesaLib/src/mesa/swrast/s_drawpix.h',
        'MesaLib/src/mesa/swrast/s_feedback.h',
        'MesaLib/src/mesa/swrast/s_fog.h',
        'MesaLib/src/mesa/swrast/s_fragprog.h',
        'MesaLib/src/mesa/swrast/s_lines.h',
        'MesaLib/src/mesa/swrast/s_linetemp.h',
        'MesaLib/src/mesa/swrast/s_logic.h',
        'MesaLib/src/mesa/swrast/s_masking.h',
        'MesaLib/src/mesa/swrast/s_points.h',
        'MesaLib/src/mesa/swrast/s_pointtemp.h',
        'MesaLib/src/mesa/swrast/s_span.h',
        'MesaLib/src/mesa/swrast/s_spantemp.h',
        'MesaLib/src/mesa/swrast/s_stencil.h',
        'MesaLib/src/mesa/swrast/s_texcombine.h',
        'MesaLib/src/mesa/swrast/s_texfilter.h',
        'MesaLib/src/mesa/swrast/s_triangle.h',
        'MesaLib/src/mesa/swrast/s_trispan.h',
        'MesaLib/src/mesa/swrast/s_tritemp.h',
        'MesaLib/src/mesa/swrast/s_zoom.h',
        'MesaLib/src/mesa/main/scissor.h',
        'MesaLib/src/mesa/shader/shader_api.h',
        'MesaLib/src/mesa/main/shaders.h',
        'MesaLib/src/mesa/main/shared.h',
        'MesaLib/src/mesa/main/simple_list.h',
        'MesaLib/src/mesa/shader/slang/slang_builtin.h',
        'MesaLib/src/mesa/shader/slang/slang_codegen.h',
        'MesaLib/src/mesa/shader/slang/slang_compile.h',
        'MesaLib/src/mesa/shader/slang/slang_compile_function.h',
        'MesaLib/src/mesa/shader/slang/slang_compile_operation.h',
        'MesaLib/src/mesa/shader/slang/slang_compile_struct.h',
        'MesaLib/src/mesa/shader/slang/slang_compile_variable.h',
        'MesaLib/src/mesa/shader/slang/slang_emit.h',
        'MesaLib/src/mesa/shader/slang/slang_ir.h',
        'MesaLib/src/mesa/shader/slang/slang_label.h',
        'MesaLib/src/mesa/shader/slang/slang_link.h',
        'MesaLib/src/mesa/shader/slang/slang_log.h',
        'MesaLib/src/mesa/shader/slang/slang_mem.h',
        'MesaLib/src/mesa/shader/slang/slang_preprocess.h',
        'MesaLib/src/mesa/shader/slang/slang_print.h',
        'MesaLib/src/mesa/shader/slang/slang_simplify.h',
        'MesaLib/src/mesa/shader/slang/slang_storage.h',
        'MesaLib/src/mesa/shader/slang/slang_typeinfo.h',
        'MesaLib/src/mesa/shader/slang/slang_utility.h',
        'MesaLib/src/mesa/shader/slang/slang_vartable.h',
        'MesaLib/src/mesa/swrast_setup/ss_context.h',
        'MesaLib/src/mesa/swrast_setup/ss_triangle.h',
        'MesaLib/src/mesa/swrast_setup/ss_tritmp.h',
        'MesaLib/src/mesa/swrast_setup/ss_vb.h',
        'MesaLib/src/mesa/main/state.h',
        'MesaLib/src/mesa/main/stencil.h',
        'MesaLib/src/mesa/swrast/swrast.h',
        'MesaLib/src/mesa/swrast_setup/swrast_setup.h',
        'MesaLib/src/mesa/shader/symbol_table.h',
        'MesaLib/src/mesa/main/syncobj.h',
        'MesaLib/src/mesa/tnl/t_context.h',
        'MesaLib/src/mesa/tnl/t_pipeline.h',
        'MesaLib/src/mesa/tnl/t_vb_cliptmp.h',
        'MesaLib/src/mesa/tnl/t_vb_lighttmp.h',
        'MesaLib/src/mesa/tnl/t_vb_rendertmp.h',
        'MesaLib/src/mesa/tnl/t_vertex.h',
        'MesaLib/src/mesa/tnl/t_vp_build.h',
        'MesaLib/src/mesa/main/texcompress.h',
        'MesaLib/src/mesa/main/texenv.h',
        'MesaLib/src/mesa/main/texenvprogram.h',
        'MesaLib/src/mesa/main/texfetch.h',
        'MesaLib/src/mesa/main/texformat.h',
        'MesaLib/src/mesa/main/texfetch_tmp.h',
        'MesaLib/src/mesa/main/texgen.h',
        'MesaLib/src/mesa/main/texgetimage.h',
        'MesaLib/src/mesa/main/teximage.h',
        'MesaLib/src/mesa/main/texobj.h',
        'MesaLib/src/mesa/main/texparam.h',
        'MesaLib/src/mesa/main/texrender.h',
        'MesaLib/src/mesa/main/texstate.h',
        'MesaLib/src/mesa/main/texstore.h',
        'MesaLib/src/mesa/tnl/tnl.h',
        'MesaLib/src/mesa/main/varray.h',
        'MesaLib/src/mesa/vbo/vbo.h',
        'MesaLib/src/mesa/vbo/vbo_attrib.h',
        'MesaLib/src/mesa/vbo/vbo_attrib_tmp.h',
        'MesaLib/src/mesa/vbo/vbo_context.h',
        'MesaLib/src/mesa/vbo/vbo_exec.h',
        'MesaLib/src/mesa/vbo/vbo_save.h',
        'MesaLib/src/mesa/vbo/vbo_split.h',
        'MesaLib/src/mesa/main/version.h',
        'MesaLib/src/mesa/main/viewport.h',
        'MesaLib/src/mesa/main/vtxfmt.h',
        'MesaLib/src/mesa/main/vtxfmt_tmp.h',
      ],
    },
    # Building this target will hide the native OpenGL shared library and
    # replace it with a slow software renderer.
    {
      'target_name': 'osmesa',
      'type': 'loadable_module',
      'mac_bundle': 0,
      'dependencies': [
        'mesa',
      ],
      'include_dirs': [
        'MesaLib/include',
        'MesaLib/src/mesa',
        'MesaLib/src/mesa/drivers',
      ],
      'sources': [
        'MesaLib/src/mesa/drivers/common/driverfuncs.c',
        'MesaLib/src/mesa/drivers/common/driverfuncs.h',
        'MesaLib/src/mesa/drivers/common/meta.c',
        'MesaLib/src/mesa/drivers/common/meta.h',
        'MesaLib/src/mesa/drivers/osmesa/osmesa.c',
        'MesaLib/src/mesa/drivers/osmesa/osmesa.def',
      ],
    },
  ],
}
