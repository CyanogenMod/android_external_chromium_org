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
#ifndef PLUGIN_API_PTR_H_
#define PLUGIN_API_PTR_H_

#include <string>
#include "base/basictypes.h"
#include "net/libnetxt/libnetxt_base.h"
#include "net/libnetxt/plugin_api.h"

//Additional APIs

#define LIBNETXT_API_VERSION_MAJOR 1
#define LIBNETXT_API_VERSION_MINOR 0

#if defined( __GNUC__)
    #define LIBNETXT_API_COMP_VERSION GCC
#elif defined( __llvm__)
    #define LIBNETXT_API_COMP_VERSION LLVM
#elif defined( __clang__)
    #define LIBNETXT_API_COMP_VERSION CLANG
#endif

#define LIBNETXT_API_VERSION LIBNETXT_VAL_TO_STR(LIBNETXT_API_VERSION_MAJOR.LIBNETXT_API_VERSION_MINOR.LIBNETXT_API_COMP_VERSION)

class LibnetxtPluginApi {

public:
    LibnetxtPluginApi();
    ~LibnetxtPluginApi();

static LibnetxtPluginApi* GetInstance();

    // ================================ net::HttpRequestHeaders  ====================================
    LIBNETXT_API_CPP_PTR_DEF_CON_0(LibNetXt, net, HttpRequestHeaders)
    LIBNETXT_API_CPP_PTR_DEF_DES(LibNetXt, net, HttpRequestHeaders)
    LIBNETXT_API_CPP_PTR_DEF_1(LibNetXt, net, HttpRequestHeaders, AddHeadersFromString, void, const std::string&)
    LIBNETXT_API_CPP_PTR_DEF_2(LibNetXt, net, HttpRequestHeaders, GetHeader, bool, const std::string&, std::string*)
    LIBNETXT_API_CPP_PTR_DEF_2(LibNetXt, net, HttpRequestHeaders, SetHeader, void, const std::string&, std::string&)
    LIBNETXT_API_CPP_PTR_DEF_0(LibNetXt, net, HttpRequestHeaders, ToString, std::string)

    // ================================ net::HttpRequestInfo  ====================================
    LIBNETXT_API_CPP_PTR_DEF_CON_0(LibNetXt, net, HttpRequestInfo)

    // ================================ net::HttpResponseHeaders  ====================================
    LIBNETXT_API_CPP_PTR_DEF_0(LibNetXt, net, HttpResponseHeaders, GetContentLength, int64)
    LIBNETXT_API_CPP_PTR_DEF_1(LibNetXt, net, HttpResponseHeaders, IsRedirect, bool, std::string*)

    // ================================ net::HttpCache ====================================
    LIBNETXT_API_CPP_PTR_DEF_0(LibNetXt, net, HttpCache, GetSession, net::HttpNetworkSession*)

    // ================================ ::GURL  ====================================
    LIBNETXT_API_CPP_PTR_DEF_CON_1(LibNetXt, , GURL, std::string&)
    LIBNETXT_API_CPP_PTR_DEF_DES(LibNetXt, , GURL)
    LIBNETXT_API_CPP_PTR_DEF_0(LibNetXt, , GURL, GetOrigin, GURL)
    LIBNETXT_API_CPP_PTR_DEF_1(LibNetXt, , GURL, Resolve, GURL, const std::string&)

    // ================================ Common Interface ====================================
    LIBNETXT_API_PTR_DEF_0(LibNetXt, GetSystemTime, base::Time)
    LIBNETXT_API_PTR_DEF_2(LibNetXt, GetTimeDeltaInMs, int, const base::Time&, const base::Time&)
    LIBNETXT_API_PTR_DEF_2(LibNetXt, GetHostFromUrl, const char*, const std::string& , std::string&)
    LIBNETXT_API_PTR_DEF_2(LibNetXt, GetHostPortFromUrl, const char*, const std::string&, std::string&)
    LIBNETXT_API_PTR_DEF_0(LibNetXt, GetMaxSocketsPerGroup, int)
    LIBNETXT_API_PTR_DEF_3(LibNetXt, SysPropertyGet, int, const char*, char* , const char*)
    LIBNETXT_API_PTR_DEF_1(LibNetXt, DebugLog, int, const char*)

};

#endif /* PLUGIN_API_PTR_H_ */
