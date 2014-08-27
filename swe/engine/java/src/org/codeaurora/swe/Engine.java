/*
 *  Copyright (c) 2012-2014 The Linux Foundation. All rights reserved.
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

package org.codeaurora.swe;

import org.chromium.base.PathUtils;
import org.chromium.base.library_loader.LibraryLoader;
import org.chromium.content.browser.BrowserStartupController;
import org.chromium.content.browser.DeviceUtils;
import org.chromium.content.browser.ResourceExtractor;
import org.codeaurora.swe.utils.Logger;
import org.chromium.base.CommandLine;
import org.chromium.android_webview.AwBrowserContext;
import org.chromium.android_webview.AwBrowserProcess;
import org.chromium.android_webview.AwDevToolsServer;
import org.chromium.android_webview.AwResource;
import org.chromium.android_webview.AwGeolocationPermissions;

import android.content.Context;
import android.content.SharedPreferences;

import org.codeaurora.swe.R;
import android.content.res.Resources;


public class Engine {

    private static final String[] MP_MANDATORY_PAKS = {"webviewchromium.pak", "en-US.pak"};
    public static final String COMMAND_LINE_FILE = "/data/local/tmp/swe-command-line";

    private static boolean sInitialized = false;

    public static final String AWC_RENDERING_SWITCH = "enable-awc-engine";
    public static final String SINGLE_PROCESS_SWITCH = "single-process";
    private static int mEngineProcesses;
    private static boolean mAWCRenderingMode;
    private static AwDevToolsServer mDevToolsServer;
    protected static AwBrowserContext sBrowserContext = null;

    /**
     * Engine initialization at the application level. Handles initialization of
     * information that needs to be shared across the main activity and the
     * sandbox services created.
     *
     * @param context
     */
    public static void initialize(Context context) {
        if (sInitialized) {
            return;
        }
        registerResources(context);

        CommandLine.initFromFile(COMMAND_LINE_FILE);
        if (CommandLine.getInstance().hasSwitch(AWC_RENDERING_SWITCH)) {
            Logger.warn("SWE using AWC rendering - Single Process");
            mEngineProcesses = BrowserStartupController.MAX_RENDERERS_SINGLE_PROCESS;
            mAWCRenderingMode = true;
        } else if (CommandLine.getInstance().hasSwitch(SINGLE_PROCESS_SWITCH)) {
            Logger.warn("SWE using SurfaceView - Single Process");
            //SWE-FIXME : Not implemented yet.
            mEngineProcesses = BrowserStartupController.MAX_RENDERERS_SINGLE_PROCESS;
            mAWCRenderingMode = false;
        } else {
            Logger.warn("SWE using SurfaceView - Multi-Process");
            mEngineProcesses = BrowserStartupController.MAX_RENDERERS_LIMIT;
            mAWCRenderingMode = false;
        }

        CommandLine.getInstance().appendSwitch("enable-experimental-form-filling");
        CommandLine.getInstance().appendSwitch("enable-interactive-autocomplete");
        CommandLine.getInstance().appendSwitch("enable-top-controls-position-calculation");
        CommandLine.getInstance().appendSwitchWithValue("top-controls-height", "52");
        CommandLine.getInstance().appendSwitchWithValue("top-controls-show-threshold", "0.5");
        CommandLine.getInstance().appendSwitchWithValue("top-controls-hide-threshold", "0.5");

        ResourceExtractor.setMandatoryPaksToExtract(MP_MANDATORY_PAKS);
        ResourceExtractor.setExtractImplicitLocaleForTesting(false);
        AwBrowserProcess.loadLibrary();
        AwBrowserProcess.start(context, mEngineProcesses);
        //Enable remote debugging by default
        setWebContentsDebuggingEnabled(true);

        SharedPreferences sharedPreferences =
            context.getSharedPreferences("webview", Context.MODE_PRIVATE);
        // Create Browser Context
        sBrowserContext = AwBrowserContext.getInstance(sharedPreferences);
        // initialize Geolocation Permission for Incognito
        sBrowserContext.setGeolocationPermissions((AwGeolocationPermissions)
                    GeolocationPermissions.create(sharedPreferences, true));
        // initialize Geolocation Permission for Normal tab
        sBrowserContext.setIncognitoGeolocationPermissions((AwGeolocationPermissions)
                    GeolocationPermissions.create(sharedPreferences, false));
        // initialize AwEncryptionHelper
        sBrowserContext.createAwEncryptionHelper(context);
        // initialize WebStorage
        WebStorage.getInstance();
        // initialize CookieSyncManager
        CookieSyncManager.createInstance(context);
        // initialize WebViewDatabase
        WebViewDatabase.getInstance(context);
        // initialize CookieManager
        CookieManager.getInstance();
        sInitialized = true;
    }

    protected static void registerResources(Context context) {

        AwResource.setResources(context.getResources());
        AwResource.setErrorPageResources(R.raw.loaderror, R.raw.nodomain);
        AwResource.setDefaultTextEncoding(R.string.default_encoding);
    }

    static void setWebContentsDebuggingEnabled(boolean enable) {
        if (mDevToolsServer == null) {
            if (!enable) return;
            mDevToolsServer = new AwDevToolsServer();
        }
        mDevToolsServer.setRemoteDebuggingEnabled(enable);
    }

    /**
     * Used by classes that want to know whether the engine was initialized
     * (libraries loaded, engine set-up) or not.
     *
     * @return true if the web.* classes are usable.
     */
    public static boolean getIsInitialized() {
        return sInitialized;
    }

    protected static boolean getIsAWCRendering() {
        return mAWCRenderingMode;
    }

    protected static AwBrowserContext getAwBrowserContext() {
        return sBrowserContext;
    }

}
