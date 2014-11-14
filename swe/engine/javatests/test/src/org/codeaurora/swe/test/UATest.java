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
import android.test.ActivityInstrumentationTestCase2;
import android.util.Pair;
import android.os.Bundle;

import java.util.concurrent.atomic.AtomicReference;
import java.util.ArrayList;
import java.util.List;

import junit.framework.Assert;

import org.chromium.base.test.util.Feature;
import org.chromium.net.test.util.TestWebServer;
import org.chromium.content.browser.test.util.DOMUtils;
import org.chromium.content.browser.test.util.HistoryUtils;

import org.codeaurora.swe.WebView;
import org.codeaurora.swe.WebViewClient;
import org.codeaurora.swe.WebBackForwardList;
import org.codeaurora.swe.BrowserCommandLine;

public class UATest extends SWETestBase {
    private static final String LOGTAG = "UATest";
    private static final String SWE_WEBVIEW_HTML = "<html><body> SWE WebView </body></html>";

    @Feature({"UA"})
    public void testUserAgent() throws Exception, java.lang.Throwable {
        TestWebServer webServer = new TestWebServer(false);
        WebView wv = getWebView();
        setupWebviewClient(wv);
        wv.getSettings().setJavaScriptEnabled(true);
        // Check if default is mobile  UA
        assertFalse(wv.getUseDesktopUserAgent());

        String url1 = webServer.setResponse(
                     "/webview1.html", SWE_WEBVIEW_HTML , null);
        String url2 = webServer.setResponse(
                     "/webview2.html", SWE_WEBVIEW_HTML , null);
        String url3 = webServer.setResponse(
                     "/webview3.html", SWE_WEBVIEW_HTML , null);
        // load a html page and check the UA
        loadUrlSync(wv, mTestWebViewClient.mOnPageFinishedHelper, url1);
        assertFalse(wv.getUseDesktopUserAgent());
        String ua = executeJavaScriptAndWaitForResult(wv,
              "window.navigator.userAgent");

        // check if we used mobile UA
        assertTrue(ua.indexOf("Mobile") != -1);
        assertFalse(wv.getUseDesktopUserAgent());

        // toggle the UA
        setUseDesktopUserAgentSync(wv, true, true, mTestWebViewClient.mOnPageFinishedHelper);
        ua = executeJavaScriptAndWaitForResult(wv,
              "window.navigator.userAgent");

        // check if we used desktop UA
        assertTrue(ua.indexOf("Mobile") == -1);
        assertTrue(wv.getUseDesktopUserAgent());

        // Check Navigation to keep the UA
        loadUrlSync(wv, mTestWebViewClient.mOnPageFinishedHelper, url2);
        ua = executeJavaScriptAndWaitForResult(wv,
              "window.navigator.userAgent");
        assertTrue(ua.indexOf("Mobile") == -1);
        assertTrue(wv.getUseDesktopUserAgent());

        // toggle URL2 to Mobile
        setUseDesktopUserAgentSync(wv, false, true, mTestWebViewClient.mOnPageFinishedHelper);
        ua = executeJavaScriptAndWaitForResult(wv,
              "window.navigator.userAgent");
        assertTrue(ua.indexOf("Mobile") != -1);
        assertFalse(wv.getUseDesktopUserAgent());

        // Navigate to URL3 and ensure its mobile
        loadUrlSync(wv, mTestWebViewClient.mOnPageFinishedHelper, url3);
        ua = executeJavaScriptAndWaitForResult(wv,
              "window.navigator.userAgent");
        assertTrue(ua.indexOf("Mobile") != -1);
        assertFalse(wv.getUseDesktopUserAgent());

        // toggle URL3 to desktop and check again
        setUseDesktopUserAgentSync(wv, true, true, mTestWebViewClient.mOnPageFinishedHelper);
        ua = executeJavaScriptAndWaitForResult(wv,
              "window.navigator.userAgent");
        assertTrue(ua.indexOf("Mobile") == -1);
        assertTrue(wv.getUseDesktopUserAgent());

        // Check back and forward navigation with UA
        checkUANavigation(wv);

        // Save state
        Bundle bundle = new Bundle();
        WebBackForwardList saveList = saveState(wv, bundle);
        assertNotNull(saveList);
        assertEquals(3, saveList.getSize());
        assertEquals(2, saveList.getCurrentIndex());

        // Check restore
        WebView newWebView = createWebView(false);
        setupWebviewClient(newWebView);
        newWebView.getSettings().setJavaScriptEnabled(true);

        final WebBackForwardList restoreList = restoreState(newWebView, bundle);
        assertNotNull(restoreList);
        assertEquals(3, saveList.getSize());
        assertEquals(2, saveList.getCurrentIndex());

        ua = executeJavaScriptAndWaitForResult(newWebView,
              "window.navigator.userAgent");
        assertTrue(ua.indexOf("Mobile") == -1);
        assertTrue(newWebView.getUseDesktopUserAgent());

        checkUANavigation(newWebView);

        // destroy the new webview
        destroyWebView(newWebView);

        // Check command-line override
        BrowserCommandLine bcl = BrowserCommandLine.getInstance();
        bcl.appendSwitchWithValue("user-agent", "SWE Mobile");
        newWebView = createWebView(false);
        setupWebviewClient(newWebView);
        newWebView.getSettings().setJavaScriptEnabled(true);
        loadUrlSync(newWebView, mTestWebViewClient.mOnPageFinishedHelper, url1);
        assertFalse(newWebView.getUseDesktopUserAgent());
        ua = executeJavaScriptAndWaitForResult(newWebView,
              "window.navigator.userAgent");

        // check if we used given UA
        assertEquals("\"SWE Mobile\"", ua);
        assertFalse(newWebView.getUseDesktopUserAgent());

        // destroy the new webview
        destroyWebView(newWebView);
    }

    private void checkUANavigation(WebView wv) throws Exception, java.lang.Throwable {
        // check the back navigation with the UA now
        // i.e URL3 => desktop, URL2 => Mobile , URL1 => Desktop
        HistoryUtils.goBackSync(getInstrumentation(), wv.getContentViewCore(),
            mTestWebViewClient.mOnPageFinishedHelper);
        String ua = executeJavaScriptAndWaitForResult(wv,
              "window.navigator.userAgent");
        // URL 2
        assertTrue(ua.indexOf("Mobile") != -1);
        assertFalse(wv.getUseDesktopUserAgent());

        HistoryUtils.goBackSync(getInstrumentation(), wv.getContentViewCore(),
            mTestWebViewClient.mOnPageFinishedHelper);
        ua = executeJavaScriptAndWaitForResult(wv,
              "window.navigator.userAgent");
        // URL 1
        assertTrue(ua.indexOf("Mobile") == -1);
        assertTrue(wv.getUseDesktopUserAgent());

        // Check forward navigation
        HistoryUtils.goForwardSync(getInstrumentation(), wv.getContentViewCore(),
            mTestWebViewClient.mOnPageFinishedHelper);
        ua = executeJavaScriptAndWaitForResult(wv,
              "window.navigator.userAgent");
        // URL 2
        assertTrue(ua.indexOf("Mobile") != -1);
        assertFalse(wv.getUseDesktopUserAgent());
        // URL 3
        HistoryUtils.goForwardSync(getInstrumentation(), wv.getContentViewCore(),
            mTestWebViewClient.mOnPageFinishedHelper);
        ua = executeJavaScriptAndWaitForResult(wv,
              "window.navigator.userAgent");
        assertTrue(ua.indexOf("Mobile") == -1);
        assertTrue(wv.getUseDesktopUserAgent());
    }
}
