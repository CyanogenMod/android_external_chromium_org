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

#include "net/http/http_response_headers.h"
#include "net/http/http_request_headers.h"
#include "net/http/http_util.h"
#include "base/strings/stringprintf.h"
#include "net/http/http_byte_range.h"
#include "net/libsta/common/interfaces/external_types.h"

#include "utils.h"

const char* net::STARequestMetaData::USER_DATA_KEY  = "sta_data";

namespace sta{

const std::string HttpByteRangeToString(int a, int b){
    static const int NaN = -1;
    if(a == NaN) return base::StringPrintf("Range: bytes=-%d", b);
    if(b == NaN) return base::StringPrintf("Range: bytes=%d-", a);
    return base::StringPrintf("Range: bytes=%d-%d", a,b);
}

std::string GetResponseHeaderLines(const net::HttpResponseHeaders& headers) {
  std::string raw_headers = headers.raw_headers();
  const char* null_separated_headers = raw_headers.c_str();
  const char* header_line = null_separated_headers;
  std::string cr_separated_headers;
  while (header_line[0] != 0) {
    cr_separated_headers += header_line;
    cr_separated_headers += "\n";
    header_line += strlen(header_line) + 1;
  }
  return cr_separated_headers;
}

// Look for Range request header
//return value - false if we the req-range is not in the "bytes: 199-500" form
bool GetRequestRange(const net::HttpRequestHeaders& req_headers, int64& req_range_start, int64& req_range_end){

    std::string range_header;
    net::HttpByteRange byte_range;

    if (!req_headers.GetHeader(net::HttpRequestHeaders::kRange, &range_header))
        return false;
    std::vector<net::HttpByteRange> ranges;
    if (!net::HttpUtil::ParseRangeHeader(range_header, &ranges))
        return false;
    if (!(ranges.size() == 1))
        return false;
    byte_range = ranges[0];
    req_range_start = byte_range.first_byte_position();
    req_range_end = byte_range.last_byte_position();

    if (byte_range.IsSuffixByteRange()) { // Example: "bytes:-500"
        req_range_start = 0;
        req_range_end = byte_range.suffix_length();
    } else if (req_range_end == -1)
        return false;
    return true;  // Example: "bytes: 199-500"
}
}
