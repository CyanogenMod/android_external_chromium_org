/*
 *  Copyright (c) 2013, The Linux Foundation. All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions are
 *  met:
 *      * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above
 *       copyright notice, this list of conditions and the following
 *       disclaimer in the documentation and/or other materials provided
 *       with the distribution.
 *     * Neither the name of The Linux Foundation nor the names of its
 *       contributors may be used to endorse or promote products derived
 *       from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED "AS IS" AND ANY EXPRESS OR IMPLIED
 *  WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 *  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT
 *  ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS
 *  BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 *  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 *  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
 *  BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 *  WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
 *  OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
 *  IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */

package org.chromium.android_webview;

import android.util.Log;

import java.security.cert.CertificateEncodingException;
import java.security.cert.X509Certificate;
import java.security.PrivateKey;

import org.chromium.base.CalledByNative;
import org.chromium.base.JNINamespace;
import org.chromium.base.ThreadUtils;


@JNINamespace("android_webview")
public class AwClientCertRequestHandler {

    static final String TAG = "SSLClientCertificateRequest";
    private long mNativeAwClientCertRequestHandler;

    public void proceed(final PrivateKey privateKey, final X509Certificate[] chain) {
        ThreadUtils.runOnUiThread(new Runnable() {
            @Override
            public void run() {
                if (mNativeAwClientCertRequestHandler != 0) {
                    // Get the encoded certificate chain.
                    byte[][] encoded_chain = new byte[chain.length][];
                    try {
                        for (int i = 0; i < chain.length; ++i) {
                            encoded_chain[i] = chain[i].getEncoded();
                    }
                    } catch (CertificateEncodingException e) {
                        Log.w(TAG, "Could not retrieve encoded certificate chain: " + e);
                        return;
                    }
                    nativeProceed(mNativeAwClientCertRequestHandler,
                                  encoded_chain,
                                  privateKey);
                    mNativeAwClientCertRequestHandler = 0;
                }
            }
        });
    }

    public void cancel() {
        if (mNativeAwClientCertRequestHandler != 0) {
            nativeCancel(mNativeAwClientCertRequestHandler);
            mNativeAwClientCertRequestHandler = 0;
        }
    }
    public static AwClientCertRequestHandler create(long nativeAwClientCertRequestHandler) {
        return new AwClientCertRequestHandler(nativeAwClientCertRequestHandler);
    }

    private AwClientCertRequestHandler(long nativeAwClientCertRequestHandler) {
        mNativeAwClientCertRequestHandler = nativeAwClientCertRequestHandler;
    }

    private native void nativeProceed(long nativeAwClientCertRequestHandler,
                                      byte[][] certChain,
                                      PrivateKey privateKey);
    private native void nativeCancel(long nativeAwClientCertRequestHandler);

}
