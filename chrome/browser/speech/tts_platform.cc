// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/speech/tts_platform.h"

#include <string>

bool TtsPlatformImpl::LoadBuiltInTtsExtension(Profile* profile) {
  return false;
}

std::string TtsPlatformImpl::gender() {
  return std::string();
}

std::string TtsPlatformImpl::error() {
  return error_;
}

void TtsPlatformImpl::clear_error() {
  error_ = std::string();
}

void TtsPlatformImpl::set_error(const std::string& error) {
  error_ = error;
}
