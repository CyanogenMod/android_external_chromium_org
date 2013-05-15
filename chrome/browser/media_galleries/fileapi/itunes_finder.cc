// Copyright (c) 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/media_galleries/fileapi/itunes_finder.h"

#include "base/bind.h"
#include "base/logging.h"
#include "chrome/browser/storage_monitor/media_storage_util.h"
#include "content/public/browser/browser_thread.h"

#if defined(OS_WIN)
#include "chrome/browser/media_galleries/fileapi/itunes_finder_win.h"
#elif defined(OS_MACOSX)
#include "chrome/browser/media_galleries/fileapi/itunes_finder_mac.h"
#endif

namespace itunes {

ITunesFinder::~ITunesFinder() {}

// static
void ITunesFinder::FindITunesLibrary(const ITunesFinderCallback& callback) {
  DCHECK(content::BrowserThread::CurrentlyOn(content::BrowserThread::UI));
  ITunesFinder* finder = NULL;
#if defined(OS_WIN)
  finder = new ITunesFinderWin(callback);
#elif defined(OS_MACOSX)
  finder = new ITunesFinderMac(callback);
#endif
  if (finder) {
    content::BrowserThread::PostTask(
        content::BrowserThread::FILE,
        FROM_HERE,
        base::Bind(&ITunesFinder::FindITunesLibraryOnFileThread,
                   base::Unretained(finder)));
  }
}

ITunesFinder::ITunesFinder(const ITunesFinderCallback& callback)
    : callback_(callback) {
}

void ITunesFinder::PostResultToUIThread(const std::string& unique_id) {
  DCHECK(content::BrowserThread::CurrentlyOn(content::BrowserThread::FILE));
  // The use of base::Owned() below will cause this class to delete as soon as
  // FinishOnUIThread() finishes.
  content::BrowserThread::PostTask(
      content::BrowserThread::UI,
      FROM_HERE,
      base::Bind(&ITunesFinder::FinishOnUIThread,
                 base::Owned(this),
                 unique_id));
}

void ITunesFinder::FinishOnUIThread(const std::string& unique_id) const {
  DCHECK(content::BrowserThread::CurrentlyOn(content::BrowserThread::UI));

  using chrome::MediaStorageUtil;
  if (!unique_id.empty()) {
    callback_.Run(
        MediaStorageUtil::MakeDeviceId(MediaStorageUtil::ITUNES, unique_id));
  }
}

}  // namespace itunes
