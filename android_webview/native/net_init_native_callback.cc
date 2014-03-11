// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "android_webview/browser/net/init_native_callback.h"

#include "android_webview/browser/net/aw_url_request_job_factory.h"
#include "android_webview/native/android_protocol_handler.h"
#include "android_webview/native/cookie_manager.h"
#include "base/logging.h"
#include "net/url_request/url_request_interceptor.h"

namespace android_webview {

//SWE-feature-incognito
void DidCreateCookieMonster(net::CookieMonster* cookie_monster) {
  DCHECK(cookie_monster);
  SetCookieMonsterOnNetworkStackInit(cookie_monster);
}

void DidCreateIncognitoCookieMonster(net::CookieMonster* incognito_cookie_monster) {
  DCHECK(incognito_cookie_monster);
  SetIncognitoCookieMonsterOnNetworkStackInit(incognito_cookie_monster);
}
//SWE-feature-incognito

scoped_ptr<net::URLRequestInterceptor>
CreateAndroidAssetFileRequestInterceptor() {
  return CreateAssetFileRequestInterceptor();
}

scoped_ptr<net::URLRequestInterceptor>
CreateAndroidContentRequestInterceptor() {
  return CreateContentSchemeRequestInterceptor();
}

}  // namespace android_webview
