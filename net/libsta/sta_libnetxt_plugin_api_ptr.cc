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

#include "net/libnetxt/plugin_api_ptr.h"

namespace net{
class HttpNetworkTransaction;
}

// implementation of the bridge functions (from the mangled global function to the actual object)

//
// ==== TransportService ===
//

sta::TransportService*  STAgetTransportServiceInstance(){
  return sta::TransportService::getInstance();
}


// for each method in the class, do forwarding to the object (which is passed as first argument)
void STAstaTransportServiceset_accelerator_prefs(sta::TransportService* p, const sta::AcceleratorPreferences& prefs){
    p->set_accelerator_prefs(prefs);
}

LIBNETXT_API_CPP_FORWARDER_4(STA, sta, TransportService, GetTransportPoolStats, bool, net::HttpNetworkSession::SocketPoolType,  net::HttpNetworkSession*, const net::HostPortPair, sta::TransportPoolStats& )
LIBNETXT_API_CPP_FORWARDER_3(STA, sta, TransportService, StartTimer, base::Timer*, const sta::TimerType, sta::TimerCallback&, int)
LIBNETXT_API_CPP_FORWARDER_1V(STA, sta, TransportService, GetNetStackProperties, void, sta::NetStackProperties&)
LIBNETXT_API_CPP_FORWARDER_2V(STA, sta, TransportService, NotifyAcceleratorEvent, void, const sta::AcceleratorEventType,const std::string  )
LIBNETXT_API_CPP_FORWARDER_1V(STA, sta, TransportService, ResetTimer, void, base::Timer*)
LIBNETXT_API_CPP_FORWARDER_1V(STA, sta, TransportService, StopTimer,  void, base::Timer*)
LIBNETXT_API_CPP_FORWARDER_2(STA, sta, TransportService, GetAcceleratorNetworkSession, net::HttpNetworkSession*, net::HttpNetworkSession*,const sta::NetworkSessionParams&)
LIBNETXT_API_CPP_FORWARDER_0(STA, sta, TransportService, GetAccelerationService, sta::AccelerationService*)
LIBNETXT_API_CPP_FORWARDER_1V(STA, sta, TransportService, RegisterAccelerationService, void, sta::AccelerationService*)
LIBNETXT_API_CPP_FORWARDER_4V(STA, sta, TransportService, GetPoolLimits, void, net::HttpNetworkSession* , net::HttpNetworkSession::SocketPoolType , int& , int& )
LIBNETXT_API_CPP_FORWARDER_0V(STA, sta, TransportService, ReleaseAcceleratorResources, void)
//
// ===== TwrapperHelper =====
// NOTE Some of the functions are not needed since they are in the header file,
// so accessible from the caller side
LIBNETXT_API_CPP_FORWARDER_CON_2(STA, sta, TwrapperHelper,  net::HttpNetworkTransaction*, sta::SimpleCallback*)
LIBNETXT_API_CPP_FORWARDER_DES(STA, sta, TwrapperHelper)
LIBNETXT_API_CPP_FORWARDER_2V(STA, sta, TwrapperHelper, ConnectReadBuffer, void, int, int)
LIBNETXT_API_CPP_FORWARDER_1(STA, sta, TwrapperHelper, Read,int, int)
LIBNETXT_API_CPP_FORWARDER_0V(STA, sta, TwrapperHelper, CreateReadBuffer, void)
LIBNETXT_API_CPP_FORWARDER_1(STA, sta, TwrapperHelper, Start, int,const net::BoundNetLog&)
LIBNETXT_API_CPP_FORWARDER_1V(STA, sta, TwrapperHelper, PostCommand, void, sta::TwrapperHelper::Command)
LIBNETXT_API_CPP_FORWARDER_3V(STA, sta, TwrapperHelper, CopyToBuff, void, char* , int , int )
LIBNETXT_API_CPP_FORWARDER_0V(STA, sta, TwrapperHelper, ShutdownNetTransaction, void)
LIBNETXT_API_CPP_FORWARDER_0(STA, sta, TwrapperHelper, NetTransaction, net::HttpNetworkTransaction*)
LIBNETXT_API_CPP_FORWARDER_1V(STA, sta, TwrapperHelper, setRequestInfo, void, net::HttpRequestInfo*)
LIBNETXT_API_CPP_FORWARDER_0(STA, sta, TwrapperHelper, RequestInfo, net::HttpRequestInfo*)
LIBNETXT_API_CPP_FORWARDER_0(STA, sta, TwrapperHelper, dependant_read_buffer_size, int)
LIBNETXT_API_CPP_FORWARDER_0(STA, sta, TwrapperHelper, GetTotalReceivedBytes, uint64)

//
// ===== TProcHelper =====
//
LIBNETXT_API_CPP_FORWARDER_CON_1(STA, sta, TProcHelper,  sta::SimplTprocCallback*)
LIBNETXT_API_CPP_FORWARDER_DES(STA, sta, TProcHelper)
LIBNETXT_API_CPP_FORWARDER_3(STA, sta, TProcHelper, StartBypassTransaction, int, net::HttpNetworkTransaction*, const net::HttpRequestInfo*,const net::BoundNetLog& )

void  STAstaTProcHelperPostCommand(sta::TProcHelper* this_ptr, sta::TProcHelper::Command param1) \
        {this_ptr->PostCommand_from_opensource(param1);}

LIBNETXT_API_CPP_FORWARDER_1V(STA, sta, TProcHelper, setOuterStartCompletedCb ,void, const net::CompletionCallback&)
LIBNETXT_API_CPP_FORWARDER_0(STA, sta, TProcHelper, getOuterStartCompletedCb,net::CompletionCallback*)
LIBNETXT_API_CPP_FORWARDER_0(STA, sta, TProcHelper, getLastReadCb,net::CompletionCallback*)
LIBNETXT_API_CPP_FORWARDER_1V(STA, sta, TProcHelper, setLastReadBuffer ,void, net::IOBuffer*)
LIBNETXT_API_CPP_FORWARDER_0(STA, sta, TProcHelper, getLastReadBuffer,net::IOBuffer*)
LIBNETXT_API_CPP_FORWARDER_1V(STA, sta, TProcHelper, SetBeforeNetworkStartCallback,void, const net::HttpTransaction::BeforeNetworkStartCallback&)

void InitStaLibnetxtPluginApi(LibnetxtPluginApi* plugin_api) {
    //list here all the methods you will want to use from the PR side
    // This code assigns function pointers to the respective fields in the LibnetxtPluginApi object
    //||     plugin_api->Foo_func_ptr = global_func_ptr;

    // ==== TransportService ===
    LIBNETXT_API_PTR_IMP(plugin_api, STA, getTransportServiceInstance)

    LIBNETXT_API_CPP_PTR_IMP(plugin_api, STA, sta, TransportService, set_accelerator_prefs)
    LIBNETXT_API_CPP_PTR_IMP(plugin_api, STA, sta, TransportService, GetTransportPoolStats)
    LIBNETXT_API_CPP_PTR_IMP(plugin_api, STA, sta, TransportService, GetNetStackProperties)
    LIBNETXT_API_CPP_PTR_IMP(plugin_api, STA, sta, TransportService, NotifyAcceleratorEvent)

    LIBNETXT_API_CPP_PTR_IMP(plugin_api, STA, sta, TransportService, StartTimer)
    LIBNETXT_API_CPP_PTR_IMP(plugin_api, STA, sta, TransportService, ResetTimer)
    LIBNETXT_API_CPP_PTR_IMP(plugin_api, STA, sta, TransportService, StopTimer)

    LIBNETXT_API_CPP_PTR_IMP(plugin_api, STA, sta, TransportService, GetAcceleratorNetworkSession)
    LIBNETXT_API_CPP_PTR_IMP(plugin_api, STA, sta, TransportService, GetAccelerationService)
    LIBNETXT_API_CPP_PTR_IMP(plugin_api, STA, sta, TransportService, RegisterAccelerationService)
    LIBNETXT_API_CPP_PTR_IMP(plugin_api, STA, sta, TransportService, GetPoolLimits)
    LIBNETXT_API_CPP_PTR_IMP(plugin_api, STA, sta, TransportService, ReleaseAcceleratorResources)

     //
     // ===== TwrapperHelper =====
     //
     LIBNETXT_API_CPP_PTR_IMP_CON(plugin_api, STA, sta, TwrapperHelper)
     LIBNETXT_API_CPP_PTR_IMP_DES(plugin_api, STA, sta, TwrapperHelper)
     LIBNETXT_API_CPP_PTR_IMP(plugin_api, STA, sta, TwrapperHelper, ConnectReadBuffer)
     LIBNETXT_API_CPP_PTR_IMP(plugin_api, STA, sta, TwrapperHelper, Read)
     LIBNETXT_API_CPP_PTR_IMP(plugin_api, STA, sta, TwrapperHelper, CreateReadBuffer)
     LIBNETXT_API_CPP_PTR_IMP(plugin_api, STA, sta, TwrapperHelper, Start)
     LIBNETXT_API_CPP_PTR_IMP(plugin_api, STA, sta, TwrapperHelper, PostCommand)
     LIBNETXT_API_CPP_PTR_IMP(plugin_api, STA, sta, TwrapperHelper, CopyToBuff)
     LIBNETXT_API_CPP_PTR_IMP(plugin_api, STA, sta, TwrapperHelper, ShutdownNetTransaction)
     LIBNETXT_API_CPP_PTR_IMP(plugin_api, STA, sta, TwrapperHelper, NetTransaction)
     LIBNETXT_API_CPP_PTR_IMP(plugin_api, STA, sta, TwrapperHelper, setRequestInfo)
     LIBNETXT_API_CPP_PTR_IMP(plugin_api, STA, sta, TwrapperHelper, RequestInfo)
     LIBNETXT_API_CPP_PTR_IMP(plugin_api, STA, sta, TwrapperHelper, dependant_read_buffer_size)
     LIBNETXT_API_CPP_PTR_IMP(plugin_api, STA, sta, TwrapperHelper, GetTotalReceivedBytes)

     //
     // ===== TProcHelper =====
     //
     LIBNETXT_API_CPP_PTR_IMP_CON(plugin_api, STA, sta, TProcHelper)
     LIBNETXT_API_CPP_PTR_IMP_DES(plugin_api, STA, sta, TProcHelper)
     LIBNETXT_API_CPP_PTR_IMP(plugin_api, STA, sta, TProcHelper, StartBypassTransaction)
     LIBNETXT_API_CPP_PTR_IMP(plugin_api, STA, sta, TProcHelper, PostCommand)
     LIBNETXT_API_CPP_PTR_IMP(plugin_api, STA, sta, TProcHelper, setOuterStartCompletedCb)
     LIBNETXT_API_CPP_PTR_IMP(plugin_api, STA, sta, TProcHelper, getOuterStartCompletedCb)
     LIBNETXT_API_CPP_PTR_IMP(plugin_api, STA, sta, TProcHelper, getLastReadCb)
     LIBNETXT_API_CPP_PTR_IMP(plugin_api, STA, sta, TProcHelper, setLastReadBuffer)
     LIBNETXT_API_CPP_PTR_IMP(plugin_api, STA, sta, TProcHelper, getLastReadBuffer)
     LIBNETXT_API_CPP_PTR_IMP(plugin_api, STA, sta, TProcHelper, SetBeforeNetworkStartCallback)
     LIBNETXT_API_CPP_PTR_IMP(plugin_api, STA, sta, TProcHelper, SetBeforeNetworkStartCallback)
};
