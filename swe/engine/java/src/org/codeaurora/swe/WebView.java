/*
 * Copyright (c) 2013-2014 The Linux Foundation. All rights reserved.
 * Not a contribution.
 *
 * Copyright (C) 2012 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */


package org.codeaurora.swe;

import java.lang.reflect.Constructor;
import java.lang.reflect.Method;
import java.io.File;
import java.io.FileOutputStream;
import java.io.InputStream;
import java.io.OutputStream;
import java.util.Map;

import org.chromium.base.CalledByNative;
import org.chromium.base.JNINamespace;
import org.chromium.base.ThreadUtils;
import org.chromium.components.web_contents_delegate_android.WebContentsDelegateAndroid;
import org.chromium.components.navigation_interception.InterceptNavigationDelegate;
import org.chromium.components.navigation_interception.NavigationParams;
import org.chromium.content.browser.ContentSettings;
import org.chromium.content.browser.ContentVideoView;
import org.chromium.content.browser.ContentViewCore;
import org.chromium.content.browser.ContentViewRenderView;
import org.chromium.content.browser.ContentViewStatics;
import org.chromium.content.browser.LoadUrlParams;
import org.chromium.content.browser.NavigationHistory;
import org.chromium.content.browser.WebContentsObserverAndroid;
import org.chromium.content.browser.ContentReadbackHandler;
import org.chromium.content.browser.ContentReadbackHandler.GetBitmapCallback;
import org.chromium.ui.gfx.DeviceDisplayInfo;
import org.chromium.ui.base.WindowAndroid;
import org.codeaurora.swe.utils.Logger;

import android.app.Activity;
import android.content.Context;
import android.content.res.Configuration;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.graphics.Canvas;
import android.graphics.Picture;
import android.graphics.Rect;
import android.net.http.SslCertificate;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.util.AttributeSet;
import android.util.Log;
import android.view.KeyEvent;
import android.view.MotionEvent;
import android.view.View;
import android.view.accessibility.AccessibilityEvent;
import android.view.accessibility.AccessibilityNodeInfo;
import android.view.accessibility.AccessibilityNodeProvider;
import android.view.inputmethod.EditorInfo;
import android.view.inputmethod.InputConnection;
import android.widget.FrameLayout;
import android.net.http.SslError;
import android.webkit.ConsoleMessage;
import android.webkit.ValueCallback;

import org.chromium.android_webview.AwBrowserContext;
import org.chromium.android_webview.AwContents;
// SWE-feature-create-window
import org.chromium.android_webview.AwContents.CreateWindowParams;
// SWE-feature-create-window
import org.chromium.android_webview.AwLayoutSizer;
import org.chromium.android_webview.AwContentsClient;
import org.chromium.android_webview.AwSettings;
import org.chromium.android_webview.AwHttpAuthHandler;
import org.chromium.android_webview.JsPromptResultReceiver;
import org.chromium.android_webview.JsResultReceiver;
import org.codeaurora.swe.Engine.StartupCallback;
import org.codeaurora.swe.GeolocationPermissions;
import org.chromium.ui.base.ActivityWindowAndroid;

import android.view.ViewGroup.LayoutParams;
import android.view.WindowManager;

import android.util.Log;

import com.google.common.annotations.VisibleForTesting;

@JNINamespace("content")
public class WebView extends FrameLayout {

    public static final String SCHEME_TEL = "tel:";
    public static final String SCHEME_MAILTO = "mailto:";
    public static final String SCHEME_GEO = "geo:0,0?q=";
    private static final String WEBVIEW_FACTORY =
        "org.codeaurora.swe.WebViewFactoryProvider";
    private static final String TTFP_TAG = "TTFP";
    static final String WEB_ARCHIVE_EXTENSION = ".mht";


    public static class HitTestResult {

        public static final int UNKNOWN_TYPE = 0;
        public static final int ANCHOR_TYPE = 1;
        public static final int PHONE_TYPE = 2;
        public static final int GEO_TYPE = 3;
        public static final int EMAIL_TYPE = 4;
        public static final int IMAGE_TYPE = 5;
        public static final int IMAGE_ANCHOR_TYPE = 6;
        public static final int SRC_ANCHOR_TYPE = 7;
        public static final int SRC_IMAGE_ANCHOR_TYPE = 8;
        public static final int EDIT_TEXT_TYPE = 9;

        private int mType;
        private String mExtra;

        public HitTestResult() {
            mType = UNKNOWN_TYPE;
        }

        public void setType(int type) {
            mType = type;
        }

        public void setExtra(String extra) {
            mExtra = extra;
        }

        public int getType() {
            return mType;
        }

        public String getExtra() {
            return mExtra;
        }
    }

    public interface PictureListener {
        @Deprecated
        public void onNewPicture(WebView view, Picture picture);
    }

    public interface FindListener {
        public void onFindResultReceived(int activeMatchOrdinal, int numberOfMatches,
            boolean isDoneCounting);
    }

    public interface WebViewDelegate {
        public void loadUrl(String url, Map<String, String> headers);
        public void stopLoading();
        public void destroy();
        public void setWebChromeClient(WebChromeClient client);
        public void setWebViewClient(WebViewClient client);
        public ContentViewRenderView getRenderTarget();
        public WebView getNewWebViewWithAcceleratorDiabled(Context context, AttributeSet attrs, int defStyle, boolean privateBrowsing);
        public void onPause();
        public void onResume();
    }

    public class WebViewTransport {
        private WebView mWebview;

        public synchronized void setWebView(WebView webview) {
            mWebview = webview;
        }

        public synchronized WebView getWebView() {
            return mWebview;
        }
    }

    private static final String LOGTAG = "SWEWebTab";

    private AwContents mAwContents = null;
    private AwSettings mAwSettings = null;
    private String mUrl;
    private boolean mLoaded = false;
    private int mProgress = 100;
    private AwContentsClientProxy mAwContentsClientProxy = null;
    private float mCurrentTouchOffsetX;
    private float mCurrentTouchOffsetY = Float.MAX_VALUE;
    private WebSettings mWebSettings = null;
    private double mDIPScale;
    private WebView.FindListener mFindlistener;
    private ContentViewRenderView mRenderTarget = null;
    private WindowAndroid mWindowAndroid = null;
    private boolean mOverlayHorizontalScrollbar = true;
    private boolean mOverlayVerticalScrollbar = false;
    protected WebViewHandler mWebViewHandler = null;
    protected Object mUserData = null;
    private PictureListener mPictureListener = null;
    private Picture mLastCapturedPicture = null;
    private boolean mPendingCapturePictureAsync = false;
    private boolean mPendingCaptureBitmapAsync = false;
    private ValueCallback<Bitmap> mCaptureBitmapAsyncCallback = null;
    private FindActionModeCallback mFindCallback;
    private boolean mFindIsUp;
    private boolean mReady = false;
    private boolean mCrashed = false;

    private boolean mLastMotionEventUp = false;
    private boolean mEnableAccelerator = true;
    private Accelerator mAccelerator = null;
    private ContentReadbackHandler mContentReadbackHandler;

    public WebView(Context context) {
        this(context, null);
    }

    public WebView(Context context, AttributeSet attrs) {
        this(context, attrs, 0);
    }

    public WebView(Context context, AttributeSet attrs, int defStyle) {
        this(context, attrs, defStyle, false);
    }

    public WebView(Context context, AttributeSet attrs, int defStyle,
            boolean privateBrowsing) {
        this(context, attrs, defStyle, privateBrowsing, true);
    }

    private WebView(Context context, AttributeSet attrs, int defStyle,
            boolean privateBrowsing, boolean enableAccelerator) {
        super(context, attrs, defStyle);
        if (context == null) {
            throw new IllegalArgumentException("Invalid context argument");
        }

        setFocusable(true);
        setFocusableInTouchMode(true);

        Engine.initialize(context);

        if (!Engine.getIsAWCRendering()) {
            if (context instanceof android.app.Activity) {
                mWindowAndroid = new ActivityWindowAndroid((Activity) context);
            }
        }

        mAwContentsClientProxy = new AwContentsClientProxy(this);
        mAwSettings = new AwSettings(context, true, false);
        mAwContents = new AwContents(Engine.getAwBrowserContext(), this, this.getContext() ,new InternalAccessAdapter(),
                           new NativeGLDelegate(), mAwContentsClientProxy, mAwSettings,
                           new AwContents.DependencyFactory(), mWindowAndroid, privateBrowsing);
        mWebViewHandler = new WebViewHandler(this);

        if (AwContents.isUsingSurfaceView() && mWindowAndroid != null) {

            mRenderTarget = new ContentViewRenderView(context) {
                @Override
                protected void onReadyToRender() {
                    super.onReadyToRender();
                    Logger.warn("RenderTarget ready");
                    requestFocus();
                    mReady = true;
                }
            };

            mRenderTarget.onNativeLibraryLoaded(mWindowAndroid);
            // add this rendertarget to Webview
            addView(mRenderTarget,
                    new FrameLayout.LayoutParams(
                            FrameLayout.LayoutParams.MATCH_PARENT,
                            FrameLayout.LayoutParams.MATCH_PARENT));

            mRenderTarget.setCurrentContentViewCore(mAwContents.getContentViewCore());
            mAwContents.getContentViewCore().onShow();

            mContentReadbackHandler = new ContentReadbackHandler() {
                    @Override
                    protected boolean readyForReadback() {
                        return true;
                    }
                };

            mContentReadbackHandler.initNativeContentReadbackHandler();
        }

        mWebSettings = new WebSettings(this);
        if (!AwContents.isUsingSurfaceView()) {
            setOverScrollMode(View.OVER_SCROLL_ALWAYS);
        }

        if (privateBrowsing) {
            mWebSettings.setPrivateBrowsingEnabled(true);
        }
        //SWE-FIXME : Sweet disabled.
        mEnableAccelerator = false;//enableAccelerator && !AwContents.isFastWebViewDisabled();
        if (mEnableAccelerator) {
            try {
                Constructor<?> constructor = Class.forName(
                        "com.qualcomm.qti.sweetview.SweetAccelerator")
                        .getConstructor(WebView.class,
                                WebView.WebViewDelegate.class, Context.class,
                                AttributeSet.class, Integer.TYPE, Boolean.TYPE);
                WebViewDelegate webViewDelegate = new WebViewDelegate() {
                    @Override
                    public void loadUrl(String url, Map<String, String> headers) {
                        if (!url.startsWith("javascript"))
                            Log.v(TTFP_TAG, "B:loading");
                        loadUrlDirectly(url, headers);
                    }

                    @Override
                    public void stopLoading() {
                        stopLoadingDirectly();
                    }

                    @Override
                    public void destroy() {
                        destroyDirectly();
                    }

                    @Override
                    public void setWebChromeClient(WebChromeClient client) {
                        setWebChromeClientDirectly(client);
                    }

                    @Override
                    public void setWebViewClient(WebViewClient client) {
                        setWebViewClientDirectly(client);
                    }

                    @Override
                    public ContentViewRenderView getRenderTarget() {
                        return mRenderTarget;
                    }

                    @Override
                    public WebView getNewWebViewWithAcceleratorDiabled(
                            Context context, AttributeSet attrs, int defStyle,
                            boolean privateBrowsing) {
                        WebView webView = new WebView(context, attrs, defStyle,
                                privateBrowsing, false);
                        return webView;
                    }

                    @Override
                    public void onPause() {
                        onPauseDirectly();
                    }

                    @Override
                    public void onResume() {
                        onResumeDirectly();
                    }
                };
                mAccelerator = (Accelerator)constructor.newInstance(this, webViewDelegate, context, attrs, defStyle, privateBrowsing);
            } catch (Exception e) {
                Logger.dumpTrace(e);
            }
        }
    }

    public void onPause() {
        if (mAccelerator != null) {
            mAccelerator.onPause();
            return;
        }
        onPauseDirectly();
    }

    private void onPauseDirectly() {
        mReady = false;
        //onPause is not handled to emulate Chrome's behavior
        //pages are active and JavaScript keeps running even in
        //the background
        mAwContents.onPause();
    }

    public void onResume() {
        if (mAccelerator != null) {
            mAccelerator.onResume();
            return;
        }
        onResumeDirectly();
    }

    private void onResumeDirectly() {
        // Android configuration change for example locale could trigger
        // destroying the activity we would need to
        // read the new resources once again.
        Engine.registerResources(getContext());
        mAwContents.onResume();
    }

    public void loadData(String data, String mimeType, String encoding) {
        loadDataWithBaseURL(null, data, mimeType, encoding, null);
    }

    public void loadDataWithBaseURL(String baseUrl, String data, String mimeType, String encoding, String historyUrl) {
        LoadUrlParams params = LoadUrlParams.createLoadDataParamsWithBaseUrl(data, mimeType, false, baseUrl, historyUrl, encoding);
        mAwContents.loadUrl(params);
    }

    public void loadUrl(String url) {
        loadUrl(url, null);
    }

    public void loadUrl(String url, Map<String, String> headers) {
        if (!url.startsWith("javascript"))
            Log.v(TTFP_TAG, "disable-fast-webview flag:" + AwContents.isFastWebViewDisabled());
        if (mAccelerator != null) {
            mAccelerator.loadUrl(url, headers);
            return;
        }
        loadUrlDirectly(url, headers);
    }

    private void loadUrlDirectly(String url, Map<String, String> headers) {
        url = sanitizeUrl(url);
        mUrl = url;
        LoadUrlParams params = new LoadUrlParams(url);
        if (headers != null) {
            params.setExtraHeaders(headers);
        }
        mAwContents.loadUrl(params);
    }

    public void saveViewState(String filename, android.webkit.ValueCallback<String> callback) {
        String path = this.getContext().getFilesDir().getAbsolutePath() +
                "/" + filename + WEB_ARCHIVE_EXTENSION;
        saveWebArchive(path, false, callback);
    }

    public void loadViewState(String filename) {
        String url = sanitizeUrl("file://" + this.getContext().getFilesDir().getAbsolutePath() +
                "/" + filename);
        mUrl = url;
        LoadUrlParams params = new LoadUrlParams(url);
        params.setCanLoadLocalResources(true);
        mAwContents.loadUrl(params);
    }

    public void destroy() {
        if (mAccelerator != null) {
            mAccelerator.destroy();
            return;
        }
        destroyDirectly();
    }

    private void destroyDirectly() {
        if (AwContents.isUsingSurfaceView() && mWindowAndroid != null) {
            removeView(mRenderTarget);
            mRenderTarget.destroy();
            mRenderTarget = null;
        }
        mAwContents.destroy();
        // removing clients as well.
        mAwContentsClientProxy.setWebViewClient(null);
        mAwContentsClientProxy.setWebChromeClient(null);
        mAwContents = null;
    }

    @VisibleForTesting
    public ContentViewCore getContentViewCore() {
        if (mAwContents != null)
            return mAwContents.getContentViewCore();

        return null;

    }

    public String getTitle() {
        String title = mAwContents.getTitle();
        if (title != null && title.trim().isEmpty())
            return null;
        return title;
    }

    @Deprecated
    public Bitmap capturePicture() {
        // Not implemented; use captureBitmapAsync() instead.
        return null;
    }

    /**
     * @param callback will receive API callback with Bitmap of content.
     * @param x the x coordinate of the top left corner of content rect.
     * @param y the y coordinate of the top left corner of content rect.
     * @param width the width of the needed content specified in CSS pixels.
     * @param height the height of the needed content specified in CSS pixels.
     * @param scale bitmap returned will be scaled to specified value.
     *
     * This api is used to get portion or all of the content bitmap.
     * The returned bitmap(JPEG encoded) will be bounded to the content
     * rectangle before painting.
     *
     * 'scale' is bounded between 0.0 and 1.0
     *
     * Empty bitmap is returned in case where after applying devicescalefactor,
     * 'scale' and bounded rect if resultant width or height of bitmap exceeds
     * that of the max JPEG limit of 64K pixels.
     */

    public void captureContentBitmap(final ValueCallback<Bitmap> callback,
                                     int x,
                                     int y,
                                     int width,
                                     int height,
                                     float scale) {
        if (AwContents.isUsingSurfaceView()) {
            mCaptureBitmapAsyncCallback = callback;
            if (!mPendingCaptureBitmapAsync && callback != null) {
                mPendingCaptureBitmapAsync = mAwContents.captureBitmapAsync(
                                                                 x,
                                                                 y,
                                                                 width,
                                                                 height,
                                                                 scale);
            }
            return;
        }
        callback.onReceiveValue(null);
    }

    void onNewAsyncBitmap(byte[] data, int length, int width, int height) {
        mPendingCaptureBitmapAsync = false;
        if (mCaptureBitmapAsyncCallback != null) {
            //create bitmap.
            Bitmap bmp;
            final BitmapFactory.Options options = new BitmapFactory.Options();
            options.inMutable = true;
            options.inPurgeable = true;
            options.inInputShareable = true;
            //TODO take config params as input?
            options.inPreferredConfig = Bitmap.Config.ARGB_8888;
            try {
                bmp = BitmapFactory.decodeByteArray(data, 0, data.length, options);
                mCaptureBitmapAsyncCallback.onReceiveValue(bmp);
            } catch (OutOfMemoryError e) {
                Logger.warn("Out of memory error captureContentBitmap Failed");
                mCaptureBitmapAsyncCallback.onReceiveValue(null);
            }
        }
    }

    public Bitmap getViewportBitmap() {

        if (AwContents.isUsingSurfaceView())
            return null;

        Bitmap b = Bitmap.createBitmap(getWidth(), getHeight(), Bitmap.Config.ARGB_8888);
        Canvas c = new Canvas(b);
        final int left = getViewScrollX();
        final int top = getViewScrollY();
        // We might need to consider embedded titlebar height, if we ever support it.
        c.translate(-left, -top);
        draw(c);
        c.setBitmap(null);
        return b;
    }


    public void getContentBitmapAsync(float scale, Rect srcRect,
                                      final ValueCallback<Bitmap> callback) {
        if (!AwContents.isUsingSurfaceView())
            return;

        GetBitmapCallback mycallback = new GetBitmapCallback() {
           @Override
           public void onFinishGetBitmap(Bitmap bitmap) {
               callback.onReceiveValue(bitmap);
           }
        };

        mContentReadbackHandler.getContentBitmapAsync(scale, srcRect,
                                                      mAwContents.getContentViewCore(),
                                                      mycallback);

    }

    public void stopLoading() {
        if (mAccelerator != null) {
            mAccelerator.stopLoading();
            return;
        }
        stopLoadingDirectly();
    }

    private void stopLoadingDirectly() {
        mAwContents.stopLoading();
        mProgress = 100;
    }

    public void setWebViewClient(WebViewClient client) {
        if (mAccelerator != null) {
            mAccelerator.setWebViewClient(client);
            return;
        }
        setWebViewClientDirectly(client);
    }

    private void setWebViewClientDirectly(WebViewClient client) {
        mAwContentsClientProxy.setWebViewClient(client);
    }

    public void setWebChromeClient(WebChromeClient client) {
        if (mAccelerator != null) {
            mAccelerator.setWebChromeClient(client);
            return;
        }
        setWebChromeClientDirectly(client);
    }

    private void setWebChromeClientDirectly(WebChromeClient client) {
        mAwContentsClientProxy.setWebChromeClient(client);
    }

    public boolean canGoBack() {
        return mAwContents.canGoBack();
    }

    public void goBack() {
        mAwContents.goBack();
    }

    public void goForward() {
        mAwContents.goForward();
    }

    public boolean canGoForward() {
        return mAwContents.canGoForward();
    }

    public String getUrl() {
        return mAwContents.getUrl();
    }

    public boolean canZoomIn() {
        return mAwContents.canZoomIn();
    }

    public boolean canZoomOut() {
        return mAwContents.canZoomOut();
    }

    public boolean zoomIn() {
        return mAwContents.zoomIn();
    }

    public boolean zoomOut() {
        return mAwContents.zoomOut();
    }

    public int getProgress() {
        return mProgress;
    }

    public void reload() {
        mAwContents.reload();
    }

    public WebSettings getSettings() {
        return mWebSettings;
    }

    public void invokeZoomPicker() {
        mAwContents.invokeZoomPicker();
    }

    public float getScale() {
        return mAwContents.getScale();
    }


    public boolean pageDown(boolean b) {
        return mAwContents.pageDown(b);
    }

    public boolean pageUp(boolean b) {
        return mAwContents.pageUp(b);
    }

    public Bitmap getBitmap() {
        return null;
    }

    public Bitmap getBitmap(int width, int height) {
        return null;
    }

    public void removeJavascriptInterface(String name) {
        mAwContents.removeJavascriptInterface(name);
    }

    public void addJavascriptInterface(Object object, String name) {
        mAwContents.addPossiblyUnsafeJavascriptInterface(object, name, null);
    }

    public void evaluateJavascript (String script, ValueCallback<String> resultCallback) {
        mAwContents.evaluateJavaScript(script, resultCallback);
    }

    public void flingScroll(int vx, int vy) {
        mAwContents.flingScroll(vx, vy);
    }

    public void goBackOrForward(int offset) {
        mAwContents.goBackOrForward(offset);
    }

    public boolean canGoBackOrForward(int offset) {
        return mAwContents.canGoBackOrForward(offset);
    }

    public WebBackForwardList copyBackForwardList() {
        return new WebBackForwardList(mAwContents.getNavigationHistory());
    }

    public Bitmap getFavicon() {
        NavigationHistory history = mAwContents.getNavigationHistory();
        int currentIndex = history.getCurrentEntryIndex();
        if (currentIndex >= 0 && currentIndex < history.getEntryCount()) {
            return history.getEntryAtIndex(currentIndex).getFavicon();
        }
        return null;
    }

    public SslCertificate getCertificate() {
        return mAwContents.getCertificate();
    }

    public WebBackForwardList restoreState(Bundle mSavedState) {
        return (mAwContents.restoreState(mSavedState) == true) ?
                copyBackForwardList() : null;
    }

    public WebBackForwardList saveState(Bundle mSavedState) {
        return (mAwContents.saveState(mSavedState) == true) ?
                copyBackForwardList() : null;
    }

    public String getOriginalUrl() {
        NavigationHistory history = mAwContents.getNavigationHistory();
        int currentIndex = history.getCurrentEntryIndex();
        if (currentIndex >= 0 && currentIndex < history.getEntryCount()) {
            return history.getEntryAtIndex(currentIndex).getOriginalUrl();
        }
        return null;
    }

    public void clearHistory() {
        mAwContents.clearHistory();
    }

    public void requestImageRef(Message msg) {
        mAwContents.requestImageRef(msg);
    }

    public HitTestResult getHitTestResult() {
        AwContents.HitTestData currData =  mAwContents.getLastHitTestResult();
        HitTestResult result = new HitTestResult();
        result.setType(currData.hitTestResultType);
        result.setExtra(convertToHitTestExtra(currData));
        return result;
    }

    public void requestFocusNodeHref(Message msg) {
        mAwContents.requestFocusNodeHref(msg);
    }

    public void setFindListener(WebView.FindListener listener){
        mAwContentsClientProxy.setFindListener(listener);
    }

    public void documentHasImages(Message message) {
        mAwContents.documentHasImages(message);
    }

    public void clearSslPreferences() {
        mAwContents.clearSslPreferences();
    }

    public void findAllAsync(String searchString) {
         mAwContents.findAllAsync(searchString);
    }

    public void findNext(boolean forward) {
        mAwContents.findNext(forward);
    }

    public void clearMatches() {
        mAwContents.clearMatches();
    }

    public String[] getHttpAuthUsernamePassword(String host, String realm) {
        return mAwContents.getHttpAuthUsernamePassword(host, realm);
    }

    public void setHttpAuthUsernamePassword(String host, String realm, String username,
            String password) {
       mAwContents.setHttpAuthUsernamePassword(host, realm, username, password);
    }

    public int findAll(String find) {
        //SWE-FIXME
        return 0;
    }

    public static String findAddress(String addr) {
        return ContentViewStatics.findAddress(addr);
    }

    @Override
    // SWE: WebView maintains state on the scrollBar style set by the
    // user but it does not yet support the View.SCROLLBARS_INSIDE_INSET
    // and View.SCROLLBARS_OUTSIDE_INSET styles. Scrollbars are owned by
    // cc/layers/scrollbar_layer.cc and there is currently no api exposed to
    // set the scrollBar style.
    public void setScrollBarStyle(int style) {
        //TODO: Temporary solution until we figure out who owns and draws the
        //scrollbars.
        if (style == View.SCROLLBARS_INSIDE_INSET
                || style == View.SCROLLBARS_OUTSIDE_INSET) {
            mOverlayHorizontalScrollbar = mOverlayVerticalScrollbar = false;
        } else {
            mOverlayHorizontalScrollbar = mOverlayVerticalScrollbar = true;
        }
    }

    public boolean overlayHorizontalScrollbar() {
        return mOverlayHorizontalScrollbar;
    }

    public boolean overlayVerticalScrollbar() {
        return mOverlayVerticalScrollbar;
    }

    public void setHorizontalScrollbarOverlay(boolean overlay) {
        mOverlayHorizontalScrollbar = overlay;
    }

    public void setVerticalScrollbarOverlay(boolean overlay) {
        mOverlayVerticalScrollbar = overlay;
    }

    public void savePassword(String host, String userName, String password) {
        // TODO Auto-generated method stub
    }

    public void setCertificate(SslCertificate certificate) {
        // TODO Auto-generated method stub
    }

    public void debugDump() {
        // TODO Auto-generated method stub
    }

    public void clearCache(boolean includeDiskFiles) {
        mAwContents.clearCache(includeDiskFiles);
    }

    public void clearFormData() {
        mAwContents.hideAutofillPopup();
    }

    public void pauseTimers() {
        mAwContents.pauseTimers();
    }

    public void resumeTimers() {
        mAwContents.resumeTimers();
    }

    public void setInitialScale(int scale) {
        mAwContents.getSettings().setInitialPageScale((float)scale);
    }

    public void setNetworkAvailable(boolean networkUp) {
        mAwContents.setNetworkAvailable(networkUp);
    }

    public void freeMemory() {
        // TODO Auto-generated method stub
    }

    public void setDownloadListener(DownloadListener listener) {
        mAwContentsClientProxy.setDownloadListener(listener);
    }

    public void setMapTrackballToArrowKeys(boolean b) {
        // TODO Auto-generated method stub
    }

    public boolean isPrivateBrowsingEnabled() {
        return (mWebSettings != null) ? mWebSettings.isPrivateBrowsingEnabled() : false;
    }

    public void setPictureListener(WebView.PictureListener listener) {
        mPictureListener = listener;
    }

    public void saveWebArchive(final String basename, final boolean autoname,
            final ValueCallback<String> callback) {
        if (!ThreadUtils.runningOnUiThread()) {
            ThreadUtils.postOnUiThread(new Runnable() {
                @Override
                public void run() {
                    saveWebArchive(basename, autoname, callback);
                }
            });
            return;
        }
        mAwContents.saveWebArchive(basename, autoname, callback);
    }

    public void saveWebArchive(String filename) {
        mAwContents.saveWebArchive(filename, false, null);
    }

    /**
     * Enable/disable debugging of web contents (HTML/CSS/JavaScript) loaded
     * into any WebView. This setting pertains to ALL WebView instances.
     */
    public static void setWebContentsDebuggingEnabled(boolean enabled) {
        Engine.setWebContentsDebuggingEnabled(enabled);
    }


    //Android WebView hidden methods and class

    /**
     * Gets the chrome handler.
     * @return the current WebChromeClient instance.
     *
     * This is an implementation detail.
     */
    public WebChromeClient getWebChromeClient() {
        return mAwContentsClientProxy.getWebChromeClient();
    }

    /**
     * Gets the WebViewClient
     * @return the current WebViewClient instance.
     *
     * This is an implementation detail.
     */
    public WebViewClient getWebViewClient() {
        return mAwContentsClientProxy.getWebViewClient();
    }

    /**
     * Set the back/forward list client. This is an implementation of
     * WebBackForwardListClient for handling new items and changes in the
     * history index.
     * @param client An implementation of WebBackForwardListClient.
     */
    public void setWebBackForwardListClient(WebBackForwardListClient client) {
        mAwContentsClientProxy.setWebBackForwardListClient(client);
    }

    /**
     * Gets the WebBackForwardListClient.
     */
    public WebBackForwardListClient getWebBackForwardListClient() {
        return mAwContentsClientProxy.getWebBackForwardListClient();
    }

    // This method returns the height of the embedded title bar if one is set via the
    // hidden setEmbeddedTitleBar method.
    public int getVisibleTitleHeight() {
        return 0;
    }

    /**
     * Disables platform notifications of data state and proxy changes.
     */
    public static void disablePlatformNotifications() {
        ContentViewStatics.disablePlatformNotifications();
    }

    /**
     * Enables platform notifications of data state and proxy changes.
     */
    public static void enablePlatformNotifications() {
        ContentViewStatics.enablePlatformNotifications();
    }

    /**
     * Interface to enable the browser to override title bar handling.
     */
    public interface TitleBarDelegate {
        int getTitleHeight();
        void onSetEmbeddedTitleBar(View title);
    }

    /**
     * Returns the height (in pixels) of the embedded title bar (if any). Does not care about
     * scrolling
     */
    protected int getTitleHeight() {
        return 0;
    }
    /**
     * Set all the Javascript Interfaces in jsInterfaces.
     */
    public void setJavascriptInterfaces(Map<String, Object> jsInterfaces) {
    }

    /**
     * Request the scroller to abort any ongoing animation
     */
    public void stopScroll() {
    }

    /**
     * Used to determine if we should monitor the WebCore thread for responsiveness.
     */
    public static void setShouldMonitorWebCoreThread() {
    }

    /**
     * Sets JavaScript engine flags.
     *
     * @param flags JS engine flags in a String
     *
     * This is an implementation detail.
     */
    public void setJsFlags(String jsFlags) {
    }

    /**
     * Start an ActionMode for finding text in this WebView.  Only works if this
     *              WebView is attached to the view system.
     * @param text If non-null, will be the initial text to search for.
     *             Otherwise, the last String searched for in this WebView will
     *             be used to start.
     * @param showIme If true, show the IME, assuming the user will begin typing.
     *             If false and text is non-null, perform a find all.
     * @return boolean True if the find dialog is shown, false otherwise.
     */
    public boolean showFindDialog(String text, boolean showIme) {
        FindActionModeCallback callback = new FindActionModeCallback(getContext());
        if (getParent() == null || startActionMode(callback) == null) {
            // Could not start the action mode, so end Find on page
            return false;
        }
        mFindCallback = callback;
        setFindIsUp(true);
        mFindCallback.setWebView(this);
        if (showIme) {
            mFindCallback.showSoftInput();
        } else if (text != null) {
            mFindCallback.setText(text);
            mFindCallback.findAll();
            return true;
        }
        if (text != null) {
            mFindCallback.setText(text);
            mFindCallback.findAll();
        }
        return true;
    }

    void notifyFindDialogDismissed() {
        mFindCallback = null;
        clearMatches();
        setFindIsUp(false);
    }

    private void setFindIsUp(boolean isUp) {
        mFindIsUp = isUp;
    }

    private MotionEvent createOffsetMotionEvent(MotionEvent src) {
        MotionEvent dst = MotionEvent.obtain(src);
        dst.offsetLocation(mCurrentTouchOffsetX, mCurrentTouchOffsetY);
        return dst;
    }
    /**
     * Get the background color
     */
    public int getPageBackgroundColor() {
        return mAwContents.getContentViewCore().getBackgroundColor();
    }

    /**
     * Inform WebView about the current network type.
     */
    public void setNetworkType(String type, String subtype) {
    }

    /**
     * Saves the view data to the output stream. The output is highly
     * version specific, and may not be able to be loaded by newer versions
     * of WebView.
     * @param stream The {@link OutputStream} to save to
     * @param callback The {@link ValueCallback} to call with the result
     */
    public void saveViewState(OutputStream stream,
            ValueCallback<Boolean> callback) {
    }

    public String getTouchIconUrl() {
        WebBackForwardList backForwardList = copyBackForwardList();
        WebHistoryItem historyItem = backForwardList.getCurrentItem();
        return historyItem != null ? historyItem.getTouchIconUrl() : null;
    }

    /**
     * Select the word at the last click point.
     *
     */
    public boolean selectText() {
        return false;
    }

    /**
     * Select all of the text in this WebView.
     *
     * This is an implementation detail.
     */
    public void selectAll() {
    }

    /**
     * Copy the selection to the clipboard
     *
     * This is an implementation detail.
     */
    public void copySelection() {
    }

    /**
     * Loads the view data from the input stream. See
     * {@link #saveViewState(java.io.OutputStream, ValueCallback)} for more information.
     * @param stream The {@link InputStream} to load from
     */
    public void loadViewState(InputStream stream) {
    }


    /**
     * Dump the dom tree to adb shell if "toFile" is False, otherwise dump it to
     * "/sdcard/domTree.txt"
     *
     * debug only
     */
    public void dumpDomTree(boolean b) {
    }

    /**
     * Dump the render tree to adb shell if "toFile" is False, otherwise dump it
     * to "/sdcard/renderTree.txt"
     *
     * debug only
     */
    public void dumpRenderTree(boolean b) {
    }

    /**
     * Dump the display tree to "/sdcard/displayTree.txt"
     *
     * debug only
     */
    public void dumpDisplayTree() {
    }

    /**
     * Clears the view state set with {@link #loadViewState(InputStream)}.
     * This WebView will then switch to showing the content from webkit
     */
    public void clearViewState() {
    }

    //Android WebView ends


    /**
    * getContentWidth() & getContentHeight() call directly into ContentViewCore as opposed to
    * going through AwContents because the rendering path which these values are derived from
    * within AwContents is not used by SWE, thus are not properly set in AwContents for SWE.
    */
    public int getContentWidth() {
        return (int) Math.ceil(mAwContents.getContentViewCore().getContentWidthCss());
    }

    public int getContentHeight() {
        return (int) Math.ceil(mAwContents.getContentViewCore().getContentHeightCss());
    }

    public void onOffsetsForFullscreenChanged(float topControlsOffsetYPix,
                                              float contentOffsetYPix,
                                              float overdrawBottomHeightPix) {
        if (mCurrentTouchOffsetY == Float.MAX_VALUE) {
            mCurrentTouchOffsetY = -contentOffsetYPix;
            mLastMotionEventUp = false;
        }

        if (topControlsOffsetYPix == 0.0f || contentOffsetYPix == 0.0f) {
            if (mLastMotionEventUp) {
                mCurrentTouchOffsetY = -contentOffsetYPix;
                mLastMotionEventUp = false;
            }

            //Let the engine know about the viewport size change
            //float to int will take the floor value of the offset
            mAwContents.getContentViewCore()
                .setViewportSizeOffset(0, (int)contentOffsetYPix);
        }
    }

    protected AwSettings getAWSettings() {
        return mAwContents.getSettings();
    }

    protected ContentSettings getContentSettings() {
        return mAwContents.getContentSettings();
    }

    // Whether we can move in navigation history by given offset
    private boolean canGoToOffset(int offset) {
        return mAwContents.canGoBackOrForward(offset);
    }

    private void goToOffset(int offset) {
        mAwContents.goBackOrForward(offset);
    }

    private String convertToHitTestExtra(AwContents.HitTestData currData) {
        String extra = "";
        if (currData.hitTestResultType == HitTestResult.IMAGE_TYPE)
            extra = currData.imgSrc;
        else
            extra = currData.hitTestResultExtraData;
        return extra;
    }

    private static String sanitizeUrl(String url) {
        if (url == null) return url;
        if (url.startsWith("www.") || url.indexOf(":") == -1) url = "http://" + url;
        return url;
    }

    private void onProgressChanged(int progress){
        mAwContentsClientProxy.getWebChromeClient().onProgressChanged(this, progress);
    }

    private static class NativeGLDelegate implements AwContents.NativeGLDelegate {
        @Override
        public boolean requestDrawGL(Canvas canvas, boolean waitForCompletion,
                View containerview) {
            return false;
        }

        @Override
        public void detachGLFunctor() {
            // Intentional no-op.
        }

    }

    // Needed by AwContents.InternalAccessDelegate start
    private class InternalAccessAdapter implements AwContents.InternalAccessDelegate {
        @Override
        public boolean drawChild(Canvas canvas, View child, long drawingTime) {
            return WebView.super.drawChild(canvas, child, drawingTime);
        }

        @Override
        public boolean super_onKeyUp(int keyCode, KeyEvent event) {
            return WebView.super.onKeyUp(keyCode, event);
        }

        @Override
        public boolean super_dispatchKeyEventPreIme(KeyEvent event) {
            return WebView.super.dispatchKeyEventPreIme(event);
        }

        @Override
        public boolean super_dispatchKeyEvent(KeyEvent event) {
            return WebView.super.dispatchKeyEvent(event);
        }

        @Override
        public boolean super_onGenericMotionEvent(MotionEvent event) {
            return WebView.super.onGenericMotionEvent(event);
        }

        public void showContextMenu() {
            WebView.super.performLongClick();
        }

        @Override
        public void super_onConfigurationChanged(Configuration newConfig) {
            WebView.super.onConfigurationChanged(newConfig);
        }

        @Override
        public void super_scrollTo(int scrollX, int scrollY) {
            // We're intentionally not calling super.scrollTo here to make testing easier.
            WebView.this.scrollTo(scrollX, scrollY);
        }

        @Override
        public void overScrollBy(int deltaX, int deltaY,
                int scrollX, int scrollY,
                int scrollRangeX, int scrollRangeY,
                int maxOverScrollX, int maxOverScrollY,
                boolean isTouchEvent) {
            // We're intentionally not calling super.scrollTo here to make testing easier.
            WebView.this.overScrollBy(deltaX, deltaY, scrollX, scrollY,
                     scrollRangeX, scrollRangeY, maxOverScrollX, maxOverScrollY, isTouchEvent);
        }

        @Override
        public void onScrollChanged(int l, int t, int oldl, int oldt) {
            WebView.super.onScrollChanged(l, t, oldl, oldt);
        }

        @Override
        public boolean awakenScrollBars() {
            return WebView.super.awakenScrollBars();
        }

        @Override
        public boolean super_awakenScrollBars(int startDelay, boolean invalidate) {
            return WebView.super.awakenScrollBars(startDelay, invalidate);
        }

        public void setMeasuredDimension(int measuredWidth, int measuredHeight) {
            WebView.super.setMeasuredDimension(measuredWidth, measuredHeight);
        }

        @Override
        public int super_getScrollBarStyle() {
            return WebView.super.getScrollBarStyle();
        }

    }

    // SWE-feature-create-window
    public class CreateWindowParams {
        public boolean mUserGesture;
        public boolean mIsGuest;
        public boolean mOpenerSuppressed;
        public String mURL;

        public CreateWindowParams() {
            mUserGesture = false;
            mIsGuest = false;
            mOpenerSuppressed = false;
            mURL = null;
        }
    }

    public CreateWindowParams getCreateWindowParams() {
        AwContents.CreateWindowParams awparams = mAwContents.getCreateWindowParams();
        CreateWindowParams params = new CreateWindowParams();
        params.mUserGesture = awparams.mUserGesture;
        params.mIsGuest = awparams.mIsGuest;
        params.mOpenerSuppressed = awparams.mOpenerSuppressed;
        params.mURL = awparams.mURL;
        return params;
    }
    // SWE-feature-create-window

    // Needed by AwContents.InternalAccessDelegate end


    // Framelayout methods start

    // Added in Jellybean
    @Override
    public void onInitializeAccessibilityEvent(AccessibilityEvent event) {
        super.onInitializeAccessibilityEvent(event);
        mAwContents.onInitializeAccessibilityEvent(event);
    }

    @Override
    public void onInitializeAccessibilityNodeInfo(AccessibilityNodeInfo info) {
        super.onInitializeAccessibilityNodeInfo(info);
        mAwContents.onInitializeAccessibilityNodeInfo(info);
    }

    @Override
    public boolean performAccessibilityAction(int action, Bundle arguments) {
        if (mAwContents.supportsAccessibilityAction(action)) {
            return mAwContents.performAccessibilityAction(action, arguments);
        }
        return super.performAccessibilityAction(action, arguments);
    }

    @Override
    public AccessibilityNodeProvider getAccessibilityNodeProvider() {
        if (mAwContents != null) {
            AccessibilityNodeProvider provider = mAwContents.getAccessibilityNodeProvider();
            if (provider != null)
                return provider;
        }

        return super.getAccessibilityNodeProvider();
    }

    @Override
    public void onDraw(Canvas canvas) {
        if (!AwContents.isUsingSurfaceView()){
            mAwContents.onDraw(canvas);
            super.onDraw(canvas);
        }
    }

    @Override
    public boolean onHoverEvent(MotionEvent event) {
        MotionEvent offset = createOffsetMotionEvent(event);
        boolean consumed = mAwContents.onHoverEvent(offset);
        offset.recycle();
        return consumed;
    }

    @Override
    public boolean onGenericMotionEvent(MotionEvent event) {
        return mAwContents.onGenericMotionEvent(event);
    }

    @Override
    public InputConnection onCreateInputConnection(EditorInfo outAttrs) {
        return mAwContents.onCreateInputConnection(outAttrs);
    }

    public boolean onCheckIsText(){
        return mAwContents.getContentViewCore().onCheckIsTextEditor();
    }

    @Override
    protected void onFocusChanged(boolean gainFocus, int direction, Rect previouslyFocusedRect) {
        super.onFocusChanged(gainFocus, direction, previouslyFocusedRect);
        mAwContents.onFocusChanged(gainFocus, direction, previouslyFocusedRect);
    }

    @Override
    public boolean onKeyUp(int keyCode, KeyEvent event) {
        return mAwContents.onKeyUp(keyCode, event);
    }

    @Override
    public boolean dispatchKeyEventPreIme(KeyEvent event) {
        return mAwContents.getContentViewCore().dispatchKeyEventPreIme(event);
    }

    @Override
    public boolean dispatchKeyEvent(KeyEvent event) {

        //Special checks for menu because of event tracking in portable browser for those keys
        if (isFocused() &&  event.getKeyCode() != event.KEYCODE_BACK
                && event.getKeyCode() != event.KEYCODE_MENU ) {
            return mAwContents.dispatchKeyEvent(event);
        } else {
            return super.dispatchKeyEvent(event);
        }

    }

    @Override
    public boolean onTouchEvent(MotionEvent event) {
        super.onTouchEvent(event);
        if (event.getAction() == MotionEvent.ACTION_UP) {
            mLastMotionEventUp = true;
        } else if (event.getAction() == MotionEvent.ACTION_DOWN) {
            mLastMotionEventUp = false;
        }
        MotionEvent offset = createOffsetMotionEvent(event);
        boolean consumed = mAwContents.onTouchEvent(offset);
        offset.recycle();
        return consumed;
    }

    @Override
    protected void onSizeChanged(int w, int h, int ow, int oh) {
        super.onSizeChanged(w, h, ow, oh);
        mAwContents.onSizeChanged(w, h, ow, oh);
    }

    @Override
    public void onMeasure(int widthMeasureSpec, int heightMeasureSpec) {
        super.onMeasure(widthMeasureSpec, heightMeasureSpec);
        mAwContents.onMeasure(widthMeasureSpec, heightMeasureSpec);
    }

    @Override
    public void scrollBy(int x, int y) {
        if (AwContents.isUsingSurfaceView()) {
            mAwContents.getContentViewCore().scrollBy(x, y);
        } else {
            super.scrollBy(x,y);
        }
    }

    @Override
    protected int computeHorizontalScrollExtent() {
        if (AwContents.isUsingSurfaceView()) {
            return mAwContents.getContentViewCore().computeHorizontalScrollExtent();
        } else {
            return super.computeHorizontalScrollExtent();
        }
    }

    @Override
    public void scrollTo(int x, int y) {
        if (AwContents.isUsingSurfaceView()) {
            mAwContents.getContentViewCore().scrollTo(x, y);
        } else {
            super.scrollTo(x,y);
        }
    }

    @Override
    protected void onConfigurationChanged(Configuration newConfig) {
        super.onConfigurationChanged(newConfig);
        mAwContents.onConfigurationChanged(newConfig);
    }

    @Override
    protected int computeHorizontalScrollOffset() {
        return mAwContents.computeHorizontalScrollOffset();
    }

    @Override
    protected int computeHorizontalScrollRange() {
        return mAwContents.computeHorizontalScrollRange();
    }

    @Override
    protected int computeVerticalScrollExtent() {
        return mAwContents.computeVerticalScrollExtent();
    }

    @Override
    protected int computeVerticalScrollOffset() {
        return mAwContents.computeVerticalScrollOffset();
    }

    @Override
    protected int computeVerticalScrollRange() {
        return mAwContents.computeVerticalScrollRange();
    }

    @Override
    protected void onAttachedToWindow() {
        super.onAttachedToWindow();
        mAwContents.onAttachedToWindow();
    }

    @Override
    protected void onDetachedFromWindow() {
        super.onDetachedFromWindow();
        mAwContents.onDetachedFromWindow();
    }

    @Override
    protected void onVisibilityChanged(View changedView, int visibility) {
        super.onVisibilityChanged(changedView, visibility);
        mAwContents.onVisibilityChanged(changedView, visibility);
        if (AwContents.isUsingSurfaceView()) {
            mAwContents.getContentViewCore().onVisibilityChanged(changedView, visibility);
        }
    }

    @Override
    public void onOverScrolled(int scrollX, int scrollY, boolean clampedX, boolean clampedY) {
        mAwContents.onContainerViewOverScrolled(scrollX, scrollY, clampedX, clampedY);
    }

    @Override
    public void onScrollChanged(int l, int t, int oldl, int oldt) {
        super.onScrollChanged(l, t, oldl, oldt);
        if (mAwContents != null) {
            mAwContents.onContainerViewScrollChanged(l, t, oldl, oldt);
        }
    }

    @Override
    public void computeScroll() {
        mAwContents.computeScroll();
    }

    @Override
    public void onWindowVisibilityChanged(int visibility) {
        super.onWindowVisibilityChanged(visibility);
        mAwContents.onWindowVisibilityChanged(visibility);
    }

    @Override
    public void onWindowFocusChanged(boolean hasWindowFocus) {
        super.onWindowFocusChanged(hasWindowFocus);
        mAwContents.onWindowFocusChanged(hasWindowFocus);
    }

    @Override
    public boolean performLongClick() {
        return false;
    }

    // Framelayout methods end

    public int getViewScrollY() {
        return mAwContents.getContentViewCore().computeVerticalScrollOffset();
    }

    public int getViewScrollX() {
        return mAwContents.getContentViewCore().computeHorizontalScrollOffset();
    }

    protected class WebViewHandler extends Handler {
        // Message IDs
        protected static final int NOTIFY_CREATE_WINDOW = 1000;
        private final WebView mWebView;
        public WebViewHandler(WebView wv){
            mWebView = wv;
        }
        @Override
        public void handleMessage(Message msg) {
            switch (msg.what) {
                case NOTIFY_CREATE_WINDOW:
                    WebView wv = null;
                    WebView.WebViewTransport transport =
                        (WebView.WebViewTransport) msg.obj;
                    wv = transport.getWebView();
                    if (mAwContents != null) {
                        if (wv != null) {
                            mAwContents.supplyContentsForPopup(wv.mAwContents);
                            wv.mRenderTarget.setCurrentContentViewCore(
                                wv.mAwContents.getContentViewCore());
                        } else {
                            mAwContents.supplyContentsForPopup(null);
                        }
                    }
                    break;
            }
        }
    }

    public boolean isReady() {
        ContentViewCore core = getContentViewCore();
        if (core == null) return false;
        return (core.isReady() && mReady);
    }

    public boolean hasCrashed() {
        return mCrashed;
    }

    protected void setHasCrashed(boolean crash) {
        mCrashed = crash;
    }

    // hide set to true hides the controls
    // show set to true shows the controls
    // both set to true makes the controls to have default behavior.
    public void updateTopControls(boolean hide, boolean show, boolean animate) {
        ContentViewCore core = getContentViewCore();
        if (core == null) return;
        core.updateTopControlsState(hide, show, animate);
    }

    public void exitFullscreen() {
        ContentViewCore core = getContentViewCore();
        if (core != null) {
            core.exitFullscreen();
        }
    }
}
