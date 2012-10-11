// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "webkit/fileapi/syncable/local_file_sync_status.h"

#include "base/logging.h"

namespace fileapi {

LocalFileSyncStatus::LocalFileSyncStatus() {}

LocalFileSyncStatus::~LocalFileSyncStatus() {}

bool LocalFileSyncStatus::TryIncrementWriting(const FileSystemURL& url) {
  DCHECK(CalledOnValidThread());
  if (IsChildOrParentSyncing(url))
    return false;
  writing_[url]++;
  return true;
}

void LocalFileSyncStatus::DecrementWriting(const FileSystemURL& url) {
  DCHECK(CalledOnValidThread());
  int count = --writing_[url];
  if (count == 0) {
    writing_.erase(url);
    // TODO(kinuko): fire NeedsSynchronization notification.
  }
}

bool LocalFileSyncStatus::TryDisableWriting(const FileSystemURL& url) {
  DCHECK(CalledOnValidThread());
  if (IsChildOrParentWriting(url))
    return false;
  syncing_.insert(url);
  return true;
}

void LocalFileSyncStatus::EnableWriting(const FileSystemURL& url) {
  DCHECK(CalledOnValidThread());
  syncing_.erase(url);
}

bool LocalFileSyncStatus::IsWriting(const FileSystemURL& url) const {
  DCHECK(CalledOnValidThread());
  return IsChildOrParentWriting(url);
}

bool LocalFileSyncStatus::IsWritable(const FileSystemURL& url) const {
  DCHECK(CalledOnValidThread());
  return !IsChildOrParentSyncing(url);
}

bool LocalFileSyncStatus::IsChildOrParentWriting(
    const FileSystemURL& url) const {
  DCHECK(CalledOnValidThread());
  URLCountMap::const_iterator upper = writing_.upper_bound(url);
  URLCountMap::const_reverse_iterator rupper(upper);
  if (upper != writing_.end() && url.IsParent(upper->first))
    return true;
  if (rupper != writing_.rend() &&
      (rupper->first == url || rupper->first.IsParent(url)))
    return true;
  return false;
}

bool LocalFileSyncStatus::IsChildOrParentSyncing(
    const FileSystemURL& url) const {
  DCHECK(CalledOnValidThread());
  URLSet::const_iterator upper = syncing_.upper_bound(url);
  URLSet::const_reverse_iterator rupper(upper);
  if (upper != syncing_.end() && url.IsParent(*upper))
    return true;
  if (rupper != syncing_.rend() &&
      (*rupper == url || rupper->IsParent(url)))
    return true;
  return false;
}

}  // namespace fileapi
