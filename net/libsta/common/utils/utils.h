/*
Copyright (c) 2014, The Linux Foundation. All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are
met:
    * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above
      copyright notice, this list of conditions and the following
      disclaimer in the documentation and/or other materials provided
      with the distribution.
    * Neither the name of The Linux Foundation nor the names of its
      contributors may be used to endorse or promote products derived
      from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED "AS IS" AND ANY EXPRESS OR IMPLIED
WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT
ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS
BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef UTILS_H_
#define UTILS_H_

#include <string>
#include "base/basictypes.h"

namespace net{
    class HttpResponseHeaders;
    class HttpRequestHeaders;
    struct HttpRequestInfo;
}

// a helpful gcc attribute to force developers check ret val
#define __must_check            __attribute__((warn_unused_result))
#define __used                  __attribute__((__used__))

/// return true iff v in [a,b]
static bool __used inrange(int v, int a, int b) {return ((v >=a) &&(v <=b) ) ;}

#define SAFE_DELETE(p)  if(p){ delete p; p = NULL; }

namespace sta{

const std::string HttpByteRangeToString(int a, int b);

/// convert 'headers' to a string (delimited by \n) of headers
std::string GetResponseHeaderLines(const net::HttpResponseHeaders& headers);

bool GetRequestRange(const net::HttpRequestHeaders& req_headers, int64& req_range_start, int64& req_range_end);

} //namespace sta

#endif /* UTILS_H_ */
