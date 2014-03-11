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
import android.os.Handler;
import android.os.Looper;
import android.os.Message;
import android.os.Process;

import org.codeaurora.swe.utils.Logger;

public final class CookieSyncManager implements Runnable {

    // static variable to be given when requests for an instance of
    // CookieSyncManager
    private static CookieSyncManager sCookieSyncManager;
    // save the cookies now(100 ms)
    private static int COOKIE_SYNC_NOW = 100;
    // save the cookies later(5 min)
    private static int COOKIE_SYNC_LATER = 5 * 60 * 100;
    // message identifier for the handler tp handle message
    private static int COOKIE_SYNC_MSG = 1000;
    private Thread mBackgroundThread;
    private int startStopRefCount = 0;


    protected Handler mHandler;
    protected WebViewDatabase mWebViewDatabase;
    protected static final String LOGTAG = "websync";

    private class CookieSyncHandler extends Handler {

        @Override
        public void handleMessage(Message msg) {

            //check if message is SYNC
            if (msg.what == COOKIE_SYNC_MSG) {

                syncFromRamToFlash();

                // keep syncing after every 5 minutes until stopSync is invoked
                startDelayedSyncing();
            }
        }
    }


    public CookieSyncManager() {
        // When the request fo instance creation occcurs we need to create a new background thread
        mBackgroundThread = new Thread(this);
        mBackgroundThread.start();

        sCookieSyncManager = this;
    }

    public synchronized static CookieSyncManager createInstance(Context context) {

        // assure that context was created
        if (context == null) {
            Logger.error("Cannot create an Instance of CookieSyncManager without context");
            return null;
        }

        if (sCookieSyncManager == null) {
            sCookieSyncManager = new CookieSyncManager();
        }

        return sCookieSyncManager;
    }

    public synchronized static CookieSyncManager getInstance() {
        if (sCookieSyncManager == null) {
            Logger.error("Request for createInstance was not recieved");
            return null;
        }

        return sCookieSyncManager;
    }


    public void run() {

        // Creating Looper before initializing CookieSynchandler
        Looper.prepare();
        /*
         * Create the handler on the background thread so it is bound to the
         * background thread's message queue.
         */
        mHandler = new CookieSyncHandler();

        // assuring that priority of thread is set as  Background Thread
        Process.setThreadPriority(Process.THREAD_PRIORITY_BACKGROUND);

        startDelayedSyncing();
    }

    public void resetSync() {
        handleSyncDelay(COOKIE_SYNC_LATER);
    }

    public void  startSync() {

        // check before incrementing the ref count
        if ( mHandler == null) {
            return;
        }

        if (++startStopRefCount == 1) {
            startDelayedSyncing();
        }
    }

    public void stopSync() {
        // check before incrementing the ref count
        if ( mHandler == null) {
            return;
        }

        if (--startStopRefCount == 0) {
            mHandler.removeMessages(COOKIE_SYNC_MSG);
        }
    }

    public void sync() {
        handleSyncDelay(COOKIE_SYNC_NOW);
    }

    protected void onSyncInit() {

    }

    protected void syncFromRamToFlash() {
        final CookieManager cookieMgr = CookieManager.getInstance();

        if(cookieMgr.acceptCookie() == false) {
            Logger.error("Accept Cookie has been set to false");
            return;
        }

        cookieMgr.flushCookieStore();
    }

    protected Object clone() throws CloneNotSupportedException {
        return null;
    }

    private void handleSyncDelay(int messageDelay) {

        // assure that handler exists
        if (mHandler == null) {
            return;
        }

        // remove all the existing "COOKIE_SYNC" messages that are present in the message queue
        mHandler.removeMessages(COOKIE_SYNC_MSG);

        // start syncing Cookies from RAM to flash by sending sending SYNC messages
        Message message = mHandler.obtainMessage(COOKIE_SYNC_MSG);
        mHandler.sendMessageDelayed(message, messageDelay);
    }

    private void startDelayedSyncing() {

        if (mHandler == null) {
            Logger.error("Handler has not been set");
            return;
        }

        // start syncing Cookies from RAM to flash by sending sending SYNC messages
        Message message = mHandler.obtainMessage(COOKIE_SYNC_MSG);
        mHandler.sendMessageDelayed(message, COOKIE_SYNC_LATER);
    }

}
