// Copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef PPAPI_CPP_TRUSTED_FILE_IO_TRUSTED_H_
#define PPAPI_CPP_TRUSTED_FILE_IO_TRUSTED_H_

#include <string>

#include "ppapi/c/pp_stdint.h"

namespace pp {

class FileIO;
class CompletionCallback;

class FileIO_Trusted {
 public:
  /// Creates a FileIO_Trusted object.
  FileIO_Trusted();

  int32_t GetOSFileDescriptor(const FileIO& file_io);
};

}  // namespace pp

#endif  // PPAPI_CPP_TRUSTED_FILE_IO_TRUSTED_H_
