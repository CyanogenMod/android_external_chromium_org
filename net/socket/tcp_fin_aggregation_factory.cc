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

#include "net/socket/tcp_fin_aggregation_factory.h"
#include "net/socket/tcp_fin_aggregation_bridge.h"
#include "net/libnetxt/libnetxt_base.h"
#include "net/libnetxt/dyn_lib_loader.h"
#include "net/stat_hub/stat_hub.h"

namespace net {

static ITCPFinAggregation* (*tcpfin_create_)(internal::ClientSocketPoolBaseHelper* pool_base_helper) = NULL;

TCPFinAggregationFactory* TCPFinAggregationFactory::s_pFactory = NULL;

TCPFinAggregationFactory* TCPFinAggregationFactory::GetTCPFinFactoryInstance(internal::ClientSocketPoolBaseHelper* pool_base_helper) {
  CR_DEFINE_STATIC_LOCAL(base::Lock, mutex, ());
  base::AutoLock myLock(mutex);
  if(s_pFactory == NULL) {
    s_pFactory = new TCPFinAggregationFactory(pool_base_helper);
  }
  return s_pFactory;
}

TCPFinAggregationFactory::TCPFinAggregationFactory(internal::ClientSocketPoolBaseHelper* pool_base_helper):m_pTCPFin(NULL) {
  InitTCPFinAggregation(pool_base_helper);
}

void TCPFinAggregationFactory::InitTCPFinAggregation(internal::ClientSocketPoolBaseHelper* pool_base_helper) {
  void* libHandle = LibraryManager::GetLibraryHandle("libtcp_fin_aggregation_plugin");
  TCPFIN_API(GetCurrentTime)();
  if(libHandle) {
    *(void **)(&tcpfin_create_) = LibraryManager::GetLibrarySymbol(libHandle, "createTCPFinAggregation", false);
    if(tcpfin_create_) {
      m_pTCPFin = tcpfin_create_(pool_base_helper);
      return;
    }
  }
}
}; // namespace net
