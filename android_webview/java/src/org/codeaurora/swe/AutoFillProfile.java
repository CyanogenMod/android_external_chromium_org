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

public class AutoFillProfile {
    protected String mUniqueId;
    private String mFullName;
    private String mEmailAddress;
    private String mCompanyName;
    private String mAddressLine1;
    private String mAddressLine2;
    private String mCity;
    private String mState;
    private String mZipCode;
    private String mCountry;
    private String mPhoneNumber;

    public AutoFillProfile(String uniqueId, String fullName, String email,
            String companyName, String addressLine1, String addressLine2,
            String city, String state, String zipCode, String country,
            String phoneNumber) {
        mUniqueId = uniqueId;
        mFullName = fullName;
        mEmailAddress = email;
        mCompanyName = companyName;
        mAddressLine1 = addressLine1;
        mAddressLine2 = addressLine2;
        mCity = city;
        mState = state;
        mZipCode = zipCode;
        mCountry = country;
        mPhoneNumber = phoneNumber;
    }

    public String getUniqueId() { return mUniqueId; }
    public String getFullName() { return mFullName; }
    public String getEmailAddress() { return mEmailAddress; }
    public String getCompanyName() { return mCompanyName; }
    public String getAddressLine1() { return mAddressLine1; }
    public String getAddressLine2() { return mAddressLine2; }
    public String getCity() { return mCity; }
    public String getState() { return mState; }
    public String getZipCode() { return mZipCode; }
    public String getCountry() { return mCountry; }
    public String getPhoneNumber() { return mPhoneNumber; }
}
