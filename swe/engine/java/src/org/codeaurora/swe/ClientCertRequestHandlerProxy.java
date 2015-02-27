/*
 *  Copyright (c) 2015, The Linux Foundation. All rights reserved.
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

package org.codeaurora.swe;

import android.os.Handler;

import org.chromium.android_webview.AwContentsClientBridge;

import java.security.Principal;
import java.security.PrivateKey;
import java.security.cert.CertificateEncodingException;
import java.security.cert.X509Certificate;

public class ClientCertRequestHandlerProxy extends ClientCertRequestHandler {

    AwContentsClientBridge.ClientCertificateRequestCallback mHandler;
    private String[] mKeyTypes;
    private Principal[] mPrincipals;
    private String mHost;
    private int mPort;

    public ClientCertRequestHandlerProxy(
            AwContentsClientBridge.ClientCertificateRequestCallback handler,
            String[] keyTypes, Principal[] principals, String host, int port) {
        mHandler = handler;
        mKeyTypes = keyTypes;
        mPrincipals = principals;
        mHost = host;
        mPort = port;
    }

    @Override
    public void proceed(PrivateKey privateKey, X509Certificate[] chain) {
        mHandler.proceed(privateKey, chain);
    }

    @Override
    public void ignore() {
        mHandler.ignore();
    }

    @Override
    public void cancel() {
        mHandler.cancel();
    }

    @Override
    public String getHost() {
        return mHost;
    }

    @Override
    public int getPort() {
        return mPort;
    }

    @Override
    public Principal[] getPrincipals() {
        return mPrincipals;
    }

    @Override
    public String[] getKeyTypes() {
        return mKeyTypes;
    }
}
