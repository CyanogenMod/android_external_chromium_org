// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.chromium.android_webview;

import android.content.Context;
import android.view.KeyEvent;
import android.view.View;
import android.webkit.URLUtil;
import android.webkit.WebChromeClient;

import org.chromium.base.CommandLine;
import org.chromium.content.browser.ContentVideoView;
import org.chromium.content.browser.ContentVideoViewClient;
import org.chromium.content.browser.ContentViewClient;
import org.chromium.content.common.ContentSwitches;

/**
 * ContentViewClient implementation for WebView
 */
public class AwContentViewClient extends ContentViewClient {

    private class AwContentVideoViewClient implements ContentVideoViewClient {
        @Override
        public boolean onShowCustomView(View view) {
            WebChromeClient.CustomViewCallback cb = new WebChromeClient.CustomViewCallback() {
                @Override
                public void onCustomViewHidden() {
                    ContentVideoView contentVideoView = ContentVideoView.getContentVideoView();
                    if (contentVideoView != null)
                        contentVideoView.exitFullscreen(false);
                }
            };
            onShowCustomView(view, cb);
            return true;
        }

        private void onShowCustomView(View view, WebChromeClient.CustomViewCallback cb) {
            mAwContentsClient.onShowCustomView(view, cb);
            if (areHtmlControlsEnabled()) {
                mAwContentsClient.configureForOverlayVideoMode(true);
            }
        }

        @Override
        public void onDestroyContentVideoView() {
            mAwContentsClient.onHideCustomView();
            if (areHtmlControlsEnabled()) {
                mAwContentsClient.configureForOverlayVideoMode(false);
            }
        }

        @Override
        public View getVideoLoadingProgressView() {
            return mAwContentsClient.getVideoLoadingProgressView();
        }
    }

    private AwContentsClient mAwContentsClient;
    private AwSettings mAwSettings;
    private AwContents mAwContents;
    private Context mContext;

    public AwContentViewClient(
            AwContentsClient awContentsClient, AwSettings awSettings, AwContents awContents,
            Context context) {
        mAwContentsClient = awContentsClient;
        mAwSettings = awSettings;
        mAwContents = awContents;
        mContext = context;
    }

    @Override
    public void onBackgroundColorChanged(int color) {
        mAwContentsClient.onBackgroundColorChanged(color);
    }

    @Override
    public void onStartContentIntent(Context context, String contentUrl) {
        //  Callback when detecting a click on a content link.
        mAwContentsClient.shouldOverrideUrlLoading(contentUrl);
    }

    @Override
    public void onUpdateTitle(String title) {
        mAwContentsClient.onReceivedTitle(title);
    }

    @Override
    public boolean shouldOverrideKeyEvent(KeyEvent event) {
        return mAwContentsClient.shouldOverrideKeyEvent(event);
    }

    @Override
    public final ContentVideoViewClient getContentVideoViewClient() {
        return new AwContentVideoViewClient();
    }

    @Override
    public boolean shouldBlockMediaRequest(String url) {
        return mAwSettings != null ?
                mAwSettings.getBlockNetworkLoads() && URLUtil.isNetworkUrl(url) : true;
    }

    private static boolean areHtmlControlsEnabled() {
        return !CommandLine.getInstance().hasSwitch(
                ContentSwitches.DISABLE_OVERLAY_FULLSCREEN_VIDEO_SUBTITLE);
    }

//SWE-feature-hide-title-bar
    /**
     * Notifies the client that the position of the top controls has changed.
     * @param topControlsOffsetYPix The Y offset of the top controls in physical pixels.
     * @param contentOffsetYPix The Y offset of the content in physical pixels.
     * @param overdrawBottomHeightPix The overdraw height.
     */
    @Override
    public void onOffsetsForFullscreenChanged(
            float topControlsOffsetYPix, float contentOffsetYPix, float overdrawBottomHeightPix) {
        mAwContentsClient.onOffsetsForFullscreenChanged(topControlsOffsetYPix,
                                                        contentOffsetYPix,
                                                        overdrawBottomHeightPix);
    }
//SWE-feature-hide-title-bar

//SWE-feature-reload-tab-oncrash
    @Override
    public void webContentsConnected() {
        mAwContentsClient.webContentsConnected();
    }
//SWE-feature-reload-tab-oncrash

// SWE-feature-immersive-mode
    public void onKeyboardStateChange(boolean popup) {
        mAwContentsClient.onKeyboardStateChange(popup);
    }
// SWE-feature-immersive-mode
}
