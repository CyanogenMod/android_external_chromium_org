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
 */
package org.codeaurora.swe.test;

import android.content.Context;
import android.os.Bundle;
import android.util.Pair;
import android.util.Log;

import android.test.ActivityInstrumentationTestCase2;

import org.chromium.net.test.util.TestWebServer;

import org.chromium.base.test.util.Feature;
import org.codeaurora.swe.WebView;
import org.codeaurora.swe.WebSettings;
import org.codeaurora.swe.WebBackForwardList;

public class TouchIconTest  extends SWETestBase {

    private static final String LOGTAG = "TouchIconTest";
    private static final String TOUCHICON_REL_LINK = "touch.png";
    private static final String TOUCHICON_REL_URL = "/" + TOUCHICON_REL_LINK;
    private TestWebServer mWebServer;
    private WebView mWebView;

    private void setupWebview(WebView wv) {
        assertNotNull(wv);
        setupWebviewClient(wv);
    }

    @Override
    protected void setUp() throws Exception {
        super.setUp();
        mWebServer = new TestWebServer(false);
        mWebView = createWebView(false);
        setupWebview(mWebView);
    }

    @Override
    protected void tearDown() throws Exception {
        if (mWebView != null)
            destroyWebView(mWebView);
        if (mWebServer != null) {
            mWebServer.shutdown();
        }
        super.tearDown();
    }


    private String getHtml(String headers, String body) {
        return "<!doctype html><html>" +
                 "<head>" +
                     "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1\""+
                     "<style type=\"text/css\">" +
                     "</style>" +
                     headers +
                 "</head>" +
                 "<body>" +
                     body +
                 "</body>" +
             "</html>";
    }


    @Feature({"TouchIcon"})
    public void testBasicTouchIcon() throws Exception {

        String touchIconHtml = getHtml(
                "<link rel=\"apple-touch-icon\" href=\"" + TOUCHICON_REL_URL + "\" />",
                "Touch Icon Page"
            );

        String nonTouchIconHtml = getHtml(
                "<titl>Foo</title>",
                "Foo Bar Bax"
            );

        String touchIconUrl =  mWebServer.setResponse(
                "/touchicon.html",touchIconHtml,null);

        String normalUrl =  mWebServer.setResponse(
                "/foo.html",touchIconHtml,null);

        loadUrlSync(mWebView, mTestWebViewClient.mOnPageFinishedHelper, touchIconUrl);
        assertNotNull(mWebView.getTouchIconUrl());
        assertFalse( mWebView.getTouchIconUrl().isEmpty());
        assertEquals(mWebView.getTouchIconUrl(),mWebServer.getBaseUrl()+TOUCHICON_REL_LINK);


        loadUrlSync(mWebView, mTestWebViewClient.mOnPageFinishedHelper, normalUrl);
        assertEquals(1, mWebServer.getRequestCount("/foo.html"));

        goBack(mWebView);
        goForward(mWebView);
        goBack(mWebView);

        assertEquals(mWebView.getTouchIconUrl(),mWebServer.getBaseUrl()+TOUCHICON_REL_LINK);
    }

    @Feature({"TouchIcon"})
    public void testTouchIconWithWebViewCrash() throws Exception {
        WebView oldWebView = createWebView(false);
        setupWebview(oldWebView);

        String touchIconHtml = getHtml(
            "<link rel=\"apple-touch-icon\" href=\"" + TOUCHICON_REL_URL + "\" />",
            "Touch Icon Page"
        );

        String touchIconLink = "/touchicon.html";

        String touchIconUrl =  mWebServer.setResponse(
                touchIconLink,touchIconHtml,null);

        loadUrlSync(oldWebView, mTestWebViewClient.mOnPageFinishedHelper, touchIconUrl);
        //validate the touch url
        assertEquals(oldWebView.getTouchIconUrl(),mWebServer.getBaseUrl()+TOUCHICON_REL_LINK);
        assertEquals(1, mWebServer.getRequestCount(touchIconLink));

         // save the list
        Bundle bundle = new Bundle();
        WebBackForwardList saveList = saveState(oldWebView, bundle);
        assertNotNull(saveList);
        assertEquals(1, saveList.getSize());
        assertEquals(0, saveList.getCurrentIndex());

        // Create a another normal webview check if we load from cache
        WebView newWebView = createWebView(false);
        setupWebview(newWebView);

        //destroy the old webview
        destroyWebView(oldWebView);

        final WebBackForwardList restoreList = restoreState(newWebView, bundle);
        assertNotNull(restoreList);
        assertEquals(1, restoreList.getSize());
        assertEquals(0, restoreList.getCurrentIndex());

        //go back to touchIcon page and assure that touchicon gets called
        assertEquals(newWebView.getTouchIconUrl(),mWebServer.getBaseUrl()+TOUCHICON_REL_LINK);

        destroyWebView(newWebView);
    }


}
