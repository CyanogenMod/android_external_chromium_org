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

#include <unistd.h>
#include <string>
#include <set>
#include <stdio.h>

#include "url/gurl.h"
#include "net/base/host_port_pair.h"
#include "net/http/http_cache_transaction.h"
#include "net/http/http_request_info.h"
#include "net/http/http_request_headers.h"
#include "net/http/http_response_info.h"
#include "net/http/http_response_headers.h"
#include "net/socket/client_socket_pool_manager.h"
#include "base/time/time.h"

#include "net/libnetxt/plugin_api.h"

// ================================ net::HttpRequestHeaders  ====================================
LIBNETXT_API_CPP_FORWARDER_CON_0(LibNetXt, net, HttpRequestHeaders)
LIBNETXT_API_CPP_FORWARDER_DES(LibNetXt, net, HttpRequestHeaders)
LIBNETXT_API_CPP_FORWARDER_1V(LibNetXt, net, HttpRequestHeaders, AddHeadersFromString, void, const std::string&)
LIBNETXT_API_CPP_FORWARDER_2(LibNetXt, net, HttpRequestHeaders, GetHeader, bool, const std::string&, std::string*)
LIBNETXT_API_CPP_FORWARDER_2V(LibNetXt, net, HttpRequestHeaders, SetHeader, void, const std::string&, std::string&)
LIBNETXT_API_CPP_FORWARDER_0(LibNetXt, net, HttpRequestHeaders, ToString, std::string)

// ================================ net::HttpRequestInfo  ====================================
LIBNETXT_API_CPP_FORWARDER_CON_0(LibNetXt, net, HttpRequestInfo)

// ================================ net::HttpResponseHeaders  ====================================
LIBNETXT_API_CPP_FORWARDER_0(LibNetXt, net, HttpResponseHeaders, GetContentLength, int64)
LIBNETXT_API_CPP_FORWARDER_1(LibNetXt, net, HttpResponseHeaders, IsRedirect, bool, std::string*)

// ================================ net::HttpCache ====================================
LIBNETXT_API_CPP_FORWARDER_0(LibNetXt, net, HttpCache, GetSession, net::HttpNetworkSession*)

// ================================ ::GURL  ====================================
LIBNETXT_API_CPP_FORWARDER_CON_1(LibNetXt, , GURL, std::string&)
LIBNETXT_API_CPP_FORWARDER_DES(LibNetXt, , GURL)
LIBNETXT_API_CPP_FORWARDER_0(LibNetXt, , GURL, GetOrigin, GURL)
LIBNETXT_API_CPP_FORWARDER_1(LibNetXt, , GURL, Resolve, GURL, const std::string&)

// ======================================= Common ==============================================
base::Time LIBNETXT_API(GetSystemTime)() {
    return base::Time::NowFromSystemTime();
}

int LIBNETXT_API(GetTimeDeltaInMs)(const base::Time& start_time, const base::Time& finish_time) {
    base::TimeDelta delta = finish_time - start_time;
    return (int)delta.InMilliseconds(); //int64
}

const char* LIBNETXT_API(GetHostFromUrl)(const std::string& url, std::string& host) {
    GURL dest(url);
    host = dest.GetOrigin().spec();
    return host.c_str();
}

const char* LIBNETXT_API(GetHostPortFromUrl)(const std::string& url, std::string& host_port) {
    GURL dest(url);

    net::HostPortPair origin_host_port =
        net::HostPortPair(dest.HostNoBrackets(),dest.EffectiveIntPort());
    host_port = origin_host_port.ToString();
    return host_port.c_str();
}

int LIBNETXT_API(GetMaxSocketsPerGroup)() {
    return net::ClientSocketPoolManager::max_sockets_per_group(net::HttpNetworkSession::NORMAL_SOCKET_POOL);
}

int LIBNETXT_API(SysPropertyGet)(const char *key, char *value, const char *default_value) {
    return LIBNETXT_PROPERTY_GET(key, value, default_value);
}

int LIBNETXT_API(DebugLog)(const char* str) {
    LIBNETXT_LOGD("%s", str);
    return strlen(str);
}
