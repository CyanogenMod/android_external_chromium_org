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
#include "base/compiler_specific.h"
#include "build/build_config.h"

#include "base/time/time.h"
#include "url/gurl.h"
#include "net/libnetxt/plugin_api_ptr.h"

//=========================================================================
void InitLibnetxtPluginApi(LibnetxtPluginApi* plugin_api) {

    // ================================ net::HttpRequestHeaders ====================================
    LIBNETXT_API_CPP_PTR_IMP_CON(plugin_api, LibNetXt, net, HttpRequestHeaders)
    LIBNETXT_API_CPP_PTR_IMP_DES(plugin_api, LibNetXt, net, HttpRequestHeaders)
    LIBNETXT_API_CPP_PTR_IMP(plugin_api, LibNetXt, net, HttpRequestHeaders, AddHeaderFromString)
    LIBNETXT_API_CPP_PTR_IMP(plugin_api, LibNetXt, net, HttpRequestHeaders, GetHeader)
    LIBNETXT_API_CPP_PTR_IMP(plugin_api, LibNetXt, net, HttpRequestHeaders, SetHeader)
    LIBNETXT_API_CPP_PTR_IMP(plugin_api, LibNetXt, net, HttpRequestHeaders, ToString)

   // ================================ net::HttpRequestInfo  ====================================
    LIBNETXT_API_CPP_PTR_IMP_CON(plugin_api, LibNetXt, net, HttpRequestInfo)
    LIBNETXT_API_CPP_PTR_IMP_DES(plugin_api, LibNetXt, net, HttpRequestInfo)

   // ================================ net::HttpResponseInfo  ====================================
    LIBNETXT_API_CPP_PTR_IMP_CON(plugin_api, LibNetXt, net, HttpResponseInfo)
    LIBNETXT_API_CPP_PTR_IMP_DES(plugin_api, LibNetXt, net, HttpResponseInfo)

    // ================================ net::HttpResponseHeaders ====================================
    LIBNETXT_API_CPP_PTR_IMP_CON(plugin_api, LibNetXt, net, HttpResponseHeaders)
    LIBNETXT_API_CPP_PTR_IMP(plugin_api, LibNetXt, net, HttpResponseHeaders, GetContentLength)
    LIBNETXT_API_CPP_PTR_IMP(plugin_api, LibNetXt, net, HttpResponseHeaders, IsRedirect)
    LIBNETXT_API_CPP_PTR_IMP(plugin_api,LibNetXt, net, HttpResponseHeaders, ReplaceStatusLine)
    LIBNETXT_API_CPP_PTR_IMP(plugin_api, LibNetXt, net, HttpResponseHeaders, RemoveHeader)
    LIBNETXT_API_CPP_PTR_IMP(plugin_api, LibNetXt, net, HttpResponseHeaders, IsChunkEncoded)
    LIBNETXT_API_CPP_PTR_IMP(plugin_api, LibNetXt, net, HttpResponseHeaders, HasHeaderValue)
    LIBNETXT_API_CPP_PTR_IMP(plugin_api, LibNetXt, net, HttpResponseHeaders, HasHeader)
    LIBNETXT_API_CPP_PTR_IMP(plugin_api, LibNetXt, net, HttpResponseHeaders, GetStatusLine)
    LIBNETXT_API_CPP_PTR_IMP(plugin_api, LibNetXt, net, HttpResponseHeaders, GetContentRange)
    LIBNETXT_API_CPP_PTR_IMP(plugin_api, LibNetXt, net, HttpResponseHeaders, AddHeader)
    LIBNETXT_API_CPP_PTR_IMP(plugin_api, LibNetXt, net, HttpResponseHeaders, GetHttpVersion)
    LIBNETXT_API_CPP_PTR_IMP(plugin_api, LibNetXt, net, HttpResponseHeaders, EnumerateHeader)
    LIBNETXT_API_CPP_PTR_IMP(plugin_api, LibNetXt, net, HttpResponseHeaders, response_code)
    LIBNETXT_API_CPP_PTR_IMP(plugin_api, LibNetXt, net, HttpResponseHeaders, raw_headers)
    LIBNETXT_API_CPP_PTR_IMP(plugin_api, LibNetXt, net, HttpResponseHeaders, EnumerateHeaderLines)
    LIBNETXT_API_CPP_PTR_IMP_SP_CON(plugin_api, LibNetXt, net, HttpResponseHeaders)
    LIBNETXT_API_CPP_PTR_IMP_SP_DES(plugin_api, LibNetXt, net, HttpResponseHeaders)

    // ================================ net::HttpByteRange =================================
    LIBNETXT_API_CPP_PTR_IMP_CON(plugin_api, LibNetXt, net, HttpByteRange)
    LIBNETXT_API_CPP_PTR_IMP_DES(plugin_api,LibNetXt, net, HttpByteRange)
    LIBNETXT_API_CPP_PTR_IMP(plugin_api, LibNetXt, net, HttpByteRange, IsSuffixByteRange)

    // ================================ net::HttpNetworkTransaction =================================
    LIBNETXT_API_CPP_PTR_IMP_CON(plugin_api,LibNetXt, net, HttpNetworkTransaction)
    LIBNETXT_API_CPP_PTR_IMP_DES(plugin_api,LibNetXt, net, HttpNetworkTransaction)
    LIBNETXT_API_CPP_PTR_IMP(plugin_api,LibNetXt, net, HttpNetworkTransaction, SetUseStaPool)

    // ================================ net::HttpCache ====================================
    LIBNETXT_API_CPP_PTR_IMP(plugin_api, LibNetXt, net, HttpCache, GetSession)

    // ================================ ::GURL ====================================
    LIBNETXT_API_CPP_PTR_IMP_CON(plugin_api, LibNetXt, , GURL)
    LIBNETXT_API_CPP_PTR_IMP_DES(plugin_api, LibNetXt, , GURL)
    LIBNETXT_API_CPP_PTR_IMP(plugin_api, LibNetXt, , GURL, GetOrigin)
    LIBNETXT_API_CPP_PTR_IMP(plugin_api, LibNetXt, , GURL, Resolve)
    LIBNETXT_API_CPP_PTR_IMP(plugin_api, LibNetXt, , GURL, ExtractFileName)

    // ================================ net::HostPortPair ====================================
    LIBNETXT_API_CPP_PTR_IMP(plugin_api, LibNetXt, net, HostPortPair, ToString)

    // ================================ base::IOBufferWithSize ====================================
    LIBNETXT_API_CPP_PTR_IMP_CON(plugin_api, LibNetXt,net, IOBufferWithSize)

    // ================================ base::SystemMemoryInfoKB ====================================
    LIBNETXT_API_CPP_PTR_IMP_CON(plugin_api, LibNetXt, base, SystemMemoryInfoKB)
    LIBNETXT_API_CPP_PTR_IMP_DES(plugin_api, LibNetXt, base, SystemMemoryInfoKB)

    // ================================ logging::LogMessage ====================================
    LIBNETXT_API_CPP_PTR_IMP_CON(plugin_api, LibNetXt, logging, LogMessage)
    LIBNETXT_API_CPP_PTR_IMP_DES(plugin_api, LibNetXt, logging, LogMessage)

    // ================================ Common Interface ====================================
    LIBNETXT_API_PTR_IMP(plugin_api, LibNetXt, GetSystemMemoryInfo)
    LIBNETXT_API_PTR_IMP(plugin_api, LibNetXt, GetSystemTime)
    LIBNETXT_API_PTR_IMP(plugin_api, LibNetXt, GetTimeDeltaInMs)
    LIBNETXT_API_PTR_IMP(plugin_api, LibNetXt, GetHostFromUrl)
    LIBNETXT_API_PTR_IMP(plugin_api, LibNetXt, SysPropertyGet)
    LIBNETXT_API_PTR_IMP(plugin_api, LibNetXt, GetHostPortFromUrl)
    LIBNETXT_API_PTR_IMP(plugin_api, LibNetXt, GetMaxSocketsPerGroup)
    LIBNETXT_API_PTR_IMP(plugin_api, LibNetXt, DebugLog)
    LIBNETXT_API_PTR_IMP(plugin_api, LibNetXt, NetPreconnect)
    LIBNETXT_API_PTR_IMP(plugin_api, LibNetXt, ParseHostAndPort)
    LIBNETXT_API_PTR_IMP(plugin_api, LibNetXt, GetRequestRange)
    LIBNETXT_API_PTR_IMP(plugin_api, LibNetXt, ParseRangeHeader)
    LIBNETXT_API_PTR_IMP(plugin_api, LibNetXt, AssembleRawHeadersAndAssign)
    LIBNETXT_API_PTR_IMP(plugin_api, LibNetXt, HttpByteRangeToString)
    LIBNETXT_API_PTR_IMP(plugin_api, LibNetXt, GetResponseHeaderLines)
    LIBNETXT_API_PTR_IMP(plugin_api, LibNetXt, PathExists)
    LIBNETXT_API_PTR_IMP(plugin_api, LibNetXt, GetUrlOriginSpec)
    LIBNETXT_API_PTR_IMP(plugin_api, LibNetXt, AssignHttpResponseHeaders)
    LIBNETXT_API_PTR_IMP(plugin_api, LibNetXt, PostTask)
    LIBNETXT_API_PTR_IMP(plugin_api, LibNetXt, ConvertHeadersBackToHTTPResponse)
    LIBNETXT_API_PTR_IMP(plugin_api, LibNetXt, IsVerboseEnabled)
    LIBNETXT_API_PTR_IMP(plugin_api, LibNetXt, GetVerboseLevel)
    LIBNETXT_API_PTR_IMP(plugin_api, LibNetXt, EscapeForHTML)

    //Additional APIs
    InitStatHubLibnetxtPluginApi(plugin_api);
    InitTcpFinAggLibnetxtPluginApi(plugin_api);
    InitGetZipLibnetxtPluginApi(plugin_api);
    InitStaLibnetxtPluginApi(plugin_api);

}

//=========================================================================
LibnetxtPluginApi* LibnetxtPluginApi::GetInstance() {
    CR_DEFINE_STATIC_LOCAL(LibnetxtPluginApi, plugin_api, ());
    return &plugin_api;
}

//=========================================================================
LibnetxtPluginApi::~LibnetxtPluginApi() {
}

//=========================================================================
LibnetxtPluginApi::LibnetxtPluginApi() {
    InitLibnetxtPluginApi(this);
}
