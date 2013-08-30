// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.chromium.android_webview;

import android.os.CancellationSignal;
import android.os.ParcelFileDescriptor;
import android.util.Log;
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

    private static final String TAG = "AwPdfExporter";
    private int mNativeAwPdfExporter;
    // TODO(sgurun) result callback should return an int/object indicating errors.
    // potential errors: invalid print parameters, already pending, IO error
    private ValueCallback<Boolean> mResultCallback;

    private AwPdfExportAttributes mAttributes;

    private ParcelFileDescriptor mFd;

    AwPdfExporter() { }

    public void exportToPdf(final ParcelFileDescriptor fd, AwPdfExportAttributes attributes,
            ValueCallback<Boolean> resultCallback, CancellationSignal cancellationSignal)
            throws java.io.IOException {

        if (fd == null) {
            throw new IllegalArgumentException("fd cannot be null");
        }
        if (resultCallback == null) {
            throw new IllegalArgumentException("resultCallback cannot be null");
        }
        if (mResultCallback != null) {
            throw new IllegalStateException("printing is already pending");
        }
        mResultCallback = resultCallback;
        mAttributes = attributes;
        mFd = fd;
        nativeExportToPdf(mNativeAwPdfExporter, mFd.getFd(), cancellationSignal);
    }

    @CalledByNative
    private void setNativeAwPdfExporter(int nativePdfExporter) {
        mNativeAwPdfExporter = nativePdfExporter;
    }

    @CalledByNative
    private void didExportPdf(boolean success) {
        mResultCallback.onReceiveValue(success);
        mResultCallback = null;
        mAttributes = null;
        // The caller should close the file.
        mFd = null;
    }

    @CalledByNative
    private int getPageWidth() {
        return mAttributes.pageWidth;
    }

    @CalledByNative
    private int getPageHeight() {
        return mAttributes.pageHeight;
    }

    @CalledByNative
    private int getDpi() {
        return mAttributes.dpi;
    }

    @CalledByNative
    private int getLeftMargin() {
        return mAttributes.leftMargin;
    }

    @CalledByNative
    private int getRightMargin() {
        return mAttributes.rightMargin;
    }

    @CalledByNative
    private int getTopMargin() {
        return mAttributes.topMargin;
    }

    @CalledByNative
    private int getBottomMargin() {
        return mAttributes.bottomMargin;
    }

    private native void nativeExportToPdf(int nativeAwPdfExporter, int fd,
            CancellationSignal cancellationSignal);
}
