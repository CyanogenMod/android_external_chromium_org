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

#ifndef INSTANCE_CREATION_H
#define INSTANCE_CREATION_H

#include "net/http/http_network_session.h"
#include "net/http/http_transaction.h"

extern bool staLibraryLoaded;

/// abstract class for STA HttpTransaction object creation
struct StaTransactionFactory{
    virtual ~StaTransactionFactory() {}
    virtual net::HttpTransaction* CreateTransaction(const net::RequestPriority& priority, net::HttpNetworkSession*) = 0;
};

/// called from net::HttpNetworkSession::HttpNetworkSession
void OnSessionCreation(const net::HttpNetworkSession::Params& params);

/// create an instance of sta::HttpTransactionProcessor
/// \return 0 if ok.
int CreateInstance_TATransaction(net::HttpTransaction ** ppInteface,
        net::RequestPriority, net::HttpNetworkSession*, std::string* err);

#endif
