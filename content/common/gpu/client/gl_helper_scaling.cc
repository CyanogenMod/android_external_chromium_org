// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "content/common/gpu/client/gl_helper_scaling.h"

#include <deque>
#include <string>
#include <vector>

#include "base/bind.h"
#include "base/debug/trace_event.h"
#include "base/lazy_instance.h"
#include "base/logging.h"
#include "base/memory/ref_counted.h"
#include "base/message_loop.h"
#include "base/time.h"
#include "third_party/WebKit/public/platform/WebCString.h"
#include "third_party/skia/include/core/SkRegion.h"
#include "ui/gfx/rect.h"
#include "ui/gfx/size.h"
#include "ui/gl/gl_bindings.h"

using WebKit::WebGLId;
using WebKit::WebGraphicsContext3D;

namespace content {

GLHelperScaling::GLHelperScaling(WebKit::WebGraphicsContext3D* context,
                GLHelper* helper)
  : context_(context),
    helper_(helper),
    vertex_attributes_buffer_(context_, context_->createBuffer()) {
  InitBuffer();
}

GLHelperScaling::~GLHelperScaling() {
}

// Used to keep track of a generated shader program. The program
// is passed in as text through Setup and is used by calling
// UseProgram() with the right parameters. Note that |context_|
// and |helper_| are assumed to live longer than this program.
class ShaderProgram : public base::RefCounted<ShaderProgram> {
 public:
  ShaderProgram(WebGraphicsContext3D* context,
                GLHelper* helper)
      : context_(context),
        helper_(helper),
        program_(context, context->createProgram()) {
  }

  // Compile shader program, return true if successful.
  bool Setup(const WebKit::WGC3Dchar* vertex_shader_text,
             const WebKit::WGC3Dchar* fragment_shader_text);

  // UseProgram must be called with GL_TEXTURE_2D bound to the
  // source texture and GL_ARRAY_BUFFER bound to a vertex
  // attribute buffer.
  void UseProgram(const gfx::Size& src_size,
                  const gfx::Rect& src_subrect,
                  const gfx::Size& dst_size,
                  bool scale_x,
                  bool flip_y,
                  GLfloat color_weights[4]);

 private:
  friend class base::RefCounted<ShaderProgram>;
  ~ShaderProgram() {}

  WebGraphicsContext3D* context_;
  GLHelper* helper_;

  // A program for copying a source texture into a destination texture.
  ScopedProgram program_;

  // The location of the position in the program.
  WebKit::WGC3Dint position_location_;
  // The location of the texture coordinate in the program.
  WebKit::WGC3Dint texcoord_location_;
  // The location of the source texture in the program.
  WebKit::WGC3Dint texture_location_;
  // The location of the texture coordinate of
  // the sub-rectangle in the program.
  WebKit::WGC3Dint src_subrect_location_;
  // Location of size of source image in pixels.
  WebKit::WGC3Dint src_pixelsize_location_;
  // Location of size of destination image in pixels.
  WebKit::WGC3Dint dst_pixelsize_location_;
  // Location of vector for scaling direction.
  WebKit::WGC3Dint scaling_vector_location_;
  // Location of color weights.
  WebKit::WGC3Dint color_weights_location_;

  DISALLOW_COPY_AND_ASSIGN(ShaderProgram);
};


// Implementation of a single stage in a scaler pipeline. If the pipeline has
// multiple stages, it calls Scale() on the subscaler, then further scales the
// output. Caches textures and framebuffers to avoid allocating/deleting
// them once per frame, which can be expensive on some drivers.
class ScalerImpl : public GLHelper::ScalerInterface {
 public:
  // |context| and |copy_impl| are expected to live longer than this object.
  // |src_size| is the size of the input texture in pixels.
  // |dst_size| is the size of the output texutre in pixels.
  // |src_subrect| is the portion of the src to copy to the output texture.
  // If |scale_x| is true, we are scaling along the X axis, otherwise Y.
  // If we are scaling in both X and Y, |scale_x| is ignored.
  // If |vertically_flip_texture| is true, output will be upside-down.
  // If |swizzle| is true, RGBA will be transformed into BGRA.
  // |color_weights| are only used together with SHADER_PLANAR to specify
  //   how to convert RGB colors into a single value.
  ScalerImpl(WebGraphicsContext3D* context,
             GLHelperScaling* scaler_helper,
             const GLHelperScaling::ScalerStage &scaler_stage,
             ScalerImpl* subscaler,
             const float* color_weights) :
      context_(context),
      scaler_helper_(scaler_helper),
      spec_(scaler_stage),
      intermediate_texture_(0),
      dst_framebuffer_(context, context_->createFramebuffer()),
      subscaler_(subscaler) {
    if (color_weights) {
      color_weights_[0] = color_weights[0];
      color_weights_[1] = color_weights[1];
      color_weights_[2] = color_weights[2];
      color_weights_[3] = color_weights[3];
    } else {
      color_weights_[0] = 0.0;
      color_weights_[1] = 0.0;
      color_weights_[2] = 0.0;
      color_weights_[3] = 0.0;
    }
    shader_program_ = scaler_helper_->GetShaderProgram(spec_.shader,
                                                       spec_.swizzle);

    if (subscaler_) {
      intermediate_texture_ = context_->createTexture();
      ScopedTextureBinder<GL_TEXTURE_2D> texture_binder(
          context_,
          intermediate_texture_);
      context_->texImage2D(GL_TEXTURE_2D,
                           0,
                           GL_RGBA,
                           spec_.src_size.width(),
                           spec_.src_size.height(),
                           0,
                           GL_RGBA,
                           GL_UNSIGNED_BYTE,
                           NULL);
    }
  }

  virtual ~ScalerImpl() {
    if (intermediate_texture_) {
      context_->deleteTexture(intermediate_texture_);
    }
  }

  // GLHelper::ScalerInterface implementation.
  virtual void Scale(WebKit::WebGLId source_texture,
                     WebKit::WebGLId dest_texture) OVERRIDE {
    if (subscaler_) {
      subscaler_->Scale(source_texture, intermediate_texture_);
      source_texture = intermediate_texture_;
    }

    ScopedFramebufferBinder<GL_FRAMEBUFFER> framebuffer_binder(
        context_,
        dst_framebuffer_);
    {
      ScopedTextureBinder<GL_TEXTURE_2D> texture_binder(context_,
                                                        dest_texture);
      context_->framebufferTexture2D(GL_FRAMEBUFFER,
                                     GL_COLOR_ATTACHMENT0,
                                     GL_TEXTURE_2D,
                                     dest_texture,
                                     0);
    }
    ScopedTextureBinder<GL_TEXTURE_2D> texture_binder(context_,
                                                      source_texture);

    context_->texParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER,
                            GL_LINEAR);
    context_->texParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
                            GL_LINEAR);
    context_->texParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S,
                            GL_CLAMP_TO_EDGE);
    context_->texParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T,
                            GL_CLAMP_TO_EDGE);

    ScopedBufferBinder<GL_ARRAY_BUFFER> buffer_binder(
        context_,
        scaler_helper_->vertex_attributes_buffer_);
    shader_program_->UseProgram(spec_.src_size,
                                spec_.src_subrect,
                                spec_.dst_size,
                                spec_.scale_x,
                                spec_.vertically_flip_texture,
                                color_weights_);
    context_->viewport(0, 0, spec_.dst_size.width(), spec_.dst_size.height());

    // Conduct texture mapping by drawing a quad composed of two triangles.
    context_->drawArrays(GL_TRIANGLE_STRIP, 0, 4);
  }

  virtual const gfx::Size& SrcSize() OVERRIDE {
    if (subscaler_) {
      return subscaler_->SrcSize();
    }
    return spec_.src_size;
  }
  virtual const gfx::Rect& SrcSubrect() OVERRIDE {
    if (subscaler_) {
      return subscaler_->SrcSubrect();
    }
    return spec_.src_subrect;
  }
  virtual const gfx::Size& DstSize() OVERRIDE {
    return spec_.dst_size;
  }

 private:
  WebGraphicsContext3D* context_;
  GLHelperScaling* scaler_helper_;
  GLHelperScaling::ScalerStage spec_;
  GLfloat color_weights_[4];
  WebKit::WebGLId intermediate_texture_;
  scoped_refptr<ShaderProgram> shader_program_;
  ScopedFramebuffer dst_framebuffer_;
  scoped_ptr<ScalerImpl> subscaler_;
};

GLHelperScaling::ScalerStage::ScalerStage(
    ShaderType shader_,
    gfx::Size src_size_,
    gfx::Rect src_subrect_,
    gfx::Size dst_size_,
    bool scale_x_,
    bool vertically_flip_texture_,
    bool swizzle_)
    : shader(shader_),
      src_size(src_size_),
      src_subrect(src_subrect_),
      dst_size(dst_size_),
      scale_x(scale_x_),
      vertically_flip_texture(vertically_flip_texture_),
      swizzle(swizzle_) {
}

// The important inputs for this function is |x_ops| and
// |y_ops|. They represent scaling operations to be done
// on an imag of size |src_size|. If |quality| is SCALER_QUALITY_BEST,
// then we will interpret these scale operations literally and we'll
// create one scaler stage for each ScaleOp.  However, if |quality|
// is SCALER_QUALITY_GOOD, then we can do a whole bunch of optimizations
// by combining two or more ScaleOps in to a single scaler stage.
// Normally we process ScaleOps from |y_ops| first and |x_ops| after
// all |y_ops| are processed, but sometimes we can combine one or more
// operation from both queues essentially for free. This is the reason
// why |x_ops| and |y_ops| aren't just one single queue.
void GLHelperScaling::ConvertScalerOpsToScalerStages(
    GLHelper::ScalerQuality quality,
    gfx::Size src_size,
    gfx::Rect src_subrect,
    const gfx::Size& dst_size,
    bool vertically_flip_texture,
    bool swizzle,
    std::deque<GLHelperScaling::ScaleOp>* x_ops,
    std::deque<GLHelperScaling::ScaleOp>* y_ops,
    std::vector<ScalerStage> *scaler_stages) {
  while (!x_ops->empty() || !y_ops->empty()) {
    gfx::Size intermediate_size = src_subrect.size();
    std::deque<ScaleOp>* current_queue = NULL;

    if (!y_ops->empty()) {
      current_queue = y_ops;
    } else {
      current_queue = x_ops;
    }

    ShaderType current_shader = SHADER_BILINEAR;
    switch (current_queue->front().scale_factor) {
      case 0:
        if (quality == GLHelper::SCALER_QUALITY_BEST) {
          current_shader = SHADER_BICUBIC_UPSCALE;
        }
        break;
      case 2:
        if (quality == GLHelper::SCALER_QUALITY_BEST) {
          current_shader = SHADER_BICUBIC_HALF_1D;
        }
        break;
      case 3:
        DCHECK(quality != GLHelper::SCALER_QUALITY_BEST);
        current_shader = SHADER_BILINEAR3;
        break;
      default:
        NOTREACHED();
    }
    bool scale_x = current_queue->front().scale_x;
    current_queue->front().UpdateSize(&intermediate_size);
    current_queue->pop_front();

    // Optimization: Sometimes we can combine 2-4 scaling operations into
    // one operation.
    if (quality == GLHelper::SCALER_QUALITY_GOOD) {
      if (!current_queue->empty() && current_shader == SHADER_BILINEAR) {
        // Combine two steps in the same dimension.
        current_queue->front().UpdateSize(&intermediate_size);
        current_queue->pop_front();
        current_shader = SHADER_BILINEAR2;
        if (!current_queue->empty()) {
          // Combine three steps in the same dimension.
          current_queue->front().UpdateSize(&intermediate_size);
          current_queue->pop_front();
          current_shader = SHADER_BILINEAR4;
        }
      }
      // Check if we can combine some steps in the other dimension as well.
      // Since all shaders currently use GL_LINEAR, we can easily scale up
      // or scale down by exactly 2x at the same time as we do another
      // operation. Currently, the following mergers are supported:
      // * 1 bilinear Y-pass with 1 bilinear X-pass (up or down)
      // * 2 bilinear Y-passes with 2 bilinear X-passes
      // * 1 bilinear Y-pass with N bilinear X-pass
      // * N bilinear Y-passes with 1 bilinear X-pass (down only)
      // Measurements indicate that generalizing this for 3x3 and 4x4
      // makes it slower on some platforms, such as the Pixel.
      if (!scale_x && x_ops->size() > 0 &&
          x_ops->front().scale_factor <= 2) {
        int x_passes = 0;
        if (current_shader == SHADER_BILINEAR2 && x_ops->size() >= 2) {
          // 2y + 2x passes
          x_passes = 2;
          current_shader = SHADER_BILINEAR2X2;
        } else if (current_shader == SHADER_BILINEAR) {
          // 1y + Nx passes
          scale_x = true;
          switch (x_ops->size()) {
            case 0:
              NOTREACHED();
            case 1:
              if (x_ops->front().scale_factor == 3) {
                current_shader = SHADER_BILINEAR3;
              }
              x_passes = 1;
              break;
            case 2:
              x_passes = 2;
              current_shader = SHADER_BILINEAR2;
              break;
            default:
              x_passes = 3;
              current_shader = SHADER_BILINEAR4;
              break;
          }
        } else if (x_ops->front().scale_factor == 2) {
          // Ny + 1x-downscale
          x_passes = 1;
        }

        for (int i = 0; i < x_passes; i++) {
          x_ops->front().UpdateSize(&intermediate_size);
          x_ops->pop_front();
        }
      }
    }

    scaler_stages->push_back(ScalerStage(current_shader,
                                         src_size,
                                         src_subrect,
                                         intermediate_size,
                                         scale_x,
                                         vertically_flip_texture,
                                         swizzle));
    src_size = intermediate_size;
    src_subrect = gfx::Rect(intermediate_size);
    vertically_flip_texture = false;
    swizzle = false;
  }
}

void GLHelperScaling::ComputeScalerStages(
    GLHelper::ScalerQuality quality,
    const gfx::Size& src_size,
    const gfx::Rect& src_subrect,
    const gfx::Size& dst_size,
    bool vertically_flip_texture,
    bool swizzle,
    std::vector<ScalerStage> *scaler_stages) {
  if (quality == GLHelper::SCALER_QUALITY_FAST ||
      src_subrect.size() == dst_size) {
    scaler_stages->push_back(ScalerStage(SHADER_BILINEAR,
                                         src_size,
                                         src_subrect,
                                         dst_size,
                                         false,
                                         vertically_flip_texture,
                                         swizzle));
    return;
  }

  std::deque<GLHelperScaling::ScaleOp> x_ops, y_ops;
  GLHelperScaling::ScaleOp::AddOps(src_subrect.width(),
                                   dst_size.width(),
                                   true,
                                   quality == GLHelper::SCALER_QUALITY_GOOD,
                                   &x_ops);
  GLHelperScaling::ScaleOp::AddOps(src_subrect.height(),
                                   dst_size.height(),
                                   false,
                                   quality == GLHelper::SCALER_QUALITY_GOOD,
                                   &y_ops);

  ConvertScalerOpsToScalerStages(
      quality,
      src_size,
      src_subrect,
      dst_size,
      vertically_flip_texture,
      swizzle,
      &x_ops,
      &y_ops,
      scaler_stages);
}

GLHelper::ScalerInterface*
GLHelperScaling::CreateScaler(GLHelper::ScalerQuality quality,
                              gfx::Size src_size,
                              gfx::Rect src_subrect,
                              const gfx::Size& dst_size,
                              bool vertically_flip_texture,
                              bool swizzle) {
  std::vector<ScalerStage> scaler_stages;
  ComputeScalerStages(quality,
                      src_size,
                      src_subrect,
                      dst_size,
                      vertically_flip_texture,
                      swizzle,
                      &scaler_stages);

  ScalerImpl* ret = NULL;
  for (unsigned int i = 0; i < scaler_stages.size(); i++) {
    ret = new ScalerImpl(context_, this, scaler_stages[i], ret, NULL);
  }
  return ret;
}

GLHelper::ScalerInterface*
GLHelperScaling::CreatePlanarScaler(
    const gfx::Size& src_size,
    const gfx::Rect& src_subrect,
    const gfx::Size& dst_size,
    bool vertically_flip_texture,
    const float color_weights[4]) {
  ScalerStage stage(SHADER_PLANAR,
                    src_size,
                    src_subrect,
                    dst_size,
                    true,
                    vertically_flip_texture,
                    false);
  return new ScalerImpl(context_, this, stage, NULL, color_weights);
}

const WebKit::WGC3Dfloat GLHelperScaling::kVertexAttributes[] = {
  -1.0f, -1.0f, 0.0f, 0.0f,
  1.0f, -1.0f, 1.0f, 0.0f,
  -1.0f, 1.0f, 0.0f, 1.0f,
  1.0f, 1.0f, 1.0f, 1.0f,
};

void GLHelperScaling::InitBuffer() {
  ScopedBufferBinder<GL_ARRAY_BUFFER> buffer_binder(
      context_, vertex_attributes_buffer_);
  context_->bufferData(GL_ARRAY_BUFFER,
                       sizeof(kVertexAttributes),
                       kVertexAttributes,
                       GL_STATIC_DRAW);
}

scoped_refptr<ShaderProgram>
GLHelperScaling::GetShaderProgram(ShaderType type,
                                  bool swizzle) {
  ShaderProgramKeyType key(type, swizzle);
  scoped_refptr<ShaderProgram>& cache_entry(shader_programs_[key]);
  if (!cache_entry.get()) {
    cache_entry = new ShaderProgram(context_, helper_);
    std::basic_string<WebKit::WGC3Dchar> vertex_program;
    std::basic_string<WebKit::WGC3Dchar> fragment_program;
    std::basic_string<WebKit::WGC3Dchar> vertex_header;
    std::basic_string<WebKit::WGC3Dchar> fragment_header;
    std::basic_string<WebKit::WGC3Dchar> shared_variables;

    vertex_header.append(
        "precision highp float;\n"
        "attribute vec2 a_position;\n"
        "attribute vec2 a_texcoord;\n");

    fragment_header.append(
        "precision mediump float;\n"
        "uniform sampler2D s_texture;\n");

    shared_variables.append(
        "uniform vec4 src_subrect;\n"
        "uniform vec2 src_pixelsize;\n"
        "uniform vec2 dst_pixelsize;\n"
        "uniform vec2 scaling_vector;\n");

    vertex_program.append(
        "  gl_Position = vec4(a_position, 0.0, 1.0);\n"
        "  vec2 texcoord = src_subrect.xy + a_texcoord * src_subrect.zw;\n"
        "  vec2 step = scaling_vector * src_subrect.zw / dst_pixelsize;\n");

    switch (type) {
      case SHADER_BILINEAR:
        shared_variables.append("varying vec2 v_texcoord;\n");
        vertex_program.append("  v_texcoord = texcoord;\n");
        fragment_program.append(
            "  gl_FragColor = texture2D(s_texture, v_texcoord);\n");
        break;

      case SHADER_BILINEAR2:
        // This is equivialent to two passes of the BILINEAR shader above.
        // It can be used to scale an image down 1.0x-2.0x in either dimension,
        // or exactly 4x.
        shared_variables.append(
            "varying vec4 v_texcoords;\n");  // 2 texcoords packed in one quad
        vertex_program.append(
            "  step /= 4.0;\n"
            "  v_texcoords.xy = texcoord + step;\n"
            "  v_texcoords.zw = texcoord - step;\n");

        fragment_program.append(
            "  gl_FragColor = (texture2D(s_texture, v_texcoords.xy) +\n"
            "                  texture2D(s_texture, v_texcoords.zw)) / 2.0;\n");
         break;

      case SHADER_BILINEAR3:
        // This is kind of like doing 1.5 passes of the BILINEAR shader.
        // It can be used to scale an image down 1.5x-3.0x, or exactly 6x.
        shared_variables.append(
            "varying vec4 v_texcoords1;\n"  // 2 texcoords packed in one quad
            "varying vec2 v_texcoords2;\n");
        vertex_program.append(
            "  step /= 3.0;\n"
            "  v_texcoords1.xy = texcoord + step;\n"
            "  v_texcoords1.zw = texcoord;\n"
            "  v_texcoords2 = texcoord - step;\n");
        fragment_program.append(
            "  gl_FragColor = (texture2D(s_texture, v_texcoords1.xy) +\n"
            "                  texture2D(s_texture, v_texcoords1.zw) +\n"
            "                  texture2D(s_texture, v_texcoords2)) / 3.0;\n");
         break;

      case SHADER_BILINEAR4:
        // This is equivialent to three passes of the BILINEAR shader above,
        // It can be used to scale an image down 2.0x-4.0x or exactly 8x.
        shared_variables.append(
            "varying vec4 v_texcoords[2];\n");
        vertex_program.append(
            "  step /= 8.0;\n"
            "  v_texcoords[0].xy = texcoord - step * 3.0;\n"
            "  v_texcoords[0].zw = texcoord - step;\n"
            "  v_texcoords[1].xy = texcoord + step;\n"
            "  v_texcoords[1].zw = texcoord + step * 3.0;\n");
        fragment_program.append(
            "  gl_FragColor = (\n"
            "      texture2D(s_texture, v_texcoords[0].xy) +\n"
            "      texture2D(s_texture, v_texcoords[0].zw) +\n"
            "      texture2D(s_texture, v_texcoords[1].xy) +\n"
            "      texture2D(s_texture, v_texcoords[1].zw)) / 4.0;\n");
         break;

      case SHADER_BILINEAR2X2:
        // This is equivialent to four passes of the BILINEAR shader above.
        // Two in each dimension. It can be used to scale an image down
        // 1.0x-2.0x in both X and Y directions. Or, it could be used to
        // scale an image down by exactly 4x in both dimensions.
        shared_variables.append(
            "varying vec4 v_texcoords[2];\n");
        vertex_program.append(
            "  step = src_subrect.zw / 4.0 / dst_pixelsize;\n"
            "  v_texcoords[0].xy = texcoord + vec2(step.x, step.y);\n"
            "  v_texcoords[0].zw = texcoord + vec2(step.x, -step.y);\n"
            "  v_texcoords[1].xy = texcoord + vec2(-step.x, step.y);\n"
            "  v_texcoords[1].zw = texcoord + vec2(-step.x, -step.y);\n");
        fragment_program.append(
            "  gl_FragColor = (\n"
            "      texture2D(s_texture, v_texcoords[0].xy) +\n"
            "      texture2D(s_texture, v_texcoords[0].zw) +\n"
            "      texture2D(s_texture, v_texcoords[1].xy) +\n"
            "      texture2D(s_texture, v_texcoords[1].zw)) / 4.0;\n");
         break;

      case SHADER_BICUBIC_HALF_1D:
        // This scales down texture by exactly half in one dimension.
        // directions in one pass. We use bilinear lookup to reduce
        // the number of texture reads from 8 to 4
        shared_variables.append(
            "const float CenterDist = 99.0 / 140.0;\n"
            "const float LobeDist = 11.0 / 4.0;\n"
            "const float CenterWeight = 35.0 / 64.0;\n"
            "const float LobeWeight = -3.0 / 64.0;\n"
            "varying vec4 v_texcoords[2];\n");
        vertex_program.append(
            "  step = src_subrect.zw * scaling_vector / src_pixelsize;\n"
            "  v_texcoords[0].xy = texcoord - LobeDist * step;\n"
            "  v_texcoords[0].zw = texcoord - CenterDist * step;\n"
            "  v_texcoords[1].xy = texcoord + CenterDist * step;\n"
            "  v_texcoords[1].zw = texcoord + LobeDist * step;\n");
        fragment_program.append(
            "  gl_FragColor = \n"
            // Lobe pixels
            "      (texture2D(s_texture, v_texcoords[0].xy) +\n"
            "       texture2D(s_texture, v_texcoords[1].zw)) *\n"
            "          LobeWeight +\n"
            // Center pixels
            "      (texture2D(s_texture, v_texcoords[0].zw) +\n"
            "       texture2D(s_texture, v_texcoords[1].xy)) *\n"
            "          CenterWeight;\n");
         break;

      case SHADER_BICUBIC_UPSCALE:
        // When scaling up, we need 4 texture reads, but we can
        // save some instructions because will know in which range of
        // the bicubic function each call call to the bicubic function
        // will be in.
        // Also, when sampling the bicubic function like this, the sum
        // is always exactly one, so we can skip normalization as well.
        shared_variables.append(
            "varying vec2 v_texcoord;\n");
        vertex_program.append(
            "  v_texcoord = texcoord;\n");
        fragment_header.append(
            "const float a = -0.5;\n"
            // This function is equivialent to calling the bicubic
            // function with x-1, x, 1-x and 2-x
            // (assuming 0 <= x < 1)
            "vec4 filt4(float x) {\n"
            "  return vec4(x * x * x, x * x, x, 1) *\n"
            "         mat4(       a,      -2.0 * a,   a, 0.0,\n"
            "               a + 2.0,      -a - 3.0, 0.0, 1.0,\n"
            "              -a - 2.0, 3.0 + 2.0 * a,  -a, 0.0,\n"
            "                    -a,             a, 0.0, 0.0);\n"
            "}\n"
            "mat4 pixels_x(vec2 pos, vec2 step) {\n"
            "  return mat4(\n"
            "      texture2D(s_texture, pos - step),\n"
            "      texture2D(s_texture, pos),\n"
            "      texture2D(s_texture, pos + step),\n"
            "      texture2D(s_texture, pos + step * 2.0));\n"
            "}\n");
        fragment_program.append(
            "  vec2 pixel_pos = v_texcoord * src_pixelsize - \n"
            "      scaling_vector / 2.0;\n"
            "  float frac = fract(dot(pixel_pos, scaling_vector));\n"
            "  vec2 base = (floor(pixel_pos) + vec2(0.5)) / src_pixelsize;\n"
            "  vec2 step = scaling_vector / src_pixelsize;\n"
            "  gl_FragColor = pixels_x(base, step) * filt4(frac);\n");
        break;

      case SHADER_PLANAR:
        // Converts four RGBA pixels into one pixel. Each RGBA
        // pixel will be dot-multiplied with the color weights and
        // then placed into a component of the output. This is used to
        // convert RGBA textures into Y, U and V textures. We do this
        // because single-component textures are not renderable on all
        // architectures.
        shared_variables.append(
            "varying vec4 v_texcoords[2];\n"
            "uniform vec4 color_weights;\n");
        vertex_program.append(
            "  step /= 4.0;\n"
            "  v_texcoords[0].xy = texcoord - step * 1.5;\n"
            "  v_texcoords[0].zw = texcoord - step * 0.5;\n"
            "  v_texcoords[1].xy = texcoord + step * 0.5;\n"
            "  v_texcoords[1].zw = texcoord + step * 1.5;\n");
        fragment_program.append(
            "  gl_FragColor = color_weights * mat4(\n"
            "    vec4(texture2D(s_texture, v_texcoords[0].xy).rgb, 1.0),\n"
            "    vec4(texture2D(s_texture, v_texcoords[0].zw).rgb, 1.0),\n"
            "    vec4(texture2D(s_texture, v_texcoords[1].xy).rgb, 1.0),\n"
            "    vec4(texture2D(s_texture, v_texcoords[1].zw).rgb, 1.0));\n");
        // Swizzle makes no sense for this shader.
        DCHECK(!swizzle);
        break;
    }
    if (swizzle) {
      fragment_program.append("  gl_FragColor = gl_FragColor.bgra;\n");
    }

    vertex_program =
        vertex_header +
        shared_variables +
        "void main() {\n" +
        vertex_program +
        "}\n";

    fragment_program =
        fragment_header +
        shared_variables +
        "void main() {\n" +
        fragment_program +
        "}\n";

    bool result = cache_entry->Setup(vertex_program.c_str(),
                                     fragment_program.c_str());
    DCHECK(result)
        << "vertex_program =\n" << vertex_program
        << "fragment_program =\n" << fragment_program;
  }
  return cache_entry;
}

bool ShaderProgram::Setup(const WebKit::WGC3Dchar* vertex_shader_text,
                          const WebKit::WGC3Dchar* fragment_shader_text) {
  // Shaders to map the source texture to |dst_texture_|.
  ScopedShader vertex_shader(context_, helper_->CompileShaderFromSource(
      vertex_shader_text, GL_VERTEX_SHADER));
  if (vertex_shader.id() == 0) {
    return false;
  }
  context_->attachShader(program_, vertex_shader);
  ScopedShader fragment_shader(context_, helper_->CompileShaderFromSource(
      fragment_shader_text, GL_FRAGMENT_SHADER));
  if (fragment_shader.id() == 0) {
    return false;
  }
  context_->attachShader(program_, fragment_shader);
  context_->linkProgram(program_);

  WebKit::WGC3Dint link_status = 0;
  context_->getProgramiv(program_, GL_LINK_STATUS, &link_status);
  if (!link_status) {
    LOG(ERROR) << std::string(context_->getProgramInfoLog(program_).utf8());
    return false;
  }
  position_location_ = context_->getAttribLocation(program_, "a_position");
  texcoord_location_ = context_->getAttribLocation(program_, "a_texcoord");
  texture_location_ = context_->getUniformLocation(program_, "s_texture");
  src_subrect_location_ = context_->getUniformLocation(program_, "src_subrect");
  src_pixelsize_location_ = context_->getUniformLocation(program_,
                                                         "src_pixelsize");
  dst_pixelsize_location_ = context_->getUniformLocation(program_,
                                                         "dst_pixelsize");
  scaling_vector_location_ = context_->getUniformLocation(program_,
                                                          "scaling_vector");
  color_weights_location_ = context_->getUniformLocation(program_,
                                                         "color_weights");
  return true;
}

void ShaderProgram::UseProgram(
    const gfx::Size& src_size,
    const gfx::Rect& src_subrect,
    const gfx::Size& dst_size,
    bool scale_x,
    bool flip_y,
    GLfloat color_weights[4]) {
  context_->useProgram(program_);

  WebKit::WGC3Dintptr offset = 0;
  context_->vertexAttribPointer(position_location_,
                                2,
                                GL_FLOAT,
                                GL_FALSE,
                                4 * sizeof(WebKit::WGC3Dfloat),
                                offset);
  context_->enableVertexAttribArray(position_location_);

  offset += 2 * sizeof(WebKit::WGC3Dfloat);
  context_->vertexAttribPointer(texcoord_location_,
                                2,
                                GL_FLOAT,
                                GL_FALSE,
                                4 * sizeof(WebKit::WGC3Dfloat),
                                offset);
  context_->enableVertexAttribArray(texcoord_location_);

  context_->uniform1i(texture_location_, 0);

  // Convert |src_subrect| to texture coordinates.
  GLfloat src_subrect_texcoord[] = {
    static_cast<float>(src_subrect.x()) / src_size.width(),
    static_cast<float>(src_subrect.y()) / src_size.height(),
    static_cast<float>(src_subrect.width()) / src_size.width(),
    static_cast<float>(src_subrect.height()) / src_size.height(),
  };
  if (flip_y) {
    src_subrect_texcoord[1] += src_subrect_texcoord[3];
    src_subrect_texcoord[3] *= -1.0;
  }
  context_->uniform4fv(src_subrect_location_, 1, src_subrect_texcoord);

  context_->uniform2f(src_pixelsize_location_,
                      src_size.width(),
                      src_size.height());
  context_->uniform2f(dst_pixelsize_location_,
                      static_cast<float>(dst_size.width()),
                      static_cast<float>(dst_size.height()));

  context_->uniform2f(scaling_vector_location_,
                      scale_x ? 1.0 : 0.0,
                      scale_x ? 0.0 : 1.0);
  context_->uniform4fv(color_weights_location_, 1, color_weights);
}

}  // namespace content
