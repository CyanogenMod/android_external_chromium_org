// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "android_webview/native/skia_java_output_stream.h"

#include "base/logging.h"

// Disable "Warnings treated as errors" for input_stream_jni as it's a Java
// system class and we have to generate C++ hooks for all methods in the class
// even if they're unused.
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-function"
#include "jni/OutputStream_jni.h"
#include "jni/CancellationSignal_jni.h"
#pragma GCC diagnostic pop
using base::android::ClearException;
using base::android::JavaRef;
using JNI_OutputStream::Java_OutputStream_writeV_AB_I_I;
using JNI_OutputStream::Java_OutputStream_flush;
using JNI_CancellationSignal::Java_CancellationSignal_isCanceled;

namespace android_webview {

const size_t SkiaJavaOutputStream::kBufferSize = 4096;

SkiaJavaOutputStream::SkiaJavaOutputStream(
    JNIEnv* env,
    const JavaRef<jobject>& stream,
    const JavaRef<jobject>& cancel_signal)
    : env_(env),
      stream_(stream),
      cancel_signal_(cancel_signal) {
}

SkiaJavaOutputStream::~SkiaJavaOutputStream() { }

bool SkiaJavaOutputStream::write(const void* buffer, size_t size) {

  if (buffer == NULL) {
    LOG(WARNING) << "write: buffer pointer is NULL";
    return false;
  }

  if (!buffer_.obj()) {
    // Allocate transfer buffer.
    base::android::ScopedJavaLocalRef<jbyteArray> temp(
        env_, env_->NewByteArray(kBufferSize));
    buffer_.Reset(temp);
    if (ClearException(env_))
      return false;
  }
  const jbyte* bufptr = reinterpret_cast<const jbyte*>(buffer);
  while (size > 0) {
    if (cancel_signal_.obj() != NULL &&
        Java_CancellationSignal_isCanceled(env_, cancel_signal_.obj())) {
      return false;
    }
    size_t requested = size;
    if (requested > kBufferSize) {
      requested = kBufferSize;
    }

    env_->SetByteArrayRegion(buffer_.obj(), 0, requested, bufptr);
    if (ClearException(env_)) {
      LOG(WARNING) << "write:SetByteArrayRegion threw an exception";
      return false;
    }

    Java_OutputStream_writeV_AB_I_I(
        env_, stream_.obj(), buffer_.obj(), 0, requested);
    if (ClearException(env_)) {
      LOG(WARNING) << "write:write threw an exception";
      return false;
    }
    bufptr += requested;
    size -= requested;
  }

  return true;
}

void SkiaJavaOutputStream::flush() {
  Java_OutputStream_flush(env_, stream_.obj());
  if (ClearException(env_)) {
      LOG(WARNING) << "flush threw an exception";
  }
}


bool RegisterSkiaJavaOutputStream(JNIEnv* env) {
  return JNI_OutputStream::RegisterNativesImpl(env) &&
      JNI_CancellationSignal::RegisterNativesImpl(env);
}

}  // namespace android_webview
