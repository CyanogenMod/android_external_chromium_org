// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.chromium.android_webview;

import android.view.ViewGroup;

import org.chromium.base.CalledByNative;
import org.chromium.base.JNINamespace;
import org.chromium.content.browser.ContentViewCore;
import org.chromium.ui.autofill.AutofillPopup;
import org.chromium.ui.autofill.AutofillSuggestion;
import org.codeaurora.swe.AutoFillProfile;

/**
 * Java counterpart to the AwAutofillClient. This class is owned by AwContents and has
 * a weak reference from native side.
 */
@JNINamespace("android_webview")
public class AwAutofillClient {

    private final long mNativeAwAutofillClient;
    private AutofillPopup mAutofillPopup;
    private ViewGroup mContainerView;
    private ContentViewCore mContentViewCore;

    @CalledByNative
    public static AwAutofillClient create(long nativeClient) {
        return new AwAutofillClient(nativeClient);
    }

    private AwAutofillClient(long nativeAwAutofillClient) {
        mNativeAwAutofillClient = nativeAwAutofillClient;
    }

    public void init(ContentViewCore contentViewCore) {
        mContentViewCore = contentViewCore;
        mContainerView = contentViewCore.getContainerView();
    }

    @CalledByNative
    private void showAutofillPopup(float x, float y, float width, float height,
            boolean isRtl, AutofillSuggestion[] suggestions) {

        if (mContentViewCore == null) return;

        if (mAutofillPopup == null) {
            mAutofillPopup = new AutofillPopup(
                mContentViewCore.getContext(),
                mContentViewCore.getViewAndroidDelegate(),
                new AutofillPopup.AutofillPopupDelegate() {
                    @Override
                    public void requestHide() { }
                    @Override
                    public void suggestionSelected(int listIndex) {
                        nativeSuggestionSelected(mNativeAwAutofillClient, listIndex);
                    }
                });
        }
        mAutofillPopup.setAnchorRect(x, y, width, height);
        mAutofillPopup.filterAndShow(suggestions, isRtl);
    }

    @CalledByNative
    public void hideAutofillPopup() {
        if (mAutofillPopup == null)
            return;
        mAutofillPopup.hide();
        mAutofillPopup = null;
    }

// SWE-feature-autofill-profile
    @CalledByNative
    private static AutoFillProfile createAutoFillProfile(String uniqueId, String fullName,
          String emailAddress, String companyName, String addressLine1, String addressLine2,
          String city, String state, String zipCode, String country, String phoneNumber) {
        return new AutoFillProfile(uniqueId, fullName, emailAddress, companyName,
            addressLine1, addressLine2, city, state, zipCode, country, phoneNumber);
    }

    @CalledByNative
    private static AutoFillProfile[] createAutoFillProfileArray(int size) {
        return new AutoFillProfile[size];
    }

    @CalledByNative
    private static void addToAutoFillProfileArray(AutoFillProfile[] array, int index,
          String uniqueId, String fullName, String emailAddress,
          String companyName, String addressLine1, String addressLine2,
          String city, String state, String zipCode, String country,
          String phoneNumber) {
        array[index] = new AutoFillProfile(uniqueId, fullName, emailAddress, companyName,
            addressLine1, addressLine2, city, state, zipCode, country, phoneNumber);
    }

    @CalledByNative
    private static AutofillSuggestion[] createAutofillSuggestionArray(int size) {
        return new AutofillSuggestion[size];
    }
// SWE-feature-autofill-profile

    /**
     * @param array AutofillSuggestion array that should get a new suggestion added.
     * @param index Index in the array where to place a new suggestion.
     * @param name Name of the suggestion.
     * @param label Label of the suggestion.
     * @param uniqueId Unique suggestion id.
     */
    @CalledByNative
    private static void addToAutofillSuggestionArray(AutofillSuggestion[] array, int index,
            String name, String label, int uniqueId) {
        array[index] = new AutofillSuggestion(name, label, uniqueId);
    }

    private native void nativeSuggestionSelected(long nativeAwAutofillClient,
            int position);
}
