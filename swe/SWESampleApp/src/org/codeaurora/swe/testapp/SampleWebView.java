/*
 *  Copyright (c) 2012-2013, The Linux Foundation. All rights reserved.
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

package org.codeaurora.swe.testapp;

import android.app.Activity;
import android.os.Bundle;
import org.codeaurora.swe.WebView;
import org.codeaurora.swe.WebViewClient;
//import android.webkit.WebView;
//import android.webkit.WebViewClient;

import android.content.Intent;
import android.content.pm.PackageManager;
import android.net.Uri;
import android.view.View.OnClickListener;

import android.view.ViewGroup;
import android.widget.FrameLayout;
import android.view.View;
import android.widget.ImageButton;
import android.widget.EditText;
import android.view.ViewGroup.LayoutParams;
import android.widget.TextView.OnEditorActionListener;
import android.widget.LinearLayout;
import android.widget.TextView;
import android.view.KeyEvent;
import android.view.inputmethod.InputMethodManager;
import android.view.inputmethod.EditorInfo;
import android.content.Context;
import android.view.View.OnFocusChangeListener;
import android.util.Log;

import org.codeaurora.swe.WebSettings.ZoomDensity;
import org.codeaurora.swe.Engine;
import android.webkit.WebResourceResponse;



public class SampleWebView extends Activity {
    private static final String TAG = "SWETest";

    private final static String INITIAL_URL = "http://www.google.com/";
    private WebView mWebView;
    private EditText mUrlTextView;
    private ImageButton mPrevButton;
    private ImageButton mNextButton;



    /** Called when the activity is first created. */
    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.main);
        ViewGroup main = (ViewGroup)findViewById(R.id.main);
        Engine.initialize(getApplicationContext());
        init();
    }

    public void init() {
        LinearLayout contentContainer = (LinearLayout) findViewById(R.id.content_container);
        mWebView = new WebView(this);
        mWebView.getSettings().setJavaScriptEnabled(true);
        mWebView.getSettings().setDefaultZoom(ZoomDensity.MEDIUM);

        // Get the intent that started this activity
        Intent intent = getIntent();
        Uri data = intent.getData();

                mWebView.setWebViewClient(new WebViewClient() {
                @Override
                public boolean shouldOverrideUrlLoading(WebView view, String url) {
                    //never ask the system to choose the app that loads the url
                    Log.e(TAG, "shouldOverrideUrl: " + url);
                    view.loadUrl(url);
                    return true;
                }

                @Override
                public boolean shouldOverrideKeyEvent(WebView view, KeyEvent event) {
                    Log.e(TAG, "shouldOverrideKeyEvent: " + event);
                    return false;
                }

                @Override
                public void onUnhandledKeyEvent(WebView view, KeyEvent event) {
                    Log.e(TAG, "onUnhandledKeyEvent: " + event);
                }

                @Override
                public void onReceivedError(WebView view, int errorCode,
                                            String description, String failingUrl) {
                    Log.e(TAG, "onReceivedError: " + errorCode + " " + description + " " +
                          failingUrl);
                }

                @Override
                public WebResourceResponse shouldInterceptRequest(WebView view, String url) {
                    Log.e(TAG, "shouldInterceptRequest: " + url);
                    return null;
                }
            });

        //initWebViewSettings(mWebView);
        mWebView.setLayoutParams(new LinearLayout.LayoutParams(
                LayoutParams.MATCH_PARENT, LayoutParams.MATCH_PARENT, 1f));
        contentContainer.addView(mWebView);

        initializeUrlField();
        initializeNavigationButtons();

        if ( Intent.ACTION_VIEW.equals(intent.getAction()) ) {
            String url = data.toString();
            mWebView.loadUrl(url);
            mUrlTextView.setText(url);
        } else {
            mWebView.loadUrl(INITIAL_URL);
            mUrlTextView.setText(INITIAL_URL);
        }
    }

    private void initWebViewSettings(WebView webView) {
        webView.setScrollbarFadingEnabled(true);
        webView.setScrollBarStyle(View.SCROLLBARS_OUTSIDE_OVERLAY);
        webView.setMapTrackballToArrowKeys(false); // use trackball directly
        // Enable the built-in zoom
        webView.getSettings().setBuiltInZoomControls(true);
        final PackageManager pm = getPackageManager();
        boolean supportsMultiTouch =
                pm.hasSystemFeature(PackageManager.FEATURE_TOUCHSCREEN_MULTITOUCH)
                || pm.hasSystemFeature(PackageManager.FEATURE_FAKETOUCH_MULTITOUCH_DISTINCT);
        webView.getSettings().setDisplayZoomControls(!supportsMultiTouch);
    }

    private void initializeUrlField() {
        mUrlTextView = (EditText) findViewById(R.id.url);
        mUrlTextView.setOnEditorActionListener(new OnEditorActionListener() {
            @Override
            public boolean onEditorAction(TextView v, int actionId, KeyEvent event) {
                if ((actionId != EditorInfo.IME_ACTION_GO) && (event == null ||
                        event.getKeyCode() != KeyEvent.KEYCODE_ENTER ||
                        event.getKeyCode() != KeyEvent.ACTION_DOWN)) {
                    return false;
                }

                mWebView.loadUrl(mUrlTextView.getText().toString());
                setKeyboardVisibilityForUrl(false);
                mUrlTextView.clearFocus();
                mWebView.requestFocus();
                return true;
            }
        });
        mUrlTextView.setOnFocusChangeListener(new OnFocusChangeListener() {
            @Override
            public void onFocusChange(View v, boolean hasFocus) {
                setKeyboardVisibilityForUrl(hasFocus);
                mNextButton.setVisibility(hasFocus ? View.GONE : View.VISIBLE);
                mPrevButton.setVisibility(hasFocus ? View.GONE : View.VISIBLE);
                if (!hasFocus) {
                    mUrlTextView.setText(mWebView.getUrl());
                }
            }
        });
    }

    private void initializeNavigationButtons() {
        mPrevButton = (ImageButton) findViewById(R.id.prev);
        mPrevButton.setOnClickListener(new OnClickListener() {
            @Override
            public void onClick(View v) {
                if (mWebView.canGoBack()) {
                    mWebView.goBack();
                }
            }
        });

        mNextButton = (ImageButton) findViewById(R.id.next);
        mNextButton.setOnClickListener(new OnClickListener() {
            @Override
            public void onClick(View v) {
                if (mWebView.canGoForward()) {
                    mWebView.goForward();
                }
            }
        });
    }

    private void setKeyboardVisibilityForUrl(boolean visible) {
        InputMethodManager imm = (InputMethodManager) getSystemService(
                Context.INPUT_METHOD_SERVICE);
        if (visible) {
            imm.showSoftInput(mUrlTextView, InputMethodManager.SHOW_IMPLICIT);
        } else {
            imm.hideSoftInputFromWindow(mUrlTextView.getWindowToken(), 0);
        }
    }

}
