// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.chromium.android_webview;

import org.chromium.content.browser.WebContentsObserverAndroid;
import org.chromium.content_public.browser.WebContents;
import org.chromium.net.NetError;

import java.util.HashMap;

/**
 * Routes notifications from WebContents to AwContentsClient and other listeners.
 */
public class AwWebContentsObserver extends WebContentsObserverAndroid {
    private final AwContentsClient mAwContentsClient;
    private boolean mIsMainFrameLoaded;
    private long mMainFrameId;
    private HashMap mFramesMap = null;


    public AwWebContentsObserver(WebContents webContents, AwContentsClient awContentsClient) {
        super(webContents);
        mAwContentsClient = awContentsClient;
    }

    @Override
    public void didFinishLoad(long frameId, String validatedUrl, boolean isMainFrame) {
        String unreachableWebDataUrl = AwContentsStatics.getUnreachableWebDataUrl();
        boolean isErrorUrl =
                unreachableWebDataUrl != null && unreachableWebDataUrl.equals(validatedUrl);

//SWE-feature-enable-offline-after-complete-pageload
        if (isMainFrame) {
            mIsMainFrameLoaded = true;
        }

        if (mFramesMap != null) {
            if (mFramesMap.containsKey(frameId))
                mFramesMap.remove(frameId);

            if (mIsMainFrameLoaded && mFramesMap.isEmpty() && !isErrorUrl){
                mAwContentsClient.onPageFinished(validatedUrl);
            }
        }
//SWE-feature-enable-offline-after-complete-pageload
    }

//SWE-feature-enable-offline-after-complete-pageload
    @Override
    public void didStartProvisionalLoadForFrame(
        long frameId,
        long parentFrameId,
        boolean isMainFrame,
        String validatedUrl,
        boolean isErrorPage,
        boolean isIframeSrcdoc) {
        if(isMainFrame) {
            mMainFrameId = frameId;
        }

        if ( mFramesMap != null && (mMainFrameId == parentFrameId) ) {
            // add all the iframes only related to the origin
            // of the main frame into the hashmap
            mFramesMap.put(frameId, parentFrameId);
        }
    }
//SWE-feature-enable-offline-after-complete-pageload

    @Override
    public void didFailLoad(boolean isProvisionalLoad,
            boolean isMainFrame, int errorCode, String description, String failingUrl) {
        String unreachableWebDataUrl = AwContentsStatics.getUnreachableWebDataUrl();
        boolean isErrorUrl =
                unreachableWebDataUrl != null && unreachableWebDataUrl.equals(failingUrl);
        if (isMainFrame && !isErrorUrl) {
            if (errorCode != NetError.ERR_ABORTED) {
                // This error code is generated for the following reasons:
                // - WebView.stopLoading is called,
                // - the navigation is intercepted by the embedder via shouldOverrideNavigation.
                //
                // The Android WebView does not notify the embedder of these situations using
                // this error code with the WebViewClient.onReceivedError callback.
                mAwContentsClient.onReceivedError(
                        ErrorCodeConversionHelper.convertErrorCode(errorCode), description,
                                failingUrl);
            }
            // Need to call onPageFinished after onReceivedError (if there is an error) for
            // backwards compatibility with the classic webview.
            mAwContentsClient.onPageFinished(failingUrl);
        }
    }

    @Override
    public void didNavigateMainFrame(String url, String baseUrl,
            boolean isNavigationToDifferentPage, boolean isFragmentNavigation) {
// SWE-feature-enable-offline-after-complete-pageload
        mIsMainFrameLoaded = false;
        mFramesMap = new HashMap();
// SWE-feature-enable-offline-after-complete-pageload
        // This is here to emulate the Classic WebView firing onPageFinished for main frame
        // navigations where only the hash fragment changes.
        if (isFragmentNavigation) {
            mAwContentsClient.onPageFinished(url);
        }
    }

    @Override
    public void didNavigateAnyFrame(String url, String baseUrl, boolean isReload) {
        mAwContentsClient.doUpdateVisitedHistory(url, isReload);
    }

// SWE-feature-tab-crash
    @Override
    public void renderProcessGone(boolean wasOomProtected) {
        mAwContentsClient.onRendererCrash(wasOomProtected);
    }
// SWE-feature-tab-crash
}
