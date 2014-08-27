/*
 *  Copyright (c) 2012, The Linux Foundation. All rights reserved.
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

package org.codeaurora.swe.utils;

import android.content.Context;
import android.util.Log;
import android.widget.Toast;

/**
 * Helper for logging from the SWE Java code. Easy to turn it off globally,
 * integrates a single tag name for all the project.
 */

public class Logger {
    private static final String LOGTAG = "SWE_UI";
    private static final CharSequence NAMESPACE = "org.codeaurora.swe.browser.";
    private static final boolean ON = true;
    private static final boolean WTF_TO_ERROR = true;

    public static void notImplemented() {
        if (ON)
            apierror("notImplemented: " + new Throwable().getStackTrace()[1].toString().replace(NAMESPACE, ""));
    }

    public static void notImplemented(String message) {
        if (ON)
            apierror("notImplemented: " + message + ": " + new Throwable().getStackTrace()[1].toString().replace(NAMESPACE, ""));
    }

    public static void info(String msg) {
        if (ON)
            Log.i(LOGTAG, msg);
    }

    public static void debug(String msg) {
        if (ON)
            Log.d(LOGTAG, msg);
    }

    public static void warn(String msg) {
        if (ON)
            Log.w(LOGTAG, msg);
    }

    public static void apierror(String msg) {
        if (ON)
            Log.e(LOGTAG, msg);
    }

    public static void apiAssert(boolean trueCondition) {
        if (ON) {
            if (!trueCondition)
                apierror("ASSERTION at: " + new Throwable().getStackTrace()[1].toString().replace(NAMESPACE, ""));
        }
    }

    public static void error(String msg) {
        if (ON)
            Log.e(LOGTAG, msg);
    }

    public static void wtf(String msg) {
        if (ON) {
            if (WTF_TO_ERROR)
                Log.e(LOGTAG, msg);
            else
                Log.wtf(LOGTAG, msg);
        }
    }

    public static void dumpTrace(Exception e) {
        if (ON)
            e.printStackTrace();
    }

    public static void userMessagePassive(String string, Context context) {
        Toast.makeText(context, string, Toast.LENGTH_SHORT).show();
    }

    public static void developerMessagePassive(String string, Context context) {
        if (ON)
            Toast.makeText(context, string, Toast.LENGTH_SHORT).show();
    }

}
