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

#include "base/time/time.h"
#include "base/values.h" // for DictionaryValue class
#include "net/socket/transport_client_socket_pool.h"
#include "net/socket/client_socket_pool_base.h"
#include "net/socket/client_socket_pool_manager.h"
#include "net/proxy/proxy_service.h"
#include "net/libsta/common/interfaces/transport_service.h"
#include "net/libsta/common/utils/utils.h"

#include "base/logging.h"

#define LIBNETXT_API_BY_PTR
#include "net/libnetxt/plugin_api_ptr.h"

using namespace sta;
using namespace net;


// The singelton object
TransportService* TransportService::this_ = NULL;

TransportService::TransportService():
    acceleration_service_(NULL) ,
    set_accelerator_prefs_called_(false)
{
}

TransportService::~TransportService()
{
   if (!sessions_.empty())
   {
      sessions_.clear();
   }
}

TransportService* TransportService::getInstance(){
    if(this_ == NULL){ // safe because all actions are single threaded
        this_ = new TransportService();
    }
    return this_;
}

//==============================================================================
// TransportService::StartTimer
//==============================================================================
base::Timer* TransportService::StartTimer(const TimerType type, const TimerCallback& callback, const int timeout_ms)
{
    base::Timer* timer_handle = NULL;
    if (!base::MessageLoop::current())
         return NULL;

    switch (type) {
    case TIMER_TYPE_ONE_SHOT:
        timer_handle =  new base::Timer(true, false);
        break;
    case TIMER_TYPE_PERIODIC:
        timer_handle = new base::Timer(true, true);
        break;
    }

    timer_handle->Start( FROM_HERE, base::TimeDelta::FromMilliseconds(timeout_ms), callback);
    return timer_handle;
}

//==============================================================================
// TransportService::ResetTimer
//==============================================================================

void TransportService::ResetTimer(base::Timer* const timer_handle)
{
    timer_handle->Reset();
}

//==============================================================================
// TransportService::StopTimer
//==============================================================================

void TransportService::StopTimer(base::Timer* const timer_handle)
{
    delete timer_handle;
}

//==============================================================================
// TransportService::acceleration_service
//==============================================================================

AccelerationService*  TransportService::GetAccelerationService(void){
    DCHECK(acceleration_service_);
    return acceleration_service_;

}

//==============================================================================
// TransportService::RegisterAccelerationService
//==============================================================================

void TransportService::RegisterAccelerationService(AccelerationService* accel_svc){
    acceleration_service_ = accel_svc;
}


void TransportService::NotifyAcceleratorEvent(const AcceleratorEventType evt_type,
                                 const std::string evt_msg){

    DCHECK(evt_type == ACCELERATOR_EVT_ERROR_FATAL ) << "only this type is implemented";
    if (LIBNETXT_IS_VERBOSE) {
        LIBNETXT_LOGE("FATAL STA event: %s",evt_msg.c_str());
    }
}

//==============================================================================
//reset the state after printing the data.
void TransportService::ResetState(net::HttpNetworkSession::SocketPoolType socketPoolType, HttpNetworkSession* session){
    TransportClientSocketPool* socket_pool = session->GetTransportSocketPool(socketPoolType);
    socket_pool->set_was_changed(false);
}

void TransportService::PrintTransportPoolStats(net::HttpNetworkSession* session){
    TransportPoolStats stats1("RSP");
    net::HttpNetworkSession::SocketPoolType socketPoolType = net::HttpNetworkSession::NORMAL_SOCKET_POOL;
    PrintTransportPoolStats(socketPoolType, session, stats1);

    TransportPoolStats stats2("ASP");
    socketPoolType = net::HttpNetworkSession::NORMAL_SOCKET_STA_POOL;
    PrintTransportPoolStats(socketPoolType, session, stats2);
}
void TransportService::PrintTransportPoolStats(net::HttpNetworkSession::SocketPoolType socketPoolType,
        net::HttpNetworkSession* session,
        TransportPoolStats& stats){
    TransportClientSocketPool* socket_pool = session->GetTransportSocketPool(socketPoolType);
    if (!socket_pool->was_changed()){
        return;
    }
    ResetState(socketPoolType, session);

    if (LIBNETXT_IS_VERBOSE) {
        std::string rsp_msg_log;
        int open_connections_group = 0;
        std::ostringstream ss;
        std::vector<std::string> group_list;
        socket_pool->get_group_map(group_list);
        scoped_ptr<base::DictionaryValue> dict(socket_pool->GetInfoAsValue("transport_socket_pool", "transport_socket_pool", true));
        for (std::vector<string>::iterator it = group_list.begin();it != group_list.end(); ++it) {
            std::string host_name = *it;
            net::HostPortPair server = net::HostPortPair::FromString(host_name);
            if (!GetTransportPoolStats(socketPoolType,session,server,dict.get(), stats) )
                return;
            open_connections_group = stats.origin_server_stats_[server].total_socket_counts_[SOCKET_STATE_ACTIVE] + stats.origin_server_stats_[server].total_socket_counts_[SOCKET_STATE_IDLE] + stats.origin_server_stats_[server].total_socket_counts_[SOCKET_STATE_CONNECTING];
            if (it == group_list.begin()){
                ss << server.HostForURL() << "(" << open_connections_group << ")";
            }else{
                ss << ":" << server.HostForURL() << "(" << open_connections_group << ")";
            }
        }

        __attribute__((unused)) int open_connections =
                       stats.total_socket_counts_[SOCKET_STATE_ACTIVE] + stats.total_socket_counts_[SOCKET_STATE_IDLE] + stats.total_socket_counts_[SOCKET_STATE_CONNECTING];
        LIBNETXT_LOGI("STA_G %s(%d) %s",stats.pool_name_.c_str(),open_connections,ss.str().c_str());
    }
}

bool TransportService::GetTransportPoolStats(net::HttpNetworkSession::SocketPoolType socketPoolType,
        HttpNetworkSession* session,
        const net::HostPortPair server,
        base::DictionaryValue* pool_info,
        TransportPoolStats& stats) {

    // see base::DictionaryValue* ClientSocketPoolBaseHelper::GetInfoAsValue()
    // for reference how the data is collected.

    DCHECK(socketPoolType == HttpNetworkSession::NORMAL_SOCKET_STA_POOL || socketPoolType == HttpNetworkSession::NORMAL_SOCKET_POOL);
    DCHECK(session);
    DCHECK(!server.host().empty());
    DCHECK(server.port() != 0 );
    DCHECK(!stats.pool_name_.empty());
    DCHECK(pool_info);

    TransportClientSocketPool* socket_pool = session->GetTransportSocketPool(socketPoolType);
    DCHECK(socket_pool);

    // get the total values
    // see net::ClientSocketPoolBaseHelper::GetInfoAsValue()
    int val;
    base::ListValue* list_val = NULL;
    bool rv = true;

    if (!pool_info->GetInteger("handed_out_socket_count", &val)){
        if (LIBNETXT_IS_VERBOSE) {
            LIBNETXT_LOGE("%s socket pool does not have handed_out_socket_count",__FUNCTION__) ;
        }
        return false;
    }
    stats.total_socket_counts_[SOCKET_STATE_ACTIVE] = val;

    if (!pool_info->GetInteger("idle_socket_count", &val)){
        if (LIBNETXT_IS_VERBOSE) {
            LIBNETXT_LOGE("%s socket pool does not have idle_socket_count",__FUNCTION__);
        }
        return false;
    }
    stats.total_socket_counts_[SOCKET_STATE_IDLE] = val;

    if (!pool_info->GetInteger("connecting_socket_count", &val)){
        if (LIBNETXT_IS_VERBOSE) {
              LIBNETXT_LOGE("%s socket pool does not have connecting_socket_count",__FUNCTION__);
        }
        return false;
    }
    stats.total_socket_counts_[SOCKET_STATE_CONNECTING] = val;

    // get the values per origin ( == group )

    OriginServerNetworkStats origin_stats;
    origin_stats.origin_server_ = server;
    std::string group_name = server.ToString();


// group_map might be empty so we need to check first
   if(socket_pool->HasGroup(group_name) ){
       base::DictionaryValue* all_groups_dict;
       if( !pool_info->GetDictionary("groups", &all_groups_dict)){
           if (LIBNETXT_IS_VERBOSE) {
                LIBNETXT_LOGE("%s socket pool does not have groups",__FUNCTION__);
           }
       }
       DCHECK(all_groups_dict);
       base::DictionaryValue* group_dict;

       if(!all_groups_dict->GetDictionaryWithoutPathExpansion(group_name, &group_dict)){
           if (LIBNETXT_IS_VERBOSE) {
             LIBNETXT_LOGE("%s socket pool does not have group %s",__FUNCTION__,group_name.c_str());
           }
           socket_pool->printGroups();
       }
       DCHECK(group_dict);

       if (!group_dict->GetInteger("pending_request_count",&val )){
           if (LIBNETXT_IS_VERBOSE) {
                LIBNETXT_LOGE("%s socket pool does not have pending_request_count",__FUNCTION__);
           }
           return false;
       }
       origin_stats.pending_request_count_ = val;

       if (!group_dict->GetInteger("active_socket_count", &val)){
           if (LIBNETXT_IS_VERBOSE) {
                LIBNETXT_LOGE("%s socket pool does not have active_socket_count",__FUNCTION__);
           }
           return false;
       }
       origin_stats.total_socket_counts_[SOCKET_STATE_ACTIVE] = val;

       if (!group_dict->GetList("idle_sockets", &list_val)){
           if (LIBNETXT_IS_VERBOSE) {
                LIBNETXT_LOGE("%s socket pool does not have idle_sockets",__FUNCTION__);
           }
           return false;
       }
       origin_stats.total_socket_counts_[SOCKET_STATE_IDLE] = list_val->GetSize();

       if (!group_dict->GetList("connect_jobs", &list_val)){
           if (LIBNETXT_IS_VERBOSE) {
                LIBNETXT_LOGE("%s socket pool does not have connect_jobs",__FUNCTION__);
           }
           return false;
       }
       origin_stats.total_socket_counts_[SOCKET_STATE_CONNECTING] = list_val->GetSize();

   }else{
       if (LIBNETXT_IS_VERBOSE) {
            LIBNETXT_LOGE("%s socket pool does not have group %s",__FUNCTION__,group_name.c_str());
       }
       socket_pool->printGroups();
       origin_stats.total_socket_counts_[SOCKET_STATE_IDLE] = 0;
       origin_stats.total_socket_counts_[SOCKET_STATE_CONNECTING] = 0;
       rv = false;
   }
    stats.origin_server_stats_[server] = origin_stats;

    return rv;

}

//==============================================================================
void TransportService::GetNetStackProperties(NetStackProperties& net_stack_prop) const{
    net_stack_prop.max_sockets_per_origin_server_[HttpNetworkSession::NORMAL_SOCKET_POOL] = ClientSocketPoolManager::max_sockets_per_group(HttpNetworkSession::NORMAL_SOCKET_POOL);
    net_stack_prop.max_sockets_per_origin_server_[HttpNetworkSession::NORMAL_SOCKET_STA_POOL] = ClientSocketPoolManager::max_sockets_per_group(HttpNetworkSession::NORMAL_SOCKET_STA_POOL);
}

//==============================================================================
bool TransportService::GetSocketPoolType(const TransportPoolType pool_type,
                                         net::HttpNetworkSession::SocketPoolType& socket_pool_type )
{
   switch(pool_type)
   {
   case TRANSPORT_POOL_TYPE_STA_GENERIC:
   default:
       // for now, just return the STA pool type at all times
       socket_pool_type = net::HttpNetworkSession::NORMAL_SOCKET_STA_POOL;
   }

   return true;
}

//==============================================================================
void TransportService::set_accelerator_prefs(const AcceleratorPreferences& prefs){

    DCHECK(!set_accelerator_prefs_called_);
    set_accelerator_prefs_called_ = true;

    CHECK(prefs.session_prefs_.http_pipelining_enabled_ == false) << "We don't support pipelining";
    CHECK(prefs.session_prefs_.transport_pool_prefs_.size() == 1) << "Code tested only for a single network interface";

    const TransportPoolPreferences  &poolPrefs = prefs.session_prefs_.transport_pool_prefs_[0];

    DCHECK(poolPrefs.pool_type_ == TRANSPORT_POOL_TYPE_STA_GENERIC);

    //map the transport pool type to the session's socket pool type
    net::HttpNetworkSession::SocketPoolType socket_pool_type;
    GetSocketPoolType(poolPrefs.pool_type_,socket_pool_type);

    // we only set preferences for the STA  pool
    DCHECK(socket_pool_type == HttpNetworkSession::NORMAL_SOCKET_STA_POOL);

    // sanity check
    DCHECK(inrange(poolPrefs.max_connections_per_origin_server_, 6, 100));
    DCHECK(inrange(poolPrefs.max_connections_per_pool_,6,500));

    // set the same values to all the currently known sessions.
    prefs_ = prefs; //  store it for sessions that will be met later

    for(std::map<const void *,scoped_refptr<net::HttpNetworkSession> >::iterator it = sessions_.begin(); it != sessions_.end(); it++){
       TransportClientSocketPool* socket_pool =  (it->second)->GetTransportSocketPool(socket_pool_type);
    DCHECK(socket_pool);
    socket_pool->SetMaxSockets(poolPrefs.max_connections_per_pool_);
    socket_pool->SetMaxSocketsPerGroup(poolPrefs.max_connections_per_origin_server_);
    }
}

//==============================================================================
net::HttpNetworkSession* TransportService::GetAcceleratorNetworkSession(
        net::HttpNetworkSession* in_session, const NetworkSessionParams& in_session_params){

    DCHECK(set_accelerator_prefs_called_) << "must be set before getting here";
    DCHECK(in_session);

    // let's carry out the full compare as defined in the document (ICD 6.2.8)
    // first,
    bool same_params  = in_session_params.http_pipelining_enabled_ == prefs_.session_prefs_.http_pipelining_enabled_;

    //second, compare with the pool limits of the STA pool
    same_params &= in_session_params.max_sockets_per_origin_server_[TRANSPORT_POOL_TYPE_STA_GENERIC] == prefs_.session_prefs_.transport_pool_prefs_[0].max_connections_per_origin_server_;
    same_params &= in_session_params.max_sockets_per_pool_[TRANSPORT_POOL_TYPE_STA_GENERIC] == prefs_.session_prefs_.transport_pool_prefs_[0].max_connections_per_pool_;

    if(same_params)
    {
    return in_session;
    }

    net::HttpNetworkSession* cloned_session = NULL;

    // not same params. Try to find a match in other sessions
    if(sessions_.empty())
    {
        cloned_session = Clone(in_session);
        SetSessionParameters(cloned_session,prefs_);
        sessions_.insert(std::map<const void *,scoped_refptr<net::HttpNetworkSession> >
                            ::value_type((void *)in_session,cloned_session));
        return cloned_session;
            }
    else
    {
        bool match_found = false;

        if (in_session->params().is_cloned)
        {
           std::map<const void *,scoped_refptr<HttpNetworkSession> >::iterator it =
           sessions_.find(in_session);

           if (it != sessions_.end())
           {
              match_found = true;
              cloned_session = it->second;
        }
    }

        if (!match_found)
        {
           //not match found
           cloned_session = Clone(in_session);
           SetSessionParameters(cloned_session, prefs_);
           sessions_.insert(std::map<const void *,scoped_refptr<net::HttpNetworkSession> >
                            ::value_type((void *)in_session,cloned_session));
        }
    }

    return cloned_session;
}


//==============================================================================
net::HttpNetworkSession* TransportService::Clone(const net::HttpNetworkSession* other){
    // The loading of libsta.so is initiated in the ctor of HttpNetworkSession.
    // We are already loaded (obviously) and don't want another loading/init code to run
    // so we set here a flag to avoid the load.
    other->params().is_cloned = true;
    net::HttpNetworkSession* v = new net::HttpNetworkSession(other->params());
    return v;
}

//==============================================================================
void TransportService::SetSessionParameters(net::HttpNetworkSession* session, const sta::AcceleratorPreferences& accel_prefs){
    TransportClientSocketPool* socket_pool =  session->GetTransportSocketPool(HttpNetworkSession::NORMAL_SOCKET_STA_POOL);
    DCHECK(socket_pool);
    DCHECK(accel_prefs.session_prefs_.transport_pool_prefs_.size() > 0);
    int n = accel_prefs.session_prefs_.transport_pool_prefs_[0].max_connections_per_origin_server_;
    socket_pool->SetMaxSocketsPerGroup(n);

    n = accel_prefs.session_prefs_.transport_pool_prefs_[0].max_connections_per_pool_;
    socket_pool->SetMaxSockets(n);
}
//==============================================================================


//==============================================================================
void TransportService::GetPoolLimits(net::HttpNetworkSession* session, net::HttpNetworkSession::SocketPoolType pool_type, int& max_sockets_per_group, int& max_sockets) const{
    TransportClientSocketPool* socket_pool = session->GetTransportSocketPool(pool_type);
    DCHECK(socket_pool);

    scoped_ptr<base::DictionaryValue> dict(socket_pool->GetInfoAsValue("transport_socket_pool", "transport_socket_pool", true));
    dict->GetInteger("max_sockets_per_group", &max_sockets_per_group);
    dict->GetInteger("max_socket_count", &max_sockets);
}

//==============================================================================
void TransportService::ReleaseAcceleratorResources(){
    // we cannot delete the sessions becuase they are ref-counted.
    // simply clear the container. If the ref-counting is correct the sessions will be deleted.
    sessions_.clear();
}
