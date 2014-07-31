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

#include "ui/gfx/sweadreno_texture_memory.h"

#ifdef DO_ZERO_COPY

#include <dirent.h>
#include <dlfcn.h>
#include <limits.h>
#include <sys/system_properties.h>
#include <sys/resource.h>
#include <unistd.h>
#include "base/files/file_path.h"
#include "base/lazy_instance.h"
#include "base/logging.h"
#include "base/strings/string_number_conversions.h"
#include "base/strings/string_piece.h"
#include "base/strings/stringprintf.h"
#include "base/synchronization/lock.h"
#include "base/threading/thread_local_storage.h"

#define TEXTURE_MEMORY_CREATION_LIMIT_PROPERTY_STRING "persist.swe.texturememorylimit"
#define ATLAS_CREATION_LIMIT_PROPERTY_STRING "persist.swe.atlaslimit"
#define DEFAULT_TEXTURE_MEMORY_CREATION_LIMIT 200
#define DEFAULT_ATLAS_CREATION_LIMIT 20

#ifdef DO_ZERO_COPY_WITH_ATLAS
#define TEXTURE_ATLAS_WIDTH_PROPERTY_STRING "persist.swe.atlaswidth"
#define TEXTURE_ALTAS_HEIGHT_PROPERTY_STRING "persist.swe.atlasheight"
#define TEXTURE_ALTAS_PADDING_PROPERTY_STRING "persist.swe.atlaspadding"
#define MAX_TEXTURE_WIDTH_PROPERTY_STRING "persist.swe.maxtexturewidth"
#define MAX_TEXTURE_HEIGHT_PROPERTY_STRING "persist.swe.maxtextureheight"
#define DEFAULT_TEXTURE_ATLAS_WIDTH 1
#define DEFAULT_TEXTURE_ALTAS_HEIGHT 5
#define DEFAULT_TEXTURE_ALTAS_PADDING 2
#define DEFAULT_MAX_TEXTURE_WIDTH 4096
#define DEFAULT_MAX_TEXTURE_HEIGHT 4096
#endif

namespace swe {

SerializedTextureMemory::SerializedTextureMemory() :
  size_(0),
  values_(0) {}

SerializedTextureMemory::SerializedTextureMemory(const SerializedTextureMemory& other) {
  if (other.size_ > 0) {
    Allocate(other.size_);
    for (size_t i = 0; i < size_; i++) {
      values_[i] = other.values_[i];
    }
  } else {
    size_ = other.size_;
    values_ = 0;
  }
}

SerializedTextureMemory::~SerializedTextureMemory() {
  if (values_)
    delete[] values_;
}

void SerializedTextureMemory::Allocate(size_t size) {
  size_ = size;
  values_ = new uint8_t[size_];
}

SerializedTextureMemory& SerializedTextureMemory::operator= (const SerializedTextureMemory& other) {
  if (other.size_ > 0) {
    Allocate(other.size_);
    for (size_t i = 0; i < size_; i++) {
      values_[i] = other.values_[i];
    }
  } else {
    size_ = other.size_;
    values_ = 0;
  }
  return *this;
}

static void* s_libsweadrenoext_handle = NULL;
static WebTech::CreateTextureMemoryFunc s_create_texture_memory_func = 0;
static WebTech::CheckVersionFunc s_check_version_func = 0;
static bool s_load_failed = false;

int ParseStringValue(const base::StringPiece& str, int default_value) {
  int value;
  bool parsed = base::StringToInt(str, &value);
  if (parsed)
    return value;
  return default_value;
}

int GetSystemPropertyValue(const char* prop_string, int default_value) {
  char value_string[PROP_VALUE_MAX];
  int string_length;
  string_length = __system_property_get(prop_string, value_string);
  if (string_length > 0) {
    return ParseStringValue(value_string, default_value);
  }
  return default_value;
}

struct TextureMemoryLib {
public:
  TextureMemoryLib()
    : num_texture_memory_created_(0)
    , num_texture_memory_preparing_(0) {
    texture_memory_creation_limit_ = GetSystemPropertyValue(TEXTURE_MEMORY_CREATION_LIMIT_PROPERTY_STRING, DEFAULT_TEXTURE_MEMORY_CREATION_LIMIT);
    atlas_creation_limit_ = GetSystemPropertyValue(ATLAS_CREATION_LIMIT_PROPERTY_STRING, DEFAULT_ATLAS_CREATION_LIMIT);
#ifdef DO_ZERO_COPY_WITH_ATLAS
    int width, height;
    width = GetSystemPropertyValue(TEXTURE_ATLAS_WIDTH_PROPERTY_STRING, DEFAULT_TEXTURE_ATLAS_WIDTH);
    height = GetSystemPropertyValue(TEXTURE_ALTAS_HEIGHT_PROPERTY_STRING, DEFAULT_TEXTURE_ALTAS_HEIGHT);
    atlas_size_ = gfx::Size(width, height);
    width = GetSystemPropertyValue(MAX_TEXTURE_WIDTH_PROPERTY_STRING, DEFAULT_MAX_TEXTURE_WIDTH);
    height = GetSystemPropertyValue(MAX_TEXTURE_HEIGHT_PROPERTY_STRING, DEFAULT_MAX_TEXTURE_HEIGHT);
    max_texture_size_ = gfx::Size(width, height);
    atlas_padding_ = GetSystemPropertyValue(TEXTURE_ALTAS_PADDING_PROPERTY_STRING, DEFAULT_TEXTURE_ALTAS_PADDING);
#endif
  }
  base::Lock mutex_;
  int num_texture_memory_created_;
  int num_texture_memory_preparing_;
  int texture_memory_creation_limit_;
  int atlas_creation_limit_;
  base::ThreadLocalStorage::Slot newly_created_texture_;
#ifdef DO_ZERO_COPY_WITH_ATLAS
  gfx::Size atlas_size_;
  gfx::Size max_texture_size_;
  int atlas_padding_;
#endif
};

base::LazyInstance<TextureMemoryLib> s_texture_memory_lib;

bool LoadSWEAdrenoExtLib(const char* libname) {
  ZEROCOPY_LOG("loading %s", libname);
  s_libsweadrenoext_handle = dlopen(libname, RTLD_LAZY);
  if (!s_libsweadrenoext_handle) {
    ZEROCOPY_LOG_ERROR("loading %s failed ", libname);
  } else {
    s_create_texture_memory_func = (WebTech::CreateTextureMemoryFunc)(dlsym(s_libsweadrenoext_handle, CREATE_TEXTURE_MEMORY_FUNC_NAME));
    if (s_create_texture_memory_func == NULL) {
      ZEROCOPY_LOG_ERROR("loading function CreateTextureMemory failed.");
    } else {
      ZEROCOPY_LOG("loading function CreateTextureMemory succeeded.");
      s_check_version_func = (WebTech::CheckVersionFunc)(dlsym(s_libsweadrenoext_handle, CHECK_VERSION_FUNC_NAME));
      if (s_check_version_func == NULL) {
        ZEROCOPY_LOG_ERROR("loading function CheckVersion failed.");
      } else {
        ZEROCOPY_LOG("loading function CheckVersion succeeded.");
        bool supported = (s_check_version_func)(TEXTURE_MEMORY_VERSION, TEXTURE_MEMORY_DEFINITION_TO_STRING(TEXTURE_MEMORY_DEFINITION));
        if (supported) {
          ZEROCOPY_LOG_ERROR("loading %s succeeded", libname);
          return true;
        }
      }
    }
  }
  return false;
}

bool InitTextureMemory() {
  TextureMemoryLib* lib = s_texture_memory_lib.Pointer();
  base::AutoLock lock(lib->mutex_);
  if (!s_libsweadrenoext_handle && !s_load_failed) {
    if (LoadSWEAdrenoExtLib("libsweadrenoext_plugin.so"))
      return true;
    if (LoadSWEAdrenoExtLib("libsweadrenoext_18_plugin.so"))
      return true;
    s_load_failed = true;
  }
  return !s_load_failed;
}


WebTech::TextureMemory* CreateTextureMemory() {
  if (s_create_texture_memory_func == NULL)
    return 0;

  WebTech::TextureMemory* texture = (s_create_texture_memory_func)();
  TextureMemoryLib* lib = s_texture_memory_lib.Pointer();
  base::AutoLock lock(lib->mutex_);
  lib->newly_created_texture_.Set(static_cast<void*>(texture));
  if (lib->num_texture_memory_preparing_ > 0)
    lib->num_texture_memory_preparing_--;

  lib->num_texture_memory_created_++;
  ZEROCOPY_LOG("CreateTextureMemory created = %d, preparing = %d",lib->num_texture_memory_created_,  lib->num_texture_memory_preparing_);
  return texture;
}

void DestroyTextureMemory(WebTech::TextureMemory* texture) {
  TextureMemoryLib* lib = s_texture_memory_lib.Pointer();
  base::AutoLock lock(lib->mutex_);
  lib->num_texture_memory_created_--;
  texture->Release();
  ZEROCOPY_LOG("DestroyTextureMemory created = %d, preparing = %d",lib->num_texture_memory_created_,  lib->num_texture_memory_preparing_);
}

bool PrepareCreateTextureMemory() {
  if (!InitTextureMemory())
    return false;
  TextureMemoryLib* lib = s_texture_memory_lib.Pointer();
  base::AutoLock lock(lib->mutex_);
  ZEROCOPY_LOG("PrepareCreateTextureMemory created = %d, preparing = %d",lib->num_texture_memory_created_,  lib->num_texture_memory_preparing_);
  int num_in_use = lib->num_texture_memory_created_ + lib->num_texture_memory_preparing_;
  if (num_in_use >= lib->texture_memory_creation_limit_)
    return false;
  lib->num_texture_memory_preparing_++;
  return true;
}

void ResetLastTextureMemory() {
  TextureMemoryLib* lib = s_texture_memory_lib.Pointer();
  lib->newly_created_texture_.Set(0);
}

WebTech::TextureMemory* GetLastTextureMemory() {
  TextureMemoryLib* lib = s_texture_memory_lib.Pointer();
  WebTech::TextureMemory* texture = static_cast<WebTech::TextureMemory*>(lib->newly_created_texture_.Get());
  lib->newly_created_texture_.Set(0);
  return texture;
}

#ifdef DO_ZERO_COPY_WITH_ATLAS
bool ShouldUseTextureAtlas() {
  TextureMemoryLib* lib = s_texture_memory_lib.Pointer();
  base::AutoLock lock(lib->mutex_);
  int num_in_use = lib->num_texture_memory_created_ + lib->num_texture_memory_preparing_;
  if (num_in_use > (lib->texture_memory_creation_limit_ - lib->atlas_creation_limit_))
    return true;
  return false;
}

void GetDesiredAtlasProperties(gfx::Size* atlas_size, gfx::Size* max_texture_size, int* padding) {
  TextureMemoryLib* lib = s_texture_memory_lib.Pointer();
  base::AutoLock lock(lib->mutex_);
  *atlas_size = lib->atlas_size_;
  *max_texture_size = lib->max_texture_size_;
  *padding = lib->atlas_padding_;
}

#endif

}  // namespace swe

#endif
