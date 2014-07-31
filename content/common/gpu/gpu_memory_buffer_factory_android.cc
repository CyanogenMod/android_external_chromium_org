// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "content/common/gpu/gpu_memory_buffer_factory.h"

#include "base/logging.h"
#include "ui/gl/gl_image.h"
#include "ui/gl/gl_image_shared_memory.h"
#include "ui/gl/gl_image_surface_texture.h"

#ifndef NO_ZERO_COPY
#include "ui/gl/gl_image_egl.h"
#include "ui/gfx/sweadreno_texture_memory.h"
#endif

namespace content {
namespace {

class GpuMemoryBufferFactoryImpl : public GpuMemoryBufferFactory {
 public:
  // Overridden from GpuMemoryBufferFactory:
  virtual gfx::GpuMemoryBufferHandle CreateGpuMemoryBuffer(
      const gfx::GpuMemoryBufferHandle& handle,
      const gfx::Size& size,
      unsigned internalformat,
      unsigned usage) OVERRIDE {
    NOTREACHED();
    return gfx::GpuMemoryBufferHandle();
  }
  virtual void DestroyGpuMemoryBuffer(
      const gfx::GpuMemoryBufferHandle& handle) OVERRIDE {
    NOTREACHED();
  }
  virtual scoped_refptr<gfx::GLImage> CreateImageForGpuMemoryBuffer(
      const gfx::GpuMemoryBufferHandle& handle,
      const gfx::Size& size,
      unsigned internalformat,
      int client_id) OVERRIDE {
    switch (handle.type) {
      case gfx::SHARED_MEMORY_BUFFER: {
        scoped_refptr<gfx::GLImageSharedMemory> image(
            new gfx::GLImageSharedMemory(size, internalformat));
        if (!image->Initialize(handle))
          return NULL;

        return image;
      }
      case gfx::SURFACE_TEXTURE_BUFFER: {
        scoped_refptr<gfx::GLImageSurfaceTexture> image(
            new gfx::GLImageSurfaceTexture(size));
        if (!image->Initialize(handle))
          return NULL;

        return image;
      }
#ifdef DO_ZERO_COPY
      case gfx::TEXTURE_MEMORY_BUFFER: {
        scoped_refptr<gfx::GLImageEGL> image(new gfx::GLImageEGL(size));
        if (!image->InitializeTextureMemory(handle, internalformat))
          return NULL;

        return image;
      }
#endif
      default:
        NOTREACHED();
        return scoped_refptr<gfx::GLImage>();
    }
  }
};

}  // namespace

// static
scoped_ptr<GpuMemoryBufferFactory> GpuMemoryBufferFactory::Create() {
  return make_scoped_ptr<GpuMemoryBufferFactory>(
      new GpuMemoryBufferFactoryImpl);
}

}  // namespace content
