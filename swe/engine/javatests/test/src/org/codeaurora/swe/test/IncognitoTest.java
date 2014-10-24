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
import org.chromium.content.browser.test.util.Criteria;
import org.chromium.content.browser.test.util.CriteriaHelper;

import org.codeaurora.swe.WebView;
import org.codeaurora.swe.WebSettings;

public class IncognitoTest extends SWETestBase {
    private static final String LOGTAG = "IncognitoTest";

    private static final String INCOGNITO_HTML_START =
        "<html><body><div id='result'>";

    private static final String SCRIPT_LOCAL_STORAGE =
        "<script>" +
        "    if (typeof(Storage) != 'undefined') {" +
        "        localStorage.setItem('search', 'normal'); "+
        "        document.getElementById('result').innerHTML = localStorage.getItem('search'); "+
        "    } else { "+
        "        document.getElementById('result').innerHTML = 'Not supported'; "+
        "    } " +
        "</script>";


    private static final String INCOGNITO_SCRIPT_LOCAL_STORAGE_READ =
      "<script>" +
        "    if (typeof(Storage) != 'undefined') { "+
        "        document.getElementById('result').innerHTML = localStorage.getItem('search'); "+
        "    } else { "+
        "        document.getElementById('result').innerHTML = 'Not supported'; "+
        "    } " +
        "</script>";

    private static final String INCOGNITO_SCRIPT_LOCAL_STORAGE =
        "<script>" +
        "    if (typeof(Storage) != 'undefined') { "+
        "        localStorage.setItem('search', 'incognito'); "+
        "        document.getElementById('result').innerHTML = localStorage.getItem('search'); "+
        "    } else { "+
        "        document.getElementById('result').innerHTML = 'Not supported'; "+
        "    } " +
        "</script>";

       private static final String SCRIPT_SESSION_STORAGE =
        "<script>" +
        "    if (typeof(Storage) != 'undefined') {" +
        "        sessionStorage.search ='normal'; "+
        "        document.getElementById('result').innerHTML = sessionStorage.search; "+
        "    } else { "+
        "        document.getElementById('result').innerHTML = 'Not supported'; "+
        "    } " +
        "</script>";


    private static final String INCOGNITO_SCRIPT_SESSION_STORAGE_READ =
      "<script>" +
        "    if (typeof(Storage) != 'undefined') { "+
        "        document.getElementById('result').innerHTML = sessionStorage.search; "+
        "    } else { "+
        "        document.getElementById('result').innerHTML = 'Not supported'; "+
        "    } " +
        "</script>";

    private static final String INCOGNITO_SCRIPT_SESSION_STORAGE =
        "<script>" +
        "    if (typeof(Storage) != 'undefined') { "+
        "        sessionStorage.search = 'incognito'; "+
        "        document.getElementById('result').innerHTML = sessionStorage.search; "+
        "    } else { "+
        "        document.getElementById('result').innerHTML = 'Not supported'; "+
        "    } " +
        "</script>";

    private static final String SCRIPT_COOKIE_READ =
        "<script>" +
            "document.getElementById('result').innerHTML = getCookie();" +
            "function getCookie() { " +
                "var name = 'wv='; " +
                "var ca = document.cookie.split(';');" +
                "for(var i=0; i < ca.length; i++) { " +
                "    var c = ca[i]; " +
                "    while (c.charAt(0)==' ') c = c.substring(1); " +
                "    if (c.indexOf(name) != -1) "+
                "       return c.substring(name.length,c.length); " +
                "} " +
                "return ''; " +
            "}" +
        "</script>";

    private static final String SCRIPT_COOKIE =
        "<script>" +
        "    var d = new Date();" +
        "    d.setTime(d.getTime()+86400000);" +
        "    document.cookie = 'wv=normal; expires=' + d.toUTCString();" +
        "</script>";

    private static final String INCOGNITO_SCRIPT_COOKIE =
        "<script>" +
        "    var d = new Date();" +
        "    d.setTime(d.getTime()+86400000);" +
        "    document.cookie = 'wv=incognito; expires=' + d.toUTCString();" +
        "</script>";

    private static final String INCOGNITO_HTML_END = "</body></html>";

    private TestWebServer mWebServer;
    private WebView mNormalWV;
    private WebView mIncognitoWV;
    private WebView mIncognitoWV2;

    private void enableRequiredSettings(WebView wv) {
        wv.getSettings().setJavaScriptEnabled(true);
        wv.getSettings().setDomStorageEnabled(true);
        wv.getSettings().setDatabaseEnabled(true);
        wv.getSettings().setAppCacheEnabled(true);
        wv.getSettings().setAppCacheMaxSize(1024*1024*2);
        wv.getSettings().setAllowFileAccess(true);
        wv.getSettings().setCacheMode(WebSettings.LOAD_DEFAULT);
        wv.getSettings().setAppCachePath("incognito_app_cache");
    }

    private void setupWebview(WebView wv) {
        assertNotNull(wv);
        enableRequiredSettings(wv);
        setupWebviewClient(wv);
    }

    @Override
    protected void setUp() throws Exception {
        super.setUp();
        clearWebStorage();
        mWebServer = new TestWebServer(false);
        mNormalWV = createWebView(false);
        setupWebview(mNormalWV);
        clearCache(mNormalWV, true);
        mIncognitoWV = createWebView(true);
        setupWebview(mIncognitoWV);
        clearCache(mIncognitoWV, true);
        mIncognitoWV2 = createWebView(true);
        setupWebview(mIncognitoWV2);
        clearCache(mIncognitoWV2, true);
    }

    @Override
    protected void tearDown() throws Exception {
        super.tearDown();
        if (mNormalWV != null)
            destroyWebView(mNormalWV);
        if (mIncognitoWV != null)
            destroyWebView(mIncognitoWV);
    }

    @Feature({"Incognito"})
    public void testSessionStorage() throws Exception {
        // Write to Localstorage for normal webview.
        String url = mWebServer.setResponse(
                     "/incognito.html", INCOGNITO_HTML_START +
                     SCRIPT_SESSION_STORAGE +
                     INCOGNITO_HTML_END, null);
        loadUrlSync(mNormalWV, mTestWebViewClient.mOnPageFinishedHelper, url);
        String result =
            DOMUtils.getNodeField("innerHTML", mNormalWV.getContentViewCore(), "result");
        assertEquals("normal", result);

        // Read from incognito webview
        url = mWebServer.setResponse(
                     "/incognito.html", INCOGNITO_HTML_START +
                     INCOGNITO_SCRIPT_SESSION_STORAGE_READ + INCOGNITO_HTML_END, null);
        loadUrlSync(mIncognitoWV, mTestWebViewClient.mOnPageFinishedHelper, url);

        // Ensure we do not share between incognito and normal
        result = DOMUtils.getNodeField("innerHTML", mIncognitoWV.getContentViewCore(), "result");
        assertEquals("undefined", result);

        // Write key to incognito session storage
        url = mWebServer.setResponse(
                     "/incognito.html", INCOGNITO_HTML_START +
                     INCOGNITO_SCRIPT_SESSION_STORAGE + INCOGNITO_HTML_END, null);
        loadUrlSync(mIncognitoWV, mTestWebViewClient.mOnPageFinishedHelper, url);

        // Check if we got key written to incognito storage
        result = DOMUtils.getNodeField("innerHTML", mIncognitoWV.getContentViewCore(), "result");
        assertEquals("incognito", result);

        // Ensure we do not share between incognito
        url = mWebServer.setResponse(
                     "/incognito.html", INCOGNITO_HTML_START +
                     INCOGNITO_SCRIPT_SESSION_STORAGE_READ + INCOGNITO_HTML_END, null);
        loadUrlSync(mIncognitoWV2, mTestWebViewClient.mOnPageFinishedHelper, url);
        result = DOMUtils.getNodeField("innerHTML", mIncognitoWV2.getContentViewCore(), "result");
        assertEquals("undefined", result);

        // destroy both incognito and sure we have cleaned up session storage
        destroyWebView(mIncognitoWV);
        destroyWebView(mIncognitoWV2);
        mIncognitoWV = null;
        mIncognitoWV2 = null;

        // recreate incognito and check if we have cleaned up correctly
        mIncognitoWV = createWebView(true);
        setupWebview(mIncognitoWV);
        url = mWebServer.setResponse(
                     "/incognito.html", INCOGNITO_HTML_START +
                     INCOGNITO_SCRIPT_SESSION_STORAGE_READ + INCOGNITO_HTML_END, null);
        loadUrlSync(mIncognitoWV, mTestWebViewClient.mOnPageFinishedHelper, url);

        result = DOMUtils.getNodeField("innerHTML", mIncognitoWV.getContentViewCore(), "result");
        assertEquals("undefined", result);
    }

    @Feature({"Incognito"})
    public void testLocalStorage() throws Exception {
        // Write to Localstorage for normal webview.
        String url = mWebServer.setResponse(
                     "/incognito.html", INCOGNITO_HTML_START +
                     SCRIPT_LOCAL_STORAGE +
                     INCOGNITO_HTML_END, null);
        loadUrlSync(mNormalWV, mTestWebViewClient.mOnPageFinishedHelper, url);
        String result =
            DOMUtils.getNodeField("innerHTML", mNormalWV.getContentViewCore(), "result");
        assertEquals("normal", result);

        // Read from incognito webview
        url = mWebServer.setResponse(
                     "/incognito.html", INCOGNITO_HTML_START +
                     INCOGNITO_SCRIPT_LOCAL_STORAGE_READ + INCOGNITO_HTML_END, null);
        loadUrlSync(mIncognitoWV, mTestWebViewClient.mOnPageFinishedHelper, url);

        // Ensure we do not share between incognito and normal
        result = DOMUtils.getNodeField("innerHTML", mIncognitoWV.getContentViewCore(), "result");
        assertEquals("", result);

        // Write key to incognito local storage
        url = mWebServer.setResponse(
                     "/incognito.html", INCOGNITO_HTML_START +
                     INCOGNITO_SCRIPT_LOCAL_STORAGE + INCOGNITO_HTML_END, null);
        loadUrlSync(mIncognitoWV, mTestWebViewClient.mOnPageFinishedHelper, url);

        // Check if we got key written to incognito storage
        result = DOMUtils.getNodeField("innerHTML", mIncognitoWV.getContentViewCore(), "result");
        assertEquals("incognito", result);

        // Ensure we share between incognito
        url = mWebServer.setResponse(
                     "/incognito.html", INCOGNITO_HTML_START +
                     INCOGNITO_SCRIPT_LOCAL_STORAGE_READ + INCOGNITO_HTML_END, null);
        loadUrlSync(mIncognitoWV2, mTestWebViewClient.mOnPageFinishedHelper, url);
        result = DOMUtils.getNodeField("innerHTML", mIncognitoWV2.getContentViewCore(), "result");
        assertEquals("incognito", result);

        // destroy both incognito and sure we have cleaned up localstorage
        destroyWebView(mIncognitoWV);
        destroyWebView(mIncognitoWV2);
        mIncognitoWV = null;
        mIncognitoWV2 = null;

        // recreate incognito and check if we have cleaned up correctly
        mIncognitoWV = createWebView(true);
        setupWebview(mIncognitoWV);
        url = mWebServer.setResponse(
                     "/incognito.html", INCOGNITO_HTML_START +
                     INCOGNITO_SCRIPT_LOCAL_STORAGE_READ + INCOGNITO_HTML_END, null);
        loadUrlSync(mIncognitoWV, mTestWebViewClient.mOnPageFinishedHelper, url);

        result = DOMUtils.getNodeField("innerHTML", mIncognitoWV.getContentViewCore(), "result");
        assertEquals("", result);
    }

    @Feature({"Incognito"})
    public void testHTTPCache() throws Exception {
        List<Pair<String, String>> headers = new ArrayList<Pair<String, String>>();
        // Set Cache-Control headers to 10 days
        headers.add(Pair.create("Cache-Control", "max-age=864000"));
        headers.add(Pair.create("Last-Modified", "Wed, 1 May 2013 00:00:00 GMT"));
        final String page = "/cache.html";
        final String testUrl = mWebServer.setResponse(
                    page, "<html><body>Cache</body></html>", headers);

        // clear cache to start off
        clearCache(mNormalWV, true);

        // Write to Cache for normal webview.
        // load to populate cache.
        loadUrlSync(mNormalWV, mTestWebViewClient.mOnPageFinishedHelper, testUrl);
        assertEquals(1, mWebServer.getRequestCount(page));

        // Load about:blank so next load is not treated as reload by webkit and force
        // revalidate with the server.
        loadUrlSync(mNormalWV, mTestWebViewClient.mOnPageFinishedHelper, "about:blank");

        // Ensure we load from cache
        loadUrlSync(mNormalWV, mTestWebViewClient.mOnPageFinishedHelper,
                        testUrl);
        assertEquals(1, mWebServer.getRequestCount(page));

        // Create a another normal webview check if we load from cache
        WebView normalWV2 = createWebView(false);
        setupWebview(normalWV2);
        loadUrlSync(normalWV2, mTestWebViewClient.mOnPageFinishedHelper, testUrl);
        assertEquals(1, mWebServer.getRequestCount(page));
        destroyWebView(normalWV2);

        // Create a incognito webview
        loadUrlSync(mIncognitoWV, mTestWebViewClient.mOnPageFinishedHelper, testUrl);

        // Ensure we do not share between incognito and normal
        // i.e We should hit the server
        assertEquals(2, mWebServer.getRequestCount(page));

        // Load about:blank so next load is not treated as reload by webkit and force
        // revalidate with the server.
        loadUrlSync(mIncognitoWV, mTestWebViewClient.mOnPageFinishedHelper, "about:blank");

        // Ensure we load from cache for incognito
        loadUrlSync(mIncognitoWV, mTestWebViewClient.mOnPageFinishedHelper,
                        testUrl);
        assertEquals(2, mWebServer.getRequestCount(page));

        // Ensure we share cache between incognito
        loadUrlSync(mIncognitoWV2, mTestWebViewClient.mOnPageFinishedHelper,
                        testUrl);
        assertEquals(2, mWebServer.getRequestCount(page));

        // destroy both incognito and sure we have cleaned up cache
        destroyWebView(mIncognitoWV);
        destroyWebView(mIncognitoWV2);
        mIncognitoWV = null;
        mIncognitoWV2 = null;

        // recreate incognito and check if we have cleaned up correctly
        mIncognitoWV = createWebView(true);
        setupWebview(mIncognitoWV);
        loadUrlSync(mIncognitoWV, mTestWebViewClient.mOnPageFinishedHelper,
                        testUrl);

        // Ensure we do not share between incognito and normal
        // i.e We should hit the server
        assertEquals(3, mWebServer.getRequestCount(page));
    }

    @Feature({"Incognito"})
    public void testCookies() throws Exception {
        // Write cookies for normal webview.
        clearCache(mNormalWV, true);
        String url = mWebServer.setResponse(
                     "/incognito.html", INCOGNITO_HTML_START +
                     SCRIPT_COOKIE + SCRIPT_COOKIE_READ +
                     INCOGNITO_HTML_END, null);
        loadUrlSync(mNormalWV, mTestWebViewClient.mOnPageFinishedHelper, url);
        String result =
            DOMUtils.getNodeField("innerHTML", mNormalWV.getContentViewCore(), "result");
        assertEquals("normal", result);

        // Setup read for incognito
        url = mWebServer.setResponse(
                     "/incognito.html", INCOGNITO_HTML_START +
                     SCRIPT_COOKIE_READ + INCOGNITO_HTML_END, null);
        loadUrlSync(mIncognitoWV, mTestWebViewClient.mOnPageFinishedHelper, url);

        // Ensure we do not share between incognito and normal
        result = DOMUtils.getNodeField("innerHTML", mIncognitoWV.getContentViewCore(), "result");
        assertEquals("", result);

        // Write cookie to incognito
        url = mWebServer.setResponse(
                     "/incognito.html", INCOGNITO_HTML_START +
                     INCOGNITO_SCRIPT_COOKIE + SCRIPT_COOKIE_READ + INCOGNITO_HTML_END, null);
        loadUrlSync(mIncognitoWV, mTestWebViewClient.mOnPageFinishedHelper, url);

        // Check if we got the key written to incognito
        result = DOMUtils.getNodeField("innerHTML", mIncognitoWV.getContentViewCore(), "result");
        assertEquals("incognito", result);

        // Ensure we share cookies between incognito
        url = mWebServer.setResponse(
                     "/incognito.html", INCOGNITO_HTML_START +
                     INCOGNITO_SCRIPT_COOKIE + SCRIPT_COOKIE_READ + INCOGNITO_HTML_END, null);
        loadUrlSync(mIncognitoWV2, mTestWebViewClient.mOnPageFinishedHelper, url);
        result = DOMUtils.getNodeField("innerHTML", mIncognitoWV2.getContentViewCore(), "result");
        assertEquals("incognito", result);

        // destroy both incognito and sure we have cleaned up cookies
        destroyWebView(mIncognitoWV);
        destroyWebView(mIncognitoWV2);
        mIncognitoWV = null;
        mIncognitoWV2 = null;

        // recreate incognito and check if we have cleaned up correctly
        mIncognitoWV = createWebView(true);
        setupWebview(mIncognitoWV);
        url = mWebServer.setResponse(
                     "/incognito.html", INCOGNITO_HTML_START +
                     SCRIPT_COOKIE_READ + INCOGNITO_HTML_END, null);
        loadUrlSync(mIncognitoWV, mTestWebViewClient.mOnPageFinishedHelper, url);

        result = DOMUtils.getNodeField("innerHTML", mIncognitoWV.getContentViewCore(), "result");
        assertEquals("", result);

    }

    final String APP_CACHE_PAGE_PATH = "/incognitoappcache.html";
    final String APP_CACHE_FILE_PATH = "/test.js";
    final String APP_CACHE_MANIFEST_PATH = "/incognito.manifest";

    private void setupAppCacheParams(WebView wv,
             String jsContents) throws Exception {
        List<Pair<String, String>> headers = new ArrayList<Pair<String, String>>();
        headers.add(Pair.create("Content-Type", "text/javascript"));
        final String scriptUrl = mWebServer.setResponse(APP_CACHE_FILE_PATH, jsContents, headers);
        final String MANIFEST_CONTENTS = "CACHE MANIFEST\nCACHE:\n" + APP_CACHE_FILE_PATH;
        List<Pair<String, String>> manifestHeaders = new ArrayList<Pair<String, String>>();
        manifestHeaders.add(Pair.create("Content-Disposition", "text/cache-manifest"));
        mWebServer.setResponse(APP_CACHE_MANIFEST_PATH, MANIFEST_CONTENTS, manifestHeaders);
        loadAppCacheURL(wv);
    }

    private void loadAppCacheURL(WebView wv)
            throws Exception {
        final String PAGE_CONTENTS =
                "<html manifest='" + APP_CACHE_MANIFEST_PATH + "'>" +
                    "<body onload='setResult()'> "+
                    "<script src='" + APP_CACHE_FILE_PATH + "'></script> " +
                    "<div id='result'></div></body>"+
                "</html>";
        String url = mWebServer.setResponse(APP_CACHE_PAGE_PATH, PAGE_CONTENTS, null);

        loadUrlSync(wv, mTestWebViewClient.mOnPageFinishedHelper, url);
        executeJavaScriptAndWaitForResult(wv,
              "window.applicationCache.update();");
    }

    @Feature({"Incognito"})
    public void testAppCache() throws Exception {
        // load appcache for normal webview.
        setupAppCacheParams( mNormalWV,
            "function setResult() {document.getElementById('result').innerHTML = 'normal';}");
        String result =
            DOMUtils.getNodeField("innerHTML", mNormalWV.getContentViewCore(), "result");
        assertEquals("normal", result);

        // setup for incognito webview
        setupAppCacheParams(mIncognitoWV, "console.log('incognito app cache test')");

        // Ensure we do not share between incognito and normal
        result = DOMUtils.getNodeField("innerHTML", mIncognitoWV.getContentViewCore(), "result");
        assertEquals("", result);

        // destroying inorder for next test to redownload the AppCache
        destroyWebView(mIncognitoWV);
        destroyWebView(mIncognitoWV2);

        // recreate incognito and check if we have cleaned up correctly
        mIncognitoWV = createWebView(true);
        setupWebview(mIncognitoWV);
        mIncognitoWV2 = createWebView(true);
        setupWebview(mIncognitoWV2);
        // Write different value to incognito
        setupAppCacheParams(mIncognitoWV,
            "function setResult() {document.getElementById('result').innerHTML = 'incognito';}");

        // Check if we got key written to incognito's app cache
        result = DOMUtils.getNodeField("innerHTML", mIncognitoWV.getContentViewCore(), "result");
        assertEquals("incognito", result);

        // Ensure we share between incognito
        loadAppCacheURL(mIncognitoWV2);
        result = DOMUtils.getNodeField("innerHTML", mIncognitoWV2.getContentViewCore(), "result");
        assertEquals("incognito", result);

        // destroy both incognito and sure we have cleaned up appcache
        destroyWebView(mIncognitoWV);
        destroyWebView(mIncognitoWV2);
        mIncognitoWV = null;
        mIncognitoWV2 = null;

        // Clear the response from server
        mWebServer.setResponseWithNotFoundStatus(APP_CACHE_FILE_PATH);

        // recreate incognito and check if we have cleaned up correctly
        mIncognitoWV = createWebView(true);
        setupWebview(mIncognitoWV);
        loadAppCacheURL(mIncognitoWV);

        result = DOMUtils.getNodeField("innerHTML", mIncognitoWV.getContentViewCore(), "result");
        assertEquals("", result);

        // Cleanup
        destroyWebView(mNormalWV);
        destroyWebView(mIncognitoWV);
    }

    private String setHTMLWithJS(String jsName) {
        return  "<html>"+
                   "<head>"+
                    "<script src='"+jsName+"'></script>"+
                "</head>"+
                "<body onload='myTest.isReady(function(){console.log(\"Ready\");})'>"+
                    "<div id='result'></div>"+
                    "<div id='ready'></div>"+
                "</body>"+
                "</html>";
    }

    private String setupIndexedDbJS(String value) {
    return  "function test() {" +
            "  request = indexedDB.open('test', 1);" +
            "  var db = null;" +
            "  var ready = false;" +
            "  request.onsuccess = function(e) {" +
            "        console.log('onsuccess');" +
            "        db = e.target.result;" +
            "        ready = true;" +
            "  };" +
            "  request.onupgradeneeded = function(e) {" +
            "        console.log('onupgradeneeded');" +
            "        db = e.target.result;" +
            "        ready = true;" +
            "        if (db.objectStoreNames.contains('info')) {" +
            "              db.deleteObjectStore('info');" +
            "        }" +
            "        var store = db.createObjectStore('info', {" +
            "             keyPath: 'type'" +
            "         });" +
            "  };" +
            "  this.addEntry = function () {" +
            "        var trans = db.transaction(['info'], 'readwrite');" +
            "        var store = trans.objectStore('info');" +
            "        var insertReq = store.put({" +
            "              'type': '"+value+"'," +
            "        });" +
            "        var this_ = this; " +
            "        trans.oncomplete = function(e) {" +
            "              console.log('addEntry Done! ');" +
            "              this_.getWVType(); " +
            "        }" +
            "  };" +
            "  this.isReady = function(cb) {" +
            "        console.log('isReady.. ' + ready);" +
            "        if (ready == true) {" +
            "              document.getElementById('ready').innerHTML = 'true';" +
            "              console.log('callback.. ');" +
            "              cb();" +
            "        } else {" +
            "              console.log('setTimeout.. ');" +
            "              setTimeout(function() {myTest.isReady(cb)}, 100);" +
            "        }" +
            "  };" +
            "  this.getWVType = function() {" +
            "        var trans = db.transaction(['info'], 'readonly');" +
            "        var store = trans.objectStore('info');" +
            "        var cursorRequest = store.openCursor();" +
            "        cursorRequest.onsuccess = function(e) {" +
            "              var result = e.target.result;" +
            "              if (result) {" +
            "                    console.log('getWVType : ' + result.value.type);" +
            "                    document.getElementById('result').innerHTML = result.value.type;" +
            "                    return result.value.type;" +
            "              } " +
            "              console.log('getWVType ERROR ');" +
            "        }" +
            " };" +
            "  return this;" +
        "};" +
        "var myTest = new test();";
    }

    private static class ValueReadyCriteria implements Criteria {
        String mValue;
        String mNodeId;
        ContentViewCore mViewCore;
        public ValueReadyCriteria(ContentViewCore viewCore, String nodeId, String value) {
            mValue = value;
            mViewCore = viewCore;
            mNodeId = nodeId;
        }

        @Override
        public boolean isSatisfied() {
            try {
                String contents = DOMUtils.getNodeField("innerHTML", mViewCore, mNodeId);
                return mValue.equals(contents);
            } catch (Throwable e) {
                Assert.fail("Failed to retrieve node contents: " + e);
                return false;
            }
        }
    }

    private void setUpJSResponse(String filename, String jsContent) {
        List<Pair<String, String>> headers = new ArrayList<Pair<String, String>>();
        headers.add(Pair.create("Content-Type", "text/javascript"));
        mWebServer.setResponse(filename, jsContent, headers);
    }

    @Feature({"Incognito"})
    public void testIndexedDB() throws Exception {
        // Write to IndexedDB for normal webview.
        // setup the JS to return string normal from indexedDb
        setUpJSResponse("/testIndexedDb.js", setupIndexedDbJS("normal"));
        String url = mWebServer.setResponse(
                     "/incognito.html", setHTMLWithJS("/testIndexedDb.js"), null);
        loadUrlSync(mNormalWV, mTestWebViewClient.mOnPageFinishedHelper, url);
        assertTrue("IndexedDB was not ready",
                CriteriaHelper.pollForCriteria(new ValueReadyCriteria(
                     mNormalWV.getContentViewCore(), "ready", "true")));
        executeJavaScriptAndWaitForResult(mNormalWV,
              "myTest.addEntry();");
        assertTrue("Reading from Normal WebView",
                CriteriaHelper.pollForCriteria(new ValueReadyCriteria(
                     mNormalWV.getContentViewCore(), "result", "normal")));

        // Read from incognito webview
        url = mWebServer.setResponse(
                     "/incognito.html", setHTMLWithJS("/testIndexedDb.js"), null);
        loadUrlSync(mIncognitoWV, mTestWebViewClient.mOnPageFinishedHelper, url);
        assertTrue("IndexedDB was not ready",
                CriteriaHelper.pollForCriteria(new ValueReadyCriteria(
                     mIncognitoWV.getContentViewCore(), "ready", "true")));
        executeJavaScriptAndWaitForResult(mIncognitoWV,
              "myTest.getWVType();");
        assertTrue("Reading from incognito WebView first time",
                CriteriaHelper.pollForCriteria(new ValueReadyCriteria(
                     mIncognitoWV.getContentViewCore(), "result", "")));

        // Write key to incognito's indexedDb
        setUpJSResponse("/testIndexedDbIncog.js", setupIndexedDbJS("incognito"));
        url = mWebServer.setResponse(
                     "/incognito.html", setHTMLWithJS("/testIndexedDbIncog.js"), null);
        loadUrlSync(mIncognitoWV, mTestWebViewClient.mOnPageFinishedHelper, url);
        assertTrue("IndexedDB was not ready",
                CriteriaHelper.pollForCriteria(new ValueReadyCriteria(
                     mIncognitoWV.getContentViewCore(), "ready", "true")));
        executeJavaScriptAndWaitForResult(mIncognitoWV,
              "myTest.addEntry();");
        assertTrue("Reading from Incognito WebView",
                CriteriaHelper.pollForCriteria(new ValueReadyCriteria(
                     mIncognitoWV.getContentViewCore(), "result", "incognito")));

        // Ensure we do share between incognito
        loadUrlSync(mIncognitoWV2, mTestWebViewClient.mOnPageFinishedHelper, url);
        assertTrue("IndexedDB was not ready",
                CriteriaHelper.pollForCriteria(new ValueReadyCriteria(
                     mIncognitoWV2.getContentViewCore(), "ready", "true")));
        // Check if we got key written to incognito's indexedDb
        executeJavaScriptAndWaitForResult(mIncognitoWV2,
              "myTest.getWVType();");
        assertTrue("Reading from othere incognito WebView",
                CriteriaHelper.pollForCriteria(new ValueReadyCriteria(
                     mIncognitoWV2.getContentViewCore(), "result", "incognito")));

        // destroy both incognito and sure we have cleaned up indexedDb
        destroyWebView(mIncognitoWV);
        destroyWebView(mIncognitoWV2);
        mIncognitoWV = null;
        mIncognitoWV2 = null;

        // recreate incognito and check if we have cleaned up correctly
        mIncognitoWV = createWebView(true);
        setupWebview(mIncognitoWV);
        url = mWebServer.setResponse(
                     "/incognito.html", setHTMLWithJS("/testIndexedDb.js"), null);
        loadUrlSync(mIncognitoWV, mTestWebViewClient.mOnPageFinishedHelper, url);
        assertTrue("IndexedDB was not ready",
                CriteriaHelper.pollForCriteria(new ValueReadyCriteria(
                     mIncognitoWV.getContentViewCore(), "ready", "true")));
        executeJavaScriptAndWaitForResult(mIncognitoWV,
              "myTest.getWVType();");
        assertTrue("Reading from incognito WebView after destroy",
                CriteriaHelper.pollForCriteria(new ValueReadyCriteria(
                     mIncognitoWV.getContentViewCore(), "result", "")));

    }

    private String setupWebSQLJS(String value) {
        return "function test() {" +
                    "var db = openDatabase('incognito', '1.0', 'Incognito DB', 1 * 1024);" +
                    "this.write = function () {" +
                    "    db.transaction(function(tx) {" +
                    "        console.log('Create Db');" +
                    "        tx.executeSql('CREATE TABLE IF NOT EXISTS WebViewType (id unique, type)');" +
                    "        console.log('Write to Db');" +
                    "       tx.executeSql('INSERT INTO WebViewType (id, type) VALUES (1, \""+value+"\")');" +
                    "    });" +
                    "};" +
                    "this.read = function() {" +
                    "   db.transaction(function(tx) {" +
                    "       tx.executeSql('SELECT * FROM WebViewType', []," +
                    "           function(tx, results) {" +
                    "               console.log('Read Db');" +
                    "               var len = results.rows.length, i;" +
                    "               console.log('len' + len);" +
                    "               for (i = 0; i < len; i++) {" +
                    "                   console.log('read : ' + results.rows.item(i).type);" +
                    "                   document.getElementById('result').innerHTML = results.rows.item(i).type;" +
                    "               }" +
                    "            }, null);" +
                    "       });" +
                    "};" +
                    "this.isReady = function() {" +
                    "   document.getElementById('ready').innerHTML = 'true';" +
                    "};" +
                "};" +
                "var myTest = new test();";
    }

    @Feature({"Incognito"})
    public void testWebSQLStorage() throws Exception {
        // Write to WebSQL for normal webview.
        // setup the JS to return string normal from WebSQL
        setUpJSResponse("/testWebSQL.js", setupWebSQLJS("normal"));
        String url = mWebServer.setResponse(
                     "/incognito.html", setHTMLWithJS("/testWebSQL.js"), null);
        loadUrlSync(mNormalWV, mTestWebViewClient.mOnPageFinishedHelper, url);
        executeJavaScriptAndWaitForResult(mNormalWV,
              "myTest.write();");
        executeJavaScriptAndWaitForResult(mNormalWV,
              "myTest.read();");
        assertTrue("Reading from Normal WebView",
                CriteriaHelper.pollForCriteria(new ValueReadyCriteria(
                     mNormalWV.getContentViewCore(), "result", "normal")));

        // Read from incognito webview
        url = mWebServer.setResponse(
                     "/incognito.html", setHTMLWithJS("/testWebSQL.js"), null);
        loadUrlSync(mIncognitoWV, mTestWebViewClient.mOnPageFinishedHelper, url);
        executeJavaScriptAndWaitForResult(mIncognitoWV,
              "myTest.read();");
        assertTrue("Reading from incognito WebView first time",
                CriteriaHelper.pollForCriteria(new ValueReadyCriteria(
                     mIncognitoWV.getContentViewCore(), "result", "")));


        // Write key to incognito's WebSQL
        setUpJSResponse("/testWebSQLIncog.js", setupWebSQLJS("incognito"));
        url = mWebServer.setResponse(
                     "/incognito.html", setHTMLWithJS("/testWebSQLIncog.js"), null);
        loadUrlSync(mIncognitoWV, mTestWebViewClient.mOnPageFinishedHelper, url);
        executeJavaScriptAndWaitForResult(mIncognitoWV,
              "myTest.write();");
        executeJavaScriptAndWaitForResult(mIncognitoWV,
              "myTest.read();");
        assertTrue("Reading from Incognito WebView",
                CriteriaHelper.pollForCriteria(new ValueReadyCriteria(
                     mIncognitoWV.getContentViewCore(), "result", "incognito")));

        // // Ensure we do share between incognito
        loadUrlSync(mIncognitoWV2, mTestWebViewClient.mOnPageFinishedHelper, url);
        executeJavaScriptAndWaitForResult(mIncognitoWV2,
              "myTest.read();");
        // Check if we got key written to incognito's indexedDb
        assertTrue("Reading from othere incognito WebView",
                CriteriaHelper.pollForCriteria(new ValueReadyCriteria(
                     mIncognitoWV2.getContentViewCore(), "result", "incognito")));

        // destroy both incognito and sure we have cleaned up indexedDb
        destroyWebView(mIncognitoWV);
        destroyWebView(mIncognitoWV2);
        mIncognitoWV = null;
        mIncognitoWV2 = null;

        // recreate incognito and check if we have cleaned up correctly
        mIncognitoWV = createWebView(true);
        setupWebview(mIncognitoWV);
        url = mWebServer.setResponse(
                     "/incognito.html", setHTMLWithJS("/testWebSQL.js"), null);
        loadUrlSync(mIncognitoWV, mTestWebViewClient.mOnPageFinishedHelper, url);
        executeJavaScriptAndWaitForResult(mIncognitoWV,
              "myTest.read();");
        assertTrue("Reading from incognito WebView first time",
                CriteriaHelper.pollForCriteria(new ValueReadyCriteria(
                     mIncognitoWV.getContentViewCore(), "result", "")));
    }
}
