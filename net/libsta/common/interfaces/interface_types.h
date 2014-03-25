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


#ifndef NET_INTERFACE_TYPES_H_
#define NET_INTERFACE_TYPES_H_

#include <map>
#include <vector>
#include <string>
#include <stdio.h>
#include "base/basictypes.h"
#include "base/supports_user_data.h"
#include "net/base/request_priority.h"
#include "base/timer/timer.h"
#include "net/base/host_port_pair.h"
#include "net/http/http_network_session.h"

namespace net{
struct HttpRequestInfo;
class HttpResponseInfo;
class HttpResponseHeaders;
class BoundNetLog;
}

namespace base{
class SupportsUserData;
}

namespace sta {

//TimerCallback
typedef base::Closure TimerCallback;

//TimerType
typedef enum
{
   TIMER_TYPE_ONE_SHOT = 0,
   TIMER_TYPE_PERIODIC,

} TimerType;

//TransportPoolType
typedef enum {

  TRANSPORT_POOL_TYPE_STA_GENERIC = 0,
  TRANSPORT_POOL_TYPE_MAX,

} TransportPoolType;

typedef enum {

  REQ_STATE_SEND_PENDING = 0,
  REQ_STATE_SEND_COMPLETE,
  REQ_STATE_RESPONSE_HEADER_COMPLETE,
  REQ_STATE_RESPONSE_BODY_PENDING,
  REQ_STATE_RESPONSE_COMPLETE,
  REQ_STATE_RETRY_PENDING,
  REQ_STATE_CANCEL_PENDING,
  REQ_STATE_CANCELLED,
  REQ_STATE_ERROR,
  REQ_STATE_COMPLETE,

} RequestState;

//RequestType
typedef enum {

  REQ_TYPE_BYPASS = 0,
  REQ_TYPE_ACCELERATED,
  REQ_TYPE_DEFAULT = REQ_TYPE_BYPASS,

} RequestType;

//RequestStatus
typedef enum {

  REQ_STATUS_SUCCESS = 0,
  REQ_STATUS_FAILED,

} RequestStatus;

//AcceleratorEventType
typedef enum
{
  ACCELERATOR_EVT_ERROR_FATAL = 0,

} AcceleratorEventType;

//SocketState
typedef enum
{
   SOCKET_STATE_ACTIVE = 0,
   SOCKET_STATE_IDLE,
   SOCKET_STATE_CONNECTING,
   SOCKET_STATE_MAX

} SocketState;


//OriginServer
typedef net::HostPortPair OriginServer;

class OriginServerNetworkStats
{
public:

   OriginServerNetworkStats();

   OriginServerNetworkStats(const OriginServer& os);

   OriginServerNetworkStats(const OriginServerNetworkStats& obj);

   ~OriginServerNetworkStats()
   { }

   std::string ToString() const;

   OriginServer   origin_server_;
   int            total_socket_counts_[SOCKET_STATE_MAX];
   int            pending_request_count_;

};

class TransportPoolStats
{
public:

   TransportPoolStats(const std::string& pool_name)
   : pool_name_(pool_name)
   { }

   ~TransportPoolStats()
   { }

   std::string                pool_name_;

   ///the number of socket counts in each socket state across the total socket pool (for a given transport pool type)
   int                   total_socket_counts_[SOCKET_STATE_MAX];

   typedef std::map<OriginServer,OriginServerNetworkStats>::iterator ServerStatsIter;

   std::map<OriginServer,OriginServerNetworkStats> origin_server_stats_;

   /// string representation of this object. May contain partial information
   /// so do not use it for serialization.
   std::string ToString() const { return pool_name_;}
};

//NetStackProperties
class NetStackProperties
{
public:

   int max_sockets_per_origin_server_[net::HttpNetworkSession::NUM_SOCKET_POOL_TYPES];
   const std::string ToString() const;
};

//NetworkSessionParams
class NetworkSessionParams {

 public:

  NetworkSessionParams()
  : http_pipelining_enabled_(false),
    proxy_configured_(false),
    spdy_enabled_(false),
    quic_enabled_(false)
  { }

  int     max_sockets_per_origin_server_[TRANSPORT_POOL_TYPE_MAX];
  int     max_sockets_per_pool_[TRANSPORT_POOL_TYPE_MAX];
  bool    http_pipelining_enabled_;
  bool    proxy_configured_;
  bool    spdy_enabled_;
  bool    quic_enabled_;

}; //class NetworkSessionParams

class TransportPoolPreferences
{
public:

   TransportPoolPreferences()
   : pool_type_(TRANSPORT_POOL_TYPE_STA_GENERIC),
     max_connections_per_origin_server_(0),
     max_connections_per_pool_(0)
   { }

   ~TransportPoolPreferences()
   { }

   TransportPoolPreferences(const TransportPoolPreferences& obj)
   : pool_type_(obj.pool_type_),
     max_connections_per_origin_server_(obj.max_connections_per_origin_server_),
     max_connections_per_pool_(obj.max_connections_per_pool_)
   { }

   TransportPoolType  pool_type_;
   int                max_connections_per_origin_server_;
   int                max_connections_per_pool_;

}; //class TransportPoolPreferences

class NetworkSessionPreferences
{
public:

   NetworkSessionPreferences()
   : http_pipelining_enabled_(false)
   { }

   ~NetworkSessionPreferences()
   { }

   NetworkSessionPreferences(const NetworkSessionPreferences& obj)
   : http_pipelining_enabled_(obj.http_pipelining_enabled_)
   {
      for (int i=0; i < (int)obj.transport_pool_prefs_.size(); i++)
      {
          transport_pool_prefs_.push_back(obj.transport_pool_prefs_[i]);
      }
   }

   std::vector<TransportPoolPreferences>  transport_pool_prefs_;
   bool                                   http_pipelining_enabled_;

}; //class NetworkSessionPreferences

class AcceleratorPreferences
{
public:

   NetworkSessionPreferences  session_prefs_;

}; //AcceleratorPreferences

//ResourceRequestInfo
class ResourceRequestInfo {

 public:

  ResourceRequestInfo(const uint64 req_id,
                            net::HttpNetworkSession* session,
                      const net::RequestPriority req_priority,
                      const std::string& host,
                      const uint16 port,
                      const base::SupportsUserData::Data* user_data,
                      const net::HttpRequestInfo& http_req_info);

  ResourceRequestInfo(const ResourceRequestInfo& obj);

  const uint64                        req_id_;
  const net::RequestPriority          priority_;
  const OriginServer                  origin_server_;
  const base::SupportsUserData::Data* user_data_;
        net::HttpNetworkSession*      in_session_;
  const net::HttpRequestInfo&         http_info_;

};//class ResourceRequestInfo

//ResourceRequest
class ResourceRequest {

 public:

  ResourceRequest(const ResourceRequestInfo& req_info,
                  const net::BoundNetLog& net_log)
  : req_info_(req_info),
    req_state_(REQ_STATE_SEND_PENDING),
    req_type_(REQ_TYPE_BYPASS),
    rsp_info_(NULL),
    req_status_(REQ_STATUS_SUCCESS),
    req_error_code_(net::OK),
    sched_session_(NULL),
    bytes_pending_scheduling_(0),
    net_log_(net_log)
  { }

  const ResourceRequestInfo req_info_;
  RequestState              req_state_;
  RequestType               req_type_;
  net::HttpResponseInfo*    rsp_info_;
  RequestStatus             req_status_;
  int                       req_error_code_;
  net::HttpNetworkSession*  sched_session_;
  int64                     bytes_pending_scheduling_;
  const net::BoundNetLog&   net_log_;

  virtual ~ResourceRequest() { delete req_info_.user_data_; }
 private:

  ResourceRequest();
  DISALLOW_COPY_AND_ASSIGN(ResourceRequest);

}; //class ResourceRequest

} // namespace sta

#endif
