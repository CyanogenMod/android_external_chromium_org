/**
 * Copyright (c) 2012, 2013, The Linux Foundation. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *   * Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *   * Redistributions in binary form must reproduce the above
 *     copyright notice, this list of conditions and the following
 *     disclaimer in the documentation and/or other *materials provided
 *     with the distribution.
 *   * Neither the name of The Linux Foundation nor the names of its
 *     contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission.

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
 **/

#ifndef HTTP_GETZIP_BRIDGE_H_
#define HTTP_GETZIP_BRIDGE_H_

// ================================ LibNetXt global ====================================
#define GETZIP_API(name) \
    LIBNETXT_API_NAME(GetZip, name)

#include "net/libnetxt/plugin_api_def.h"
#include <string>

namespace net {
    class HttpRequestHeaders;
    class HttpResponseHeaders;
}

//HttpRequestHeader method delegates
//@argument it - should be initialized to NULL
LIBNETXT_API_DEF_2(GetZip, GetFirstHttpRequestHeader, void, net::HttpRequestHeaders&, void*&)
#define GetFirstHttpRequestHeader GETZIP_API(GetFirstHttpRequestHeader)

LIBNETXT_API_DEF_1(GetZip, GetHttpRequestHeaderName, const std::string&, void*&)
#define GetHttpRequestHeaderName GETZIP_API(GetHttpRequestHeaderName)

LIBNETXT_API_DEF_1(GetZip, GetHttpRequestHeaderValue, const std::string&, void*&)
#define GetHttpRequestHeaderValue GETZIP_API(GetHttpRequestHeaderValue)

LIBNETXT_API_DEF_3(GetZip, GetHttpRequestHeaderByValue, bool, net::HttpRequestHeaders&,
    const std::string&, std::string*)
#define GetHttpRequestHeaderByValue GETZIP_API(GetHttpRequestHeaderByValue)

LIBNETXT_API_DEF_1(GetZip, GetNextHttpRequestHeader, void, void*&)
#define GetNextHttpRequestHeader GETZIP_API(GetNextHttpRequestHeader)

LIBNETXT_API_DEF_3(GetZip, SetHttpRequestHeader, void, net::HttpRequestHeaders&,
    const std::string&, const std::string&)
#define SetHttpRequestHeader GETZIP_API(SetHttpRequestHeader)

LIBNETXT_API_DEF_2(GetZip, RemoveHttpRequestHeader, void, net::HttpRequestHeaders&,
    const std::string&)
#define RemoveHttpRequestHeader GETZIP_API(RemoveHttpRequestHeader)

////HttpResponseHeader method delegates
LIBNETXT_API_DEF_2(GetZip, RemoveHttpResponseHeader, void, net::HttpResponseHeaders*, const std::string&)
#define RemoveHttpResponseHeader GETZIP_API(RemoveHttpResponseHeader)

LIBNETXT_API_DEF_2(GetZip, GetHttpResponseHeaderValue, std::string, net::HttpResponseHeaders*, const std::string&)
#define GetHttpResponseHeaderValue GETZIP_API(GetHttpResponseHeaderValue)

LIBNETXT_API_DEF_1(GetZip, GetHttpResponseCode, int, net::HttpResponseHeaders*)
#define GetHttpResponseCode GETZIP_API(GetHttpResponseCode)

LIBNETXT_API_DEF_2(GetZip, HasHttpResponseHeader, bool, net::HttpResponseHeaders*, const std::string&)
#define HasHttpResponseHeader GETZIP_API(HasHttpResponseHeader)

#endif /* HTTP_GETZIP_BRIDGE_H_ */
