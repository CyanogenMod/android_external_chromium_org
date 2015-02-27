/*
 *  Copyright (c) 2013-2014, The Linux Foundation. All rights reserved.
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

import android.app.AlertDialog;
import android.content.Context;
import android.content.DialogInterface;
import android.graphics.Bitmap;
import android.graphics.Picture;
import android.net.http.SslError;
import android.os.Message;
import android.view.KeyEvent;
import android.view.View;
import android.webkit.ConsoleMessage;
import android.webkit.GeolocationPermissions;
import android.webkit.ValueCallback;
import android.webkit.WebChromeClient.CustomViewCallback;
import android.webkit.WebResourceResponse;
import android.widget.EditText;

import org.chromium.android_webview.AwContentsClient;
import org.chromium.android_webview.AwContentsClientBridge;
import org.chromium.android_webview.AwHttpAuthHandler;
import org.chromium.android_webview.AwWebResourceResponse;
import org.chromium.android_webview.JsPromptResultReceiver;
import org.chromium.android_webview.JsResultReceiver;
import org.chromium.content.browser.NavigationEntry;
import org.codeaurora.swe.WebView.FindListener;
import org.codeaurora.swe.utils.Logger;

import java.security.Principal;

class AwContentsClientProxy extends AwContentsClient {

    private WebViewClient mWebViewClient;
    private WebChromeClient mWebChromeClient;
    private AwContentsClient mAcceleratorClient;
    private final WebView mWebView;
    private AlertDialog mAlertDialog = null;
    private DownloadListener mDownloadListener;
    private FindListener mFindListener;
    private boolean mZoomControls;

    public void setWebViewClient(WebViewClient client) {
        if (client != null)
            mWebViewClient = client;
        else
            mWebViewClient = new WebViewClient();
    }

    public WebViewClient getWebViewClient() {
        return mWebViewClient;
    }

    public WebChromeClient getWebChromeClient() {
        return mWebChromeClient;
    }

    public void setWebChromeClient(WebChromeClient client) {
        if (client != null)
            mWebChromeClient = client;
        else
            mWebChromeClient = new WebChromeClient();
    }

    public void setAcceleratorClient(AwContentsClient client) {
        mAcceleratorClient = client;
    }

    public void setDownloadListener(DownloadListener listener) {
        mDownloadListener = listener;
    }

    public AwContentsClientProxy (WebView webview) {
        mWebView = webview;
        mWebViewClient = new WebViewClient();
        mWebChromeClient = new WebChromeClient();
    }

    public void setFindListener(FindListener listener) {
        mFindListener = listener;
    }

    @Override
    public boolean shouldOverrideUrlLoading(String url) {
        Logger.warn("Redirect to URL: "+url);
        return mWebViewClient.shouldOverrideUrlLoading(mWebView, url);
    }

    @Override
    public boolean shouldDownloadFavicon(String url) {
        return mWebViewClient.shouldDownloadFavicon(mWebView, url);
    }

    @Override
    public void onUnhandledKeyEvent(KeyEvent event) {
        mWebViewClient.onUnhandledKeyEvent(mWebView, event);
    }

    @Override
    public boolean isTabFullScreen(){
        return mWebChromeClient.isTabFullScreen();
    }

    @Override
    public void toggleFullscreenModeForTab(boolean enterFullscreen) {
        mWebChromeClient.toggleFullscreenModeForTab(enterFullscreen);
    }

    @Override
    public void getVisitedHistory(ValueCallback<String[]> callback) {
        mWebChromeClient.getVisitedHistory(callback);
    }

    @Override
    public void doUpdateVisitedHistory(String url, boolean isReload) {
        mWebViewClient.doUpdateVisitedHistory(mWebView, url, isReload);
    }

    @Override
    public void onProgressChanged(int progress) {
        mWebChromeClient.onProgressChanged(mWebView, progress);
        if (mAcceleratorClient != null) {
            mAcceleratorClient.onProgressChanged(progress);
        }
    }

    @Override
    public AwWebResourceResponse shouldInterceptRequest(
            AwContentsClient.ShouldInterceptRequestParams params) {
        WebResourceResponse response =
          mWebViewClient.shouldInterceptRequest(mWebView, params.url);
        if (response != null) {
            return new AwWebResourceResponse(response.getMimeType(),
                response.getEncoding(), response.getData());
        }
        else
            return null;
    }

    @Override
    public boolean shouldOverrideKeyEvent(KeyEvent event) {
        return mWebViewClient.shouldOverrideKeyEvent(mWebView, event);
    }

    @Override
    public void onLoadResource(String url) {
        mWebViewClient.onLoadResource(mWebView, url);
    }

    @Override
    public boolean onConsoleMessage(ConsoleMessage consoleMessage) {
        return mWebChromeClient.onConsoleMessage(consoleMessage);
    }

    @Override
    public void onReceivedHttpAuthRequest(AwHttpAuthHandler handler,
                                          String host,
                                          String realm) {

        HttpAuthHandlerProxy httpAuthHandlerProxy =
                    new HttpAuthHandlerProxy(handler, mWebView, host, realm);
        mWebViewClient.onReceivedHttpAuthRequest(
                       mWebView, httpAuthHandlerProxy, host, realm);
    }

    @Override
    public void onReceivedSslError(ValueCallback<Boolean> callback, SslError error) {
        SslErrorHandlerProxy sslHandler = new SslErrorHandlerProxy(callback);
        mWebViewClient.onReceivedSslError(mWebView, sslHandler, error);
    }

    @Override
    public void onReceivedLoginRequest(String realm, String account, String args) {
        mWebViewClient.onReceivedLoginRequest(mWebView, realm, account,
                          args);
    }

    @Override
    public void onGeolocationPermissionsShowPrompt(String origin,
                GeolocationPermissions.Callback callback) {
        mWebChromeClient.onGeolocationPermissionsShowPrompt(origin, callback);
    }

    @Override
    public void onGeolocationPermissionsHidePrompt() {
        mWebChromeClient.onGeolocationPermissionsHidePrompt();
    }

    @Override
    public void handleJsAlert(String url, String message,
                                          JsResultReceiver receiver) {
        JsResult jsResult = new JsResult(new JsResultReceiverProxy(receiver));
        if(mWebChromeClient.onJsAlert(mWebView, url, message, jsResult)) {
            return;
        }
        //SWE: provide default dialog in case client doesn't provides
        dialog(mWebView, message, url, null, false, jsResult);
    }

    @Override
    public void handleJsBeforeUnload(String url, String message,
                                          JsResultReceiver receiver) {
        JsResult jsResult = new JsResult(new JsResultReceiverProxy(receiver));
        if(mWebChromeClient.onJsBeforeUnload(mWebView, url, message, jsResult)) {
            return;
        }
        //SWE: provide default dialog in case client doesn't provides
        dialog(mWebView, message, url, null, true, jsResult);
    }

    @Override
    public void handleJsConfirm(String url, String message, JsResultReceiver receiver) {
        JsResult jsResult = new JsResult(new JsResultReceiverProxy(receiver));
        if(mWebChromeClient.onJsConfirm(mWebView, url, message, jsResult)) {
            return;
        }
        //SWE: provide default dialog in case client doesn't provides
        dialog(mWebView, message, url, null, true, jsResult);
    }

    @Override
    public void handleJsPrompt(String url, String message,
                   String defaultValue, JsPromptResultReceiver receiver) {
        JsPromptResult jsPromtResult = new JsPromptResult(
                              new JsPromptResultReceiverProxy(receiver));
        if(mWebChromeClient.onJsPrompt(mWebView, url, message, defaultValue,
                                       jsPromtResult)) {
            return;
        }
        //SWE: provide default dialog in case client doesn't provides
        dialog(mWebView, message, url, defaultValue, true, jsPromtResult);
    }

    @Override
    public void onFindResultReceived(int activeMatchOrdinal,
                                     int numberOfMatches,
                                     boolean isDoneCounting) {
        if(mFindListener != null) {
            mFindListener.onFindResultReceived(activeMatchOrdinal, numberOfMatches, isDoneCounting);
        }
    }

    @Override
    public void onNewPicture(Picture picture) {
    }

    @Override
    public void onNewAsyncBitmap(byte[] data, int size, int width, int height) {
        mWebView.onNewAsyncBitmap(data, size, width, height);
    }

    @Override
    public void onPageStarted(String url) {
        if (mAcceleratorClient != null) {
            mAcceleratorClient.onPageStarted(url);
        }
        mWebViewClient.onPageStarted(mWebView, url, null);
    }

    @Override
    public void onPageFinished(String url) {
        mWebViewClient.onPageFinished(mWebView, url);
        if (mAcceleratorClient != null) {
            mAcceleratorClient.onPageFinished(url);
        }
    }

    @Override
    public void onReceivedError(int errorCode, String description, String failingUrl) {
        mWebViewClient.onReceivedError(mWebView, errorCode, description, failingUrl);
    }

    @Override
    public void onFormResubmission(Message dontResend, Message resend) {
        mWebViewClient.onFormResubmission(mWebView, dontResend, resend);
    }

    @Override
    public void onDownloadStart(String url,
                                String userAgent,
                                String contentDisposition,
                                String mimeType,
                                String referer,
                                long contentLength) {
        if (mDownloadListener != null) {
            if (mDownloadListener instanceof BrowserDownloadListener) {
                ((BrowserDownloadListener) mDownloadListener).onDownloadStart(url,
                    userAgent, contentDisposition, mimeType, referer, contentLength);
            } else {
                mDownloadListener.onDownloadStart(url, userAgent,
                    contentDisposition, mimeType, contentLength);
            }
        }
    }

    @Override
    public boolean onCreateWindow(boolean isDialog, boolean isUserGesture) {
        WebView.WebViewTransport transport = mWebView.new WebViewTransport();
        final Message msgTransport =
            mWebView.mWebViewHandler.obtainMessage(WebView.WebViewHandler.NOTIFY_CREATE_WINDOW);
        msgTransport.obj = transport;
        return mWebChromeClient.onCreateWindow(mWebView, false, isUserGesture, msgTransport);
    }

    @Override
    public void onCloseWindow() {
        mWebChromeClient.onCloseWindow(mWebView);
    }

    @Override
    public void onRequestFocus() {
         mWebChromeClient.onRequestFocus(mWebView);
    }

    @Override
    public void onReceivedTouchIconUrl(String url, boolean precomposed) {
        mWebChromeClient.onReceivedTouchIconUrl(mWebView, url, precomposed);
    }

    @Override
    public void onReceivedIcon(Bitmap bitmap) {
        mWebChromeClient.onReceivedIcon(mWebView, bitmap);
    }

    @Override
    public void onReceivedTitle(String title) {
        mWebChromeClient.onReceivedTitle(mWebView, title);
        if (mAcceleratorClient != null) {
            mAcceleratorClient.onReceivedTitle(title);
        }
    }

    private void saveAndDisableZoomControls() {
        mZoomControls = mWebView.getSettings().getBuiltInZoomControls();
        if (mZoomControls)
            mWebView.getSettings().setBuiltInZoomControls(false);
    }

    private void restoreZoomControls() {
        if (mZoomControls != mWebView.getSettings().getBuiltInZoomControls())
            mWebView.getSettings().setBuiltInZoomControls(mZoomControls);
    }

    @Override
    public void onShowCustomView(View view, int requestedOrientation,
            CustomViewCallback callback) {
        mWebChromeClient.onShowCustomView(view, requestedOrientation, callback);
    }

    @Override
    public void onHideCustomView() {
        mWebChromeClient.onHideCustomView();
    }

    public void configureForOverlayVideoMode(boolean enable) {
        mWebView.setOverlayVideoMode(enable);
        if (enable) {
            // WebView must be visible in overlay mode
            mWebView.setVisibility(View.VISIBLE);
            // Disable zooming of WebView media controls
            saveAndDisableZoomControls();
        } else {
            restoreZoomControls();
        }
    }

    @Override
    public void onScaleChangedScaled(float oldScale, float newScale) {
        mWebViewClient.onScaleChanged(mWebView, oldScale, newScale);
    }

    @Override
    protected View getVideoLoadingProgressView() {
        return null;
    }

    @Override
    public Bitmap getDefaultVideoPoster() {
        return null;
    }

    @Override
    public void onRendererCrash(boolean crashedWhileOomProtected) {
        mWebViewClient.onRendererCrash(mWebView, crashedWhileOomProtected);
//SWE-feature-reload-tab-oncrash
        mWebView.setHasCrashed(true);
//SWE-feature-reload-tab-oncrash
    }

//SWE-feature-client-certificate
    @Override
    public void onReceivedClientCertRequest(
            final AwContentsClientBridge.ClientCertificateRequestCallback handler,
            final String[] keyTypes, final Principal[] principals, final String host,
            final int port) {
         ClientCertRequestHandlerProxy clientCertRequestHandlerProxy =
                new ClientCertRequestHandlerProxy(handler, keyTypes, principals, host, port);
        mWebViewClient.onReceivedClientCertRequest(
              mWebView, clientCertRequestHandlerProxy);
    }
//SWE-feature-client-certificate

    @Override
    public void showFileChooser(ValueCallback<String[]> uploadFilePathsCallback,
            FileChooserParams fileChooserParams) {
        if (mWebChromeClient != null) {
            mWebChromeClient.showFileChooser(uploadFilePathsCallback,
                    fileChooserParams.acceptTypes,
                    fileChooserParams.capture);
        }
    }

//SWE-feature-reload-tab-oncrash
    @Override
    public void webContentsConnected() {
        mWebView.setHasCrashed(false);
    }
//SWE-feature-reload-tab-oncrash

    private void dialog(WebView view, String message, String url, String defPromptValue, boolean cancelable,JsResult result) {

        final JsResult jsResult = result;
        final String promptValue = defPromptValue;
        Context context = view.getContext();
        AlertDialog.Builder builder = new AlertDialog.Builder(context);
        final EditText input = new EditText(context);

        String dialogTitle = context.getResources().getString(R.string.dialog_alert_title);
        builder.setTitle(String.format(dialogTitle, url ));
        builder.setMessage(message);

        if (promptValue != null) {
            // Set an EditText view to get user input
            input.setText(promptValue);
            builder.setView(input);
        }

        builder.setPositiveButton(android.R.string.ok, new DialogInterface.OnClickListener() {
            @Override
            public void onClick(DialogInterface dialog, int id) {
                String value = null;
                // if prompt
                if (promptValue != null) {
                    value = input.getText().toString();
                    ((JsPromptResult)jsResult).confirm(value);
                } else {
                    jsResult.confirm();
                }
                mAlertDialog.dismiss();
            }
        });

        if (cancelable == true) {
            builder.setNegativeButton(android.R.string.cancel, new DialogInterface.OnClickListener() {
                @Override
                public void onClick(DialogInterface dialog, int id) {
                    jsResult.cancel();
                    mAlertDialog.dismiss();
                }
            });
        }

        mAlertDialog = builder.create();
        // Chrome does not let you dismiss the dialog following the same
        mAlertDialog.setCanceledOnTouchOutside(false);
        mAlertDialog.show();
    }

    private void showRememberPasswordDialog(WebView view,
            ValueCallback<Integer> cb) {

        final ValueCallback<Integer> callback = cb;

        // Constants to reflect user response. They should be in sync with those
        // defined in android_webview/browser/aw_password_manager_handler.cc
        final int NEVER_REMEMBER_PASSWORD = 0;
        final int REMEMBER_PASSWORD = 1;
        final int NOT_NOW = 2;
        final int DIALOG_DISMISSED = 3;

        Context context = view.getContext();
        AlertDialog.Builder builder = new AlertDialog.Builder(context);

        builder.setTitle(context.getResources()
                .getString(R.string.password_prompt_title));
        builder.setMessage(context.getResources()
                .getString(R.string.password_prompt_message));

        builder.setNegativeButton(R.string.password_prompt_never,
                new DialogInterface.OnClickListener() {
            @Override
            public void onClick(DialogInterface dialog, int id) {
                mAlertDialog.dismiss();
                callback.onReceiveValue(new Integer(NEVER_REMEMBER_PASSWORD));
            }
        });

        builder.setNeutralButton(R.string.password_prompt_remember,
                new DialogInterface.OnClickListener() {
            @Override
            public void onClick(DialogInterface dialog, int id) {
                mAlertDialog.dismiss();
                callback.onReceiveValue(new Integer(REMEMBER_PASSWORD));
            }
        });

        builder.setPositiveButton(R.string.password_prompt_not_now,
                new DialogInterface.OnClickListener() {
            @Override
            public void onClick(DialogInterface dialog, int id) {
                mAlertDialog.dismiss();
                callback.onReceiveValue(new Integer(NOT_NOW));
            }
        });

        mAlertDialog = builder.create();
        mAlertDialog.show();
        mAlertDialog.setOnDismissListener(new DialogInterface.OnDismissListener() {
            public void onDismiss(DialogInterface dialog) {
                callback.onReceiveValue(new Integer(DIALOG_DISMISSED));
                return;
            }
        });

    }

    @Override
    protected void promptUserToSavePassword(ValueCallback<Integer> callback) {
        showRememberPasswordDialog(mWebView, callback);
    }

//SWE-feature-hide-title-bar
    @Override
    public void onOffsetsForFullscreenChanged(
            float topControlsOffsetYPix,
            float contentOffsetYPix,
            float overdrawBottomHeightPix) {
        mWebView.onOffsetsForFullscreenChanged(topControlsOffsetYPix,
            contentOffsetYPix, overdrawBottomHeightPix);
        mWebChromeClient.onOffsetsForFullscreenChanged(topControlsOffsetYPix,
            contentOffsetYPix, overdrawBottomHeightPix);
    }
//SWE-feature-hide-title-bar

//SWE-feature-load-notification
    @Override
    public void documentLoadedInFrame(long frameId, boolean isMainFrame) {
        if (mAcceleratorClient != null) {
            mAcceleratorClient.documentLoadedInFrame(frameId, isMainFrame);
        }
    }

    @Override
    public void didFirstVisuallyNonEmptyPaint() {
        if (mAcceleratorClient != null) {
            mAcceleratorClient.didFirstVisuallyNonEmptyPaint();
        }
    }
//SWE-feature-load-notification

}
