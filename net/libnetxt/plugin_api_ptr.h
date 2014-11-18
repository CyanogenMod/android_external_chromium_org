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
#ifndef PLUGIN_API_PTR_H_
#define PLUGIN_API_PTR_H_

#define LIBNETXT_API_BY_PTR

#include <string>
#include "base/basictypes.h"
#include "net/http/http_network_transaction.h"
#include "net/libnetxt/libnetxt_base.h"
#include "net/libnetxt/plugin_api.h"

//Additional APIs
#include "net/stat_hub/stat_hub_net_plugin_ptr.h"
#include "net/socket/tcp_fin_aggregation_plugin_ptr.h"
#include "net/http/http_getzip_plugin_ptr.h"
#include "net/libsta/sta_libnetxt_plugin_api_ptr.h"

#define LIBNETXT_API_VERSION_MAJOR 1
#define LIBNETXT_API_VERSION_MINOR 3

#if defined( __GNUC__)
    #define LIBNETXT_API_COMP_VERSION GCC
#elif defined( __llvm__)
    #define LIBNETXT_API_COMP_VERSION LLVM
#elif defined( __clang__)
    #define LIBNETXT_API_COMP_VERSION CLANG
#endif

#define LIBNETXT_API_VERSION LIBNETXT_VAL_TO_STR(LIBNETXT_API_VERSION_MAJOR.LIBNETXT_API_VERSION_MINOR.LIBNETXT_API_COMP_VERSION)

class LibnetxtPluginApi : public StatHubLibnetxtPluginApi,
                          public GetZipLibnetxtPluginApi,
                          public Sta_plugin_API,
                          public TcpFinAggLibnetxtPluginApi {
public:
    LibnetxtPluginApi();
    ~LibnetxtPluginApi();

static LibnetxtPluginApi* GetInstance();

    // ================================ net::HttpRequestHeaders  ====================================
    LIBNETXT_API_CPP_PTR_DEF_CON_0(LibNetXt, net, HttpRequestHeaders)
    LIBNETXT_API_CPP_PTR_DEF_DES(LibNetXt, net, HttpRequestHeaders)
    LIBNETXT_API_CPP_PTR_DEF_1(LibNetXt, net, HttpRequestHeaders, AddHeaderFromString, void, const base::StringPiece& )
    LIBNETXT_API_CPP_PTR_DEF_2(LibNetXt, net, HttpRequestHeaders, GetHeader, bool, const std::string&, std::string*)
    LIBNETXT_API_CPP_PTR_DEF_2(LibNetXt, net, HttpRequestHeaders, SetHeader, void, const std::string&, std::string&)
    LIBNETXT_API_CPP_PTR_DEF_0(LibNetXt, net, HttpRequestHeaders, ToString, std::string)

    // ================================ net::HttpRequestInfo  ====================================
    LIBNETXT_API_CPP_PTR_DEF_CON_0(LibNetXt, net, HttpRequestInfo)
    LIBNETXT_API_CPP_PTR_DEF_DES(LibNetXt, net, HttpRequestInfo)
    LIBNETXT_API_CPP_PTR_DEF_0(LibNetXt, net, HttpRequestInfo, ToString, std::string*)
    LIBNETXT_API_CPP_PTR_DEF_2(LibNetXt, net, HttpRequestInfo ,GetHeader, std::string*, base::BasicStringPiece<std::string> const&, std::string*)

    // ================================ net::HttpResponseInfo  ====================================
    LIBNETXT_API_CPP_PTR_DEF_CON_0(LibNetXt, net, HttpResponseInfo)
    LIBNETXT_API_CPP_PTR_DEF_DES(LibNetXt, net, HttpResponseInfo)

    // ================================ net::HttpResponseHeaders  ====================================
    LIBNETXT_API_CPP_PTR_DEF_CON_1(LibNetXt, net, HttpResponseHeaders, std::string)
    LIBNETXT_API_CPP_PTR_DEF_DES(LibNetXt, net, HttpResponseHeaders)
    LIBNETXT_API_CPP_PTR_DEF_0(LibNetXt, net, HttpResponseHeaders, GetContentLength, int64)
    LIBNETXT_API_CPP_PTR_DEF_1(LibNetXt, net, HttpResponseHeaders, IsRedirect, bool, std::string*)
    LIBNETXT_API_CPP_PTR_DEF_1(LibNetXt, net, HttpResponseHeaders, ReplaceStatusLine, void, std::string const&)
    LIBNETXT_API_CPP_PTR_DEF_1(LibNetXt, net, HttpResponseHeaders, RemoveHeader, void, std::string const&)
    LIBNETXT_API_CPP_PTR_DEF_0(LibNetXt, net, HttpResponseHeaders, IsChunkEncoded, bool)
    LIBNETXT_API_CPP_PTR_DEF_2(LibNetXt, net, HttpResponseHeaders, HasHeaderValue,bool,const base::StringPiece&,const base::StringPiece&)
    LIBNETXT_API_CPP_PTR_DEF_1(LibNetXt, net, HttpResponseHeaders, HasHeader, bool, base::BasicStringPiece<std::string> const&)
    LIBNETXT_API_CPP_PTR_DEF_Const_2(LibNetXt, net, HttpResponseHeaders, GetHeader, int, std::string, const std::string)
    LIBNETXT_API_CPP_PTR_DEF_0(LibNetXt, net, HttpResponseHeaders, GetStatusLine, std::string)
    LIBNETXT_API_CPP_PTR_DEF_3(LibNetXt, net, HttpResponseHeaders, GetContentRange,bool,int64*,int64*,int64*)
    LIBNETXT_API_CPP_PTR_DEF_1(LibNetXt, net, HttpResponseHeaders, AddHeader, void,std::string)
    LIBNETXT_API_CPP_PTR_DEF_0(LibNetXt, net, HttpResponseHeaders, GetHttpVersion, net::HttpVersion)

    // ================================ net::HttpByteRange =================================
    LIBNETXT_API_CPP_PTR_DEF_CON_0(LibNetXt, net, HttpByteRange)
    LIBNETXT_API_CPP_PTR_DEF_DES(LibNetXt, net, HttpByteRange)
    LIBNETXT_API_CPP_PTR_DEF_0(LibNetXt, net, HttpByteRange, IsSuffixByteRange, bool)

    // ================================ net::HttpNetworkTransaction =================================
    LIBNETXT_API_CPP_PTR_DEF_CON_2(LibNetXt, net, HttpNetworkTransaction, net::RequestPriority, net::HttpNetworkSession* )
    LIBNETXT_API_CPP_PTR_DEF_DES(LibNetXt, net, HttpNetworkTransaction)
    LIBNETXT_API_CPP_PTR_DEF_0(LibNetXt, net, HttpNetworkTransaction, SetUseStaPool, void)

    // ================================ net::HttpCache ====================================
    LIBNETXT_API_CPP_PTR_DEF_0(LibNetXt, net, HttpCache, GetSession, net::HttpNetworkSession*)

    // ================================ ::GURL  ====================================
    LIBNETXT_API_CPP_PTR_DEF_CON_1(LibNetXt, , GURL, std::string&)
    LIBNETXT_API_CPP_PTR_DEF_DES(LibNetXt, , GURL)
    LIBNETXT_API_CPP_PTR_DEF_0(LibNetXt, , GURL, GetOrigin, GURL)
    LIBNETXT_API_CPP_PTR_DEF_1(LibNetXt, , GURL, Resolve, GURL, const std::string&)
    LIBNETXT_API_CPP_PTR_DEF_0(LibNetXt, , GURL, ExtractFileName, std::string)

    // ================================ net::HostPortPair ====================================
    LIBNETXT_API_CPP_PTR_DEF_0(LibNetXt, net, HostPortPair, ToString, std::string)

    // ================================ base::IOBufferWithSize ====================================
    LIBNETXT_API_CPP_PTR_DEF_CON_1(LibNetXt,net, IOBufferWithSize , int)

    // ================================ base::DependantIOBufferWithSize ====================================
    LIBNETXT_API_CPP_PTR_DEF_CON_0(LibNetXt,net, DependantIOBufferWithSize)

    // ================================ base::SystemMemoryInfoKB ====================================
    LIBNETXT_API_CPP_PTR_DEF_CON_0(LibNetXt, base, SystemMemoryInfoKB)
    LIBNETXT_API_CPP_PTR_DEF_DES(LibNetXt, base, SystemMemoryInfoKB)

    // ================================ logging::LogMessage ====================================
    LIBNETXT_API_CPP_PTR_DEF_CON_3(LibNetXt,logging, LogMessage, const char*,int, logging::LogSeverity)
    LIBNETXT_API_CPP_PTR_DEF_DES(LibNetXt, logging , LogMessage)

    // ================================ Common Interface ====================================
    LIBNETXT_API_PTR_DEF_1(LibNetXt, GetSystemMemoryInfo, bool, base::SystemMemoryInfoKB*)
    LIBNETXT_API_PTR_DEF_0(LibNetXt, GetSystemTime, base::Time)
    LIBNETXT_API_PTR_DEF_2(LibNetXt, GetTimeDeltaInMs, int, const base::Time&, const base::Time&)
    LIBNETXT_API_PTR_DEF_2(LibNetXt, GetHostFromUrl, const char*, const std::string& , std::string&)
    LIBNETXT_API_PTR_DEF_2(LibNetXt, GetHostPortFromUrl, const char*, const std::string&, std::string&)
    LIBNETXT_API_PTR_DEF_0(LibNetXt, GetMaxSocketsPerGroup, int)
    LIBNETXT_API_PTR_DEF_3(LibNetXt, SysPropertyGet, int, const char*, char* , const char*)
    LIBNETXT_API_PTR_DEF_1(LibNetXt, DebugLog, int, const char*)
    LIBNETXT_API_PTR_DEF_3(LibNetXt, NetPreconnect, void, net::HttpNetworkSession*, GURL const&, int)
    LIBNETXT_API_PTR_DEF_3(LibNetXt, ParseHostAndPort, void, const GURL&, std::string *, int*)
    LIBNETXT_API_PTR_DEF_3(LibNetXt, GetRequestRange, bool,const net::HttpRequestHeaders& , int64& , int64& )
    LIBNETXT_API_PTR_DEF_2(LibNetXt, ParseRangeHeader,bool,const std::string& , std::vector<net::HttpByteRange>*)
    LIBNETXT_API_PTR_DEF_2(LibNetXt, AssembleRawHeadersAndAssign, void,  std::string, sta::ResourceRequest* )
    LIBNETXT_API_PTR_DEF_2(LibNetXt, HttpByteRangeToString ,std::string , int64, int64)
    LIBNETXT_API_PTR_DEF_1(LibNetXt, GetResponseHeaderLines, std::string, const net::HttpResponseHeaders&)
    LIBNETXT_API_PTR_DEF_1(LibNetXt, PathExists, bool, const std::string&)
    LIBNETXT_API_PTR_DEF_1(LibNetXt, GetUrlOriginSpec, const char*, const GURL&)

    LIBNETXT_API_CPP_PTR_DEF_SP_CON_0(LibNetXt,net,HttpResponseHeaders)
    LIBNETXT_API_CPP_PTR_DEF_SP_DES(LibNetXt,net,HttpResponseHeaders)
    LIBNETXT_API_PTR_DEF_2(LibNetXt, AssignHttpResponseHeaders, void, scoped_refptr<net::HttpResponseHeaders>*, const net::HttpResponseHeaders*)
    LIBNETXT_API_PTR_DEF_1(LibNetXt, PostTask, void, const base::Closure&)
    LIBNETXT_API_PTR_DEF_1(LibNetXt, ConvertHeadersBackToHTTPResponse, std::string, const std::string&)

    // ================================ Libnetxt API version 1.3 ====================================

    // ================================ Common Interface ====================================
    LIBNETXT_API_PTR_DEF_0(LibNetXt, IsVerboseEnabled, bool)
    LIBNETXT_API_PTR_DEF_0(LibNetXt, GetVerboseLevel, LibnetxtVerboseLevel)

    // ================================ net::HttpResponseHeaders  ====================================
    LIBNETXT_API_CPP_PTR_DEF_3(LibNetXt, net, HttpResponseHeaders, EnumerateHeader, bool, void**, const std::string&, std::string*)
    LIBNETXT_API_CPP_PTR_DEF_0(LibNetXt, net, HttpResponseHeaders, response_code, int)
};

 void sta_assign(scoped_refptr<net::HttpResponseHeaders>* dest , const net::HttpResponseHeaders*  src) __attribute__ ((visibility ("default"), used));

#endif /* PLUGIN_API_PTR_H_ */
