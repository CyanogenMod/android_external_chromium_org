// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/extensions/api/system_storage/storage_api_test_util.h"

#include "base/strings/utf_string_conversions.h"

namespace extensions {
namespace test {

const struct TestStorageUnitInfo kRemovableStorageData = {
    "dcim:device:001", "/media/usb1", 4098, 1000
};

StorageInfo BuildStorageInfoFromTestStorageUnitInfo(
    const TestStorageUnitInfo& unit) {
  return StorageInfo(
      unit.device_id,
      base::UTF8ToUTF16(unit.name),
      base::FilePath::StringType(), /* no location */
      base::string16(), /* no storage label */
      base::string16(), /* no storage vendor */
      base::string16(), /* no storage model */
      unit.capacity);
}

}  // namespace test
}  // namespace extensions
