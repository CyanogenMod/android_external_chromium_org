// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "android_webview/browser/browser_view_renderer.h"
#include "android_webview/native/aw_pdf_exporter.h"
#include "android_webview/native/skia_java_output_stream.h"
#include "base/android/jni_android.h"
#include "base/logging.h"
#include "content/public/browser/browser_thread.h"
#include "jni/AwPdfExporter_jni.h"
#include "third_party/skia/include/pdf/SkPDFDevice.h"
#include "third_party/skia/include/pdf/SkPDFDocument.h"
#include "third_party/skia/include/core/SkPicture.h"

using base::android::AttachCurrentThread;
using base::android::ScopedJavaGlobalRef;
using base::WeakPtr;
using content::BrowserThread;

namespace android_webview {

AwPdfExporter::AwPdfExporter(JNIEnv* env,
                             jobject obj,
                             BrowserViewRenderer* view_renderer)
    : java_ref_(env, obj),
      view_renderer_(view_renderer),
      weak_factory_(this) {

  DCHECK(obj);
  Java_AwPdfExporter_setNativeAwPdfExporter(
      env, obj, reinterpret_cast<jint>(this));
}

AwPdfExporter::~AwPdfExporter() {
  JNIEnv* env = AttachCurrentThread();
  ScopedJavaLocalRef<jobject> obj = java_ref_.get(env);
  if (obj.is_null())
    return;
  // Clear the Java peer's weak pointer to |this| object.
  Java_AwPdfExporter_setNativeAwPdfExporter(env, obj.obj(), 0);
}

void AwPdfExporter::ExportToPdf(JNIEnv* env,
                                jobject obj,
                                jobject output_stream,
                                int width,
                                int height,
                                int horizontal_scroll_range,
                                int vertical_scroll_range,
                                jobject callback,
                                jobject cancel_signal) {
  DCHECK(BrowserThread::CurrentlyOn(BrowserThread::UI));
  skia::RefPtr<SkPicture> picture =
      view_renderer_->CapturePicture(horizontal_scroll_range,
                                     vertical_scroll_range);
  // TODO(sgurun) check if the picture is valid

  PrintParams params;
  params.width = width;
  params.height = height = height;
  params.horizontal_scroll_range = horizontal_scroll_range;
  params.vertical_scroll_range = vertical_scroll_range;

  ScopedJavaGlobalRef<jobject> jstream;
  jstream.Reset(env, output_stream);
  ScopedJavaGlobalRef<jobject> jcallback;
  jcallback.Reset(env, callback);
  ScopedJavaGlobalRef<jobject> jcancel_signal;
  jcancel_signal.Reset(env, cancel_signal);
  BrowserThread::PostBlockingPoolTask(
      FROM_HERE,
      base::Bind(&AwPdfExporter::ExportPage,
                 weak_factory_.GetWeakPtr(),
                 params,
                 picture,
                 jstream,
                 jcallback,
                 jcancel_signal));
}

// TODO(sgurun) rather than scaling all content to a single page,
// split into multiple pages.
// static
void AwPdfExporter::ExportPage(
    const WeakPtr<AwPdfExporter>& self,
    PrintParams params,
    const skia::RefPtr<SkPicture>& picture,
    const ScopedJavaGlobalRef<jobject>& stream,
    const ScopedJavaGlobalRef<jobject>& callback,
    const ScopedJavaGlobalRef<jobject>& cancel_signal) {

  SkMatrix transform;
  float scale_factor_x = params.horizontal_scroll_range / params.width;
  float scale_factor_y = params.vertical_scroll_range / params.height;
  transform.setScale(SkFloatToScalar(1 / scale_factor_x),
                     SkFloatToScalar(1 / scale_factor_y));
  SkISize pageSize = SkISize::Make(params.width, params.height);
  SkISize content_size = SkISize::Make(params.horizontal_scroll_range,
                                       params.vertical_scroll_range);
  SkPDFDevice* device = new SkPDFDevice(pageSize, content_size, transform);
  SkCanvas canvas(device);
  picture->draw(&canvas);
  SkPDFDocument doc;
  doc.appendPage(device);

  JNIEnv* env = AttachCurrentThread();
  SkiaJavaOutputStream sk_stream(env, stream, cancel_signal);
  bool success = doc.emitPDF(&sk_stream);
  BrowserThread::PostTask(
      content::BrowserThread::UI,
      FROM_HERE,
      base::Bind(&AwPdfExporter::DidExportPdf,
                 self,
                 callback,
                 success));
}

void AwPdfExporter::DidExportPdf(
    const ScopedJavaGlobalRef<jobject>& callback,
    int success) {
  JNIEnv* env = AttachCurrentThread();
  ScopedJavaLocalRef<jobject> obj = java_ref_.get(env);
  if (obj.is_null())
    return;
  Java_AwPdfExporter_didExportPdf(env, obj.obj(), callback.obj(), success);
}

bool RegisterAwPdfExporter(JNIEnv* env) {
  return RegisterNativesImpl(env) >= 0;
}

}  // namespace android_webview
