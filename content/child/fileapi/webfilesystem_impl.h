// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CONTENT_CHILD_FILEAPI_WEBFILESYSTEM_IMPL_H_
#define CONTENT_CHILD_FILEAPI_WEBFILESYSTEM_IMPL_H_

#include "base/basictypes.h"
#include "base/compiler_specific.h"
#include "base/memory/ref_counted.h"
#include "third_party/WebKit/public/platform/WebFileSystem.h"
#include "webkit/child/worker_task_runner.h"

namespace base {
class MessageLoopProxy;
}

namespace WebKit {
class WebURL;
class WebFileWriter;
class WebFileWriterClient;
}

namespace content {

class WebFileSystemImpl
    : public WebKit::WebFileSystem,
      public webkit_glue::WorkerTaskRunner::Observer {
 public:
  // Returns thread-specific instance.
  static WebFileSystemImpl* ThreadSpecificInstance(
      base::MessageLoopProxy* main_thread_loop);

  // Deletes thread-specific instance (if exists). For workers it deletes
  // itself in OnWorkerRunLoopStopped(), but for an instance created on the
  // main thread this method must be called.
  static void DeleteThreadSpecificInstance();

  explicit WebFileSystemImpl(base::MessageLoopProxy* main_thread_loop);
  virtual ~WebFileSystemImpl();

  // webkit_glue::WorkerTaskRunner::Observer implementation.
  virtual void OnWorkerRunLoopStopped() OVERRIDE;

  // WebFileSystem implementation.
  virtual void openFileSystem(
      const WebKit::WebURL& storage_partition,
      const WebKit::WebFileSystemType type,
      bool create,
      WebKit::WebFileSystemCallbacks*);
  virtual void deleteFileSystem(
      const WebKit::WebURL& storage_partition,
      const WebKit::WebFileSystemType type,
      WebKit::WebFileSystemCallbacks*);
  virtual void move(
      const WebKit::WebURL& src_path,
      const WebKit::WebURL& dest_path,
      WebKit::WebFileSystemCallbacks*) OVERRIDE;
  virtual void copy(
      const WebKit::WebURL& src_path,
      const WebKit::WebURL& dest_path,
      WebKit::WebFileSystemCallbacks*) OVERRIDE;
  virtual void remove(
      const WebKit::WebURL& path,
      WebKit::WebFileSystemCallbacks*) OVERRIDE;
  virtual void removeRecursively(
      const WebKit::WebURL& path,
      WebKit::WebFileSystemCallbacks*) OVERRIDE;
  virtual void readMetadata(
      const WebKit::WebURL& path,
      WebKit::WebFileSystemCallbacks*) OVERRIDE;
  virtual void createFile(
      const WebKit::WebURL& path,
      bool exclusive,
      WebKit::WebFileSystemCallbacks*) OVERRIDE;
  virtual void createDirectory(
      const WebKit::WebURL& path,
      bool exclusive,
      WebKit::WebFileSystemCallbacks*) OVERRIDE;
  virtual void fileExists(
      const WebKit::WebURL& path,
      WebKit::WebFileSystemCallbacks*) OVERRIDE;
  virtual void directoryExists(
      const WebKit::WebURL& path,
      WebKit::WebFileSystemCallbacks*) OVERRIDE;
  virtual void readDirectory(
      const WebKit::WebURL& path,
      WebKit::WebFileSystemCallbacks*) OVERRIDE;
  virtual WebKit::WebFileWriter* createFileWriter(
      const WebKit::WebURL& path, WebKit::WebFileWriterClient*) OVERRIDE;
  virtual void createFileWriter(
      const WebKit::WebURL& path,
      WebKit::WebFileWriterClient*,
      WebKit::WebFileSystemCallbacks*) OVERRIDE;
  virtual void createSnapshotFileAndReadMetadata(
      const WebKit::WebURL& path,
      WebKit::WebFileSystemCallbacks*);

 private:
  scoped_refptr<base::MessageLoopProxy> main_thread_loop_;
};

}  // namespace content

#endif  // CONTENT_CHILD_FILEAPI_WEBFILESYSTEM_IMPL_H_
