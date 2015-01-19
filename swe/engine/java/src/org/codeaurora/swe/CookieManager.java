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

import android.net.ParseException;
import android.util.Log;

import org.chromium.base.JNINamespace;
import org.chromium.base.ThreadUtils;

import java.util.concurrent.Callable;

import org.chromium.android_webview.AwCookieManager;

public final class CookieManager {

    private AwCookieManager mAwCookieManager;
    private static CookieManager sCookieManager;

    public CookieManager() {
        if (mAwCookieManager == null) {
            mAwCookieManager = new AwCookieManager();
        }
        sCookieManager = this;
    }

    public synchronized boolean acceptCookie() {
        return mAwCookieManager.acceptCookie();
    }

    public boolean allowFileSchemeCookies() {
        return mAwCookieManager.allowFileSchemeCookies();
    }

    public String getCookie(final String url) {
        return mAwCookieManager.getCookie(url);
    }

    public String getCookie(final String url, final boolean privateBrowsing) {
        //SWE-FIXME
        //return privateBrowsing ? mAwCookieManager.getIncognitoCookie(url) :
        //    mAwCookieManager.getCookie(url);
        return mAwCookieManager.getCookie(url);
    }

    public static CookieManager getInstance() {
        if (sCookieManager == null) {
            sCookieManager = new CookieManager();
        }

        return sCookieManager;
    }

    public boolean hasCookies() {
        return mAwCookieManager.hasCookies();
    }

    public boolean hasCookies(boolean privateBrowsing) {
        //SWE-FIXME
        //return privateBrowsing ? mAwCookieManager.hasIncognitoCookies() :
          //  mAwCookieManager.hasCookies();
        return mAwCookieManager.hasCookies();
    }

    public void removeAllCookie() {
        mAwCookieManager.removeAllCookie();
    }

    public void removeExpiredCookie() {
        mAwCookieManager.removeExpiredCookie();
    }

    public void removeSessionCookie() {
        mAwCookieManager.removeSessionCookie();
    }

    public void setAcceptCookie(boolean accept) {
       mAwCookieManager.setAcceptCookie(accept);
    }

    public void setAcceptFileSchemeCookies(boolean accept) {
        mAwCookieManager.setAcceptFileSchemeCookies(accept);
    }

    public void setCookie(final String url, final String value) {
        mAwCookieManager.setCookie(url, value);
    }

    public void flushCookieStore() {
        mAwCookieManager.flushCookieStore();
    }
}
