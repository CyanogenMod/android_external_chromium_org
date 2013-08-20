// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ANDROID_WEBVIEW_NATIVE_AW_PDF_EXPORTER_H_
#define ANDROID_WEBVIEW_NATIVE_AW_PDF_EXPORTER_H_

#include <jni.h>

#include "base/android/jni_helper.h"
#include "base/android/scoped_java_ref.h"
#include "base/basictypes.h"
#include "base/memory/weak_ptr.h"
#include "third_party/skia/include/core/SkPicture.h"

namespace android_webview {

class BrowserViewRenderer;

class AwPdfExporter {

 public:
  AwPdfExporter(JNIEnv* env, jobject obj, BrowserViewRenderer* view_renderer);

  virtual ~AwPdfExporter();

  void ExportToPdf(JNIEnv* env,
                   jobject obj,
                   jobject outStream,
                   int width,
                   int height,
                   int horizontal_scroll_range,
                   int vertical_scroll_range,
                   jobject callback,
                   jobject cancel_signal);

 private:
  struct PrintParams {
    int width;
    int height;
    int horizontal_scroll_range;
    int vertical_scroll_range;
  };

  static void ExportPage(
      const base::WeakPtr<AwPdfExporter>& self,
      PrintParams params,
      const skia::RefPtr<SkPicture>& picture,
      const base::android::ScopedJavaGlobalRef<jobject>& stream,
      const base::android::ScopedJavaGlobalRef<jobject>& callback,
      const base::android::ScopedJavaGlobalRef<jobject>& cancel_signal);

  void DidExportPdf(
      const base::android::ScopedJavaGlobalRef<jobject>& callback,
      int success);

  JavaObjectWeakGlobalRef java_ref_;
  BrowserViewRenderer* view_renderer_;
  base::WeakPtrFactory<AwPdfExporter> weak_factory_;

  DISALLOW_COPY_AND_ASSIGN(AwPdfExporter);
};

bool RegisterAwPdfExporter(JNIEnv* env);

}  // namespace android_webview

#endif  // ANDROID_WEBVIEW_NATIVE_AW_PDF_EXPORTER_H_
