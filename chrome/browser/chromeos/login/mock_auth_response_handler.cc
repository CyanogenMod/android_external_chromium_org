// Copyright (c) 2010 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/chromeos/login/mock_auth_response_handler.h"

#include <string>

#include "chrome/browser/net/url_fetcher.h"
#include "googleurl/src/gurl.h"
#include "net/url_request/url_request_status.h"
#include "testing/gmock/include/gmock/gmock.h"

namespace chromeos {

using ::testing::_;
using ::testing::Invoke;

MockAuthResponseHandler::MockAuthResponseHandler(const GURL& url,
                                                 const URLRequestStatus& status,
                                                 const int code,
                                                 const std::string& data)
    : remote_(url),
      status_(status),
      http_response_code_(code),
      data_(data) {
  // Take the args sent to Handle() and pass them to MockNetwork(), which will
  // use the data passed to the constructor here to fill out the call to
  // OnURLFetchComplete().
  ON_CALL(*this, Handle(_,_))
      .WillByDefault(Invoke(this, &MockAuthResponseHandler::MockNetwork));
}

URLFetcher* MockAuthResponseHandler::MockNetwork(
    std::string data,
    URLFetcher::Delegate* delegate) {
  delegate->OnURLFetchComplete(NULL,
                               remote_,
                               status_,
                               http_response_code_,
                               ResponseCookies(),
                               data_);
  return new URLFetcher(GURL(), URLFetcher::GET, delegate);
}

}  // namespace chromeos
