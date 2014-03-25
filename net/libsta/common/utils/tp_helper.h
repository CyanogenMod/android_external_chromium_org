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

#ifndef TP_HELPER_H_
#define TP_HELPER_H_

#include "base/memory/weak_ptr.h"

namespace sta{
struct SimplTprocCallback{
    // delayed commands ( to be posted and then executed)
    virtual void DelayedDestructor() = 0;
};

/// container of an object needed by the TP object
/// The TP object instantiates this object and is responsible
/// for its lifecycle.
class TProcHelper{

public:
    typedef enum {cmdDelayedDestructor } Command;

    TProcHelper(SimplTprocCallback* pCb) : cb_weak_ptr_factory_(pCb), net_http_tr_(NULL) {}

    virtual ~TProcHelper(){}

    int  StartBypassTransaction(net::HttpNetworkTransaction* net_transact, const net::HttpRequestInfo* request_info, const net::BoundNetLog& net_log){
        net_http_tr_ = net_transact;
        DCHECK(!outer_start_completed_cb_.is_null() );
        return net_http_tr_->Start(request_info , outer_start_completed_cb_, net_log);
    }

    void PostCommand_from_opensource( Command cmd){
          switch(cmd){
          case cmdDelayedDestructor:
              base::MessageLoop::current()->PostTask(FROM_HERE, base::Bind(&SimplTprocCallback::DelayedDestructor,  cb_weak_ptr_factory_.GetWeakPtr()));
              break;
          default:
              NOTREACHED();
          }
      }

    net::HttpNetworkTransaction* getNetTransaction() { return net_http_tr_;}

    void setOuterStartCompletedCb(const net::CompletionCallback& cb){
        outer_start_completed_cb_ = cb;
    }

    net::CompletionCallback* getOuterStartCompletedCb(void){
        DCHECK(!outer_start_completed_cb_.is_null());
        return &outer_start_completed_cb_;
    }

    void setLastReadCb(const net::CompletionCallback& cb){
        last_read_completed_cb_ = cb;
    }

    net::CompletionCallback* getLastReadCb(void){
        return &last_read_completed_cb_;
    }

    void setLastReadBuffer(net::IOBuffer* b){
        last_read_buf_ = b;
    }

    net::IOBuffer* getLastReadBuffer(void){
        return last_read_buf_.get();
    }

    void SetBeforeNetworkStartCallback(const net::HttpTransaction::BeforeNetworkStartCallback& callback){
        if(net_http_tr_){
            net_http_tr_->SetBeforeNetworkStartCallback(callback);
        }
    }

    bool LastReadCb_is_null() { return last_read_completed_cb_.is_null();}
    void LastReadCb_Reset(){last_read_completed_cb_.Reset();}

private:

    /// use this CB to report to the caller that our Start() was completed.
    net::CompletionCallback outer_start_completed_cb_;
    net::CompletionCallback last_read_completed_cb_;

    base::WeakPtrFactory<SimplTprocCallback> cb_weak_ptr_factory_;

    // used for bypass mode
    net::HttpNetworkTransaction* net_http_tr_;

    scoped_refptr<net::IOBuffer> last_read_buf_;
};

} // namespace sta
#endif /* TP_HELPER_H_ */
