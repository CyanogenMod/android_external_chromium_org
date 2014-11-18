/*
* Copyright (c) 2014, The Linux Foundation. All rights reserved.
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions are
* met:
*     * Redistributions of source code must retain the above copyright
*       notice, this list of conditions and the following disclaimer.
*     * Redistributions in binary form must reproduce the above
*       copyright notice, this list of conditions and the following
*       disclaimer in the documentation and/or other materials provided
*       with the distribution.
*     * Neither the name of The Linux Foundation nor the names of its
*       contributors may be used to endorse or promote products derived
*       from this software without specific prior written permission.
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
*/

#ifndef ALT_TRANSACTION_H_
#define ALT_TRANSACTION_H_

#include "base/basictypes.h"
#include "base/compiler_specific.h"
#include "base/memory/scoped_ptr.h"
#include "base/threading/thread.h"
#include "net/socket/alt_transport_def.h"

namespace net {

class AltClientSocket;
class StreamSocket;
class HttpRequestHeaders;
class HttpResponseHeaders;

class AltTransaction {
public:
    AltTransaction(const char* url, base::MessageLoop* message_loop);

    ~AltTransaction();

    std::string& GetUrl() {
        return url_;
    }

    alt_transport::TransactionId GetId() {
        return transaction_id_;
    }

    void SetSocket(AltClientSocket* socket) {
        socket_ = socket;
    }

    AltClientSocket* GetSocket() {
        return(socket_);
    }

    base::MessageLoop* GetMessageLoop() {
        return message_loop_;
    }

    static AltTransaction* HandleRequest(
            const char* url,
            HttpRequestHeaders& headers);

    static StreamSocket* HandleResponse(
            AltTransaction* transaction,
            HttpResponseHeaders& headers,
            StreamSocket* original_socket);

    bool Read(
            char* buf,
            uint32_t buf_size);

    static void DeleteTransaction(
            AltTransaction* transaction);

private:

    static void InitOnce();

    std::string url_;
    base::MessageLoop* message_loop_;
    alt_transport::TransactionId transaction_id_;
    AltClientSocket* socket_;

static alt_transport::TransactionId transaction_id_cnt_;
static bool ready_;

    DISALLOW_COPY_AND_ASSIGN(AltTransaction);
};

}  // namespace net

#endif  // ALT_TRANSACTION_H_
