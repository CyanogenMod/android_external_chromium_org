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

#include "net/socket/alt_client_socket.h"

#include <list>
#include <ctype.h>

#include "base/callback_helpers.h"
#include "base/logging.h"
#include "base/strings/stringprintf.h"
#include "base/synchronization/lock.h"
#include "net/base/io_buffer.h"
#include "net/base/ip_endpoint.h"
#include "net/base/net_errors.h"
#include "net/base/net_util.h"
#include "net/socket/alt_transport_def.h"
#include "net/socket/alt_transaction.h"

namespace net {

//=========================================================================
AltClientSocket::AltClientSocket(AltTransaction* alt_transction,
                                StreamSocket* original_socket):
        alt_transction_(alt_transction),
        original_socket_(original_socket),
        data_to_read_(NULL),
        read_buf_empty_(true) {
}

//=========================================================================
AltClientSocket::~AltClientSocket() {
 base::AutoLock l(lock_);
}

//=========================================================================
int AltClientSocket::Connect(const CompletionCallback& callback) {
  DCHECK(!callback.is_null());
  return original_socket_->Connect(callback);
}

//=========================================================================
void AltClientSocket::Disconnect() {
  original_socket_->Disconnect();
}

//=========================================================================
bool AltClientSocket::IsConnected() const {
  return original_socket_->IsConnected();
}

//=========================================================================
bool AltClientSocket::IsConnectedAndIdle() const {
  return original_socket_->IsConnectedAndIdle();
}

//=========================================================================
int AltClientSocket::GetPeerAddress(IPEndPoint* address) const {
  return original_socket_->GetPeerAddress(address);
}

//=========================================================================
int AltClientSocket::GetLocalAddress(IPEndPoint* address) const {
  DCHECK(address);
  return original_socket_->GetLocalAddress(address);
}

//=========================================================================
const BoundNetLog& AltClientSocket::NetLog() const {
  return original_socket_->NetLog();
}

//=========================================================================
void AltClientSocket::SetSubresourceSpeculation() {
  original_socket_->SetSubresourceSpeculation();
}

//=========================================================================
void AltClientSocket::SetOmniboxSpeculation() {
  original_socket_->SetOmniboxSpeculation();
}

//=========================================================================
bool AltClientSocket::WasEverUsed() const {
  return original_socket_->WasEverUsed();
}

//=========================================================================
bool AltClientSocket::UsingTCPFastOpen() const {
  return original_socket_->UsingTCPFastOpen();
}

//=========================================================================
bool AltClientSocket::WasNpnNegotiated() const {
  return original_socket_->WasNpnNegotiated();
}

//=========================================================================
NextProto AltClientSocket::GetNegotiatedProtocol() const {
  return original_socket_->GetNegotiatedProtocol();
}

//=========================================================================
bool AltClientSocket::GetSSLInfo(SSLInfo* ssl_info) {
  return original_socket_->GetSSLInfo(ssl_info);
}

//=========================================================================
int AltClientSocket::Read(IOBuffer* buf,
                          int buf_len,
                          const CompletionCallback& callback) {
  DCHECK(!callback.is_null());
  int result = ERR_CONNECTION_CLOSED;

  base::AutoLock l(lock_);

  read_callback_ = callback;
  if (alt_transction_->Read(buf->data(), buf_len)) {
      result = ERR_IO_PENDING;
  }
  return result;
}

//=========================================================================
int AltClientSocket::Write(IOBuffer* buf,
                           int buf_len,
                           const CompletionCallback& callback) {
  DCHECK(!callback.is_null());
  return original_socket_->Write(buf, buf_len, callback);
}

//=========================================================================
int AltClientSocket::SetReceiveBufferSize(int32 size) {
  return original_socket_->SetReceiveBufferSize(size);
}

//=========================================================================
int AltClientSocket::SetSendBufferSize(int32 size) {
  return original_socket_->SetSendBufferSize(size);
}

//=========================================================================
scoped_ptr<StreamSocket> AltClientSocket::PassOriginalSocket() {
    return original_socket_.Pass();
}

//=========================================================================
void AltClientSocket::DidCompleteRead(int result) {
    if (!read_callback_.is_null()) {
        read_callback_.Run(result);
    }
}

} //net
