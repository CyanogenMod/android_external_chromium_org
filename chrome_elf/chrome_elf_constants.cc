// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome_elf/chrome_elf_constants.h"

const wchar_t kUserDataDirName[] = L"User Data";

#if defined(GOOGLE_CHROME_BUILD)
const wchar_t kAppDataDirName[] = L"Google\\Chrome";
#else
const wchar_t kAppDataDirName[] = L"Chromium";
#endif
