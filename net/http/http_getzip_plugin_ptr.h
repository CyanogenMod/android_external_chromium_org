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
#ifndef HTTP_GETZIP_PLUGIN_PTR_H_
#define HTTP_GETZIP_PLUGIN_PTR_H_

#include "net/http/http_getzip_bridge.h"

class LibnetxtPluginApi;

class GetZipLibnetxtPluginApi {
public:
    LIBNETXT_API_PTR_DEF_2(GetZip, GetFirstHttpRequestHeader, void, net::HttpRequestHeaders&, void*&)
    LIBNETXT_API_PTR_DEF_1(GetZip, GetHttpRequestHeaderName, const std::string&, void*&)
    LIBNETXT_API_PTR_DEF_1(GetZip, GetHttpRequestHeaderValue, const std::string&, void*&)
    LIBNETXT_API_PTR_DEF_3(GetZip, GetHttpRequestHeaderByValue, bool, net::HttpRequestHeaders&,
        const std::string&, std::string*)
    LIBNETXT_API_PTR_DEF_1(GetZip, GetNextHttpRequestHeader, void, void*&)
    LIBNETXT_API_PTR_DEF_3(GetZip, SetHttpRequestHeader, void, net::HttpRequestHeaders&,
        const std::string&, const std::string&)
    LIBNETXT_API_PTR_DEF_2(GetZip, RemoveHttpRequestHeader, void, net::HttpRequestHeaders&,
        const std::string&)
    LIBNETXT_API_PTR_DEF_2(GetZip, RemoveHttpResponseHeader, void, net::HttpResponseHeaders*, const std::string&)
    LIBNETXT_API_PTR_DEF_2(GetZip, GetHttpResponseHeaderValue, std::string, net::HttpResponseHeaders*, const std::string&)
    LIBNETXT_API_PTR_DEF_1(GetZip, GetHttpResponseCode, int, net::HttpResponseHeaders*)
    LIBNETXT_API_PTR_DEF_2(GetZip, HasHttpResponseHeader, bool, net::HttpResponseHeaders*, const std::string&)
};

void InitGetZipLibnetxtPluginApi(LibnetxtPluginApi* plugin_api);

#endif /* HTTP_GETZIP_PLUGIN_PTR_H_ */
