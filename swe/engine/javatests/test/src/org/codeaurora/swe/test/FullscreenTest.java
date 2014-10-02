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
 * Portions of this file derived from Chromium code, which is BSD licensed, copyright The Chromium Authors.
 */
package org.codeaurora.swe.test;

import android.util.Pair;
import android.content.Context;
import android.test.ActivityInstrumentationTestCase2;

import java.util.concurrent.atomic.AtomicReference;
import java.util.List;
import java.util.ArrayList;
import java.util.concurrent.ExecutionException;
import java.util.concurrent.TimeoutException;

import junit.framework.Assert;

import org.chromium.base.ThreadUtils;
import org.chromium.content.browser.ContentViewCore;
import org.chromium.base.test.util.Feature;
import org.chromium.net.test.util.TestWebServer;
import org.chromium.content.browser.test.util.DOMUtils;

import org.codeaurora.swe.WebView;
import org.codeaurora.swe.WebSettings;

public class FullscreenTest extends SWETestBase {
    private static final String LOGTAG = "FullscreenTest";

    private static final String FULLSCREEN_HTML =
        "<!DOCTYPE html>" +
        "<html><head>" +
        "<meta name='viewport' content='width=device-width, initial-scale=1'></head>" +
        "<body><div id='x'>This is a div box</div>" +
        "<button id='btn' onclick=\"getElementById('x').webkitRequestFullscreen()\">" +
        "Go full screen</button>" +
        "</body></html>";

    private TestWebServer mWebServer;
    private WebView mWebView;

    @Feature({"Fullscreen"})
    public void testFullscreen() throws Exception {
        mWebServer = new TestWebServer(false);
        String url = mWebServer.setResponse("/fsreen.html", FULLSCREEN_HTML, null);
        mWebView = getWebView();
        setupWebviewClient(mWebView);
        mWebView.getSettings().setFullscreenSupported(true);
        mWebView.getSettings().setJavaScriptEnabled(true);

        loadUrlSync(mWebView, mTestWebViewClient.mOnPageFinishedHelper, url);

        //click the fullscreen button
        DOMUtils.clickNode(this, mWebView.getContentViewCore(), "btn");

        //Now, are we in fullscreen mode?
        assertTrue("Not in fullscreen",
                   matchJSReturnValue(mWebView, "document.webkitIsFullScreen", "true"));

        //Exit fullscreen
        getInstrumentation().runOnMainSync(new Runnable() {
            @Override
            public void run() {
                mWebView.exitFullscreen();
            }
        });

        //make sure, we are NOT in fullscreen
        assertTrue("Still in fullscreen?",
                   matchJSReturnValue(mWebView, "document.webkitIsFullScreen", "false"));
    }
}
