// Copyright 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.chromium.android_webview;

import android.os.CancellationSignal;
import android.webkit.ValueCallback;

import java.io.OutputStream;

import org.chromium.base.CalledByNative;
import org.chromium.base.JNINamespace;

/**
 * Export the android webview as a PDF.
 * @TODO(sgurun) explain the ownership of this class and its native counterpart
 */
@JNINamespace("android_webview")
public class AwPdfExporter {

    private AwContents mAwContents;
    private int mNativeAwPdfExporter;

    AwPdfExporter(AwContents awContents) {
        mAwContents = awContents;
    }

    public void exportToPdf(final OutputStream stream, int width, int height,
            ValueCallback<Boolean> resultCallback, CancellationSignal cancellationSignal) {

        if (stream == null) {
            throw new IllegalArgumentException("Stream cannot be null");
        }
        if (width < 1 || height < 1) {
            throw new IllegalArgumentException("width and height must be larger than 0");
        }
        if (resultCallback == null) {
            throw new IllegalArgumentException("Callback cannot be null");
        }
        nativeExportToPdf(mNativeAwPdfExporter, stream, width, height,
                mAwContents.computeHorizontalScrollRange(),
                mAwContents.computeVerticalScrollRange(),
                resultCallback, cancellationSignal);
    }

    @CalledByNative
    private void setNativeAwPdfExporter(int nativePdfExporter) {
        mNativeAwPdfExporter = nativePdfExporter;
    }

    @CalledByNative
    private void didExportPdf(ValueCallback<Boolean> resultCallback, boolean success) {
        resultCallback.onReceiveValue(success);
    }

    private native void nativeExportToPdf(int nativeAwPdfExporter, OutputStream outStream,
            int width, int height, int horizontal_scroll_range, int vertical_scroll_range,
            ValueCallback<Boolean> resultCallback, CancellationSignal cancellationSignal);

}
