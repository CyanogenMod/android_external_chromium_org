/*
* Copyright (c) 2013-2014, The Linux Foundation. All rights reserved.
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions are
* met:
*     * Redistributions of source code must retain the above copyright
*       notice, this list of conditions and the following disclaimer.
*     * Redistributions in binary form must reproduce the above
*       copyright notice, this list of conditions and the following
*       disclaimer in the documentation and/or other materials provided
*       with the distribution.
*     * Neither the name of The Linux Foundation nor the names of its
*       contributors may be used to endorse or promote products derived
*       from this software without specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED "AS IS" AND ANY EXPRESS OR IMPLIED
* WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT
* ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS
* BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
* CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
* SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
* BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
* WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
* OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
* IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
#ifndef LIBNETXT_BASE_H_
#define LIBNETXT_BASE_H_

#include "base/basictypes.h"

#define LIBNETXT_VAL_TO_STR(val) LIBNETXT_VAL_TO_STR_HELPER(val)
#define LIBNETXT_VAL_TO_STR_HELPER(val) #val

typedef enum LibnetxtVerboseLevel {
    LIBNETXT_VERBOSE_LEVEL_DISABLED,// 0
    LIBNETXT_VERBOSE_LEVEL_ERROR,   // 1
    LIBNETXT_VERBOSE_LEVEL_WARNING, // 2
    LIBNETXT_VERBOSE_LEVEL_INFO,    // 3
    LIBNETXT_VERBOSE_LEVEL_DEBUG    // 4
} LibnetxtVerboseLevel;

#if defined(ANDROID)

#include <cutils/log.h>
#include <cutils/properties.h>
#include "base/android/path_utils.h"

#define LIBNETXT_IS_VERBOSE libnetxt_isVerboseEnabled()
#define LIBNETXT_LOGD   SLOGD
#define LIBNETXT_LOGI   SLOGI
#define LIBNETXT_LOGW   SLOGE
#define LIBNETXT_LOGE   SLOGE

#define LIBNETXT_PROPERTY_GET   libnetxt_property_get
#define LIBNETXT_SYSPROP_GET    property_get
#define LIBNETXT_LIBDIR_GET     base::android::GetNativeLibraryDirectory
#define LIBNETXT_CACHEDIR_GET   base::android::GetCacheDirectory
#define LIBNETXT_DATADIR_GET    base::android::GetDataDirectory

#ifdef LOG_TAG
    #undef LOG_TAG
    #define LOG_TAG "libnetxt"
#endif

#else //defined(ANDROID)
#include <string.h>

#define LIBNETXT_IS_VERBOSE false
#define LIBNETXT_LOGD(...)
#define LIBNETXT_LOGI(...)
#define LIBNETXT_LOGW(...)
#define LIBNETXT_LOGE(...)

#define PROPERTY_VALUE_MAX          92
#define LIBNETXT_SYSPROP_GET(prop_name, value, default)  strlen(strncpy(value, default,PROPERTY_VALUE_MAX)); value[PROPERTY_VALUE_MAX-1] = '\0'
#define LIBNETXT_PROPERTY_GET LIBNETXT_SYSPROP_GET
#define LIBNETXT_LIBDIR_GET(...)
#define LIBNETXT_CACHEDIR_GET(...)
#define LIBNETXT_DATADIR_GET(...)

#endif //defined(ANDROID)

#ifdef __cplusplus
extern "C" {
#endif

extern int libnetxt_property_get(const char *key, char *value, const char *default_value)
    __attribute__((visibility("default"), used));

extern bool libnetxt_isVerboseEnabled()
    __attribute__((visibility("default"), used));

#ifdef __cplusplus
}
#endif

#endif /* LIBNETXT_BASE_H_ */
