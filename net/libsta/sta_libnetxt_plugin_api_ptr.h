/*
 * Copyright (c) 2014, The Linux Foundation. All rights reserved.
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

#ifndef STA_PLUGIN_PTR_H_
#define STA_PLUGIN_PTR_H_

#include "net/libsta/common/interfaces/interface_types.h"
#include "net/libsta/common/interfaces/transport_service.h"
#include "net/libsta/common/utils/tw_helper.h"
#include "net/libsta/common/utils/tp_helper.h"

#define STA_API(name) LIBNETXT_API_NAME(STA, name)

#define STA_API_CPP(type, name)  LIBNETXT_API_CPP_NAME(STA, sta, type, name)

class Sta_plugin_API {
public:

    LIBNETXT_API_CPP_PTR_DEF_CON_2(STA, net, HttpNetworkTransaction,  net::RequestPriority, net::HttpNetworkSession*)

    //
    // ===== TransportService =====
    //

    // get the (sole) instance of TransportService using a C function
    LIBNETXT_API_PTR_DEF_0(STA, getTransportServiceInstance, sta::TransportService*)

    LIBNETXT_API_CPP_PTR_DEF_1(STA, sta, TransportService, set_accelerator_prefs, void,
            const sta::AcceleratorPreferences&)

    LIBNETXT_API_CPP_PTR_DEF_4(STA,sta, TransportService, GetTransportPoolStats, bool,
            net::HttpNetworkSession::SocketPoolType ,
            net::HttpNetworkSession* ,
            const net::HostPortPair ,
            sta::TransportPoolStats&)

    LIBNETXT_API_CPP_PTR_DEF_1(STA, sta, TransportService,GetNetStackProperties, void,
            sta::NetStackProperties&)

    LIBNETXT_API_CPP_PTR_DEF_2(STA, sta, TransportService,NotifyAcceleratorEvent, void,
            const sta::AcceleratorEventType,
            const std::string )

    LIBNETXT_API_CPP_PTR_DEF_3(STA, sta, TransportService,StartTimer, base::Timer*,
              const sta::TimerType,
              sta::TimerCallback&,
              int )
    LIBNETXT_API_CPP_PTR_DEF_1(STA, sta, TransportService, ResetTimer, void,
                       base::Timer* const)

    LIBNETXT_API_CPP_PTR_DEF_1(STA, sta, TransportService, StopTimer, void,
                  base::Timer* const)

    LIBNETXT_API_CPP_PTR_DEF_2(STA, sta, TransportService, GetAcceleratorNetworkSession, net::HttpNetworkSession*,
           net::HttpNetworkSession*,
           const sta::NetworkSessionParams&)

    LIBNETXT_API_CPP_PTR_DEF_0(STA, sta, TransportService, GetAccelerationService, sta::AccelerationService*)

    LIBNETXT_API_CPP_PTR_DEF_1(STA, sta, TransportService, RegisterAccelerationService, void,
            sta::AccelerationService*)

    LIBNETXT_API_CPP_PTR_DEF_4(STA, sta, TransportService, GetPoolLimits, void,
            net::HttpNetworkSession*,
            net::HttpNetworkSession::SocketPoolType,
            int&,
            int& )

    LIBNETXT_API_CPP_PTR_DEF_0(STA, sta, TransportService, ReleaseAcceleratorResources, void)

    //
    // ===== TwrapperHelper =====
    //
    LIBNETXT_API_CPP_PTR_DEF_CON_2(STA, sta, TwrapperHelper,  net::HttpNetworkTransaction* , sta::SimpleCallback*)
    LIBNETXT_API_CPP_PTR_DEF_DES(STA, sta, TwrapperHelper)
    LIBNETXT_API_CPP_PTR_DEF_2(STA, sta, TwrapperHelper, ConnectReadBuffer, void, int, int)
    LIBNETXT_API_CPP_PTR_DEF_1(STA, sta, TwrapperHelper, Read, int,int)
    LIBNETXT_API_CPP_PTR_DEF_0(STA, sta, TwrapperHelper, CreateReadBuffer, void)
    LIBNETXT_API_CPP_PTR_DEF_1(STA, sta, TwrapperHelper, Start, int,const net::BoundNetLog&)
    LIBNETXT_API_CPP_PTR_DEF_1(STA, sta, TwrapperHelper, PostCommand, void, sta::TwrapperHelper::Command)
    LIBNETXT_API_CPP_PTR_DEF_3(STA, sta, TwrapperHelper, CopyToBuff, void, char* , int , int )
    LIBNETXT_API_CPP_PTR_DEF_0(STA, sta, TwrapperHelper, ShutdownNetTransaction, void)
    LIBNETXT_API_CPP_PTR_DEF_0(STA, sta, TwrapperHelper, NetTransaction, net::HttpNetworkTransaction*)
    LIBNETXT_API_CPP_PTR_DEF_1(STA, sta, TwrapperHelper, setRequestInfo, void, net::HttpRequestInfo*)
    LIBNETXT_API_CPP_PTR_DEF_0(STA, sta, TwrapperHelper, RequestInfo, net::HttpRequestInfo*)
    LIBNETXT_API_CPP_PTR_DEF_0(STA, sta, TwrapperHelper, local_body_buff_size, int)
    LIBNETXT_API_CPP_PTR_DEF_0(STA, sta, TwrapperHelper, dependant_read_buffer_size, int)
    LIBNETXT_API_CPP_PTR_DEF_0(STA, sta, TwrapperHelper, GetTotalReceivedBytes, uint64)

    //
    // ===== TProcHelper =====
    //
    LIBNETXT_API_CPP_PTR_DEF_CON_1(STA, sta, TProcHelper,  sta::SimplTprocCallback*)
    LIBNETXT_API_CPP_PTR_DEF_DES(STA, sta, TProcHelper)
    LIBNETXT_API_CPP_PTR_DEF_3(STA, sta, TProcHelper, StartBypassTransaction, int, net::HttpNetworkTransaction*, const net::HttpRequestInfo*,const net::BoundNetLog& )
    LIBNETXT_API_CPP_PTR_DEF_1(STA, sta, TProcHelper, PostCommand, void, sta::TProcHelper::Command)
    LIBNETXT_API_CPP_PTR_DEF_0(STA, sta, TProcHelper, getNetTransaction,net::HttpNetworkTransaction*)
    LIBNETXT_API_CPP_PTR_DEF_1(STA, sta, TProcHelper, setOuterStartCompletedCb ,void, const net::CompletionCallback&)
    LIBNETXT_API_CPP_PTR_DEF_0(STA, sta, TProcHelper, getOuterStartCompletedCb,net::CompletionCallback*)
    LIBNETXT_API_CPP_PTR_DEF_0(STA, sta, TProcHelper, getLastReadCb,net::CompletionCallback*)
    LIBNETXT_API_CPP_PTR_DEF_1(STA, sta, TProcHelper, setLastReadBuffer ,void, net::IOBuffer*)
    LIBNETXT_API_CPP_PTR_DEF_0(STA, sta, TProcHelper, getLastReadBuffer,net::IOBuffer*)
    LIBNETXT_API_CPP_PTR_DEF_1(STA, sta, TProcHelper, SetBeforeNetworkStartCallback,void, const net::HttpTransaction::BeforeNetworkStartCallback&)
};

class LibnetxtPluginApi;
void InitStaLibnetxtPluginApi(LibnetxtPluginApi* plugin_api);

#endif
