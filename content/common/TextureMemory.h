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

#ifndef WebTech_TextureMemory_h
#define WebTech_TextureMemory_h

#include <stdint.h>

//interface TextureMemory
//define the TextureMemory class definition in the TEXTURE_MEMORY_DEFINITION macro
//so that it can be stringified.  A string that contains the definition of the class
//can be created using TEXTURE_MEMORY_DEFINITION_TO_STRING(TEXTURE_MEMORY_DEFINITION)
//defined below.  The string can be passed from the client to the CheckVersionFunc() function
//below so that the TextureMemory implementation can make sure it matches the definition
//used in the implementation.  The TextureMemory implementation resides in a dynamic
//library.  This string matching ensure that the .so and the client application use
//the same TextureMemory definition.

#define TEXTURE_MEMORY_DEFINITION \
class TextureMemory {\
public:\
    virtual ~TextureMemory() {};\
    virtual void Ref() = 0;\
    virtual void Release() = 0;\
    virtual bool Init(int width, int height, PixelFormat format) = 0;\
    virtual bool Init(uint8_t* serialized_data,\
                      size_t serialized_data_size,\
                      int* file_descriptors,\
                      int num_file_descriptors) = 0;\
    virtual void Map(void** addr, size_t* stride) = 0;\
    virtual size_t GetStride() = 0;\
    virtual void Unmap() = 0;\
    virtual void FinalizeRendering(int changed_x,\
                              int changed_y,\
                              int changed_width,\
                              int changed_height) = 0;\
    virtual void* GetNativeHandle() = 0;\
    virtual size_t GetSerializedDataSize() = 0;\
    virtual size_t GetNumFileDescriptors() = 0;\
    virtual void Serialize(uint8_t* buffer, int* file_descriptors) = 0;\
};\
typedef bool (*CheckVersionFunc)(int version, const char* definition_string);\
typedef WebTech::TextureMemory* (*CreateTextureMemoryFunc)();

#define TEXTURE_MEMORY_DEFINITION_TO_STRING2(s) #s
#define TEXTURE_MEMORY_DEFINITION_TO_STRING(s) TEXTURE_MEMORY_DEFINITION_TO_STRING2(s)


//the actual definition of TextureMemory class
namespace WebTech {
enum PixelFormat{FORMAT_8888, FORMAT_888X, FORMAT_565, FORMAT_4444};
TEXTURE_MEMORY_DEFINITION
}

#define TEXTURE_MEMORY_VERSION 1
#define CHECK_VERSION_FUNC_NAME "CheckVersion"
#define CREATE_TEXTURE_MEMORY_FUNC_NAME "CreateTextureMemory"

#endif // WebTech_TextureMemory_h
