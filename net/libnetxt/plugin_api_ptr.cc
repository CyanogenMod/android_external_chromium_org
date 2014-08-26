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
#include "net/libnetxt/plugin_api_ptr.h"

//=========================================================================
void InitLibnetxtPluginApi(LibnetxtPluginApi* plugin_api) {

    // ================================ net::HttpRequestHeaders ====================================
    LIBNETXT_API_CPP_PTR_IMP_CON(plugin_api, LibNetXt, net, HttpRequestHeaders)
    LIBNETXT_API_CPP_PTR_IMP_DES(plugin_api, LibNetXt, net, HttpRequestHeaders)
    LIBNETXT_API_CPP_PTR_IMP(plugin_api, LibNetXt, net, HttpRequestHeaders, AddHeadersFromString)
    LIBNETXT_API_CPP_PTR_IMP(plugin_api, LibNetXt, net, HttpRequestHeaders, GetHeader)
    LIBNETXT_API_CPP_PTR_IMP(plugin_api, LibNetXt, net, HttpRequestHeaders, SetHeader)
    LIBNETXT_API_CPP_PTR_IMP(plugin_api, LibNetXt, net, HttpRequestHeaders, ToString)

    // ================================ net::HttpResponseHeaders ====================================
    LIBNETXT_API_CPP_PTR_IMP_CON(plugin_api, LibNetXt, net, HttpRequestInfo)

    // ================================ net::HttpResponseHeaders ====================================
    LIBNETXT_API_CPP_PTR_IMP(plugin_api, LibNetXt, net, HttpResponseHeaders, GetContentLength)
    LIBNETXT_API_CPP_PTR_IMP(plugin_api, LibNetXt, net, HttpResponseHeaders, IsRedirect)

    // ================================ net::HttpCache ====================================
    LIBNETXT_API_CPP_PTR_IMP(plugin_api, LibNetXt, net, HttpCache, GetSession)

    // ================================ ::GURL ====================================
    LIBNETXT_API_CPP_PTR_IMP_CON(plugin_api, LibNetXt, , GURL)
    LIBNETXT_API_CPP_PTR_IMP_DES(plugin_api, LibNetXt, , GURL)
    LIBNETXT_API_CPP_PTR_IMP(plugin_api, LibNetXt, , GURL, GetOrigin)
    LIBNETXT_API_CPP_PTR_IMP(plugin_api, LibNetXt, , GURL, Resolve)

    // ================================ Common Interface ====================================
    LIBNETXT_API_PTR_IMP(plugin_api, LibNetXt, GetSystemTime)
    LIBNETXT_API_PTR_IMP(plugin_api, LibNetXt, GetTimeDeltaInMs)
    LIBNETXT_API_PTR_IMP(plugin_api, LibNetXt, GetHostFromUrl)
    LIBNETXT_API_PTR_IMP(plugin_api, LibNetXt, SysPropertyGet)
    LIBNETXT_API_PTR_IMP(plugin_api, LibNetXt, GetHostPortFromUrl)
    LIBNETXT_API_PTR_IMP(plugin_api, LibNetXt, GetMaxSocketsPerGroup)
    LIBNETXT_API_PTR_IMP(plugin_api, LibNetXt, DebugLog)
    LIBNETXT_API_PTR_IMP(plugin_api, LibNetXt, NetPreconnect)

    //Additional APIs
    InitStatHubLibnetxtPluginApi(plugin_api);
    InitGetZipLibnetxtPluginApi(plugin_api);

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
