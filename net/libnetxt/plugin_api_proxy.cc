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
#include "base/compiler_specific.h"
#include "build/build_config.h"

#include "base/time/time.h"
#include "url/gurl.h"
#include "net/libnetxt/plugin_api.h"

// ================================ net::HttpRequestHeaders ====================================
LIBNETXT_API_CPP_PROXY_IMP_CON_0(LibNetXt, net, HttpRequestHeaders)
LIBNETXT_API_CPP_PROXY_IMP_DES(LibNetXt, net, HttpRequestHeaders)
LIBNETXT_API_CPP_PROXY_IMP_1(LibNetXt, net, HttpRequestHeaders, AddHeadersFromString, void, const std::string&)
LIBNETXT_API_CPP_PROXY_IMP_2(LibNetXt, net, HttpRequestHeaders, GetHeader, bool, const std::string&, std::string*)
LIBNETXT_API_CPP_PROXY_IMP_2V(LibNetXt, net, HttpRequestHeaders, SetHeader, void, const std::string&, std::string&)
LIBNETXT_API_CPP_PROXY_IMP_0(LibNetXt, net, HttpRequestHeaders, ToString, std::string)

// ================================ net::HttpResponseHeaders ====================================
LIBNETXT_API_CPP_PROXY_IMP_CON_0(LibNetXt, net, HttpRequestInfo)

// ================================ net::HttpResponseHeaders ====================================
LIBNETXT_API_CPP_PROXY_IMP_0(LibNetXt, net, HttpResponseHeaders, GetContentLength, int64)
LIBNETXT_API_CPP_PROXY_IMP_1(LibNetXt, net, HttpResponseHeaders, IsRedirect, bool, std::string*)

// ================================ net::HttpCache ====================================
LIBNETXT_API_CPP_PROXY_IMP_0(LibNetXt, net, HttpCache, GetSession, net::HttpNetworkSession*)

// ================================ ::GURL ====================================
LIBNETXT_API_CPP_PROXY_IMP_CON_1(LibNetXt, , GURL, std::string&)
LIBNETXT_API_CPP_PROXY_IMP_DES(LibNetXt, , GURL)
LIBNETXT_API_CPP_PROXY_IMP_0(LibNetXt, , GURL, GetOrigin, GURL)
LIBNETXT_API_CPP_PROXY_IMP_1(LibNetXt, , GURL, Resolve, GURL, const std::string&)

// ================================ Common Interface ====================================
LIBNETXT_API_PROXY_IMP_0(LibNetXt, GetSystemTime, base::Time)
LIBNETXT_API_PROXY_IMP_2(LibNetXt, GetTimeDeltaInMs, int, const base::Time&, const base::Time&)
LIBNETXT_API_PROXY_IMP_2(LibNetXt, GetHostFromUrl, const char*, const std::string& , std::string&)
LIBNETXT_API_PROXY_IMP_3(LibNetXt, SysPropertyGet, int, const char*, char* , const char*)
LIBNETXT_API_PROXY_IMP_2(LibNetXt, GetHostPortFromUrl, const char*, const std::string&, std::string&)
LIBNETXT_API_PROXY_IMP_0(LibNetXt, GetMaxSocketsPerGroup, int)
LIBNETXT_API_PROXY_IMP_3V(LibNetXt, NetPreconnect, void, net::HttpNetworkSession*, GURL const&, int)
