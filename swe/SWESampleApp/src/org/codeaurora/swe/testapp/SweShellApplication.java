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

package org.codeaurora.swe.testapp;


import org.chromium.content.browser.ResourceExtractor;
import org.chromium.base.PathUtils;
import android.app.Application;

/**
 * Entry point for the SWE Test application.  Handles initialization of information that needs
 * to be shared across the main activity and the child services created.
 */
public class SweShellApplication extends Application {
    /**
     * icudtl.dat provides ICU (i18n library) with all the data for its
     * operation. We use to link it statically to our binary, but not any more
     * so that we have to install it along with other mandatory pak files.
     * See src/third_party/icu/README.chromium.
     */

    private static final String[] MP_MANDATORY_PAKS = new String[] {
        "webviewchromium.pak",
        "icudtl.dat"
    };

    @Override
    public void onCreate() {
        super.onCreate();
        initializeApplicationParameters();
    }

    public static void initializeApplicationParameters() {
        ResourceExtractor.setMandatoryPaksToExtract(MP_MANDATORY_PAKS);
        PathUtils.setPrivateDataDirectorySuffix("swe_webview");

    }

}
