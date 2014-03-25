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

#ifndef TW_HELPER_H_
#define TW_HELPER_H_

#include "base/memory/weak_ptr.h"
#include "net/base/dependant_iobuffer.h"
#include "net/http/http_request_info.h"

namespace sta{

struct SimpleCallback{
    virtual void OnIOComplete(int n) = 0;
    virtual void OnStartComplete(int n) = 0;

    // delayed commands ( to be posted and then executed)
    virtual void StartAutonomousRead() = 0;
    virtual void DelayedDestructor() = 0;
};

/// container of an object needed by the TW object
/// The TW object instantiates this objects and is responsible
/// for its lifecycle.
class TwrapperHelper{

public:
    typedef enum {cmd_StartAutonomousRead, cmdDelayedDestructor } Command;

    TwrapperHelper(net::HttpNetworkTransaction* netTransaction, SimpleCallback* pCb ) :
        request_info_mutable_(NULL),
        cb_weak_ptr_factory_(pCb),
        weak_ptr_factory_(this),
        local_body_buff_(NULL),
        read_buf_(NULL),
        callback_into_prop_(pCb)
    {
            network_trans_.reset(netTransaction);
    }

    virtual ~TwrapperHelper(){if (request_info_mutable_) delete request_info_mutable_;}

    void setLocalBodyBuff(scoped_refptr<net::IOBufferWithSize>& b) {local_body_buff_ = b;}
    scoped_refptr<net::IOBufferWithSize> getLocalBodyBuff(void) const {return local_body_buff_;}

    // connect the dependant buff at offset / size
    void ConnectReadBuffer(int offset, int size){
        dependant_read_buffer_.get()->Connect(const_cast<net::IOBufferWithSize*>(local_body_buff_.get()), offset, size);
        read_buf_ = dependant_read_buffer_.get();
    }

    int Read(int max_bytes_to_read){
        DCHECK(read_buf_.get());
        return network_trans_->Read(read_buf_.get(), max_bytes_to_read, inner_read_callback_);
    }

    /// instantiate the dependant buffer,
    /// and set the callback ptr
    void CreateReadBuffer(void){
        dependant_read_buffer_ = new net::DependantIOBufferWithSize();
        inner_read_callback_ = base::Bind(&TwrapperHelper::OnIOComplete, weak_ptr_factory_.GetWeakPtr());
    }

    void SetBeforeNetworkStartCallback(const net::HttpTransaction::BeforeNetworkStartCallback& callback){
        DCHECK(network_trans_.get() != NULL);
        network_trans_->SetBeforeNetworkStartCallback(callback);
    }
    int  Start(const net::BoundNetLog& net_log){
        DCHECK(network_trans_.get() != NULL);
        DCHECK( request_info_mutable_->url.is_valid());
        net::CompletionCallback inner_start_completed_cb = base::Bind(&TwrapperHelper::OnStartComplete , weak_ptr_factory_.GetWeakPtr());
        return network_trans_->Start(request_info_mutable_, inner_start_completed_cb, net_log);
    }

    void PostCommand( Command cmd){

        switch(cmd){
        case cmd_StartAutonomousRead:
            base::MessageLoop::current()->PostTask(FROM_HERE, base::Bind(&SimpleCallback::StartAutonomousRead,  cb_weak_ptr_factory_.GetWeakPtr()));
            break;
        case cmdDelayedDestructor:
            base::MessageLoop::current()->PostTask(FROM_HERE, base::Bind(&SimpleCallback::DelayedDestructor,  cb_weak_ptr_factory_.GetWeakPtr()));
            break;
        default:
            NOTREACHED();
        }
    }

    void CopyToBuff(char* buf_data, int offset, int bytes_to_copy){
        DCHECK(buf_data);
        DCHECK((offset >= 0) && (bytes_to_copy >= 0));
        DCHECK((bytes_to_copy + offset) <= local_body_buff_->size());
        memcpy(buf_data, local_body_buff_->data() + offset, bytes_to_copy);
    }

    void ShutdownNetTransaction(){
        delete network_trans_.release();
    }

    net::HttpNetworkTransaction* NetTransaction() { return network_trans_.get();}

    void setRequestInfo(net::HttpRequestInfo* info){
        request_info_mutable_ = info;
    }

    net::HttpRequestInfo* RequestInfo(){ return request_info_mutable_; }

    int getLocalBodyBuffSize(){ return local_body_buff_->size(); }

    int dependant_read_buffer_size(){ return dependant_read_buffer_->size(); }

    uint64 GetTotalReceivedBytes(){ return network_trans_->GetTotalReceivedBytes(); }

private:

    void OnStartComplete(int n){
        DCHECK(callback_into_prop_);
        callback_into_prop_->OnStartComplete(n);
    }

    void OnIOComplete(int n){
           DCHECK(callback_into_prop_);
           callback_into_prop_->OnIOComplete(n);
    }

    net::HttpRequestInfo* request_info_mutable_;

    /// this is a piggy-back buffer allowing for IOBuffer with writing at an offset.
    scoped_refptr<net::DependantIOBufferWithSize> dependant_read_buffer_; //having this variable adds dependency on base::subtle::RefCountedThreadSafeBase::Release() const
    // ... and using it adds the AddRef()

    base::WeakPtrFactory<SimpleCallback> cb_weak_ptr_factory_; // having this object add dependency on  base::internal::WeakReferenceOwner::WeakReferenceOwner() (and dtor)
    base::WeakPtrFactory<TwrapperHelper> weak_ptr_factory_; // having this object add dependency on  base::internal::WeakReferenceOwner::WeakReferenceOwner() (and dtor)

    net::CompletionCallback inner_read_callback_;

    scoped_refptr<net::IOBufferWithSize> local_body_buff_;

    /// the actual (pointer to) the buffer used by this transaction
    scoped_refptr<net::IOBuffer> read_buf_;

    /// the actual network transaction
    scoped_ptr<net::HttpNetworkTransaction> network_trans_;

    SimpleCallback* callback_into_prop_;
};

} // namespace sta
#endif /* TW_HELPER_H_ */
