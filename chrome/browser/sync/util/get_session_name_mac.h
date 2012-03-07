// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_SYNC_UTIL_GET_SESSION_NAME_MAC_H_
#define CHROME_BROWSER_SYNC_UTIL_GET_SESSION_NAME_MAC_H_
#pragma once

#include <string>

namespace browser_sync {
namespace internal {

// Returns the Hardware model name, without trailing numbers, if
// possible.  See http://www.cocoadev.com/index.pl?MacintoshModels for
// an example list of models. If an error occurs trying to read the
// model, this simply returns "Unknown".
std::string GetHardwareModelName();

}  // namespace internal
}  // namespace browser_sync

#endif  // CHROME_BROWSER_SYNC_UTIL_GET_SESSION_NAME_MAC_H_
