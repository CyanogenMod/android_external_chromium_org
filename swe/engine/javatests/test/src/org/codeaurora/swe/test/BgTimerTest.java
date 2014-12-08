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

import java.util.concurrent.atomic.AtomicReference;

import junit.framework.Assert;

import org.chromium.base.test.util.Feature;
import org.chromium.net.test.util.TestWebServer;
import org.chromium.content.browser.test.util.DOMUtils;

import org.codeaurora.swe.WebView;
import org.codeaurora.swe.WebViewClient;
import android.util.Log;

public class BgTimerTest extends SWETestBase {
    private static final String LOGTAG = "BgTimerTest";
    private static final String SWE_BGTIMER_HTML = "<html><head><title>Simple Dynamic Title</title></head></html>";

    @Feature({"BgTimer"})
    public void testBgTimer() throws Exception {
        TestWebServer webServer = new TestWebServer(false);
        WebView wv = getWebView();
        WebView wv2 = createWebView(false);
        wv.getSettings().setJavaScriptEnabled(true);
        setupWebviewClient(wv);
        String url = webServer.setResponse(
                 "/bgtimer.html", SWE_BGTIMER_HTML, null);
        loadUrlSync(wv, mTestWebViewClient.mOnPageFinishedHelper, url);
        executeJavaScriptAndWaitForResult(wv,"window.document.title=new Date().toLocaleTimeString()");
        executeJavaScriptAndWaitForResult(wv,"window.setInterval(function(){window.document.title=new Date().toLocaleTimeString()},1000)");
        String initTitle = wv.getTitle();
        pauseWebView(wv);
        Thread.sleep(2000);
        String afterpauseTitle = wv.getTitle();
        //title should get updated as widget was just hidden and not destroyed
        assertTrue(!initTitle.equals(afterpauseTitle));
        replaceWebView(wv2);
        Thread.sleep(2000);
        String afterreplaceTitle = wv.getTitle();
        //title should not get updated as widget was destroyed causing suspension of WebKitSharedTimer
        assertEquals(afterreplaceTitle,afterpauseTitle);
        Thread.sleep(8000);
        String finalTitle = wv.getTitle();
        //title should get updated due to resumption of WebKitSharedTimer after 10s
        assertTrue(!finalTitle.equals(afterreplaceTitle));
    }

}
