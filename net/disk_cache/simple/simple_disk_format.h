// Copyright (c) 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef NET_DISK_CACHE_SIMPLE_SIMPLE_DISK_FORMAT_H_
#define NET_DISK_CACHE_SIMPLE_SIMPLE_DISK_FORMAT_H_

namespace disk_cache {

const uint64 kSimpleInitialMagicNumber = 0xfcfb6d1ba7725c30;

// A file in the Simple cache consists of a SimpleFileHeader followed
// by data.

const uint32 kSimpleVersion = 1;

struct SimpleFileHeader {
  uint64 initial_magic_number;
  uint32 version;
  uint32 key_length;
  uint32 key_hash;
};

}  // namespace disk_cache

#endif  // NET_DISK_CACHE_SIMPLE_SIMPLE_DISK_FORMAT_H_
