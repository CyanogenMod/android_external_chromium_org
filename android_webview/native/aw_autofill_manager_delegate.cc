// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "android_webview/native/aw_autofill_manager_delegate.h"

#include "android_webview/browser/aw_browser_context.h"
#include "android_webview/browser/aw_content_browser_client.h"
#include "android_webview/browser/aw_form_database_service.h"
#include "android_webview/browser/aw_pref_store.h"
#include "android_webview/native/aw_contents.h"
#include "base/android/jni_android.h"
#include "base/android/jni_string.h"
#include "base/android/scoped_java_ref.h"
#include "base/logging.h"
#include "base/prefs/pref_registry_simple.h"
#include "base/prefs/pref_service.h"
#include "base/prefs/pref_service_factory.h"
#include "components/autofill/core/browser/autofill_popup_delegate.h"
#include "components/autofill/core/browser/webdata/autofill_webdata_service.h"
#include "components/autofill/core/common/autofill_pref_names.h"
#include "components/user_prefs/user_prefs.h"
#include "content/public/browser/web_contents.h"
#include "content/public/browser/web_contents_view.h"
#include "jni/AwAutofillManagerDelegate_jni.h"
#include "components/autofill/core/browser/webdata/autofill_webdata_service.h"
#include "components/autofill/core/browser/personal_data_manager.h"
#include "components/autofill/core/browser/autofill_profile.h"
#include "components/autofill/core/browser/autofill_type.h"
#include "base/strings/utf_string_conversions.h"
#include "base/logging.h"

using base::android::AttachCurrentThread;
using base::android::ConvertUTF16ToJavaString;
using base::android::ConvertUTF8ToJavaString;
using base::android::ConvertJavaStringToUTF16;
using base::android::ConvertJavaStringToUTF8;
using base::android::ScopedJavaLocalRef;
using content::WebContents;
using autofill::AutofillWebDataService;
using autofill::AutofillProfile;
using autofill::ServerFieldType;

DEFINE_WEB_CONTENTS_USER_DATA_KEY(android_webview::AwAutofillManagerDelegate);

namespace android_webview {

// Ownership: The native object is created (if autofill enabled) and owned by
// AwContents. The native object creates the java peer which handles most
// autofill functionality at the java side. The java peer is owned by Java
// AwContents. The native object only maintains a weak ref to it.
AwAutofillManagerDelegate::AwAutofillManagerDelegate(WebContents* contents)
    : web_contents_(contents),
      save_form_data_(false) {
  JNIEnv* env = AttachCurrentThread();
  ScopedJavaLocalRef<jobject> delegate;
  delegate.Reset(
      Java_AwAutofillManagerDelegate_create(
          env, reinterpret_cast<intptr_t>(this)));

  AwContents* aw_contents = AwContents::FromWebContents(web_contents_);
  aw_contents->SetAwAutofillManagerDelegate(delegate.obj());
  java_ref_ = JavaObjectWeakGlobalRef(env, delegate.obj());
}

AwAutofillManagerDelegate::~AwAutofillManagerDelegate() {
  HideAutofillPopup();
}

void AwAutofillManagerDelegate::SetSaveFormData(bool enabled) {
  save_form_data_ = enabled;
}

bool AwAutofillManagerDelegate::GetSaveFormData() {
  return save_form_data_;
}

PrefService* AwAutofillManagerDelegate::GetPrefs() {
  return user_prefs::UserPrefs::Get(AwBrowserContext::FromWebContents(web_contents_));
}

autofill::PersonalDataManager*
AwAutofillManagerDelegate::GetPersonalDataManager() {
  AwBrowserContext* context = AwBrowserContext::FromWebContents(web_contents_);
  if (!context->IsOffTheRecord()) {
    if (personal_data_.get())
      return personal_data_.get();
    personal_data_.reset(new autofill::PersonalDataManager("en-US"));
    personal_data_->Init(GetDatabase(),
      GetPrefs(), context->IsOffTheRecord());
    return personal_data_.get();
  } else {
    if (personal_data_incog.get())
      return personal_data_incog.get();
    personal_data_incog.reset(new autofill::PersonalDataManager("en-US"));
    personal_data_incog->Init(GetDatabase(),
      GetPrefs(), context->IsOffTheRecord());
    return personal_data_incog.get();
  }

}

std::string AwAutofillManagerDelegate::AddOrUpdateProfile(jstring guid, jstring name_full,
    jstring email, jstring company, jstring address1, jstring address2, jstring city,
    jstring state, jstring zipcode, jstring country, jstring phone) {
  AutofillProfile* profile = NULL;
  JNIEnv* env = AttachCurrentThread();
  std::string str_guid = ConvertJavaStringToUTF8(env,guid);
  // Check if we have an already existing profile with the GUID
  profile = GetPersonalDataManager()->GetProfileByGUID(str_guid);
  bool already;
  if (profile) {
    LOG(INFO) << "Already exisiting profile GUID : " << str_guid;
    already= true;
  } else {
    LOG(INFO) << "New profile : ";
    profile = new AutofillProfile();
    already = false;
  }
  str_guid = profile->guid();
  profile->SetRawInfo(autofill::NAME_FULL, ConvertJavaStringToUTF16(env,name_full));
  profile->SetRawInfo(autofill::EMAIL_ADDRESS, ConvertJavaStringToUTF16(env,email));
  profile->SetRawInfo(autofill::COMPANY_NAME, ConvertJavaStringToUTF16(env,company));
  profile->SetRawInfo(autofill::ADDRESS_HOME_LINE1, ConvertJavaStringToUTF16(env,address1));
  profile->SetRawInfo(autofill::ADDRESS_HOME_LINE2, ConvertJavaStringToUTF16(env,address2));
  profile->SetRawInfo(autofill::ADDRESS_HOME_CITY, ConvertJavaStringToUTF16(env,city));
  profile->SetRawInfo(autofill::ADDRESS_HOME_STATE, ConvertJavaStringToUTF16(env,state));
  profile->SetRawInfo(autofill::ADDRESS_HOME_ZIP, ConvertJavaStringToUTF16(env,zipcode));
  profile->SetRawInfo(autofill::ADDRESS_HOME_COUNTRY, ConvertJavaStringToUTF16(env,country));
  profile->SetRawInfo(autofill::PHONE_HOME_WHOLE_NUMBER, ConvertJavaStringToUTF16(env,phone));
  if (!already){
    GetPersonalDataManager()->AddProfile(*profile);
    delete profile;
  } else {
    GetPersonalDataManager()->UpdateProfile(*profile);
  }
  LOG(INFO) << "Profile GUID : " << str_guid;
  return str_guid;
}

void AwAutofillManagerDelegate::RemoveProfileByGUID(jstring guid) {
  JNIEnv* env = AttachCurrentThread();
  std::string str_guid = ConvertJavaStringToUTF8(env,guid);
  LOG(INFO) << "AwAutofillManagerDelegate::RemoveProfileByGUID " << str_guid;
  personal_data_->RemoveByGUID(str_guid);
}

ScopedJavaLocalRef<jobject> AwAutofillManagerDelegate::GetProfileByGUID(jstring guid) {
  JNIEnv* env = AttachCurrentThread();
  AutofillProfile* profile = NULL;
  ScopedJavaLocalRef<jobject> jprofile;
  if (!GetPersonalDataManager()->IsDataLoaded()) {
    LOG(INFO) << "Profiles not loaded yet....";
  } else {
    std::string str_guid = ConvertJavaStringToUTF8(env,guid);
    profile = GetPersonalDataManager()->GetProfileByGUID(str_guid);
    if (profile) {
      LOG(INFO) << "Already exisiting profile GUID : " << str_guid;
      ScopedJavaLocalRef<jstring> guid = ConvertUTF8ToJavaString(env, profile->guid());
      ScopedJavaLocalRef<jstring> name_full = ConvertUTF16ToJavaString(env, profile->GetRawInfo(autofill::NAME_FULL));
      ScopedJavaLocalRef<jstring> email = ConvertUTF16ToJavaString(env, profile->GetRawInfo(autofill::EMAIL_ADDRESS));
      ScopedJavaLocalRef<jstring> company = ConvertUTF16ToJavaString(env, profile->GetRawInfo(autofill::COMPANY_NAME));
      ScopedJavaLocalRef<jstring> address1 = ConvertUTF16ToJavaString(env, profile->GetRawInfo(autofill::ADDRESS_HOME_LINE1));
      ScopedJavaLocalRef<jstring> address2 = ConvertUTF16ToJavaString(env, profile->GetRawInfo(autofill::ADDRESS_HOME_LINE2));
      ScopedJavaLocalRef<jstring> city = ConvertUTF16ToJavaString(env, profile->GetRawInfo(autofill::ADDRESS_HOME_CITY));
      ScopedJavaLocalRef<jstring> state = ConvertUTF16ToJavaString(env, profile->GetRawInfo(autofill::ADDRESS_HOME_STATE));
      ScopedJavaLocalRef<jstring> zipcode = ConvertUTF16ToJavaString(env, profile->GetRawInfo(autofill::ADDRESS_HOME_ZIP));
      ScopedJavaLocalRef<jstring> country = ConvertUTF16ToJavaString(env, profile->GetRawInfo(autofill::ADDRESS_HOME_COUNTRY));
      ScopedJavaLocalRef<jstring> phone = ConvertUTF16ToJavaString(env, profile->GetRawInfo(autofill::PHONE_HOME_WHOLE_NUMBER));
      jprofile = Java_AwAutofillManagerDelegate_createAutoFillProfile(env,guid.obj(), name_full.obj(),
        email.obj(), company.obj(), address1.obj(), address2.obj(),
        city.obj(), state.obj(), zipcode.obj(), country.obj(), phone.obj());
    }
  }
  return jprofile;
}

ScopedJavaLocalRef<jobjectArray> AwAutofillManagerDelegate::GetAllAutoFillProfiles() {
  JNIEnv* env = AttachCurrentThread();
  size_t count =  personal_data_->web_profiles().size();
  ScopedJavaLocalRef<jobjectArray> data_array =
      Java_AwAutofillManagerDelegate_createAutoFillProfileArray(env, count);
  count = 0;
  for (std::vector<AutofillProfile*>::const_iterator i =
           personal_data_->web_profiles().begin();
       i != personal_data_->web_profiles().end(); ++i) {
    LOG(INFO) << "profile guid :" << (*i)->guid();
    ScopedJavaLocalRef<jstring> guid = ConvertUTF8ToJavaString(env, (*i)->guid());
    ScopedJavaLocalRef<jstring> name_full = ConvertUTF16ToJavaString(env, (*i)->GetRawInfo(autofill::NAME_FULL));
    ScopedJavaLocalRef<jstring> email = ConvertUTF16ToJavaString(env, (*i)->GetRawInfo(autofill::EMAIL_ADDRESS));
    ScopedJavaLocalRef<jstring> company = ConvertUTF16ToJavaString(env, (*i)->GetRawInfo(autofill::COMPANY_NAME));
    ScopedJavaLocalRef<jstring> address1 = ConvertUTF16ToJavaString(env, (*i)->GetRawInfo(autofill::ADDRESS_HOME_LINE1));
    ScopedJavaLocalRef<jstring> address2 = ConvertUTF16ToJavaString(env, (*i)->GetRawInfo(autofill::ADDRESS_HOME_LINE2));
    ScopedJavaLocalRef<jstring> city = ConvertUTF16ToJavaString(env, (*i)->GetRawInfo(autofill::ADDRESS_HOME_CITY));
    ScopedJavaLocalRef<jstring> state = ConvertUTF16ToJavaString(env, (*i)->GetRawInfo(autofill::ADDRESS_HOME_STATE));
    ScopedJavaLocalRef<jstring> zipcode = ConvertUTF16ToJavaString(env, (*i)->GetRawInfo(autofill::ADDRESS_HOME_ZIP));
    ScopedJavaLocalRef<jstring> country = ConvertUTF16ToJavaString(env, (*i)->GetRawInfo(autofill::ADDRESS_HOME_COUNTRY));
    ScopedJavaLocalRef<jstring> phone = ConvertUTF16ToJavaString(env, (*i)->GetRawInfo(autofill::PHONE_HOME_WHOLE_NUMBER));
    Java_AwAutofillManagerDelegate_addToAutoFillProfileArray(env, data_array.obj(), count,
     guid.obj(), name_full.obj(), email.obj(), company.obj(), address1.obj(), address2.obj(),
     city.obj(), state.obj(), zipcode.obj(), country.obj(), phone.obj());
    ++count;
  }
  return data_array;
}

void AwAutofillManagerDelegate::RemoveAllAutoFillProfiles() {
  for (std::vector<AutofillProfile*>::const_iterator i =
           personal_data_->web_profiles().begin();
       i != personal_data_->web_profiles().end(); ++i) {
      LOG(INFO) << "Removing profile guid :" << (*i)->guid();
      personal_data_->RemoveByGUID((*i)->guid());
  }
}

scoped_refptr<autofill::AutofillWebDataService>
AwAutofillManagerDelegate::GetDatabase() {
  android_webview::AwFormDatabaseService* service =
      static_cast<android_webview::AwBrowserContext*>(
          web_contents_->GetBrowserContext())->GetFormDatabaseService();
  return service->get_autofill_webdata_service();
}

void AwAutofillManagerDelegate::ShowAutofillPopup(
    const gfx::RectF& element_bounds,
    base::i18n::TextDirection text_direction,
    const std::vector<base::string16>& values,
    const std::vector<base::string16>& labels,
    const std::vector<base::string16>& icons,
    const std::vector<int>& identifiers,
    base::WeakPtr<autofill::AutofillPopupDelegate> delegate) {

  values_ = values;
  identifiers_ = identifiers;
  delegate_ = delegate;

  // Convert element_bounds to be in screen space.
  gfx::Rect client_area;
  web_contents_->GetView()->GetContainerBounds(&client_area);
  gfx::RectF element_bounds_in_screen_space =
      element_bounds + client_area.OffsetFromOrigin();

  ShowAutofillPopupImpl(element_bounds_in_screen_space,
                        values,
                        labels,
                        identifiers);
}

void AwAutofillManagerDelegate::ShowAutofillPopupImpl(
    const gfx::RectF& element_bounds,
    const std::vector<base::string16>& values,
    const std::vector<base::string16>& labels,
    const std::vector<int>& identifiers) {
  JNIEnv* env = AttachCurrentThread();
  ScopedJavaLocalRef<jobject> obj = java_ref_.get(env);
  if (obj.is_null())
    return;

  // We need an array of AutofillSuggestion.
  size_t count = values.size();

  ScopedJavaLocalRef<jobjectArray> data_array =
      Java_AwAutofillManagerDelegate_createAutofillSuggestionArray(env, count);

  for (size_t i = 0; i < count; ++i) {
    ScopedJavaLocalRef<jstring> name = ConvertUTF16ToJavaString(env, values[i]);
    ScopedJavaLocalRef<jstring> label =
        ConvertUTF16ToJavaString(env, labels[i]);
    Java_AwAutofillManagerDelegate_addToAutofillSuggestionArray(
        env,
        data_array.obj(),
        i,
        name.obj(),
        label.obj(),
        identifiers[i]);
  }

  Java_AwAutofillManagerDelegate_showAutofillPopup(
      env,
      obj.obj(),
      element_bounds.x(),
      element_bounds.y(), element_bounds.width(),
      element_bounds.height(), data_array.obj());
}

void AwAutofillManagerDelegate::UpdateAutofillPopupDataListValues(
    const std::vector<base::string16>& values,
    const std::vector<base::string16>& labels) {
  // Leaving as an empty method since updating autofill popup window
  // dynamically does not seem to be a useful feature for android webview.
  // See crrev.com/18102002 if need to implement.
}

void AwAutofillManagerDelegate::HideAutofillPopup() {
  JNIEnv* env = AttachCurrentThread();
  ScopedJavaLocalRef<jobject> obj = java_ref_.get(env);
  if (obj.is_null())
    return;
  delegate_.reset();
  Java_AwAutofillManagerDelegate_hideAutofillPopup(env, obj.obj());
}

bool AwAutofillManagerDelegate::IsAutocompleteEnabled() {
  return GetSaveFormData();
}

void AwAutofillManagerDelegate::DetectAccountCreationForms(
    const std::vector<autofill::FormStructure*>& forms) {}

void AwAutofillManagerDelegate::SuggestionSelected(JNIEnv* env,
                                                   jobject object,
                                                   jint position) {
  if (delegate_)
    delegate_->DidAcceptSuggestion(values_[position], identifiers_[position]);
}

void AwAutofillManagerDelegate::HideRequestAutocompleteDialog() {
}

void AwAutofillManagerDelegate::ShowAutofillSettings() {
}

void AwAutofillManagerDelegate::ConfirmSaveCreditCard(
    const autofill::AutofillMetrics& metric_logger,
    const base::Closure& save_card_callback) {
}

void AwAutofillManagerDelegate::ShowRequestAutocompleteDialog(
    const autofill::FormData& form,
    const GURL& source_url,
    const base::Callback<void(const autofill::FormStructure*)>& callback) {
}

bool RegisterAwAutofillManagerDelegate(JNIEnv* env) {
  return RegisterNativesImpl(env) >= 0;
}

} // namespace android_webview
