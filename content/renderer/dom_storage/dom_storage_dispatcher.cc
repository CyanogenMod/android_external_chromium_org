// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "content/renderer/dom_storage/dom_storage_dispatcher.h"

#include <list>
#include <map>

#include "base/string_number_conversions.h"
#include "base/synchronization/lock.h"
#include "content/common/dom_storage_messages.h"
#include "content/renderer/dom_storage/webstoragearea_impl.h"
#include "content/renderer/dom_storage/webstoragenamespace_impl.h"
#include "content/renderer/render_thread_impl.h"
#include "third_party/WebKit/public/platform/Platform.h"
#include "third_party/WebKit/Source/WebKit/chromium/public/WebKit.h"
#include "third_party/WebKit/Source/WebKit/chromium/public/WebStorageEventDispatcher.h"
#include "webkit/dom_storage/dom_storage_cached_area.h"
#include "webkit/dom_storage/dom_storage_proxy.h"
#include "webkit/dom_storage/dom_storage_types.h"

using dom_storage::DomStorageCachedArea;
using dom_storage::DomStorageProxy;
using dom_storage::ValuesMap;

namespace content {

namespace {
// MessageThrottlingFilter -------------------------------------------
// Used to limit the number of ipc messages pending completion so we
// don't overwhelm the main browser process. When the limit is reached,
// a synchronous message is sent to flush all pending messages thru.
// We expect to receive an 'ack' for each message sent. This object
// observes receipt of the acks on the IPC thread to decrement a counter.
class MessageThrottlingFilter : public IPC::ChannelProxy::MessageFilter {
 public:
  explicit MessageThrottlingFilter(RenderThreadImpl* sender)
      : pending_count_(0), sender_(sender) {}

  void SendThrottled(IPC::Message* message);
  void Shutdown() { sender_ = NULL; }

 private:
  virtual ~MessageThrottlingFilter() {}

  virtual bool OnMessageReceived(const IPC::Message& message) OVERRIDE;

  int GetPendingCount() { return IncrementPendingCountN(0); }
  int IncrementPendingCount() { return IncrementPendingCountN(1); }
  int DecrementPendingCount() { return IncrementPendingCountN(-1); }
  int IncrementPendingCountN(int increment) {
    base::AutoLock locker(lock_);
    pending_count_ += increment;
    return pending_count_;
  }

  base::Lock lock_;
  int pending_count_;
  RenderThreadImpl* sender_;
};

void MessageThrottlingFilter::SendThrottled(IPC::Message* message) {
  // Should only be used for sending of messages which will be acknowledged
  // with a separate DOMStorageMsg_AsyncOperationComplete message.
  DCHECK(message->type() == DOMStorageHostMsg_LoadStorageArea::ID ||
         message->type() == DOMStorageHostMsg_SetItem::ID ||
         message->type() == DOMStorageHostMsg_RemoveItem::ID ||
         message->type() == DOMStorageHostMsg_Clear::ID);
  DCHECK(sender_);
  if (!sender_) {
    delete message;
    return;
  }
  const int kMaxPendingMessages = 1000;
  bool need_to_flush = (IncrementPendingCount() > kMaxPendingMessages) &&
                       !message->is_sync();
  sender_->Send(message);
  if (need_to_flush) {
    sender_->Send(new DOMStorageHostMsg_FlushMessages);
    DCHECK_EQ(0, GetPendingCount());
  } else {
    DCHECK_LE(0, GetPendingCount());
  }
}

bool MessageThrottlingFilter::OnMessageReceived(const IPC::Message& message) {
  if (message.type() == DOMStorageMsg_AsyncOperationComplete::ID) {
    DecrementPendingCount();
    DCHECK_LE(0, GetPendingCount());
  }
  return false;
}
}  // namespace

// ProxyImpl -----------------------------------------------------
// An implementation of the DomStorageProxy interface in terms of IPC.
// This class also manages the collection of cached areas and pending
// operations awaiting completion callbacks.
class DomStorageDispatcher::ProxyImpl : public DomStorageProxy {
 public:
  explicit ProxyImpl(RenderThreadImpl* sender);

  // Methods for use by DomStorageDispatcher directly.
  DomStorageCachedArea* OpenCachedArea(
      int64 namespace_id, const GURL& origin);
  void CloseCachedArea(DomStorageCachedArea* area);
  DomStorageCachedArea* LookupCachedArea(
      int64 namespace_id, const GURL& origin);
  void CompleteOnePendingCallback(bool success);
  void Shutdown();

  // DomStorageProxy interface for use by DomStorageCachedArea.
  virtual void LoadArea(int connection_id, ValuesMap* values,
                        const CompletionCallback& callback) OVERRIDE;
  virtual void SetItem(int connection_id, const string16& key,
                       const string16& value, const GURL& page_url,
                       const CompletionCallback& callback) OVERRIDE;
  virtual void RemoveItem(int connection_id, const string16& key,
                          const GURL& page_url,
                          const CompletionCallback& callback) OVERRIDE;
  virtual void ClearArea(int connection_id,
                        const GURL& page_url,
                        const CompletionCallback& callback) OVERRIDE;

 private:
  // Struct to hold references to our contained areas and
  // to keep track of how many tabs have a given area open.
  struct CachedAreaHolder {
    scoped_refptr<DomStorageCachedArea> area_;
    int open_count_;
    CachedAreaHolder() : open_count_(0) {}
    CachedAreaHolder(DomStorageCachedArea* area, int count)
        : area_(area), open_count_(count) {}
  };
  typedef std::map<std::string, CachedAreaHolder> CachedAreaMap;
  typedef std::list<CompletionCallback> CallbackList;

  virtual ~ProxyImpl() {
  }

  // Sudden termination is disabled when there are callbacks pending
  // to more reliably commit changes during shutdown.
  void PushPendingCallback(const CompletionCallback& callback) {
    if (pending_callbacks_.empty())
      WebKit::Platform::current()->suddenTerminationChanged(false);
    pending_callbacks_.push_back(callback);
  }

  CompletionCallback PopPendingCallback() {
    CompletionCallback callback = pending_callbacks_.front();
    pending_callbacks_.pop_front();
    if (pending_callbacks_.empty())
      WebKit::Platform::current()->suddenTerminationChanged(true);
    return callback;
  }

  std::string GetCachedAreaKey(int64 namespace_id, const GURL& origin) {
    return base::Int64ToString(namespace_id) + origin.spec();
  }

  CachedAreaHolder* GetAreaHolder(const std::string& key) {
    CachedAreaMap::iterator found = cached_areas_.find(key);
    if (found == cached_areas_.end())
      return NULL;
    return &(found->second);
  }

  RenderThreadImpl* sender_;
  CachedAreaMap cached_areas_;
  CallbackList pending_callbacks_;
  scoped_refptr<MessageThrottlingFilter> throttling_filter_;
};

DomStorageDispatcher::ProxyImpl::ProxyImpl(RenderThreadImpl* sender)
    : sender_(sender),
      throttling_filter_(new MessageThrottlingFilter(sender)) {
  sender_->AddFilter(throttling_filter_);
}

DomStorageCachedArea* DomStorageDispatcher::ProxyImpl::OpenCachedArea(
    int64 namespace_id, const GURL& origin) {
  std::string key = GetCachedAreaKey(namespace_id, origin);
  if (CachedAreaHolder* holder = GetAreaHolder(key)) {
    ++(holder->open_count_);
    return holder->area_;
  }
  scoped_refptr<DomStorageCachedArea> area =
      new DomStorageCachedArea(namespace_id, origin, this);
  cached_areas_[key] = CachedAreaHolder(area, 1);
  return area.get();
}

void DomStorageDispatcher::ProxyImpl::CloseCachedArea(
    DomStorageCachedArea* area) {
  std::string key = GetCachedAreaKey(area->namespace_id(), area->origin());
  CachedAreaHolder* holder = GetAreaHolder(key);
  DCHECK(holder);
  DCHECK_EQ(holder->area_.get(), area);
  DCHECK_GT(holder->open_count_, 0);
  if (--(holder->open_count_) == 0) {
    cached_areas_.erase(key);
  }
}

DomStorageCachedArea* DomStorageDispatcher::ProxyImpl::LookupCachedArea(
    int64 namespace_id, const GURL& origin) {
  std::string key = GetCachedAreaKey(namespace_id, origin);
  CachedAreaHolder* holder = GetAreaHolder(key);
  if (!holder)
    return NULL;
  return holder->area_.get();
}

void DomStorageDispatcher::ProxyImpl::CompleteOnePendingCallback(bool success) {
  PopPendingCallback().Run(success);
}

void DomStorageDispatcher::ProxyImpl::Shutdown() {
  throttling_filter_->Shutdown();
  sender_->RemoveFilter(throttling_filter_);
  sender_ = NULL;
  cached_areas_.clear();
  pending_callbacks_.clear();
}

void DomStorageDispatcher::ProxyImpl::LoadArea(
    int connection_id, ValuesMap* values,
    const CompletionCallback& callback) {
  PushPendingCallback(callback);
  throttling_filter_->SendThrottled(new DOMStorageHostMsg_LoadStorageArea(
      connection_id, values));
}

void DomStorageDispatcher::ProxyImpl::SetItem(
    int connection_id, const string16& key,
    const string16& value, const GURL& page_url,
    const CompletionCallback& callback) {
  PushPendingCallback(callback);
  throttling_filter_->SendThrottled(new DOMStorageHostMsg_SetItem(
      connection_id, key, value, page_url));
}

void DomStorageDispatcher::ProxyImpl::RemoveItem(
    int connection_id, const string16& key,  const GURL& page_url,
    const CompletionCallback& callback) {
  PushPendingCallback(callback);
  throttling_filter_->SendThrottled(new DOMStorageHostMsg_RemoveItem(
      connection_id, key, page_url));
}

void DomStorageDispatcher::ProxyImpl::ClearArea(int connection_id,
                      const GURL& page_url,
                      const CompletionCallback& callback) {
  PushPendingCallback(callback);
  throttling_filter_->SendThrottled(new DOMStorageHostMsg_Clear(
      connection_id, page_url));
}

// DomStorageDispatcher ------------------------------------------------

DomStorageDispatcher::DomStorageDispatcher()
    : proxy_(new ProxyImpl(RenderThreadImpl::current())) {
}

DomStorageDispatcher::~DomStorageDispatcher() {
  proxy_->Shutdown();
}

scoped_refptr<DomStorageCachedArea> DomStorageDispatcher::OpenCachedArea(
    int connection_id, int64 namespace_id, const GURL& origin) {
  RenderThreadImpl::current()->Send(
      new DOMStorageHostMsg_OpenStorageArea(
          connection_id, namespace_id, origin));
  return proxy_->OpenCachedArea(namespace_id, origin);
}

void DomStorageDispatcher::CloseCachedArea(
    int connection_id, DomStorageCachedArea* area) {
  RenderThreadImpl::current()->Send(
      new DOMStorageHostMsg_CloseStorageArea(connection_id));
  proxy_->CloseCachedArea(area);
}

bool DomStorageDispatcher::OnMessageReceived(const IPC::Message& msg) {
  bool handled = true;
  IPC_BEGIN_MESSAGE_MAP(DomStorageDispatcher, msg)
    IPC_MESSAGE_HANDLER(DOMStorageMsg_Event, OnStorageEvent)
    IPC_MESSAGE_HANDLER(DOMStorageMsg_AsyncOperationComplete,
                        OnAsyncOperationComplete)
    IPC_MESSAGE_UNHANDLED(handled = false)
  IPC_END_MESSAGE_MAP()
  return handled;
}

void DomStorageDispatcher::OnStorageEvent(
    const DOMStorageMsg_Event_Params& params) {
  RenderThreadImpl::current()->EnsureWebKitInitialized();

  bool originated_in_process = params.connection_id != 0;
  WebStorageAreaImpl* originating_area = NULL;
  if (originated_in_process) {
    originating_area = WebStorageAreaImpl::FromConnectionId(
        params.connection_id);
  } else {
    DomStorageCachedArea* cached_area = proxy_->LookupCachedArea(
        params.namespace_id, params.origin);
    if (cached_area)
      cached_area->ApplyMutation(params.key, params.new_value);
  }

  if (params.namespace_id == dom_storage::kLocalStorageNamespaceId) {
    WebKit::WebStorageEventDispatcher::dispatchLocalStorageEvent(
        params.key,
        params.old_value,
        params.new_value,
        params.origin,
        params.page_url,
        originating_area,
        originated_in_process);
  } else {
    WebStorageNamespaceImpl
        session_namespace_for_event_dispatch(params.namespace_id);
    WebKit::WebStorageEventDispatcher::dispatchSessionStorageEvent(
        params.key,
        params.old_value,
        params.new_value,
        params.origin,
        params.page_url,
        session_namespace_for_event_dispatch,
        originating_area,
        originated_in_process);
  }
}

void DomStorageDispatcher::OnAsyncOperationComplete(bool success) {
  proxy_->CompleteOnePendingCallback(success);
}

}  // namespace content
