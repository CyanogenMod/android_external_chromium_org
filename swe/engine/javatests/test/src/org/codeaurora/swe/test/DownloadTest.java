/*
 *  Copyright (c) 2015, The Linux Foundation. All rights reserved.
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

import java.util.List;
import java.util.ArrayList;

import junit.framework.Assert;

import org.chromium.base.test.util.Feature;
import org.chromium.net.test.util.TestWebServer;

import org.chromium.content.browser.test.util.CallbackHelper;
import org.codeaurora.swe.BrowserDownloadListener;

import org.codeaurora.swe.WebView;

import java.util.Map;
import java.util.HashMap;

public class DownloadTest extends SWETestBase {

    private static class DownloadHelper extends CallbackHelper {
        String mReferer;
        long mContentLength;

        public long getContentLength() {
            assert getCallCount() > 0;
            return mContentLength;
        }

        public String getReferer() {
            assert getCallCount() > 0;
            return mReferer;
        }

        public void notifyCalled(String referer, long contentLength) {
            mContentLength = contentLength;
            mReferer = referer;
            notifyCalled();
        }
    }

    private class TestDownloadListener extends BrowserDownloadListener {
        private DownloadHelper mDownloadHelper;

        public TestDownloadListener() {
            mDownloadHelper = new DownloadHelper();
        }

        public DownloadHelper getDownloadHelper() {
            return mDownloadHelper;
        }

        public void onDownloadStart(String url,
                String userAgent,
                String contentDisposition,
                String mimeType,
                String referer,
                long contentLength) {
            getDownloadHelper().notifyCalled(referer, contentLength);
        }

    }

    @Feature({"SWEWebView"})
    public void testDownload() throws Exception {
        TestWebServer webServer = new TestWebServer(false);
        TestDownloadListener downloadListener = new TestDownloadListener();

        String data = "SWE download data";
        String contentDisposition = "attachment;filename=\"download.txt\"";
        String mimeType = "text/plain";


        List<Pair<String, String>> downloadHeaders = new ArrayList<Pair<String, String>>();
        downloadHeaders.add(Pair.create("Content-Disposition", contentDisposition));
        downloadHeaders.add(Pair.create("Content-Type", mimeType));
        downloadHeaders.add(Pair.create("Content-Length", Integer.toString(data.length())));
        String downloadUrl = webServer.setResponse(
                    "/download.txt", data, downloadHeaders);

        Map<String, String> requestHeaders = new HashMap<String, String>();
        requestHeaders.put("Referer",downloadUrl);


        WebView wv = getWebView();
        wv.getSettings().setJavaScriptEnabled(true);
        setupWebviewClient(wv);
        wv.setDownloadListener(downloadListener);
        loadUrlSync(wv, mTestWebViewClient.mOnPageFinishedHelper, downloadUrl, requestHeaders);

        DownloadHelper downloadHelper = downloadListener.getDownloadHelper();
        assertEquals(downloadHelper.getReferer(),downloadUrl);
        assertEquals(downloadHelper.getContentLength(),data.length());
    }



}
