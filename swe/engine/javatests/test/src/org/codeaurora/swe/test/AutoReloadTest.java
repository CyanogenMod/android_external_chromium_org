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

import android.app.ActivityManager;
import android.app.ActivityManager.RunningAppProcessInfo;
import android.content.Context;

import junit.framework.Assert;

import org.chromium.base.test.util.Feature;
import org.chromium.net.test.util.TestWebServer;
import org.chromium.content.browser.ChildProcessLauncher;
import org.chromium.content.browser.test.util.Criteria;
import org.chromium.content.browser.test.util.CriteriaHelper;

import org.codeaurora.swe.WebView;

public class AutoReloadTest extends SWETestBase {
    private static final String SWE_WEBVIEW_HTML = "<html><body> SWE WebView </body></html>";

    private static class WebViewCrashCriteria implements Criteria {
        WebView mWebView;
        boolean mCrashed;
        public WebViewCrashCriteria(WebView wv, boolean crashed) {
            mWebView = wv;
            mCrashed = crashed;
        }

        @Override
        public boolean isSatisfied() {
            return (mWebView.hasCrashed() == mCrashed);
        }
    }

    private int findRendererPID() {
        Context context = this.getInstrumentation().getTargetContext().getApplicationContext();
        ActivityManager manager = (ActivityManager) context.getSystemService(Context.ACTIVITY_SERVICE);
        for (RunningAppProcessInfo proc : manager.getRunningAppProcesses()) {
            if (proc.processName.contains("testapp:sandboxed_process")) {
                return proc.pid;
            }
        }
        return 0;
    }

    @Feature({"SWEWebView"})
    public void testLoadURL() throws Exception {
        TestWebServer webServer = new TestWebServer(false);
        WebView wv = getWebView();
        setupWebviewClient(wv);
        String url = webServer.setResponse(
                     "/webview.html", SWE_WEBVIEW_HTML , null);
        loadUrlSync(wv, mTestWebViewClient.mOnPageFinishedHelper, url);

        int pid = findRendererPID();

        assertTrue("PID of Renderer process could not be found", (pid != 0));

        assertTrue("Could not crash the renderer process. API crashProcessForTesting failed",
                   ChildProcessLauncher.crashProcessForTesting(pid));

        assertEquals("Renderer process could not be crashed", findRendererPID(), 0);


        assertTrue("WebView did not detect the renderer crash",
                   CriteriaHelper.pollForCriteria(new WebViewCrashCriteria(wv, true)));

        reloadWebView(wv);

        assertTrue("Reloading WebView did not resolve the renderer crash",
                   CriteriaHelper.pollForCriteria(new WebViewCrashCriteria(wv, false)));
    }

}
