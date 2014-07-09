// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "base/message_loop/message_loop.h"
#include "chrome/browser/extensions/api/file_system/file_system_api.h"
#include "chrome/browser/extensions/api/image_writer_private/operation.h"
#include "chrome/browser/extensions/api/image_writer_private/removable_storage_provider.h"
#include "chrome/browser/extensions/api/image_writer_private/test_utils.h"
#include "chrome/browser/extensions/extension_apitest.h"
#include "chrome/common/extensions/api/image_writer_private.h"
#include "content/public/browser/browser_thread.h"

namespace extensions {

using api::image_writer_private::RemovableStorageDevice;
using extensions::image_writer::FakeImageWriterClient;

class ImageWriterPrivateApiTest : public ExtensionApiTest {
 public:
  virtual void SetUpInProcessBrowserTestFixture() OVERRIDE {
    ExtensionApiTest::SetUpInProcessBrowserTestFixture();
    test_utils_.SetUp(true);

    ASSERT_TRUE(test_utils_.FillFile(test_utils_.GetImagePath(),
                                     image_writer::kImagePattern,
                                     image_writer::kTestFileSize));
    ASSERT_TRUE(test_utils_.FillFile(test_utils_.GetDevicePath(),
                                     image_writer::kDevicePattern,
                                     image_writer::kTestFileSize));

    scoped_refptr<StorageDeviceList> device_list(new StorageDeviceList);

    RemovableStorageDevice* expected1 = new RemovableStorageDevice();
    expected1->vendor = "Vendor 1";
    expected1->model = "Model 1";
    expected1->capacity = image_writer::kTestFileSize;
#if defined(OS_WIN)
    expected1->storage_unit_id = test_utils_.GetDevicePath().AsUTF8Unsafe();
#else
    expected1->storage_unit_id = test_utils_.GetDevicePath().value();
#endif

    RemovableStorageDevice* expected2 = new RemovableStorageDevice();
    expected2->vendor = "Vendor 2";
    expected2->model = "Model 2";
    expected2->capacity = image_writer::kTestFileSize << 2;
#if defined(OS_WIN)
    expected2->storage_unit_id = test_utils_.GetDevicePath().AsUTF8Unsafe();
#else
    expected2->storage_unit_id = test_utils_.GetDevicePath().value();
#endif

    linked_ptr<RemovableStorageDevice> device1(expected1);
    device_list->data.push_back(device1);
    linked_ptr<RemovableStorageDevice> device2(expected2);
    device_list->data.push_back(device2);

    RemovableStorageProvider::SetDeviceListForTesting(device_list);
  }

  virtual void TearDownInProcessBrowserTestFixture() OVERRIDE {
    ExtensionApiTest::TearDownInProcessBrowserTestFixture();
    test_utils_.TearDown();
    RemovableStorageProvider::ClearDeviceListForTesting();
    FileSystemChooseEntryFunction::StopSkippingPickerForTest();
  }

#if !defined(OS_CHROMEOS)
  void ImageWriterUtilityClientCall() {
    content::BrowserThread::PostTask(
        content::BrowserThread::FILE,
        FROM_HERE,
        base::Bind(&FakeImageWriterClient::Progress,
                   test_utils_.GetUtilityClient(),
                   0));
    content::BrowserThread::PostTask(
        content::BrowserThread::FILE,
        FROM_HERE,
        base::Bind(&FakeImageWriterClient::Progress,
                   test_utils_.GetUtilityClient(),
                   50));
    content::BrowserThread::PostTask(
        content::BrowserThread::FILE,
        FROM_HERE,
        base::Bind(&FakeImageWriterClient::Progress,
                   test_utils_.GetUtilityClient(),
                   100));
    content::BrowserThread::PostTask(
        content::BrowserThread::FILE,
        FROM_HERE,
        base::Bind(&FakeImageWriterClient::Success,
                   test_utils_.GetUtilityClient()));
  }
#endif

 protected:
  image_writer::ImageWriterTestUtils test_utils_;
};

IN_PROC_BROWSER_TEST_F(ImageWriterPrivateApiTest, TestListDevices) {
  ASSERT_TRUE(RunExtensionTest("image_writer_private/list_devices"))
      << message_;
}

IN_PROC_BROWSER_TEST_F(ImageWriterPrivateApiTest, TestWriteFromFile) {
  FileSystemChooseEntryFunction::RegisterTempExternalFileSystemForTest(
      "test_temp", test_utils_.GetTempDir());

  base::FilePath selected_image(test_utils_.GetImagePath());
  FileSystemChooseEntryFunction::SkipPickerAndAlwaysSelectPathForTest(
      &selected_image);

#if !defined(OS_CHROMEOS)
  test_utils_.GetUtilityClient()->SetWriteCallback(base::Bind(
      &ImageWriterPrivateApiTest::ImageWriterUtilityClientCall, this));
  test_utils_.GetUtilityClient()->SetVerifyCallback(base::Bind(
      &ImageWriterPrivateApiTest::ImageWriterUtilityClientCall, this));
#endif

  ASSERT_TRUE(RunPlatformAppTest("image_writer_private/write_from_file"))
      << message_;
}

}  // namespace extensions
