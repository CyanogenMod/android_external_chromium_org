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

import org.chromium.content.browser.ContentSettings;
import org.chromium.content.browser.ContentViewCore;
import org.chromium.android_webview.AwSettings;

import android.content.Context;
import android.content.pm.PackageManager;
import android.os.Build;
import android.os.Process;

public class WebSettings {

    public enum LayoutAlgorithm {
        NORMAL,
        SINGLE_COLUMN,
        NARROW_COLUMNS,
        TEXT_AUTOSIZING,
    }

    public enum TextSize {
        SMALLEST(50),
        SMALLER(75),
        NORMAL(100),
        LARGER(150),
        LARGEST(200);
        TextSize(int size) {
            value = size;
        }
        int value;
    }

    // SWE: Hard code values to be passed to setInitialPageScale.
    // These values doesn't take screen density into consideration
    public enum ZoomDensity {
        FAR(66),       // 240dpi
        MEDIUM(100),    // 160dpi
        CLOSE(133);      // 120dpi
        ZoomDensity(int size) {
            value = size;
        }
        int value;
    }

    public enum RenderPriority {
        NORMAL,
        HIGH,
        LOW
    }

    public enum PluginState {
        ON,
        ON_DEMAND,
        OFF,
    }

    /**
     * Default cache usage pattern. Use with setCacheMode.
     */
    public static final int LOAD_DEFAULT = -1;

    /**
     * Normal cache usage pattern. Use with setCacheMode.
     */
    public static final int LOAD_NORMAL = 0;

    /**
     * Use cache if content is there, even if expired. If it's not in the cache
     * load from network. Use with setCacheMode.
     */
    public static final int LOAD_CACHE_ELSE_NETWORK = 1;

    /**
     * Don't use the cache, load from network. Use with setCacheMode.
     */
    public static final int LOAD_NO_CACHE = 2;

    /**
     * Don't use the network, load from cache only. Use with setCacheMode.
     */
    public static final int LOAD_CACHE_ONLY = 3;


    private final AwSettings mAwSettings;
    private final ContentSettings mContentSettings;
    private WebView mWebView;

    private long mAppCacheMaxSize = Long.MAX_VALUE;
    private String mDatabasePath = "";
    private boolean mDatabasePathHasBeenSet = false;
    private String mGeolocationDatabasePath = "";
    private LayoutAlgorithm mLayoutAlgorithm = LayoutAlgorithm.NARROW_COLUMNS;
    private ZoomDensity mDefaultZoom = ZoomDensity.MEDIUM;
    private RenderPriority mRenderPriority = RenderPriority.NORMAL;
    private PluginState mPluginState = PluginState.OFF;
    private boolean mSavePassword = true;
    private boolean mLightTouchEnabled = false;
    private boolean mEnableSmoothTransition = false;
    private boolean mPrivateBrowsingEnabled = false;

    public WebSettings(WebView webView) {
        mWebView = webView;
        mAwSettings = webView.getAWSettings();
        mContentSettings = webView.getContentSettings();
    }

    public void setAutoFillProfile(AutoFillProfile autoFillProfile) {
        // Browser uses Webview to have only a single profile.
        if (autoFillProfile != null)
            addAutoFillProfile(autoFillProfile);
        else
            removeAllAutoFillProfiles();
    }

    public String addAutoFillProfile(AutoFillProfile autoFillProfile) {
        String guid = mAwSettings.addorUpdateAutoFillProfile(autoFillProfile.getUniqueId(),
            autoFillProfile.getFullName(), autoFillProfile.getEmailAddress(),
            autoFillProfile.getCompanyName(), autoFillProfile.getAddressLine1(),
            autoFillProfile.getAddressLine2(), autoFillProfile.getCity(),
            autoFillProfile.getState(), autoFillProfile.getZipCode(),
            autoFillProfile.getCountry(), autoFillProfile.getPhoneNumber());
        if (guid != null) {
            autoFillProfile.mUniqueId = guid;
        }
        return guid;
    }

    public void removeAutoFillProfile(AutoFillProfile autoFillProfile) {
        if (autoFillProfile != null) {
           mAwSettings.removeAutoFillProfile(autoFillProfile.getUniqueId());
        }
    }

    public AutoFillProfile getAutoFillProfile(String guid) {
        AutoFillProfile profile = null;
        if (guid != null && !guid.equals("")) {
            profile = mAwSettings.getAutoFillProfile(guid);
        }
        return profile;
    }

    public AutoFillProfile[] getAllAutoFillProfiles() {
        return mAwSettings.getAllAutoFillProfiles();
    }

    public void removeAllAutoFillProfiles() {
        mAwSettings.removeAllAutoFillProfiles();
    }

    public void setAutoFillEnabled(boolean autofillEnabled) {
    }

    public void setAllowFileAccess(boolean allow) {
        mAwSettings.setAllowFileAccess(allow);
    }

    public boolean getAllowFileAccess() {
        return mAwSettings.getAllowFileAccess();
    }

    public void setAllowContentAccess(boolean allow) {
        mAwSettings.setAllowContentAccess(allow);
    }

    public boolean getAllowContentAccess() {
        return mAwSettings.getAllowContentAccess();
    }

    public void setLoadWithOverviewMode(boolean overview) {
        mAwSettings.setLoadWithOverviewMode(overview);
    }

    public boolean getLoadWithOverviewMode() {
        return mAwSettings.getLoadWithOverviewMode();
    }

    public synchronized void setTextZoom(int textZoom) {
        mAwSettings.setTextZoom(textZoom);
    }

    public synchronized int getTextZoom() {
        return mAwSettings.getTextZoom();
    }

    public synchronized void setUseWideViewPort(boolean use) {
        mAwSettings.setUseWideViewPort(use);
    }

    public synchronized boolean getUseWideViewPort() {
        return mAwSettings.getUseWideViewPort();
    }

    public synchronized void setSupportMultipleWindows(boolean support) {
        mAwSettings.setSupportMultipleWindows(support);
    }

    public synchronized boolean supportMultipleWindows() {
        return mAwSettings.supportMultipleWindows();
    }

    public synchronized void setLayoutAlgorithm(LayoutAlgorithm l) {
        mLayoutAlgorithm = l;
        AwSettings.LayoutAlgorithm layout = mAwSettings.getLayoutAlgorithm();

        if (l == LayoutAlgorithm.NORMAL) {
            layout = AwSettings.LayoutAlgorithm.NORMAL;
        } else if (l == LayoutAlgorithm.SINGLE_COLUMN) {
            layout = AwSettings.LayoutAlgorithm.SINGLE_COLUMN;
        } else if (l == LayoutAlgorithm.NARROW_COLUMNS) {
            layout = AwSettings.LayoutAlgorithm.NARROW_COLUMNS;
        } else {
            layout = AwSettings.LayoutAlgorithm.TEXT_AUTOSIZING;
        }

        mAwSettings.setLayoutAlgorithm(layout);
    }

    public synchronized LayoutAlgorithm getLayoutAlgorithm() {
        AwSettings.LayoutAlgorithm awLayout = mAwSettings.getLayoutAlgorithm();
        if (awLayout == AwSettings.LayoutAlgorithm.NORMAL) {
            mLayoutAlgorithm = LayoutAlgorithm.NORMAL;
        } else if (awLayout == AwSettings.LayoutAlgorithm.SINGLE_COLUMN) {
            mLayoutAlgorithm = LayoutAlgorithm.SINGLE_COLUMN;
        } else if (awLayout == AwSettings.LayoutAlgorithm.NARROW_COLUMNS) {
            mLayoutAlgorithm = LayoutAlgorithm.NARROW_COLUMNS;
        } else {
            mLayoutAlgorithm = LayoutAlgorithm.TEXT_AUTOSIZING;
        }

        return mLayoutAlgorithm;
    }

    public synchronized void setStandardFontFamily(String font) {
        mAwSettings.setStandardFontFamily(font);
    }

    public synchronized String getStandardFontFamily() {
        return mAwSettings.getStandardFontFamily();
    }

    public synchronized void setFixedFontFamily(String font) {
        mAwSettings.setFixedFontFamily(font);
    }

    public synchronized String getFixedFontFamily() {
        return mAwSettings.getFixedFontFamily();
    }

    public synchronized void setSansSerifFontFamily(String font) {
        mAwSettings.setSansSerifFontFamily(font);
    }

    public synchronized String getSansSerifFontFamily() {
        return mAwSettings.getSansSerifFontFamily();
    }

    public synchronized void setSerifFontFamily(String font) {
        mAwSettings.setSerifFontFamily(font);
    }

    public synchronized String getSerifFontFamily() {
        return mAwSettings.getSerifFontFamily();
    }

    public synchronized void setCursiveFontFamily(String font) {
        mAwSettings.setCursiveFontFamily(font);
    }

    public synchronized String getCursiveFontFamily() {
        return mAwSettings.getCursiveFontFamily();
    }

    public synchronized void setFantasyFontFamily(String font) {
        mAwSettings.setFantasyFontFamily(font);
    }

    public synchronized String getFantasyFontFamily() {
        return mAwSettings.getFantasyFontFamily();
    }

    public synchronized void setMinimumFontSize(int size) {
        mAwSettings.setMinimumFontSize(size);
    }

    public synchronized int getMinimumFontSize() {
        return mAwSettings.getMinimumFontSize();
    }

    public synchronized void setMinimumLogicalFontSize(int size) {
        mAwSettings.setMinimumLogicalFontSize(size);
    }

    public synchronized int getMinimumLogicalFontSize() {
        return mAwSettings.getMinimumLogicalFontSize();
    }

    public synchronized void setDefaultFontSize(int size) {
        mAwSettings.setDefaultFontSize(size);
    }

    public synchronized int getDefaultFontSize() {
        return mAwSettings.getDefaultFontSize();
    }

    public synchronized void setDefaultFixedFontSize(int size) {
        mAwSettings.setDefaultFixedFontSize(size);
    }

    public synchronized int getDefaultFixedFontSize() {
        return mAwSettings.getDefaultFixedFontSize();
    }

    public synchronized void setLoadsImagesAutomatically(boolean flag) {
        mAwSettings.setLoadsImagesAutomatically(flag);
    }

    public synchronized boolean getLoadsImagesAutomatically() {
        return mAwSettings.getLoadsImagesAutomatically();
    }

    public synchronized void setBlockNetworkImage(boolean flag) {
        mAwSettings.setImagesEnabled(!flag);
    }

    public synchronized boolean getBlockNetworkImage() {
        return !mAwSettings.getImagesEnabled();
    }

    public synchronized void setBlockNetworkLoads(boolean flag) {
        mAwSettings.setBlockNetworkLoads(flag);
    }

    public synchronized boolean getBlockNetworkLoads() {
        return mAwSettings.getBlockNetworkLoads();
    }

    public synchronized void setJavaScriptEnabled(boolean flag) {
        mAwSettings.setJavaScriptEnabled(flag);
    }

    public synchronized void setDisableNoScriptTag(boolean flag) {
        mAwSettings.setDisableNoScriptTag(flag);
    }

    public void setAllowUniversalAccessFromFileURLs(boolean flag) {
        mAwSettings.setAllowUniversalAccessFromFileURLs(flag);
    }

    public void setAllowFileAccessFromFileURLs(boolean flag){
        mAwSettings.setAllowFileAccessFromFileURLs(flag);
    }

    public synchronized PluginState getPluginState() {
        android.webkit.WebSettings.PluginState awPluginState = mAwSettings.getPluginState();

        if (awPluginState == android.webkit.WebSettings.PluginState.ON) {
           mPluginState = PluginState.ON;
        } else if (awPluginState == android.webkit.WebSettings.PluginState.ON_DEMAND) {
          mPluginState = PluginState.ON_DEMAND;
        } else {
            mPluginState = PluginState.ON_DEMAND;
        }

        return mPluginState;
    }

    public synchronized void setPluginState(PluginState state) {
        mPluginState = state;
        android.webkit.WebSettings.PluginState awPluginState = mAwSettings.getPluginState();

        if (mPluginState == PluginState.ON) {
            awPluginState = android.webkit.WebSettings.PluginState.ON;
        } else if (mPluginState == PluginState.ON_DEMAND) {
            awPluginState = android.webkit.WebSettings.PluginState.ON_DEMAND;
        } else {
            awPluginState = android.webkit.WebSettings.PluginState.OFF;
        }

        mAwSettings.setPluginState(awPluginState);
    }

    public synchronized void setAppCacheEnabled(boolean flag) {
        mAwSettings.setAppCacheEnabled(flag);
    }

    public synchronized void setAppCachePath(String appCachePath) {
        mAwSettings.setAppCachePath(appCachePath);
    }

    public synchronized void setDatabaseEnabled(boolean flag) {
        mAwSettings.setDatabaseEnabled(flag);
    }

    public synchronized void setDomStorageEnabled(boolean flag) {
        mAwSettings.setDomStorageEnabled(flag);
    }

    public synchronized boolean getDomStorageEnabled() {
        return mAwSettings.getDomStorageEnabled();
    }

    public synchronized boolean getDatabaseEnabled() {
        return mAwSettings.getDatabaseEnabled();
    }

    public synchronized void setGeolocationEnabled(boolean flag) {
        mAwSettings.setGeolocationEnabled(flag);
    }

    public synchronized void setDoNotTrack(boolean flag) {
        mAwSettings.setDoNotTrack(flag);
    }

    public synchronized boolean getJavaScriptEnabled() {
        return mAwSettings.getJavaScriptEnabled();
    }

    public boolean getAllowUniversalAccessFromFileURLs() {
        return mAwSettings.getAllowUniversalAccessFromFileURLs();
    }

    public boolean getAllowFileAccessFromFileURLs() {
        return mAwSettings.getAllowFileAccessFromFileURLs();
    }

    public synchronized void setJavaScriptCanOpenWindowsAutomatically(boolean flag) {
        mAwSettings.setJavaScriptCanOpenWindowsAutomatically(flag);
    }

    public synchronized boolean getJavaScriptCanOpenWindowsAutomatically() {
        return mAwSettings.getJavaScriptCanOpenWindowsAutomatically();
    }

    public synchronized void setDefaultTextEncodingName(String encoding) {
        mAwSettings.setDefaultTextEncodingName(encoding);
    }

    public synchronized String getDefaultTextEncodingName() {
        return mAwSettings.getDefaultTextEncodingName();
    }

    public synchronized void setUserAgentString(String ua) {
        mAwSettings.setUserAgentString(ua);
    }

    public synchronized String getUserAgentString() {
        return mAwSettings.getUserAgentString();
    }

    public void setNeedInitialFocus(boolean flag) {
        mAwSettings.setShouldFocusFirstNode(flag);
    }

    public void setCacheMode(int mode) {
        mAwSettings.setCacheMode(mode);
    }

    public int getCacheMode() {
        return mAwSettings.getCacheMode();
    }

    public static String getDefaultUserAgent(Context context) {
        return AwSettings.getDefaultUserAgent();
    }

    public boolean getMediaPlaybackRequiresUserGesture() {
        return mAwSettings.getMediaPlaybackRequiresUserGesture();
    }

    public void setMediaPlaybackRequiresUserGesture(boolean require) {
        mAwSettings.setMediaPlaybackRequiresUserGesture(require);
    }

    public void setSupportZoom(boolean support) {
        mAwSettings.setSupportZoom(support);
    }

    public boolean supportZoom() {
        return mAwSettings.supportZoom();
    }

    public void setBuiltInZoomControls(boolean enabled) {
        mAwSettings.setBuiltInZoomControls(enabled);
    }

    public boolean getBuiltInZoomControls() {
        return mAwSettings.getBuiltInZoomControls();
    }

    public void setDisplayZoomControls(boolean enabled) {
        mAwSettings.setDisplayZoomControls(enabled);
    }

    public boolean getDisplayZoomControls() {
        return mAwSettings.getDisplayZoomControls();
    }

    public void setSaveFormData(boolean save) {
        mAwSettings.setSaveFormData(save);
    }

    public boolean getSaveFormData() {
        return mAwSettings.getSaveFormData();
    }

    public void setSavePassword(boolean save) {
        mAwSettings.setSavePassword(save);
    }

    public boolean getSavePassword() {
        return mSavePassword;
    }

    public void clearPasswords() {
        mAwSettings.clearPasswords();
    }

    /* SWE: The following APIs have no Chromium specific implementation,
     * i.e. - they do not have a direct corresponding Chromium functionality.
     * The API is to check if audio and video mimetype based urls which do not
     * have "Content-Dispostion: attachment" in response header
     * should be allowed to download or not
     * If yes user is asked for a prompt and if not not they are played
     * inline
     */
    public void setAllowMediaDownloads(boolean allow) {
        mAwSettings.setAllowMediaDownloads(allow);
    }

    /* SWE: The following APIs have no Chromium specific implementation,
     * i.e. - they do not have a direct corresponding Chromium functionality.
     * We are just storing the value locally and not propagating it anywhere.
     */
    public synchronized void setRenderPriority(RenderPriority priority) {
        if (mRenderPriority != priority) {
            mRenderPriority = priority;
        }
    }

    //Chromium supports web platform storage quotas differently from webkit
    public synchronized void setAppCacheMaxSize(long appCacheMaxSize) {
        if (appCacheMaxSize != mAppCacheMaxSize) {
            mAppCacheMaxSize = appCacheMaxSize;
        }
    }

    //Chromium stores db inside user profiles, theres no way to change it
    public synchronized void setDatabasePath(String databasePath) {
        if (databasePath != null && !mDatabasePathHasBeenSet) {
            mDatabasePath = databasePath;
            mDatabasePathHasBeenSet = true;
        }
    }

    public synchronized String getDatabasePath() {
        return mDatabasePath;
    }

    //Chromium stores geo db inside user profiles, theres no way to change it
    public synchronized void setGeolocationDatabasePath(String databasePath) {
        if (databasePath != null && !databasePath.equals(mGeolocationDatabasePath)) {
            mGeolocationDatabasePath = databasePath;
        }
    }

    public void setDefaultZoom(ZoomDensity zoom) {
        if (mDefaultZoom != zoom) {
            mDefaultZoom = zoom;
            //SetInitial scale doesn't take the screen density into account.
            //Multiply ZoomDensity value with DIPScale to get appropriate value.
            mAwSettings.setInitialPageScale((float) (zoom.value * mAwSettings.getDIPScale()));
        }
    }

    public ZoomDensity getDefaultZoom() {
        return mDefaultZoom;
    }

    //API was experimental, it has no functionality
    public void setLightTouchEnabled(boolean enabled) {
        mLightTouchEnabled = enabled;
    }

    //API was experimental, it has no functionality
    public boolean getLightTouchEnabled() {
        return mLightTouchEnabled;
    }


    /* SWE: The following are deprecated APIS */

     /**
     * Sets whether the WebView will enable smooth transition while panning or
     * zooming or while the window hosting the WebView does not have focus. If
     * it is true, WebView will choose a solution to maximize the performance.
     *
     * @deprecated - This method was deprecated in API level 17 & is now obsolete.
     * It will become a no-op in future.
     */
    @Deprecated
    public void setEnableSmoothTransition(boolean enable) {
        mEnableSmoothTransition = enable;
    }

    /**
     * Gets whether the WebView enables smooth transition while panning or zooming.
     *
     * @deprecated - This method was deprecated in API level 17 & is now obsolete.
     * It will become a no-op in future.
     */
    @Deprecated
    public boolean enableSmoothTransition() {
        return mEnableSmoothTransition;
    }

    /**
     * Return true if plugins are enabled.
     * @deprecated - This method was deprecated in API level 8.
     * This method has been replaced by getPluginState()
     */
    @Deprecated
    public boolean getPluginsEnabled() {
        return mAwSettings.getPluginsEnabled();
    }

    /**
     * Tell the WebView to enable plugins.
     * @deprecated - This method was deprecated in API level 8. This method has
     * been deprecated in favor of setPluginState(WebSettings.PluginState)
     */
    @Deprecated
    public void setPluginsEnabled(boolean flag) {
        mAwSettings.setPluginsEnabled(flag);
    }

    /**
     * Returns the directory that contains the plugin libraries.
     * @deprecated - This method was deprecated in API level 9. It is no longer
     * used. Plugins are loaded from their own APK via system's package manager.
     */
    @Deprecated
    public String getPluginsPath() {
        return "";
    }

    /**
     * Set a custom path to plugins used by the WebView.
     * @deprecated - This method was deprecated in API level 9. It is no longer
     * used. Plugins are loaded from their own APK via system's package manager.
     */
    @Deprecated
    public void setPluginsPath(String pluginPath) {
    }

    /**
     * Set the text size of the page.
     * @deprecated - This method was deprecated in API level 14.
     * Use setTextZoom(int) instead.
     */
    @Deprecated
    public void setTextSize(TextSize t) {
         setTextZoom(t.value);
    }

    /**
     * Get the text size of the page.
     * @deprecated - This method was deprecated in API level 14.
     * Use getTextZoom() instead.
     */
    @Deprecated
    public TextSize getTextSize() {
        TextSize setSize = null;
        int smallestDifference = Integer.MAX_VALUE;
        for (TextSize textSize : TextSize.values()) {
            int difference = Math.abs(getTextZoom() - textSize.value);
            if (difference == 0) {
                return textSize;
            }
            if (difference < smallestDifference) {
                smallestDifference = difference;
                setSize = textSize;
            }
        }
        return setSize != null ? setSize : TextSize.NORMAL;
    }

    // WebSettingsClassic api's

    public boolean isPrivateBrowsingEnabled() {
        return mPrivateBrowsingEnabled;
    }

    public synchronized void setPrivateBrowsingEnabled(boolean flag) {
        if (mPrivateBrowsingEnabled != flag) {
            mPrivateBrowsingEnabled = flag;
        }
    }

    // Set the property
    public void setProperty(String gfxinvertedscreen, String string) {
    }

    /**
     * Sets whether viewport metatag can disable zooming.
     * @param flag Whether or not to forceably enable user scalable.
     */
    public void setForceUserScalable(boolean forceEnableUserScalable) {
        mAwSettings.setForceUserScalable(forceEnableUserScalable);
    }

    public boolean getForceUserScalable() {
        return mAwSettings.getForceUserScalable();
    }

    /**
     * Tell the WebView to show the visual indicator
     * @param flag True if the WebView should show the visual indicator
     */
    public void setShowVisualIndicator(boolean enableVisualIndicator) {
    }

    /**
     * Set the double-tap zoom of the page in percent. Default is 100.
     * @param doubleTapZoom A percent value for increasing or decreasing the double-tap zoom.
     */
    public void setDoubleTapZoom(int doubleTapZoom) {
    }

    public void setHTTPRequestHeaders(String headers) {
        mAwSettings.setHTTPRequestHeaders(headers);
    }

    /**
     * Enables/disables HTML5 link "prefetch" parameter.
     */
    public void setLinkPrefetchEnabled(boolean mLinkPrefetchAllowed) {
    }

    /**
     * Set the number of pages cached by the WebKit for the history navigation.
     * @param size A non-negative integer between 0 (no cache) and 20 (max).
     */
    public void setPageCacheCapacity(int pageCacheCapacity) {
    }

    /**
     * Tell the WebView to use Skia's hardware accelerated rendering path
     * @param flag True if the WebView should use Skia's hw-accel path
     */
    public void setHardwareAccelSkiaEnabled(boolean skiaHardwareAccelerated) {
    }


    public void setNavDump(boolean enablenavdump) {
    }

    /**
     * Tell the WebView to enable WebWorkers API.
     * @param flag True if the WebView should enable WebWorkers.
     * Note that this flag only affects V8. JSC does not have
     * an equivalent setting.
     */
    public void setWorkersEnabled(boolean workersenabled) {
    }

    //New APIs
    public void setFullscreenSupported(boolean enable) {
        mAwSettings.setFullscreenSupported(enable);
    }

}
