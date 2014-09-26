/*
 *  Copyright (c) 2014, The Linux Foundation. All rights reserved.
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
 * Portion of this file from Android Frameworks Base, which is Apache v2 licensed, copyright
 * The Android Open Source Project.
 *
 * Copyright (C) 2008 The Android Open Source Project
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
 *
 * Portions of this file derived from Chromium code, which is BSD licensed, copyright The Chromium Authors.
 */

package org.codeaurora.swe.test;

import android.graphics.Bitmap;
import android.net.http.SslError;
import android.os.Message;
import android.view.KeyEvent;
import android.webkit.WebResourceResponse;
import org.codeaurora.swe.WebViewClient;
import org.codeaurora.swe.WebView;
import org.codeaurora.swe.SslErrorHandler;
import org.codeaurora.swe.HttpAuthHandler;
import org.codeaurora.swe.ClientCertRequestHandler;
import org.chromium.content.browser.test.util.CallbackHelper;
import org.chromium.content.browser.test.util.TestCallbackHelperContainer.OnEvaluateJavaScriptResultHelper;
import org.chromium.content.browser.test.util.TestCallbackHelperContainer.OnPageFinishedHelper;
import org.chromium.content.browser.test.util.TestCallbackHelperContainer.OnPageStartedHelper;
import org.chromium.content.browser.test.util.TestCallbackHelperContainer.OnReceivedErrorHelper;

public class TestWebViewClient extends WebViewClient {

    protected final OnPageFinishedHelper mOnPageFinishedHelper;
    protected final OnPageStartedHelper mOnPageStartedHelper;
    protected final OnReceivedErrorHelper mOnReceivedErrorHelper;
    protected final OnEvaluateJavaScriptResultHelper mOnEvaluateJavaScriptResultHelper;

    public TestWebViewClient() {
        mOnPageStartedHelper = new OnPageStartedHelper();
        mOnPageFinishedHelper = new OnPageFinishedHelper();
        mOnReceivedErrorHelper = new OnReceivedErrorHelper();
        mOnEvaluateJavaScriptResultHelper = new OnEvaluateJavaScriptResultHelper();
    }

    public boolean shouldOverrideUrlLoading(WebView view, String url) {
        return false;
    }

    public void onPageStarted(WebView view, String url, Bitmap favicon) {
        mOnPageStartedHelper.notifyCalled(url);
    }

    public void onPageFinished(WebView view, String url) {
        mOnPageFinishedHelper.notifyCalled(url);
    }

    public void onLoadResource(WebView view, String url) {}


    public WebResourceResponse shouldInterceptRequest(WebView view, String url) {
        return null;
    }

    public void onReceivedError(WebView view, int errorCode,
            String description, String failingUrl) {
        mOnReceivedErrorHelper.notifyCalled(errorCode, description, failingUrl);
    }

    public void onFormResubmission(WebView view, Message dontResend,
            Message resend) {
        dontResend.sendToTarget();
    }

    public void doUpdateVisitedHistory(WebView view, String url,
        boolean isReload) {}


    public void onReceivedSslError(WebView view, SslErrorHandler handler,
            SslError error) {
        handler.cancel();
    }

    public void onReceivedHttpAuthRequest(WebView view,
            HttpAuthHandler handler, String host, String realm) {
        handler.cancel();
    }

    public boolean shouldOverrideKeyEvent(WebView view, KeyEvent event) {

        // need to assure that keyevent is passed by user
        if (event == null || view == null) {
            return false;
        }

        int keyCode = event.getKeyCode();
        // We need to send almost every key to WebKit. However:
        // 1. We don't want to block the device on the renderer for
        // some keys like menu, home, call.
        // 2. There are no WebKit equivalents for some of these keys
        // (see app/keyboard_codes_win.h)
        // Note that these are not the same set as KeyEvent.isSystemKey:
        // for instance, AKEYCODE_MEDIA_* will be dispatched to webkit.
        if (keyCode == KeyEvent.KEYCODE_MENU ||
            keyCode == KeyEvent.KEYCODE_HOME ||
            keyCode == KeyEvent.KEYCODE_BACK ||
            keyCode == KeyEvent.KEYCODE_CALL ||
            keyCode == KeyEvent.KEYCODE_ENDCALL ||
            keyCode == KeyEvent.KEYCODE_POWER ||
            keyCode == KeyEvent.KEYCODE_HEADSETHOOK ||
            keyCode == KeyEvent.KEYCODE_CAMERA ||
            keyCode == KeyEvent.KEYCODE_FOCUS ||
            keyCode == KeyEvent.KEYCODE_VOLUME_DOWN ||
            keyCode == KeyEvent.KEYCODE_VOLUME_MUTE ||
            keyCode == KeyEvent.KEYCODE_VOLUME_UP) {
            return true;
        }

        // We also have to intercept some shortcuts before we send them to the ContentView.
        if (event.isCtrlPressed() && (
                keyCode == KeyEvent.KEYCODE_TAB ||
                keyCode == KeyEvent.KEYCODE_W ||
                keyCode == KeyEvent.KEYCODE_F4)) {
            return true;
        }

        return false;
    }


    public void onUnhandledKeyEvent(WebView view, KeyEvent event) {}


    public void onScaleChanged(WebView view, float oldScale, float newScale) {}

    public void onReceivedLoginRequest(WebView view, String realm,
            String account, String args) {

    }

    /**
     * Notify the host application that an SSL error occurred while loading a
     * resource, but the WebView chose to proceed anyway based on a
     * decision retained from a previous response to onReceivedSslError().
     * @hide
     */
    public void onProceededAfterSslError(WebView view, SslError error) {
    }

    /**
     * Notify the host application to handle a SSL client certificate
     * request (display the request to the user and ask whether to
     * proceed with a client certificate or not). The host application
     * has to call either handler.cancel() or handler.proceed() as the
     * connection is suspended and waiting for the response. The
     * default behavior is to cancel, returning no client certificate.
     *
     * @param view The WebView that is initiating the callback.
     * @param handler A ClientCertRequestHandler object that will
     *            handle the user's response.
     * @param host_and_port The host and port of the requesting server.
     *
     * @hide
     */
    public void onReceivedClientCertRequest(WebView view,
        ClientCertRequestHandler handler, String host_and_port) {
        handler.cancel();
    }

    public void onRendererCrash(WebView view, boolean crashedWhileOomProtected) {
    }

}
