// Copyright (c) 2010 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/renderer/renderer_webidbindex_impl.h"

#include "chrome/common/indexed_db_messages.h"
#include "chrome/renderer/render_thread.h"
#include "chrome/renderer/indexed_db_dispatcher.h"
#include "chrome/renderer/renderer_webidbtransaction_impl.h"
#include "third_party/WebKit/WebKit/chromium/public/WebString.h"
#include "third_party/WebKit/WebKit/chromium/public/WebVector.h"

using WebKit::WebExceptionCode;
using WebKit::WebDOMStringList;
using WebKit::WebString;
using WebKit::WebVector;

RendererWebIDBIndexImpl::RendererWebIDBIndexImpl(int32 idb_index_id)
    : idb_index_id_(idb_index_id) {
}

RendererWebIDBIndexImpl::~RendererWebIDBIndexImpl() {
  // TODO(jorlow): Is it possible for this to be destroyed but still have
  //               pending callbacks?  If so, fix!
  RenderThread::current()->Send(new IndexedDBHostMsg_IndexDestroyed(
      idb_index_id_));
}

WebString RendererWebIDBIndexImpl::name() const {
  string16 result;
  RenderThread::current()->Send(
      new IndexedDBHostMsg_IndexName(idb_index_id_, &result));
  return result;
}

WebString RendererWebIDBIndexImpl::storeName() const {
  string16 result;
  RenderThread::current()->Send(
      new IndexedDBHostMsg_IndexStoreName(idb_index_id_, &result));
  return result;
}

WebString RendererWebIDBIndexImpl::keyPath() const {
  NullableString16 result;
  RenderThread::current()->Send(
      new IndexedDBHostMsg_IndexKeyPath(idb_index_id_, &result));
  return result;
}

bool RendererWebIDBIndexImpl::unique() const {
  bool result;
  RenderThread::current()->Send(
      new IndexedDBHostMsg_IndexUnique(idb_index_id_, &result));
  return result;
}

void RendererWebIDBIndexImpl::openObjectCursor(
    const WebKit::WebIDBKeyRange& range,
    unsigned short direction,
    WebKit::WebIDBCallbacks* callbacks,
    const WebKit::WebIDBTransaction& transaction,
    WebExceptionCode& ec) {
  IndexedDBDispatcher* dispatcher =
      RenderThread::current()->indexed_db_dispatcher();
  dispatcher->RequestIDBIndexOpenObjectCursor(
      range, direction, callbacks,  idb_index_id_, transaction, &ec);
}

void RendererWebIDBIndexImpl::openKeyCursor(
    const WebKit::WebIDBKeyRange& range,
    unsigned short direction,
    WebKit::WebIDBCallbacks* callbacks,
    const WebKit::WebIDBTransaction& transaction,
    WebExceptionCode& ec) {
  IndexedDBDispatcher* dispatcher =
      RenderThread::current()->indexed_db_dispatcher();
  dispatcher->RequestIDBIndexOpenKeyCursor(
      range, direction, callbacks,  idb_index_id_, transaction, &ec);
}

void RendererWebIDBIndexImpl::getObject(
    const WebKit::WebIDBKey& key,
    WebKit::WebIDBCallbacks* callbacks,
    const WebKit::WebIDBTransaction& transaction,
    WebExceptionCode& ec) {
  IndexedDBDispatcher* dispatcher =
      RenderThread::current()->indexed_db_dispatcher();
  dispatcher->RequestIDBIndexGetObject(
      IndexedDBKey(key), callbacks, idb_index_id_, transaction, &ec);
}

void RendererWebIDBIndexImpl::getKey(
    const WebKit::WebIDBKey& key,
    WebKit::WebIDBCallbacks* callbacks,
    const WebKit::WebIDBTransaction& transaction,
    WebExceptionCode& ec) {
  IndexedDBDispatcher* dispatcher =
      RenderThread::current()->indexed_db_dispatcher();
  dispatcher->RequestIDBIndexGetKey(
      IndexedDBKey(key), callbacks, idb_index_id_, transaction, &ec);
}
