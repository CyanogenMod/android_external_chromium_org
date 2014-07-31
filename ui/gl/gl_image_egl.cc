// Copyright (c) 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ui/gl/gl_image_egl.h"

#include "ui/gl/gl_surface_egl.h"

#ifdef DO_ZERO_COPY
#include "base/debug/trace_event.h"
#endif

namespace gfx {

GLImageEGL::GLImageEGL(const gfx::Size& size)
    : egl_image_(EGL_NO_IMAGE_KHR),
#ifdef DO_ZERO_COPY
      texture_(0),
#endif
      size_(size) {
}

GLImageEGL::~GLImageEGL() {
  DCHECK_EQ(EGL_NO_IMAGE_KHR, egl_image_);
}

#ifdef DO_ZERO_COPY
bool GLImageEGL::InitializeTextureMemory(gfx::GpuMemoryBufferHandle buffer, unsigned internal_format) {
  //TRACE_EVENT0("gpu", "GLImageEGL::InitializeTextureMemory");
  if (buffer.type == TEXTURE_MEMORY_BUFFER) {
    swe::InitTextureMemory();
    texture_ = swe::CreateTextureMemory();
    if (!texture_) {
      ZEROCOPY_LOG(" CreateTextureMemory returned 0 ");
      return false;
    }
    int fds[2];
    fds[0] = buffer.fd1.fd;
    fds[1] = buffer.fd2.fd;
    texture_->Init((uint8_t*)(buffer.texture_memory_data.values_), buffer.texture_memory_data.size_, fds, 2);
    if (texture_)
      buffer.native_buffer = (EGLClientBuffer)(texture_->GetNativeHandle());
  }

  EGLint attrs[] = {
    EGL_IMAGE_PRESERVED_KHR, EGL_TRUE,
    EGL_NONE,
  };
  return Initialize(EGL_NATIVE_BUFFER_ANDROID, buffer.native_buffer, attrs);
}

#endif

bool GLImageEGL::Initialize(EGLenum target,
                            EGLClientBuffer buffer,
                            const EGLint* attrs) {
  DCHECK_EQ(EGL_NO_IMAGE_KHR, egl_image_);
  egl_image_ = eglCreateImageKHR(GLSurfaceEGL::GetHardwareDisplay(),
                                 EGL_NO_CONTEXT,
                                 target,
                                 buffer,
                                 attrs);
  if (egl_image_ == EGL_NO_IMAGE_KHR) {
    EGLint error = eglGetError();
    LOG(ERROR) << "Error creating EGLImage: " << error;
    return false;
  }

  return true;
}

void GLImageEGL::Destroy(bool have_context) {
  if (egl_image_ != EGL_NO_IMAGE_KHR) {
    eglDestroyImageKHR(GLSurfaceEGL::GetHardwareDisplay(), egl_image_);
    egl_image_ = EGL_NO_IMAGE_KHR;
  }

#ifdef DO_ZERO_COPY
  if (texture_) {
    swe::DestroyTextureMemory(texture_);
    texture_ = 0;
  }
#endif
}

gfx::Size GLImageEGL::GetSize() { return size_; }

bool GLImageEGL::BindTexImage(unsigned target) {
  DCHECK_NE(EGL_NO_IMAGE_KHR, egl_image_);
  glEGLImageTargetTexture2DOES(target, egl_image_);
  DCHECK_EQ(static_cast<GLenum>(GL_NO_ERROR), glGetError());
  return true;
}

bool GLImageEGL::ScheduleOverlayPlane(gfx::AcceleratedWidget widget,
                                      int z_order,
                                      OverlayTransform transform,
                                      const Rect& bounds_rect,
                                      const RectF& crop_rect) {
  return false;
}

}  // namespace gfx
