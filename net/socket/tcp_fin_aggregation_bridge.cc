/**
 * Copyright (c) 2013-2014 Linux Foundation. All rights reserved.
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

#include "net/socket/tcp_fin_aggregation_bridge.h"
#include "net/libnetxt/libnetxt_base.h"

void TCPFIN_API(DecrementIdleCount)(net::internal::ClientSocketPoolBaseHelper* pool_base_helper) {
  if (NULL != pool_base_helper) {
    pool_base_helper->DecrementIdleCount();
  }
}

void TCPFIN_API(RemoveGroup)(net::internal::ClientSocketPoolBaseHelper* pool_base_helper, const std::string& group_name) {
  if (NULL != pool_base_helper) {
    pool_base_helper->RemoveGroup(group_name);
  }
}

bool TCPFIN_API(ShouldCleanup)(net::internal::IdleSocket* idle_socket, base::Time now, base::TimeDelta timeout) {
  if (NULL != idle_socket) {
    return idle_socket->ShouldCleanup(now, timeout);
  }
  return false;
}

base::Time TCPFIN_API(GetCurrentTime)() {
  return base::Time::Now();
}

net::internal::ClientSocketPoolBaseHelper::GroupMap TCPFIN_API(GetGroupMap)(net::internal::ClientSocketPoolBaseHelper* pool_base_helper) {
  return pool_base_helper->group_map_;
}

int TCPFIN_API(IdleSocketCount)(net::internal::ClientSocketPoolBaseHelper* pool_base_helper) {
  if (NULL != pool_base_helper) {
    return pool_base_helper->idle_socket_count();
  }
  LIBNETXT_LOGE("netstack: IdleSocketCount called in TCPFIN_API, but pool is NULL.");
  return 0;
}
