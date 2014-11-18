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
#ifndef PLUGIN_API_H_
#define PLUGIN_API_H_

#include <string>
#include <vector>
#include "base/basictypes.h"
#include "net/libnetxt/libnetxt_base.h"
#include "net/libnetxt/plugin_api_def.h"
#include "net/base/request_priority.h"
#include "base/strings/string_piece.h"
#include "base/memory/ref_counted.h"
#include "base/callback.h"

namespace logging {
    typedef int LogSeverity;
}

namespace base {
    class Time;
    struct SystemMemoryInfoKB;
}

namespace net {
    class HostPortPair;
    class HttpRequestHeaders;
    struct HttpRequestInfo;
    class HttpResponseHeaders;
    class HttpResponseInfo;
    class HttpNetworkSession;
    class HttpNetworkTransaction;
    class HttpCache;
    class HttpVersion;
    class HttpByteRange;
    class IOBufferWithSize;
}

namespace logging{
    class LogMessage;
}

namespace sta{
class ResourceRequest;
}

class GURL;

// ================================ net::HttpRequestHeaders  ====================================
LIBNETXT_API_CPP_DEF_CON_0(LibNetXt, net, HttpRequestHeaders)
LIBNETXT_API_CPP_DEF_DES(LibNetXt, net, HttpRequestHeaders)
LIBNETXT_API_CPP_DEF_1(LibNetXt, net, HttpRequestHeaders, AddHeadersFromString, void, const std::string&)
LIBNETXT_API_CPP_DEF_2(LibNetXt, net, HttpRequestHeaders, GetHeader, bool, const std::string&, std::string*)
LIBNETXT_API_CPP_DEF_2(LibNetXt, net, HttpRequestHeaders, SetHeader, void, const std::string&, std::string&)
LIBNETXT_API_CPP_DEF_0(LibNetXt, net, HttpRequestHeaders, ToString, std::string)
LIBNETXT_API_CPP_DEF_1(LibNetXt, net, HttpRequestHeaders, AddHeaderFromString, void , const base::StringPiece&)

// ================================ net::HttpRequestInfo  ====================================
LIBNETXT_API_CPP_DEF_CON_0(LibNetXt, net, HttpRequestInfo)

LIBNETXT_API_CPP_DEF_DES(LibNetXt, net, HttpRequestInfo)

// ================================ net::HttpResponseInfo  ====================================
LIBNETXT_API_CPP_DEF_CON_0(LibNetXt, net, HttpResponseInfo)
LIBNETXT_API_CPP_DEF_DES(LibNetXt, net, HttpResponseInfo)

// ================================ net::HttpResponseHeaders  ====================================
LIBNETXT_API_CPP_DEF_CON_1(LibNetXt, net, HttpResponseHeaders, std::string)
LIBNETXT_API_CPP_DEF_0(LibNetXt, net, HttpResponseHeaders, GetContentLength, int64)
LIBNETXT_API_CPP_DEF_0(LibNetXt, net, HttpResponseHeaders, GetHttpVersion, net::HttpVersion)
LIBNETXT_API_CPP_DEF_1(LibNetXt, net, HttpResponseHeaders, IsRedirect, bool, std::string*)
LIBNETXT_API_CPP_DEF_1(LibNetXt, net, HttpResponseHeaders, ReplaceStatusLine, void, std::string const&)

LIBNETXT_API_CPP_DEF_1(LibNetXt, net, HttpResponseHeaders, RemoveHeader, void, std::string const&)
LIBNETXT_API_CPP_DEF_0(LibNetXt, net, HttpResponseHeaders, IsChunkEncoded, bool)
LIBNETXT_API_CPP_DEF_2(LibNetXt, net, HttpResponseHeaders, HasHeaderValue,bool,const base::StringPiece&,const base::StringPiece&)
LIBNETXT_API_CPP_DEF_1(LibNetXt, net, HttpResponseHeaders, HasHeader, bool, base::BasicStringPiece<std::string> const&)
LIBNETXT_API_CPP_DEF_0(LibNetXt, net, HttpResponseHeaders, GetStatusLine, std::string)
LIBNETXT_API_CPP_DEF_3(LibNetXt, net, HttpResponseHeaders, GetContentRange,bool,int64*,int64*,int64*)
LIBNETXT_API_CPP_DEF_1(LibNetXt, net, HttpResponseHeaders, AddHeader, void, std::string)
LIBNETXT_API_CPP_DEF_0(LibNetXt, net, HttpResponseHeaders, GetHttpVersion, net::HttpVersion)
LIBNETXT_API_CPP_DEF_3(LibNetXt, net, HttpResponseHeaders, EnumerateHeader, bool, void**, const std::string&, std::string*)
LIBNETXT_API_CPP_DEF_0(LibNetXt, net, HttpResponseHeaders, response_code, int)
extern scoped_refptr<net::HttpResponseHeaders>*  LibNetXtscoped_refptr_netHttpResponseHeadersconstructor()  __attribute__ ((visibility ("default"), used));
extern void LibNetXtscoped_refptr_netHttpResponseHeadersdestructor(scoped_refptr<net::HttpResponseHeaders>* p)  __attribute__ ((visibility ("default"), used));
LIBNETXT_API_DEF_2(LibNetXt, AssignHttpResponseHeaders, void, scoped_refptr<net::HttpResponseHeaders>*, const net::HttpResponseHeaders*)

// ================================ net::HttpByteRange =================================
 LIBNETXT_API_CPP_DEF_CON_0(LibNetXt, net, HttpByteRange)
 LIBNETXT_API_CPP_DEF_DES(LibNetXt, net, HttpByteRange)
 LIBNETXT_API_CPP_DEF_0(LibNetXt, net, HttpByteRange, IsSuffixByteRange,bool)

// ================================ net::HttpNetworkTransaction =================================
LIBNETXT_API_CPP_DEF_CON_2(LibNetXt, net, HttpNetworkTransaction, net::RequestPriority, net::HttpNetworkSession* )
LIBNETXT_API_CPP_DEF_DES(LibNetXt, net, HttpNetworkTransaction)
LIBNETXT_API_CPP_DEF_0(LibNetXt, net, HttpNetworkTransaction, SetUseStaPool, void)

// ================================ net::HttpCache ====================================
LIBNETXT_API_CPP_DEF_0(LibNetXt, net, HttpCache, GetSession, net::HttpNetworkSession*)

// ================================ ::GURL  ====================================
LIBNETXT_API_CPP_DEF_CON_1(LibNetXt, , GURL, std::string&)
LIBNETXT_API_CPP_DEF_DES(LibNetXt, , GURL)
LIBNETXT_API_CPP_DEF_0(LibNetXt, , GURL, GetOrigin, GURL)
LIBNETXT_API_CPP_DEF_1(LibNetXt, , GURL, Resolve, GURL, const std::string&)
LIBNETXT_API_CPP_DEF_0(LibNetXt, , GURL, ExtractFileName, std::string)

// ================================ net::HostPortPair ====================================
LIBNETXT_API_CPP_DEF_0(LibNetXt,net, HostPortPair, ToString, std::string)

// ================================ base::IOBufferWithSize ====================================
LIBNETXT_API_CPP_DEF_CON_1(LibNetXt,net, IOBufferWithSize , int)

// ================================ base::SystemMemoryInfoKB ====================================
LIBNETXT_API_CPP_DEF_CON_0(LibNetXt, base, SystemMemoryInfoKB)
LIBNETXT_API_CPP_DEF_DES(LibNetXt, base, SystemMemoryInfoKB)

// ================================ logging::LogMessage ====================================
LIBNETXT_API_CPP_DEF_CON_3(LibNetXt,logging, LogMessage, const char*,int, logging::LogSeverity)
LIBNETXT_API_CPP_DEF_DES(LibNetXt, logging, LogMessage)

// ================================ Common Interface ====================================
LIBNETXT_API_DEF_1(LibNetXt, GetSystemMemoryInfo, bool, base::SystemMemoryInfoKB*)
LIBNETXT_API_DEF_0(LibNetXt, GetSystemTime, base::Time)
LIBNETXT_API_DEF_2(LibNetXt, GetTimeDeltaInMs, int, const base::Time&, const base::Time&)
LIBNETXT_API_DEF_2(LibNetXt, GetHostFromUrl, const char*, const std::string& , std::string&)
LIBNETXT_API_DEF_2(LibNetXt, GetHostPortFromUrl, const char*, const std::string&, std::string&)
LIBNETXT_API_DEF_0(LibNetXt, GetMaxSocketsPerGroup, int)
LIBNETXT_API_DEF_3(LibNetXt, SysPropertyGet, int, const char*, char* , const char*)
LIBNETXT_API_DEF_1(LibNetXt, DebugLog, int, const char*)
LIBNETXT_API_DEF_3(LibNetXt, NetPreconnect, void, net::HttpNetworkSession*, GURL const&, int)
LIBNETXT_API_DEF_3(LibNetXt, ParseHostAndPort, void, const ::GURL&, std::string *, int*)
LIBNETXT_API_DEF_3(LibNetXt, GetRequestRange, bool,const net::HttpRequestHeaders& , int64& , int64&)
LIBNETXT_API_DEF_2(LibNetXt, ParseRangeHeader,bool,const std::string& , std::vector<net::HttpByteRange>*)
LIBNETXT_API_DEF_2(LibNetXt, AssembleRawHeadersAndAssign, void,  std::string, sta::ResourceRequest*  )
LIBNETXT_API_DEF_2(LibNetXt, HttpByteRangeToString ,std::string ,  int64, int64)
LIBNETXT_API_DEF_1(LibNetXt, GetResponseHeaderLines, std::string, const net::HttpResponseHeaders&)
LIBNETXT_API_DEF_1(LibNetXt, PathExists, bool, const std::string&)
LIBNETXT_API_DEF_1(LibNetXt, GetUrlOriginSpec, const char*, const GURL&)
LIBNETXT_API_DEF_1(LibNetXt, PostTask, void, const base::Closure&)
LIBNETXT_API_DEF_1(LibNetXt, ConvertHeadersBackToHTTPResponse, std::string, const std::string&)
LIBNETXT_API_DEF_0(LibNetXt, IsVerboseEnabled, bool)
LIBNETXT_API_DEF_0(LibNetXt, GetVerboseLevel, LibnetxtVerboseLevel)

#endif /* PLUGIN_API_H_ */
