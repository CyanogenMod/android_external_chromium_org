// Copyright (c) 2014 The Linux Foundation. All rights reserved.
// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CONTENT_COMMON_GPU_CLIENT_GPU_MEMORY_BUFFER_IMPL_TEXTURE_MEMORY_H_
#define CONTENT_COMMON_GPU_CLIENT_GPU_MEMORY_BUFFER_IMPL_TEXTURE_MEMORY_H_

#ifndef NO_ZERO_COPY
#include "ui/gfx/sweadreno_texture_memory.h"
#endif

#ifdef DO_ZERO_COPY

#include "content/common/gpu/client/gpu_memory_buffer_impl.h"
#include "content/common/TextureMemory.h"

namespace content {

// Provides implementation of a GPU memory buffer based
// on a texture memory handle.
class GpuMemoryBufferImplTextureMemory : public GpuMemoryBufferImpl {
 public:
  GpuMemoryBufferImplTextureMemory(const gfx::Size& size, unsigned internalformat);
  virtual ~GpuMemoryBufferImplTextureMemory();

  bool Initialize();
  bool Initialize(gfx::GpuMemoryBufferHandle handle);

  // Overridden from gfx::GpuMemoryBuffer:
  virtual void* Map()  OVERRIDE;
  virtual void Unmap() OVERRIDE;
  virtual uint32 GetStride() const OVERRIDE;
  virtual gfx::GpuMemoryBufferHandle GetHandle() const OVERRIDE;

  WebTech::TextureMemory* GetTexture();

 private:
  WebTech::TextureMemory* texture_;
  void* addr_;
  size_t stride_;
  DISALLOW_COPY_AND_ASSIGN(GpuMemoryBufferImplTextureMemory);
};

}  // namespace content

#endif // DO_ZERO_COPY
#endif  // CONTENT_COMMON_GPU_CLIENT_GPU_MEMORY_BUFFER_IMPL_TEXTURE_MEMORY_H_
