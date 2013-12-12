/*
 * Copyright (C) 2013-2014, The Linux Foundation. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *   * Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *   * Redistributions in binary form must reproduce the above
 *     copyright notice, this list of conditions and the following
 *     disclaimer in the documentation and/or other materials provided
 *     with the distribution.
 *   * Neither the name of The Linux Foundation nor the names of its
 *     contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission.
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
 *
 */
#include "base/compiler_specific.h"
#include "build/build_config.h"

#include "net/libnetxt/dyn_lib_loader.h"
#include "net/socket/client_socket_pool_base.h"
#include "net/socket/client_socket_pool_manager.h"
#include "net/socket/adaptive_connectivity_bridge.h"

namespace adaptive_connectivity {

static void (*DoObserveGroupCreation)(net::internal::ClientSocketPoolBaseHelper*) = NULL;
static void (*DoObserveGroupRemoval)(net::internal::ClientSocketPoolBaseHelper*) = NULL;
static void (*DoInitOnce)(int) = NULL;

static void InitOnce() {
  static bool initialized = false;
  if (!initialized) {
      initialized = true;
      void* fh = LibraryManager::GetLibraryHandle("libadaptive_connectivity_plugin");
      if (fh) {
          *(void **)(&DoObserveGroupCreation) = LibraryManager::GetLibrarySymbol(fh, "DoObserveGroupCreation", false);
          *(void **)(&DoObserveGroupRemoval) = LibraryManager::GetLibrarySymbol(fh, "DoObserveGroupRemoval", false);
          *(void **)(&DoInitOnce) = LibraryManager::GetLibrarySymbol(fh, "DoInitOnce", false);
          if (DoInitOnce) {
              DoInitOnce(net::ClientSocketPoolManager::max_sockets_per_group(net::HttpNetworkSession::NORMAL_SOCKET_POOL));
          }
      }
  }
}

void ObserveGroupCreation(net::internal::ClientSocketPoolBaseHelper* pool_base_helper) {
    InitOnce();
    if (DoObserveGroupCreation) {
        DoObserveGroupCreation(pool_base_helper);
    }
}

void ObserveGroupRemoval(net::internal::ClientSocketPoolBaseHelper* pool_base_helper) {
    InitOnce();
    if (DoObserveGroupRemoval) {
        DoObserveGroupRemoval(pool_base_helper);
    }
}

bool IsAdaptiveConnectivity(net::HttpNetworkSession::SocketPoolType pool_type)
{
    return (pool_type == net::HttpNetworkSession::NORMAL_SOCKET_POOL);
}
}//namespace adaptive_connectivity
