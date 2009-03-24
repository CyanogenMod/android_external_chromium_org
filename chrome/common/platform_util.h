// Copyright (c) 2009 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_COMMON_PLATFORM_UTIL_H_
#define CHROME_COMMON_PLATFORM_UTIL_H_

class FilePath;

namespace platform_util {

// Show the given file in a file manager. If possible, select the file.
void ShowItemInFolder(const FilePath& full_path);

}

#endif  // CHROME_COMMON_PLATFORM_UTIL_H_
