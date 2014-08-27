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

package org.codeaurora.swe;

import android.graphics.Bitmap;
import android.os.Message;
import android.view.View;
import android.net.Uri;
import android.webkit.GeolocationPermissions;
import android.webkit.ValueCallback;
import android.webkit.ConsoleMessage;
import android.webkit.WebChromeClient.CustomViewCallback;
import android.webkit.WebStorage.QuotaUpdater;



public class WebChromeClient {



    public WebChromeClient() {
    }

    public void onProgressChanged(WebView view, int newProgress) {}

    public void onReceivedTitle(WebView view, String title) {}

    public void onReceivedIcon(WebView view, Bitmap icon) {}

    public void onCloseWindow(WebView window) {}

    public boolean onJsAlert(WebView view, String url, String message, JsResult result) {
        return false;
    }

    public boolean onJsConfirm(WebView view, String url, String message, JsResult result) {
        return false;
    }

    public boolean onJsPrompt(WebView view, String url, String message, String defaultValue, JsPromptResult result) {
        return false;
    }

    public boolean onJsBeforeUnload(WebView view, String url, String message, JsResult result) {
        return false;
    }

    public void onGeolocationPermissionsShowPrompt(String origin,
            GeolocationPermissions.Callback callback) {}


    public void onGeolocationPermissionsHidePrompt() {}

    public boolean onJsTimeout() {
        return true;
    }

    public void getVisitedHistory(ValueCallback<String[]> callback) {}

    public void openFileChooser(ValueCallback<Uri> uploadFile, String acceptType, String capture) {
        uploadFile.onReceiveValue(null);
    }

    public View getVideoLoadingProgressView() {
        return null;
    }

    public Bitmap getDefaultVideoPoster() {
        return null;
    }

    public boolean onConsoleMessage(ConsoleMessage consoleMessage) {
        return false;
    }

    public void onReachedMaxAppCacheSize(long spaceNeeded, long totalUsedQuota,
            QuotaUpdater quotaUpdater) {
        quotaUpdater.updateQuota(totalUsedQuota);
    }

    public void onExceededDatabaseQuota(String url, String databaseIdentifier, long currentQuota,
            long estimatedSize, long totalUsedQuota, QuotaUpdater quotaUpdater) {
        quotaUpdater.updateQuota(currentQuota);
    }

    public void onHideCustomView() {}

    public void onShowCustomView(View view, CustomViewCallback callback) {}

    public void onShowCustomView(View view, int requestedOrientation, CustomViewCallback callback) {}

    public void onReceivedTouchIconUrl(WebView view, String url, boolean precomposed) {}

    public boolean onCreateWindow(WebView view, boolean dialog, boolean userGesture,
            Message resultMsg) {
        return false;
    }

    public void onRequestFocus(WebView view) {}

    public void setupAutoFill(Message msg) {}

    public void showFileChooser(ValueCallback<String[]> uploadFilePaths, String acceptTypes,
            boolean capture) {}

    public void toggleFullscreenModeForTab(boolean enterFullscreen) {}

    public boolean isTabFullScreen() {
        return false;
    }

    public void onOffsetsForFullscreenChanged(
        float topControlsOffsetYPix, float contentOffsetYPix, float overdrawBottomHeightPix) {
    }

}
