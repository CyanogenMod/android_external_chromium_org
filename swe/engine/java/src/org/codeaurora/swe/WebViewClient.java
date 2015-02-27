/*
 * Copyright (c) 2013 The Linux Foundation. All rights reserved.
 * Not a contribution.
 *
 * Copyright (C) 2012 The Android Open Source Project
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

import android.graphics.Bitmap;
import android.net.http.SslError;
import android.os.Message;
import android.view.KeyEvent;
import android.webkit.WebResourceResponse;

public class WebViewClient {

    public WebViewClient() {
    }

    public boolean shouldOverrideUrlLoading(WebView view, String url) {
        return false;
    }

    public boolean shouldDownloadFavicon(WebView view, String url) {
        return false;
    }

    public void onPageStarted(WebView view, String url, Bitmap favicon) {}

    public void onPageFinished(WebView view, String url) {}

    public void onLoadResource(WebView view, String url) {}


    public WebResourceResponse shouldInterceptRequest(WebView view, String url) {
        return null;
    }


    public static final int ERROR_UNKNOWN = -1;
    public static final int ERROR_HOST_LOOKUP = -2;
    public static final int ERROR_UNSUPPORTED_AUTH_SCHEME = -3;
    public static final int ERROR_AUTHENTICATION = -4;
    public static final int ERROR_PROXY_AUTHENTICATION = -5;
    public static final int ERROR_CONNECT = -6;
    public static final int ERROR_IO = -7;
    public static final int ERROR_TIMEOUT = -8;
    public static final int ERROR_REDIRECT_LOOP = -9;
    public static final int ERROR_UNSUPPORTED_SCHEME = -10;
    public static final int ERROR_FAILED_SSL_HANDSHAKE = -11;
    public static final int ERROR_BAD_URL = -12;
    public static final int ERROR_FILE = -13;
    public static final int ERROR_FILE_NOT_FOUND = -14;
    public static final int ERROR_TOO_MANY_REQUESTS = -15;

    public void onReceivedError(WebView view, int errorCode,
            String description, String failingUrl) {}

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
     *
     * @hide
     */
    public void onReceivedClientCertRequest(WebView view,
        ClientCertRequestHandler handler) {
        handler.cancel();
    }

    public void onRendererCrash(WebView view, boolean crashedWhileOomProtected) {
    }

}
