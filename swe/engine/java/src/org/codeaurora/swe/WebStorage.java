/*
 * Copyright (c) 2013 The Linux Foundation. All rights reserved.
 * Not a contribution.
 *
 * Copyright (C) 2009 The Android Open Source Project
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

import android.webkit.ValueCallback;

import java.util.HashMap;
import java.util.Map;

import org.chromium.android_webview.AwQuotaManagerBridge;

/**
 * This class is used to manage the JavaScript storage APIs provided by the
 * WebView. It manages the Application Cache API, the Web SQL Database
 * API and the HTML5 Web Storage API.
 *
 * The Application Cache API provides a mechanism to create and maintain an
 * application cache to power offline Web applications. Use of the Application
 * Cache API can be attributed to an origin, however
 * it is not possible to set per-origin quotas. Note that there can be only
 * one application cache per application.
 *
 * The Web SQL Database API provides storage which is private to a given origin.
 * Similar to the Application Cache, use of the Web SQL Database can be attributed
 * to an origin. It is also possible to set per-origin quotas.
 */

public final class WebStorage {

    /**
     * Origin class contains info regarding the amount of storage used by an origin
     */
    public static class Origin {
        private String mOrigin = null;
        private long mQuota = 0;
        private long mUsage = 0;

        protected Origin(String origin, long quota, long usage) {
            mOrigin = origin;
            mQuota = quota;
            mUsage = usage;
        }

        protected Origin(String origin, long quota) {
            mOrigin = origin;
            mQuota = quota;
        }

        protected Origin(String origin) {
            mOrigin = origin;
        }

        public String getOrigin() {
            return mOrigin;
        }

        /**
         * Gets the quota for origin of a Web SQL Database.  Returns quota in
         * bytes. For origins that do not use the Web SQL Database API, quota is set to zero.
         */
        public long getQuota() {
            return mQuota;
        }

        /**
         * Returns total amount of storage (bytes) being used by this origin for all JS storage APIs
         */
        public long getUsage() {
            return mUsage;
        }
    }

    private final AwQuotaManagerBridge mQuotaManagerBridge;
    private static WebStorage sWebStorage = null;

    public static WebStorage getInstance() {
        if (sWebStorage == null) {
            sWebStorage = new WebStorage(Engine.getAwBrowserContext().getQuotaManagerBridge());
        }
        return sWebStorage;
    }

    private WebStorage(AwQuotaManagerBridge quotaManagerBridge) {
        mQuotaManagerBridge = quotaManagerBridge;
    }

    public void getOrigins(final ValueCallback<Map> originsCallback) {
        mQuotaManagerBridge.getOrigins(new ValueCallback<AwQuotaManagerBridge.Origins>() {
            @Override
            public void onReceiveValue(AwQuotaManagerBridge.Origins origins) {
                Map<String, Origin> originsMap = new HashMap<String, Origin>();
                for (int i = 0; i < origins.mOrigins.length; ++i) {
                    Origin origin = new Origin(origins.mOrigins[i], origins.mQuotas[i],
                            origins.mUsages[i]) {
                    };
                    originsMap.put(origins.mOrigins[i], origin);
               }
                originsCallback.onReceiveValue(originsMap);
            }
        });
    }

    public void getUsageForOrigin(String origin, ValueCallback<Long> usageCallback) {
        mQuotaManagerBridge.getUsageForOrigin(origin, usageCallback);
    }

    public void getQuotaForOrigin(String origin, ValueCallback<Long> quotaCallback) {
        mQuotaManagerBridge.getQuotaForOrigin(origin, quotaCallback);
    }

    public void deleteOrigin(String origin) {
        mQuotaManagerBridge.deleteOrigin(origin);
    }

    public void deleteAllData() {
        mQuotaManagerBridge.deleteAllData();
    }

    @Deprecated
    public void setQuotaForOrigin(String origin, long quota) {
        // No-op, deprecated
    }
}
