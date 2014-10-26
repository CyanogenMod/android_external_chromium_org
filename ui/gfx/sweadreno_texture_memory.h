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

#ifndef CONTENT_COMMON_SWEADRENO_TEXTURE_MEMORY_H_
#define CONTENT_COMMON_SWEADRENO_TEXTURE_MEMORY_H_

//undef these to disable zero-copy and/or texture atlas
#if defined ANDROID
#define DO_ZERO_COPY
#define DO_TEXTURE_ATLAS
#endif

#if defined DO_ZERO_COPY
#define DO_PARTIAL_RASTERIZATION
#endif

#if (defined DO_ZERO_COPY && defined DO_TEXTURE_ATLAS)
#define DO_ZERO_COPY_WITH_ATLAS
#endif

//define COPYBACK_ON_WORKER_THREAD if the texture copy-back should be done on the worker thread
#ifdef DO_PARTIAL_RASTERIZATION
#define COPYBACK_ON_WORKER_THREAD
#endif

#ifdef DO_ZERO_COPY

#include "content/common/TextureMemory.h"
#include "ui/gfx/size.h"

#undef ZEROCOPY_LOG
#include <android/log.h>
#define LOCAL_TAG "libsweadrenoext"
#define ZEROCOPY_LOG(...)
#define ZEROCOPY_LOG_VERBOSE(...) __android_log_print(ANDROID_LOG_DEBUG, LOCAL_TAG, __VA_ARGS__);
#define ZEROCOPY_LOG_ERROR(...) __android_log_print(ANDROID_LOG_ERROR, LOCAL_TAG, __VA_ARGS__);

namespace swe {

class SerializedTextureMemory {
public:
  SerializedTextureMemory();
  SerializedTextureMemory(const SerializedTextureMemory& other);
  ~SerializedTextureMemory();
  void Allocate(size_t size);
  SerializedTextureMemory& operator= (const SerializedTextureMemory& other);
  size_t size_;
  uint8_t* values_;
};

bool InitTextureMemory();
WebTech::TextureMemory* CreateTextureMemory();
void DestroyTextureMemory(WebTech::TextureMemory* texture);
bool PrepareCreateTextureMemory();
WebTech::TextureMemory* GetLastTextureMemory();
void ResetLastTextureMemory();
#ifdef DO_ZERO_COPY_WITH_ATLAS
bool ShouldUseTextureAtlas();
void GetDesiredAtlasProperties(gfx::Size* atlas_size, gfx::Size* max_texture_size, int* padding);
#endif
}  // namespace content

#endif

#ifdef DO_PARTIAL_RASTERIZATION
#define ZEROCOPY_LOG_PARTIAL(...)
#endif

#endif  // CONTENT_COMMON_SWEADRENO_TEXTURE_MEMORY_H_
