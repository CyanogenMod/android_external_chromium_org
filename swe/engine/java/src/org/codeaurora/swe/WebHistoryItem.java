/*
 * Copyright (c) 2013 The Linux Foundation. All rights reserved.
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

import org.chromium.content.browser.NavigationEntry;

import android.graphics.Bitmap;

import java.net.MalformedURLException;
import java.net.URL;


public class WebHistoryItem {

    private static int sNextId = 0;
    private final int mId;
    private NavigationEntry mNavigationEntry;

    public WebHistoryItem(NavigationEntry navEntry) {
        mNavigationEntry = navEntry;
        mId = sNextId++;
    }

    @Deprecated
    public int getId() {
        return mId;
    }

    public String getUrl() {
        return mNavigationEntry.getUrl();
    }

    public String getOriginalUrl() {
        return mNavigationEntry.getOriginalUrl();
    }

    public String getTitle() {
        return mNavigationEntry.getTitle();
    }

    public Bitmap getFavicon() {
        return mNavigationEntry.getFavicon();
    }

    public String getTouchIconUrl() {
        String touchIconUrlFromLinkElement = mNavigationEntry.getTouchIconUrl();
        if (touchIconUrlFromLinkElement == null) {
            // If no <link> element is specified, the default location of the
            // icon is <host>/apple-touch-icon.png
            try {
                URL url = new URL(getOriginalUrl());
                touchIconUrlFromLinkElement = new URL(url.getProtocol(),
                                                 url.getHost(),
                                                 url.getPort(),
                                                 "/apple-touch-icon.png").toString();
            }
            catch (MalformedURLException e) {
                touchIconUrlFromLinkElement = null;
            }
        }
        return touchIconUrlFromLinkElement;
    }
}

