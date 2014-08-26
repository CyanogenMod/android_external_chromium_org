/*
* Copyright (c) 2013, The Linux Foundation. All rights reserved.
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

#ifndef STAT_HUB_ALL_API_DEF_H_
#define STAT_HUB_ALL_API_DEF_H_

#if defined(STAT_HUB_API_BY_PROXY)
    #define LIBNETXT_API_BY_PROXY
#elif defined(STAT_HUB_API_BY_PTR)
    #define LIBNETXT_API_BY_PTR
#elif defined(STAT_HUB_API_BY_IPC)
    #define LIBNETXT_API_BY_IPC
#else
    //ToDo: Direct
#endif

// ================================ LibNetXt global ====================================
#define STAT_HUB_API_PREFIX StatHub

#define STAT_HUB_API(name) \
    LIBNETXT_API_NAME(StatHub, name)
#define STAT_HUB_API_CPP(namesp, type, name) \
    LIBNETXT_API_CPP_NAME(StatHub, namesp, type, name)
#define STAT_HUB_API_CPP_CON(namesp, type) \
    LIBNETXT_API_CPP_CON_NAME(StatHub, namesp, type)
#define STAT_HUB_API_CPP_DES(namesp, type) \
    LIBNETXT_API_CPP_DES_NAME(StatHub, namesp, type)

#include "net/libnetxt/plugin_api_def.h"

#define STAT_HUB_IS_VERBOSE_LEVEL_ERROR     (STAT_HUB_API(GetVerboseLevel)()>=STAT_HUB_VERBOSE_LEVEL_ERROR)
#define STAT_HUB_IS_VERBOSE_LEVEL_WARNING   (STAT_HUB_API(GetVerboseLevel)()>=STAT_HUB_VERBOSE_LEVEL_WARNING)
#define STAT_HUB_IS_VERBOSE_LEVEL_INFO      (STAT_HUB_API(GetVerboseLevel)()>=STAT_HUB_VERBOSE_LEVEL_INFO)
#define STAT_HUB_IS_VERBOSE_LEVEL_DEBUG     (STAT_HUB_API(GetVerboseLevel)()>=STAT_HUB_VERBOSE_LEVEL_DEBUG)


#endif /* STAT_HUB_ALL_API_DEF_H_ */
