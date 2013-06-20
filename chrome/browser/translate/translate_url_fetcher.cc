// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/translate/translate_url_fetcher.h"

#include "chrome/browser/browser_process.h"
#include "net/base/load_flags.h"
#include "net/http/http_status_code.h"
#include "net/url_request/url_fetcher.h"
#include "net/url_request/url_request_status.h"

namespace {

// Retry parameter for fetching.
const int kMaxRetry = 5;

}  // namespace

TranslateURLFetcher::TranslateURLFetcher(int id)
    : id_(id),
      state_(IDLE) {
}

TranslateURLFetcher::~TranslateURLFetcher() {
}

bool TranslateURLFetcher::Request(
    const GURL& url,
    const TranslateURLFetcher::Callback& callback) {
  // This function is not supporsed to be called before previous operaion is not
  // finished.
  if (state_ == REQUESTING) {
    NOTREACHED();
    return false;
  }

  state_ = REQUESTING;
  url_ = url;
  callback_ = callback;

  fetcher_.reset(net::URLFetcher::Create(
      id_,
      url_,
      net::URLFetcher::GET,
      this));
  fetcher_->SetLoadFlags(net::LOAD_DO_NOT_SEND_COOKIES |
                         net::LOAD_DO_NOT_SAVE_COOKIES);
  fetcher_->SetRequestContext(g_browser_process->system_request_context());
  fetcher_->SetMaxRetriesOn5xx(kMaxRetry);
  fetcher_->Start();

  return true;
}

void TranslateURLFetcher::OnURLFetchComplete(const net::URLFetcher* source) {
  DCHECK(fetcher_.get() == source);

  std::string data;
  if (source->GetStatus().status() == net::URLRequestStatus::SUCCESS &&
      source->GetResponseCode() == net::HTTP_OK) {
    state_ = COMPLETED;
    source->GetResponseAsString(&data);
  } else {
    state_ = FAILED;
  }

  scoped_ptr<const net::URLFetcher> delete_ptr(fetcher_.release());
  callback_.Run(id_, state_ == COMPLETED, data);
}
