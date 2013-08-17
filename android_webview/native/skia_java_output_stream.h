// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ANDROID_WEBVIEW_BROWSER_SKIA_JAVA_OUTPUT_STREAM_H_
#define ANDROID_WEBVIEW_BROWSER_SKIA_JAVA_OUTPUT_STREAM_H_

#include "base/android/jni_android.h"
#include "base/android/scoped_java_ref.h"
#include "third_party/skia/include/core/SkStream.h"

namespace android_webview {

// Wraps a SkWStream around a Java OutputStream.
class SkiaJavaOutputStream : public SkWStream {
 public:
  // Maximum size of |jbuffer_|.
  static const size_t kBufferSize;

  SkiaJavaOutputStream(JNIEnv* env,
                       const base::android::JavaRef<jobject>& stream,
                       const base::android::JavaRef<jobject>& cancel_signal);
  virtual ~SkiaJavaOutputStream();

  // Override SkWStream methods
  virtual bool write(const void* buffer, size_t size) OVERRIDE;
  virtual void flush() OVERRIDE;

 private:
  JNIEnv* env_;
  base::android::ScopedJavaGlobalRef<jobject> stream_;
  base::android::ScopedJavaGlobalRef<jobject> cancel_signal_;
  base::android::ScopedJavaGlobalRef<jbyteArray> buffer_;

  DISALLOW_COPY_AND_ASSIGN(SkiaJavaOutputStream);
};

bool RegisterSkiaJavaOutputStream(JNIEnv* env);

}

#endif  // ANDROID_WEBVIEW_BROWSER_SKIA_JAVA_OUTPUT_STREAM_H_
