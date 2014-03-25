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

#include "interface_types.h"
using namespace sta;
//==============================================================================
// OriginServerNetworkStats::OriginServerNetworkStats
//==============================================================================
OriginServerNetworkStats::OriginServerNetworkStats(const OriginServerNetworkStats& obj)
: origin_server_(obj.origin_server_),
  pending_request_count_(obj.pending_request_count_)
{
   for (int i=0; i < SOCKET_STATE_MAX; i++)
   {
      total_socket_counts_[i] = obj.total_socket_counts_[i];
   }
}

//==============================================================================
// OriginServerNetworkStats::OriginServerNetworkStats
//==============================================================================
OriginServerNetworkStats::OriginServerNetworkStats()
: pending_request_count_(0)
{
   memset(total_socket_counts_,0, sizeof(total_socket_counts_));
}

//==============================================================================
// OriginServerNetworkStats::OriginServerNetworkStats
//==============================================================================
OriginServerNetworkStats::OriginServerNetworkStats(const OriginServer& os)
: pending_request_count_(0)
{
    origin_server_.set_host(os.host());
    origin_server_.set_port(os.port());
    memset(total_socket_counts_,0, sizeof(total_socket_counts_));
}

