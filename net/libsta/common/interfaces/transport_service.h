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

#ifndef STA_TRANSPORT_SERVICE_H_
#define STA_TRANSPORT_SERVICE_H_

#include <stdint.h>
#include "base/basictypes.h"
#include "base/timer/timer.h"
#include "net/base/host_port_pair.h"
#include "net/http/http_network_session.h"

#include "net/libsta/common/interfaces/interface_types.h"

namespace sta
{

class AccelerationService;
class TransportService
{
public:

    TransportService();

    virtual ~TransportService();

    static TransportService* getInstance();

    virtual void set_accelerator_prefs(const AcceleratorPreferences& prefs);

    virtual void ResetState(net::HttpNetworkSession::SocketPoolType socketPoolType, net::HttpNetworkSession* session);
    virtual void PrintTransportPoolStats(net::HttpNetworkSession* session);
    virtual void PrintTransportPoolStats(net::HttpNetworkSession::SocketPoolType socketPoolType,
            net::HttpNetworkSession* session,
            TransportPoolStats& stats);
    /**
     * Method to fetch transport (i.e.TCP) connection pool statistics,
     *
     *  such as the per origin-server active/idle connection count among others for the specified pool type,
     *  network session and origin server.
     *
     * @param pool_type [in] The transport (i.e. TCP) client socket pool type for which the statistics are being requested.
     * @param session   [in] The network session associated with the pool for which the statistics are being requested.
     * @param server    [in] The host and port information of the server for which the statistics are being requested.
     * @param stats     [in] Pool statistics for the specified origin server (i.e. group) pool part of  the specified session.
     * @return success or failure of the operation
     *
     * @note  This method is to be used to update socket statistics relevant to a specific transaction. It is expected to be invoked as part of the TA request scheduler execution, triggered by the completion of an ongoing request and/or sub-request or upon receipt of an incoming request among other events.
     */
    virtual bool GetTransportPoolStats(net::HttpNetworkSession::SocketPoolType socketPoolType, //  int pool_type,
                               net::HttpNetworkSession* session,
                               const net::HostPortPair server,
                               TransportPoolStats& stats) /*const*/;


    virtual void GetNetStackProperties(NetStackProperties& net_stack_prop) const;

    virtual void NotifyAcceleratorEvent(const AcceleratorEventType evt_type,
                                  const std::string evt_msg);

    virtual base::Timer* StartTimer(const TimerType type, const TimerCallback& callback, const int timeout_ms);

    virtual void ResetTimer(base::Timer* const timer_handle);

    virtual void StopTimer(base::Timer* const timer_handle);

    virtual net::HttpNetworkSession* GetAcceleratorNetworkSession(
                             net::HttpNetworkSession* in_session,
                             const NetworkSessionParams& in_session_params);

    AccelerationService* GetAccelerationService(void);

    void RegisterAccelerationService(AccelerationService* accel_svc);

    bool is_acceleration_service_set() { return acceleration_service_?true:false;}

    void GetPoolLimits(net::HttpNetworkSession* session, net::HttpNetworkSession::SocketPoolType pool_type, int& max_sockets_per_group, int& max_sockets) const;

    /// Method to release (deallocate memory, stop timers (if any) and so on) resources managed by TransportService on behalf of the STA.
    void ReleaseAcceleratorResources();

protected:

    AccelerationService* acceleration_service_;

private:

    net::HttpNetworkSession* Clone(const net::HttpNetworkSession*);
    void SetSessionParameters(net::HttpNetworkSession*, const AcceleratorPreferences& prefs);

    //returns the session socket pool type for the specified transport pool type
    bool GetSocketPoolType(const TransportPoolType pool_type,
                           net::HttpNetworkSession::SocketPoolType& socket_pool_type);


    // for debugging, make sure the prefs are set
    bool set_accelerator_prefs_called_;

    static TransportService* this_;

    std::map<const void *,scoped_refptr<net::HttpNetworkSession> > sessions_;
    AcceleratorPreferences prefs_;

    DISALLOW_COPY_AND_ASSIGN(TransportService);
};

} //namespace sta

#endif //STA_TRANSPORT_SERVICE_H_
