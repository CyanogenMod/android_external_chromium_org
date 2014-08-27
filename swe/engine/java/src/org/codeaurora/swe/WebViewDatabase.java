/*
 *  Copyright (c) 2013, The Linux Foundation. All rights reserved.
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

import android.content.Context;
import org.chromium.android_webview.AwFormDatabase;
import org.chromium.android_webview.HttpAuthDatabase;
import org.chromium.android_webview.AwBrowserContext;

public class WebViewDatabase {

    private static WebViewDatabase sWebViewDatabaseInstance = null;
    private Context mContext = null;

    private WebViewDatabase(Context context) {
        mContext= context;
    }

    public static WebViewDatabase getInstance(Context context) {
        if (sWebViewDatabaseInstance == null) {
            sWebViewDatabaseInstance = new WebViewDatabase(context);
        }
        return sWebViewDatabaseInstance;
    }

    public boolean hasHttpAuthUsernamePassword() {
        return Engine.getAwBrowserContext().getHttpAuthDatabase(mContext).hasHttpAuthUsernamePassword();
    }

    public void clearHttpAuthUsernamePassword() {
        Engine.getAwBrowserContext().getHttpAuthDatabase(mContext).clearHttpAuthUsernamePassword();
    }

    public boolean hasFormData() {
        return AwFormDatabase.hasFormData();
    }

    public void clearFormData() {
        AwFormDatabase.clearFormData();
    }

}
