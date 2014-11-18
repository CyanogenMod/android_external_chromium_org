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

#include "net/socket/alt_transaction.h"

#include <list>
#include <ctype.h>

#include "base/synchronization/lock.h"
#include "net/libnetxt/dyn_lib_loader.h"
#include "net/libnetxt/plugin_api.h"
#include "net/socket/alt_client_socket.h"
#include "net/socket/alt_transport_def.h"

namespace net {

//=========================================================================
alt_transport::TransactionId AltTransaction::transaction_id_cnt_ = 0;
bool AltTransaction::ready_ = false;

//=========================================================================
typedef std::map<alt_transport::TransactionId, AltTransaction*> AltTransactionMapType;
static AltTransactionMapType alt_transaction_map_;
static base::Lock lock_;

//=========================================================================
static bool (*DoInit)() = NULL;

static void (*DoHandleRequest)(
        alt_transport::TransactionId transaction_id,
        const char* url,
        net::HttpRequestHeaders& headers) = NULL;

static bool (*DoHandleResponse)(
        alt_transport::TransactionId transaction_id,
        const char* url,
        net::HttpResponseHeaders& headers,
        alt_transport::event_cb callback) = NULL;

static bool (*DoRead)(
        alt_transport::TransactionId transaction_id,
        char* buf,
        uint32_t buf_size) = NULL;

static void (*DoRelease)(
        alt_transport::TransactionId transaction_id) = NULL;

//=========================================================================
static void DoDataReadyCb(alt_transport::TransactionId transaction_id, int32_t size) {
    AltTransactionMapType::iterator iter = alt_transaction_map_.find(transaction_id);
    if (iter != alt_transaction_map_.end() ) {
        iter->second->GetSocket()->DidCompleteRead(size);
    }
}

//=========================================================================
void TransportEventCb(alt_transport::TransactionId transaction_id, const void* data, int32_t size) {
    base::AutoLock l(lock_);
    AltTransactionMapType::iterator iter = alt_transaction_map_.find(transaction_id);
    if (iter != alt_transaction_map_.end() ) {
        if (size) {
            iter->second->GetMessageLoop()->PostTask(FROM_HERE, base::Bind(&DoDataReadyCb, transaction_id, size));
        }
    }
}

//=========================================================================
AltTransaction::AltTransaction(const char* url, base::MessageLoop* message_loop) :
    url_(url),
    message_loop_(message_loop),
    transaction_id_(++transaction_id_cnt_),
    socket_(NULL) {
}

//=========================================================================
AltTransaction::~AltTransaction() {
    if (socket_) {
        delete socket_;
    }
}

//=========================================================================
void AltTransaction::InitOnce() {
    static bool initialized = false;
    if (!initialized) {
        char value[PROPERTY_VALUE_MAX] = {'\0'};
        LIBNETXT_API(SysPropertyGet)("net.alttr.on", value, "1");

        bool isEnabled = (bool) atoi(value);
        if (isEnabled) {
            void* fh = LibraryManager::GetLibraryHandle("libastreaus_plugin");
            if (fh) {
                *(void **)(&DoInit) = LibraryManager::GetLibrarySymbol(fh, "DoInit", false);
                *(void **)(&DoHandleRequest) = LibraryManager::GetLibrarySymbol(fh, "DoHandleRequest", false);
                *(void **)(&DoRead) = LibraryManager::GetLibrarySymbol(fh, "DoRead", false);
                *(void **)(&DoHandleResponse) = LibraryManager::GetLibrarySymbol(fh, "DoHandleResponse", false);
                *(void **)(&DoRelease) = LibraryManager::GetLibrarySymbol(fh, "DoRelease", false);
            }
            if (DoInit && DoHandleRequest && DoRead && DoHandleResponse && DoRelease) {
                if (DoInit()) {
                    ready_ = true;
                }
            }
        }
        initialized = true;
    }
}

//=========================================================================
AltTransaction* AltTransaction::HandleRequest(
        const char* url,
        HttpRequestHeaders& headers) {

    InitOnce();
    if (ready_&& DoHandleRequest && url) {
        AltTransaction* trans = new AltTransaction(url, base::MessageLoop::current());
        alt_transaction_map_.insert(std::pair<alt_transport::TransactionId, AltTransaction*>(trans->GetId(), trans));
        DoHandleRequest(trans->GetId(), url, headers);
        return trans;
    }
    return NULL;
}

//=========================================================================
StreamSocket* AltTransaction::HandleResponse(
        AltTransaction* transaction,
        HttpResponseHeaders& headers,
        StreamSocket* original_socket) {
    AltClientSocket* socket = NULL;

    base::AutoLock l(lock_);
    if (ready_ && DoHandleResponse && transaction && original_socket) {
        if (DoHandleResponse(transaction->GetId(), transaction->GetUrl().c_str(), headers, &TransportEventCb)) {
            socket = new AltClientSocket(transaction, original_socket);
            transaction->SetSocket(socket);
        }
    }
    return socket;
}

//=========================================================================
bool AltTransaction::Read(
        char* buf,
        uint32_t buf_size) {

    base::AutoLock l(lock_);
    if (ready_&& DoRead && buf && buf_size) {
        return DoRead(GetId(), buf, buf_size);
    }
    return false;
}

//=========================================================================
void AltTransaction::DeleteTransaction(AltTransaction* transaction) {

    base::AutoLock l(lock_);
    if (ready_ && transaction) {
        AltTransactionMapType::iterator iter = alt_transaction_map_.find(transaction->GetId());
        if (iter != alt_transaction_map_.end() ) {
            if (DoRelease) {
                DoRelease(transaction->GetId());
            }
            alt_transaction_map_.erase(iter);
            delete transaction;
        }
    }
}

}  // namespace net
