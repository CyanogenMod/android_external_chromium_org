// Copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_PRERENDER_PRERENDER_FINAL_STATUS_H_
#define CHROME_BROWSER_PRERENDER_PRERENDER_FINAL_STATUS_H_

namespace prerender {

// FinalStatus indicates whether |this| was used, or why it was cancelled.
// NOTE: New values need to be appended, since they are used in histograms.
enum FinalStatus {
  FINAL_STATUS_USED,
  FINAL_STATUS_TIMED_OUT,
  FINAL_STATUS_EVICTED,
  FINAL_STATUS_MANAGER_SHUTDOWN,
  FINAL_STATUS_CLOSED,
  FINAL_STATUS_CREATE_NEW_WINDOW,
  FINAL_STATUS_PROFILE_DESTROYED,
  FINAL_STATUS_APP_TERMINATING,
  FINAL_STATUS_JAVASCRIPT_ALERT,
  FINAL_STATUS_AUTH_NEEDED,
  FINAL_STATUS_HTTPS,
  FINAL_STATUS_DOWNLOAD,
  FINAL_STATUS_MEMORY_LIMIT_EXCEEDED,
  FINAL_STATUS_JS_OUT_OF_MEMORY,
  FINAL_STATUS_RENDERER_UNRESPONSIVE,
  FINAL_STATUS_TOO_MANY_PROCESSES,
  FINAL_STATUS_RATE_LIMIT_EXCEEDED,
  FINAL_STATUS_PENDING_SKIPPED,  // Obsolete.
  FINAL_STATUS_CONTROL_GROUP,
  FINAL_STATUS_HTML5_MEDIA,
  FINAL_STATUS_SOURCE_RENDER_VIEW_CLOSED,
  FINAL_STATUS_RENDERER_CRASHED,
  FINAL_STATUS_UNSUPPORTED_SCHEME,
  FINAL_STATUS_INVALID_HTTP_METHOD,
  FINAL_STATUS_WINDOW_PRINT,
  FINAL_STATUS_RECENTLY_VISITED,
  FINAL_STATUS_WINDOW_OPENER,
  FINAL_STATUS_PAGE_ID_CONFLICT,
  FINAL_STATUS_SAFE_BROWSING,
  FINAL_STATUS_FRAGMENT_MISMATCH,
  FINAL_STATUS_MAX,
};

void RecordFinalStatus(FinalStatus final_status);

}  // namespace prerender

#endif  // CHROME_BROWSER_PRERENDER_PRERENDER_FINAL_STATUS_H_
