// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_CHROMEOS_DRIVE_FILEAPI_FILE_SYSTEM_BACKEND_DELEGATE_H_
#define CHROME_BROWSER_CHROMEOS_DRIVE_FILEAPI_FILE_SYSTEM_BACKEND_DELEGATE_H_

#include "base/basictypes.h"
#include "base/memory/scoped_ptr.h"
#include "chrome/browser/chromeos/fileapi/file_system_backend_delegate.h"

namespace fileapi {
class AsyncFileUtil;
}  // namespace fileapi

namespace drive {

// Delegate implementation of the some methods in chromeos::FileSystemBackend
// for Drive file system.
class FileSystemBackendDelegate : public chromeos::FileSystemBackendDelegate {
 public:
  FileSystemBackendDelegate();
  virtual ~FileSystemBackendDelegate();

  // FileSystemBackend::Delegate overrides.
  virtual fileapi::AsyncFileUtil* GetAsyncFileUtil(
      fileapi::FileSystemType type) OVERRIDE;
  virtual scoped_ptr<webkit_blob::FileStreamReader> CreateFileStreamReader(
      const fileapi::FileSystemURL& url,
      int64 offset,
      const base::Time& expected_modification_time,
      fileapi::FileSystemContext* context) OVERRIDE;
  virtual scoped_ptr<fileapi::FileStreamWriter> CreateFileStreamWriter(
      const fileapi::FileSystemURL& url,
      int64 offset,
      fileapi::FileSystemContext* context) OVERRIDE;

 private:
  scoped_ptr<fileapi::AsyncFileUtil> async_file_util_;

  DISALLOW_COPY_AND_ASSIGN(FileSystemBackendDelegate);
};

}  // namespace drive

#endif  // CHROME_BROWSER_CHROMEOS_DRIVE_FILEAPI_FILE_SYSTEM_BACKEND_DELEGATE_H_
