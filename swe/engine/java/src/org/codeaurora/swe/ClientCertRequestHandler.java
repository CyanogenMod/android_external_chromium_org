/*
 * Copyright (c) 2013 The Linux Foundation. All rights reserved.
 * Not a contribution.
 *
 * Copyright (C) 2011 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package org.codeaurora.swe;

import android.os.Handler;
import java.security.PrivateKey;
import java.security.cert.CertificateEncodingException;
import java.security.cert.X509Certificate;

/**
 * ClientCertRequestHandler: class responsible for handling client
 * certificate requests.  This class is passed as a parameter to
 * BrowserCallback.displayClientCertRequestDialog and is meant to
 * receive the user's response.
 *
 * @hide
 */
public class ClientCertRequestHandler extends Handler {

    ClientCertRequestHandler() {
    }

    /**
     * Proceed with the specified private key and client certificate chain.
     */
    public void proceed(PrivateKey privateKey, X509Certificate[] chain) {
    }

    /**
     * Proceed with the specified private key bytes and client certificate chain.
     */
    private void setSslClientCertFromCtx(final long ctx, final byte[][] chainBytes) {
    }

    /**
     * Proceed with the specified private key context and client certificate chain.
     */
    private void setSslClientCertFromPKCS8(final byte[] key, final byte[][] chainBytes) {
    }

    /**
     * Igore the request for now, the user may be prompted again.
     */
    public void ignore() {
    }

    /**
     * Cancel this request, remember the users negative choice.
     */
    public void cancel() {
    }
}
