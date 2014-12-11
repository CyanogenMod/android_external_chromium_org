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

import org.chromium.android_webview.AwSettings;
import org.chromium.base.CommandLine;
import org.chromium.base.PathUtils;
import org.chromium.base.library_loader.LibraryLoader;
import org.chromium.base.library_loader.ProcessInitException;

import org.chromium.content.browser.BrowserStartupController;
import org.chromium.content.browser.ChildProcessLauncher;
import org.chromium.content.browser.DeviceUtils;
import org.chromium.content.browser.ResourceExtractor;
import org.chromium.content.browser.TracingControllerAndroid;

import org.chromium.android_webview.AwBrowserContext;
import org.chromium.android_webview.AwBrowserProcess;
import org.chromium.android_webview.AwContents;
import org.chromium.android_webview.AwDevToolsServer;
import org.chromium.android_webview.AwResource;

import org.codeaurora.swe.R;
import org.codeaurora.swe.GeolocationPermissions;
import org.codeaurora.swe.utils.Logger;

import android.content.Context;
import android.content.res.Resources;
import android.content.SharedPreferences;
import android.util.Log;

import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileInputStream;
import java.io.InputStream;

public final class Engine {

    private static final String[] MP_MANDATORY_PAKS = {
        "webviewchromium.pak", "icudtl.dat",
        "am.pak", "bn.pak", "da.pak", "en-GB.pak", "es.pak", "fil.pak", "gu.pak",
        "hr.pak", "it.pak", "ko.pak", "ml.pak", "nb.pak", "pt-BR.pak", "ru.pak",
        "sr.pak", "ta.pak", "tr.pak", "ar.pak", "ca.pak", "de.pak", "en-US.pak",
        "et.pak", "fi.pak", "he.pak", "hu.pak", "ja.pak", "lt.pak", "mr.pak",
        "nl.pak", "pt-PT.pak", "sk.pak", "sv.pak", "te.pak", "uk.pak", "zh-CN.pak",
        "bg.pak", "cs.pak", "el.pak", "es-419.pak", "fa.pak", "fr.pak", "hi.pak",
        "id.pak", "kn.pak", "lv.pak", "ms.pak", "pl.pak", "ro.pak",  "sl.pak",
        "sw.pak", "th.pak", "vi.pak", "zh-TW.pak"
    };

    public static final String COMMAND_LINE_FILE = "/data/local/tmp/swe-command-line";
    public static final String AWC_RENDERING_SWITCH = "enable-awc-engine";
    public static final String SINGLE_PROCESS_SWITCH = "single-process";

    private static boolean sInitialized = false;
    private static boolean sCommandLineInitialized = false;
    private static TracingControllerAndroid sTracingController = null;

    private static boolean mIsSingleProcess;
    private static boolean mAWCRenderingMode;
    private static AwDevToolsServer mDevToolsServer;
    protected static AwBrowserContext sBrowserContext = null;
    private static StartupCallback mStartupCallback;
    private static Context mContext;
    private static boolean mIsAsync = false;

    public static void initializeCommandLine(Context context) {

        if (sCommandLineInitialized) {
            return;
        }
        sCommandLineInitialized = true;

        mContext = context;
        registerResources(context);

        Resources resources = mContext.getResources();

        InputStream sweCmdLineStream = null;
        int sweCmdId =  resources.getIdentifier("swe_command_line", "raw", mContext.getPackageName());
        if (sweCmdId != 0)
            sweCmdLineStream = resources.openRawResource(sweCmdId);

        InputStream  usrCmdLineStream = null;
        File file = new File(COMMAND_LINE_FILE);
        if(file.exists()){
            try {
                usrCmdLineStream = new FileInputStream(COMMAND_LINE_FILE);
            } catch (FileNotFoundException e) {
            }
        }

        // Set the browser options here
        CommandLineManager.init(sweCmdLineStream, usrCmdLineStream);

        if (CommandLine.getInstance().hasSwitch(AWC_RENDERING_SWITCH)) {
            Logger.warn("SWE using AWC rendering - Single Process");
            mIsSingleProcess = true;
            mAWCRenderingMode = true;
        } else if (CommandLine.getInstance().hasSwitch(SINGLE_PROCESS_SWITCH)) {
            Logger.warn("SWE using SurfaceView - Single Process");
            //SWE-FIXME : Not implemented yet.
            mIsSingleProcess = true;
            mAWCRenderingMode = false;
        } else {
            Logger.warn("SWE using SurfaceView - Multi-Process");
            mIsSingleProcess = false;
            mAWCRenderingMode = false;
        }
    }

    //This should be called each time Application.onCreate is called.
    public static void initializeApplicationParameters() {
        ResourceExtractor.setMandatoryPaksToExtract(MP_MANDATORY_PAKS);
        PathUtils.setPrivateDataDirectorySuffix("swe_webview");
    }

    public static void startExtractingResources(Context context) {
        ResourceExtractor resourceExtractor = ResourceExtractor.get(context);
        resourceExtractor.startExtractingResources();
    }

    private static TracingControllerAndroid getTracingController(Context context) {
        if (sTracingController == null) {
            sTracingController = new TracingControllerAndroid(context);
        }
        return sTracingController;
    }

    public static void pauseTracing(Context context) {
        getTracingController(context).unregisterReceiver(context);
    }

    public static void resumeTracing(Context context) {
        getTracingController(context).registerReceiver(context);
    }

    public static void loadNativeLibraries(Context context) throws ProcessInitException {
        LibraryLoader.ensureInitialized(context, true);
    }

    public static void warmUpChildProcess(Context context) {
        ChildProcessLauncher.warmUp(context);
    }

    public static void initialize(Context context) {
        mIsAsync = false;
        if (sInitialized) {
            return;
        }
        initializeCommandLine(context);
        registerResources(context);
        try {
            BrowserStartupController.get(mContext).startBrowserProcessesSync(false);
        } catch (ProcessInitException e) {
            Log.e("TAG", "SWE WebView Initialization failed", e);
            System.exit(-1);
        }
        finishInitialization(false);
    }

    /**
     * Engine initialization at the application level. Handles initialization of
     * information that needs to be shared across the main activity and the
     * sandbox services created.
     *
     * @param context
     * @param callback
     */
    public static void initialize(Context context,
                                  final StartupCallback callback) {
        initializeApplicationParameters();
        if (sInitialized) {
            return;
        }
        mIsAsync = true;
        initializeCommandLine(context);

        mStartupCallback = callback;

        if(!mAWCRenderingMode && !mIsSingleProcess) {
            try{
                BrowserStartupController.get(context).startBrowserProcessesAsync(
                    new BrowserStartupController.StartupCallback(){
                        @Override
                        public void onSuccess(boolean alreadyStarted) {
                            finishInitialization(alreadyStarted);
                        }
                        @Override
                        public void onFailure() {
                            initializationFailed();
                        }
                });
             } catch (Exception e) {
                 Log.e("TAG", "SWE WebView Initialization failed", e);
             }
        } else if(mAWCRenderingMode) { //SWE using AWC rendering - Single Process
            //SWE-FIXME
        } else { //SWE using SurfaceView - Single Process
            //Not implemented
        }
    }



    private static void finishInitialization(boolean alreadyStarted) {
        SharedPreferences sharedPreferences =
            mContext.getSharedPreferences("webview", Context.MODE_PRIVATE);
        // Create Browser Context
        sBrowserContext = AwBrowserContext.getInstance(sharedPreferences);
// SWE-feature-username-password
        // initialize AwEncryptionHelper
        sBrowserContext.createAwEncryptionHelper(mContext);
// SWE-feature-username-password

        // initialize WebStorage
        WebStorage.getInstance();
        // initialize CookieSyncManager
        CookieSyncManager.createInstance(mContext);
        // initialize WebViewDatabase
        WebViewDatabase.getInstance(mContext);
        // initialize CookieManager
        CookieManager.getInstance();
        // initialize GeolocationPermissions
        GeolocationPermissions.getInstance(sharedPreferences);

        // Needed for onReceiveIcon API of WebChromeClient
        // to start receiving notifications from Awlayer
        AwContents.setShouldDownloadFavicons();

        //SWE using SurfaceView - Multi-Process

        sInitialized = true;
        if(mIsAsync)
            mStartupCallback.onSuccess(alreadyStarted);
    }

    private static void initializationFailed() {
        if(mIsAsync)
            mStartupCallback.onFailure();
    }

    protected static void registerResources(Context context) {

        AwResource.setResources(context.getResources());
        AwResource.setErrorPageResources(R.raw.loaderror, R.raw.nodomain);
        AwResource.setDefaultTextEncoding(R.string.default_encoding);
    }

    public static void setWebContentsDebuggingEnabled(boolean enable) {
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

    public static String getDefaultUserAgent() {
        return AwSettings.getDefaultUserAgent();
    }

    /**
     * This provides the interface to the callbacks for successful or failed startup
     */
    public interface StartupCallback {
        void onSuccess(boolean alreadyStarted);
        void onFailure();
    }

}
