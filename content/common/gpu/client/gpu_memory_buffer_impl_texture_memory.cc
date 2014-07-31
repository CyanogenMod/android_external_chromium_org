/** ---------------------------------------------------------------------------
 Copyright (c) 2014 The Linux Foundation. All rights reserved.

 Redistribution and use in source and binary forms, with or without
 modification, are permitted provided that the following conditions are
 met:
     * Redistributions of source code must retain the above copyright
       notice, this list of conditions and the following disclaimer.
     * Redistributions in binary form must reproduce the above
       copyright notice, this list of conditions and the following
       disclaimer in the documentation and/or other materials provided
       with the distribution.
     * Neither the name of The Linux Foundation nor the names of its
       contributors may be used to endorse or promote products derived
       from this software without specific prior written permission.

 THIS SOFTWARE IS PROVIDED "AS IS" AND ANY EXPRESS OR IMPLIED
 WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT
 ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS
 BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
 BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
 OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
 IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
-----------------------------------------------------------------------------**/

#include "content/common/gpu/client/gpu_memory_buffer_impl_texture_memory.h"

#ifdef DO_ZERO_COPY

#include <dirent.h>
#include <limits.h>
#include <sys/resource.h>
#include <unistd.h>
#include "base/logging.h"
#include "base/synchronization/lock.h"
#include "content/public/common/common_param_traits.h"
#include "ui/gl/gl_bindings.h"
#include "ui/gfx/sweadreno_texture_memory.h"

namespace content {

WebTech::PixelFormat InternalFormatToPixelFormat(unsigned internalformat) {
  switch (internalformat) {
    case GL_BGRA8_EXT:
    case GL_RGBA8_OES:
      return WebTech::FORMAT_8888;
    default:
      NOTREACHED();
      return WebTech::FORMAT_8888;
    }
}

unsigned InternalFormatToPixelSize(unsigned internalformat) {
  switch (internalformat) {
    case GL_BGRA8_EXT:
    case GL_RGBA8_OES:
      return 4;
    default:
      NOTREACHED();
      return 0;
    }
}

GpuMemoryBufferImplTextureMemory::GpuMemoryBufferImplTextureMemory(
    const gfx::Size& size, unsigned internalformat)
    : GpuMemoryBufferImpl(size, internalformat)
    , texture_(0)
    , addr_(0)
    , stride_(0) {
  ZEROCOPY_LOG(" GpuMemoryBufferImplTextureMemory::GpuMemoryBufferImplTextureMemory ");
}

GpuMemoryBufferImplTextureMemory::~GpuMemoryBufferImplTextureMemory() {
  ZEROCOPY_LOG(" GpuMemoryBufferImplTextureMemory::~GpuMemoryBufferImplTextureMemory ");
  if (texture_) {
    swe::DestroyTextureMemory(texture_);
  }
}

bool GpuMemoryBufferImplTextureMemory::Initialize() {
  ZEROCOPY_LOG(" GpuMemoryBufferImplTextureMemory::Initialize ");

  swe::InitTextureMemory();

  texture_ = swe::CreateTextureMemory();
  if (!texture_) {
    ZEROCOPY_LOG(" CreateTextureMemory returned 0 ");
    return false;
  }
  return texture_->Init(size_.width(), size_.height(), InternalFormatToPixelFormat(internalformat_));
}

bool GpuMemoryBufferImplTextureMemory::Initialize(gfx::GpuMemoryBufferHandle handle) {
  ZEROCOPY_LOG(" GpuMemoryBufferImplTextureMemory::Initialize(handle) size (%d, %d) ", size_.width(), size_.height());
  swe::InitTextureMemory();

  texture_ = swe::CreateTextureMemory();
  if (!texture_) {
    ZEROCOPY_LOG(" CreateTextureMemory returned 0 ");
    return false;
  }
  int fds[2];
  fds[0] = handle.fd1.fd;
  fds[1] = handle.fd2.fd;
  return texture_->Init((uint8_t*)(handle.texture_memory_data.values_), handle.texture_memory_data.size_, fds, 2);
}

WebTech::TextureMemory* GpuMemoryBufferImplTextureMemory::GetTexture()
{
  return texture_;
}

void* GpuMemoryBufferImplTextureMemory::Map() {
  DCHECK(!mapped_);
  if (!texture_) {
    ZEROCOPY_LOG(" GpuMemoryBufferImplTextureMemory::Map no texture! ");
    return 0;
  }
  void* addr;
  texture_->Map(&addr, &stride_);
  addr_ = addr;

  mapped_ = true;
  ZEROCOPY_LOG(" GpuMemoryBufferImplTextureMemory::Map %p stride = %d ", this, stride_);
  return addr;
}

void GpuMemoryBufferImplTextureMemory::Unmap() {
  DCHECK(mapped_);
  if (!texture_) {
    ZEROCOPY_LOG(" GpuMemoryBufferImplTextureMemory::Unmap no texture! ");
    return;
  }
  texture_->Unmap();
  mapped_ = false;

}

uint32 GpuMemoryBufferImplTextureMemory::GetStride() const {
  ZEROCOPY_LOG(" GpuMemoryBufferImplTextureMemory::GetStride %p stride = %d ", this, stride_);
  if (stride_ == 0) {
    return texture_->GetStride() * InternalFormatToPixelSize(internalformat_);
  }
  return stride_ * InternalFormatToPixelSize(internalformat_);
}

gfx::GpuMemoryBufferHandle GpuMemoryBufferImplTextureMemory::GetHandle() const {
  ZEROCOPY_LOG(" GpuMemoryBufferImplTextureMemory::GetHandle serializing this = %p ", this);
  gfx::GpuMemoryBufferHandle handle;
  handle.type = gfx::TEXTURE_MEMORY_BUFFER;
  int fds[2];
  size_t size;
  size = texture_->GetSerializedDataSize();
  handle.texture_memory_data.Allocate(size);
  texture_->Serialize((uint8_t*)((handle.texture_memory_data.values_)), fds);
  handle.fd1.fd = (fds[0]);
  handle.fd2.fd = (fds[1]);
  return handle;
}

}  // namespace content

#endif
