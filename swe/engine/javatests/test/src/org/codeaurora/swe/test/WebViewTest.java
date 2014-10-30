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

import android.content.Context;
import android.test.ActivityInstrumentationTestCase2;
import android.util.Pair;

import java.util.concurrent.atomic.AtomicReference;
import java.util.ArrayList;
import java.util.List;

import junit.framework.Assert;

import org.chromium.base.test.util.Feature;
import org.chromium.net.test.util.TestWebServer;
import org.chromium.content.browser.test.util.DOMUtils;

import org.codeaurora.swe.WebView;
import org.codeaurora.swe.WebViewClient;

public class WebViewTest extends SWETestBase {
    private static final String LOGTAG = "WebViewTest";
    private static final String SWE_WEBVIEW_HTML = "<html><body> SWE WebView </body></html>";

    private boolean isSavable(final WebView wv) {
        final AtomicReference<Boolean> testIsSavable = new AtomicReference<Boolean>();

        getInstrumentation().runOnMainSync(new Runnable() {
            @Override
            public void run() {
                testIsSavable.set(wv.isSavable());
            }
        });

        return testIsSavable.get();
    }


    @Feature({"SWEWebView"})
    public void testWebViewCreationDestroy() throws Exception {
        WebView normalWV = createWebView(false);
        assertNotNull(normalWV);
        destroyWebView(normalWV);
    }

    @Feature({"SWEWebView"})
    public void testPrivateWebViewCreationDestroy() throws Exception {
        WebView incognitoWV = createWebView(true);
        assertNotNull(incognitoWV);
        destroyWebView(incognitoWV);
    }

    @Feature({"SWEWebView"})
    public void testLoadURL() throws Exception {
        TestWebServer webServer = new TestWebServer(false);
        WebView wv = getWebView();
        setupWebviewClient(wv);
        String url = webServer.setResponse(
                     "/webview.html", SWE_WEBVIEW_HTML , null);
        loadUrlSync(wv, mTestWebViewClient.mOnPageFinishedHelper, url);
    }

    @Feature({"SWEWebView"})
    public void testMultiWebViewCreation() throws Exception {
        TestWebServer webServer = new TestWebServer(false);
        for (int i = 0; i < 30; ++i) {
            WebView wv = createWebView(false);
            setupWebviewClient(wv);
            String url = webServer.setResponse(
                     "/webview.html", SWE_WEBVIEW_HTML, null);
            loadUrlSync(wv, mTestWebViewClient.mOnPageFinishedHelper, url);
            destroyWebView(wv);
        }
    }

    @Feature({"SWEWebView"})
    public void testIsSavable() throws Exception {
        TestWebServer webServer = new TestWebServer(false);
        WebView wv = getWebView();
        setupWebviewClient(wv);

        // Intial before performing any load
        // isSavable should be false
        assertFalse(isSavable(wv));

        String url = webServer.setResponse(
                     "/webview.html", SWE_WEBVIEW_HTML , null);
        // load a html page and page can be saved
        loadUrlSync(wv, mTestWebViewClient.mOnPageFinishedHelper, url);
        assertTrue(isSavable(wv));

        String IMAGE_DATA = "iVBORw0KGgoAAAANSUhEUgAAAAEAAAABCAAAAAA" +
                            "6fptVAAAAAXNSR0IArs4c6QAAAA1JREFUCB0BAgD9/w" +
                            "DQANIA0U9MSZY"+"AAAAASUVORK5CYII=";

        List<Pair<String, String>> headers = new ArrayList<Pair<String, String>>();
        headers.add(Pair.create("Content-Type", "image/png"));

        final String imagePath = "/image.png";
        // load a png image and page cannot be saved
        String imageUrl =  webServer.setResponseBase64(imagePath, IMAGE_DATA,
               headers);

        loadUrlSync(wv, mTestWebViewClient.mOnPageFinishedHelper, imageUrl);
        assertFalse(isSavable(wv));

    }

}
