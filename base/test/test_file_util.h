// Copyright (c) 2010 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef BASE_TEST_TEST_FILE_UTIL_H_
#define BASE_TEST_TEST_FILE_UTIL_H_
#pragma once

// File utility functions used only by tests.

class FilePath;

namespace file_util {

// Wrapper over file_util::Delete. On Windows repeatedly invokes Delete in case
// of failure to workaround Windows file locking semantics. Returns true on
// success.
bool DieFileDie(const FilePath& file, bool recurse);

// Clear a specific file from the system cache. After this call, trying
// to access this file will result in a cold load from the hard drive.
bool EvictFileFromSystemCache(const FilePath& file);

// Like CopyFileNoCache but recursively copies all files and subdirectories
// in the given input directory to the output directory. Any files in the
// destination that already exist will be overwritten.
//
// Returns true on success. False means there was some error copying, so the
// state of the destination is unknown.
bool CopyRecursiveDirNoCache(const FilePath& source_dir,
                             const FilePath& dest_dir);

#if defined(OS_WIN)
// Returns true if the volume supports Alternate Data Streams.
bool VolumeSupportsADS(const FilePath& path);

// Returns true if the ZoneIdentifier is correctly set to "Internet" (3).
bool HasInternetZoneIdentifier(const FilePath& full_path);
#endif  // defined(OS_WIN)

}  // namespace file_util

#endif  // BASE_TEST_TEST_FILE_UTIL_H_
