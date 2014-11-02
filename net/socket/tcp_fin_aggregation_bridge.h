/**
 * Copyright (c) 2013-2014, The Linux Foundation. All rights reserved.
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
#ifndef TCP_FIN_AGGREGATION_BRIDGE_H_
#define TCP_FIN_AGGREGATION_BRIDGE_H_

// ================================ LibNetXt global ===================================
#define TCPFIN_API_PREFIX TCPFin
#define TCPFIN_API(name) \
    LIBNETXT_API_NAME(TCPFin, name)
#define TCPFIN_API_CPP(namesp, type, name) \
    LIBNETXT_API_CPP_NAME(TCPFin, namesp, type, name)
#define TCPFIN_API_CPP_CON(namesp, type) \
    LIBNETXT_API_CPP_CON_NAME(TCPFin, namesp, type)
#define TCPFIN_API_CPP_DES(namesp, type) \
    LIBNETXT_API_CPP_DES_NAME(TCPFin, namesp, type)


#include "net/libnetxt/plugin_api_def.h"
#include "net/socket/client_socket_pool_base.h"
#include "time.h"

namespace net {
namespace internal {
  class ClientSocketPoolBaseHelper;
  struct IdleSocket;
}
};
LIBNETXT_API_DEF_1(TCPFin, DecrementIdleCount, void, net::internal::ClientSocketPoolBaseHelper*)
LIBNETXT_API_DEF_2(TCPFin, RemoveGroup, void, net::internal::ClientSocketPoolBaseHelper*, const std::string&)
LIBNETXT_API_DEF_3(TCPFin, ShouldCleanup, bool, net::internal::IdleSocket*, base::Time, base::TimeDelta)
LIBNETXT_API_DEF_0(TCPFin, GetCurrentTime, base::Time)
LIBNETXT_API_DEF_1(TCPFin, GetGroupMap, net::internal::ClientSocketPoolBaseHelper::GroupMap , net::internal::ClientSocketPoolBaseHelper*)
LIBNETXT_API_DEF_1(TCPFin, IdleSocketCount, int , net::internal::ClientSocketPoolBaseHelper*)

#endif /* TCP_FIN_AGGREGATION_BRIDGE_H_ */
