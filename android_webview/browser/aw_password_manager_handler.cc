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

#include "android_webview/browser/aw_password_manager_handler.h"

#include "base/base64.h"
#include "base/bind.h"
#include "base/command_line.h"
#include "base/metrics/field_trial.h"
#include "base/metrics/histogram.h"
#include "base/prefs/pref_service.h"
#include "components/user_prefs/user_prefs.h"
#include "base/strings/string_util.h"
#include "base/strings/utf_string_conversions.h"
#include "base/threading/platform_thread.h"
#include "components/autofill/core/common/autofill_messages.h"
#include "components/autofill/content/browser/autofill_driver_impl.h"
#include "content/public/browser/web_contents.h"
#include "content/public/browser/browser_context.h"
#include "content/public/browser/navigation_details.h"
#include "content/public/browser/navigation_entry.h"
#include "content/public/browser/render_view_host.h"
#include "android_webview/native/aw_contents.h"
#include "android_webview/native/aw_settings.h"
#include "android_webview/browser/aw_browser_context.h"
#include "android_webview/native/aw_contents_client_bridge.h"
#include "base/logging.h"

using content::WebContents;

DEFINE_WEB_CONTENTS_USER_DATA_KEY(android_webview::AwPasswordManagerHandler);

namespace android_webview {



class PromptUserToSavePasswordTask :
    public base::RefCountedThreadSafe<PromptUserToSavePasswordTask> {
 public:
  PromptUserToSavePasswordTask(
      WebContents* web_contents,
      PasswordFormManager* form_to_save)
      : form_to_save_(form_to_save),
        web_contents_(web_contents) {
    DCHECK(form_to_save_);
  }

  void onRememberPasswordResult(int result) {
    switch (result) {
      case NEVER_REMEMBER_PASSWORD:
        form_to_save_->PermanentlyBlacklist();
        break;
      case REMEMBER_PASSWORD:
          form_to_save_->Save();
        break;
      case NOT_NOW:
      case DIALOG_DISMISSED:
      default:
        break;
    }
  }

  ~PromptUserToSavePasswordTask() {}

 private:
  scoped_ptr<PasswordFormManager> form_to_save_;
  content::WebContents* web_contents_;

  // The following values reflect the user response to the remember
  // password dialog and they should be in sync with those defined in
  // swe/engine/java/src/org/codeaurora/swe/AwContentsClientProxy.java
  enum ResponseType {
    NEVER_REMEMBER_PASSWORD = 0,
    REMEMBER_PASSWORD,
    NOT_NOW,
    DIALOG_DISMISSED,
    NUM_RESPONSE_TYPES,
  };

  DISALLOW_COPY_AND_ASSIGN(PromptUserToSavePasswordTask);
};

AwPasswordManagerHandler::AwPasswordManagerHandler(WebContents* web_contents)
    : content::WebContentsObserver(web_contents),
      web_contents_(web_contents),
      password_manager_(PasswordManager(static_cast<PasswordManagerClient*>(this))) {
}

AwPasswordManagerHandler::~AwPasswordManagerHandler() {

}

bool AwPasswordManagerHandler::OnMessageReceived(const IPC::Message& message) {
  bool handled = true;
  IPC_BEGIN_MESSAGE_MAP(AwPasswordManagerHandler, message)
    IPC_MESSAGE_HANDLER(AutofillHostMsg_PasswordFormsParsed,
                        OnPasswordFormsParsed)
    IPC_MESSAGE_HANDLER(AutofillHostMsg_PasswordFormsRendered,
                        OnPasswordFormsRendered)
    IPC_MESSAGE_HANDLER(AutofillHostMsg_PasswordFormSubmitted,
                        OnPasswordFormSubmitted)
    IPC_MESSAGE_UNHANDLED(handled = false)
  IPC_END_MESSAGE_MAP()
  return handled;
}

void AwPasswordManagerHandler::OnPasswordFormsParsed(
    const std::vector<PasswordForm>& forms) {
  password_manager_.OnPasswordFormsParsed(forms);
}

void AwPasswordManagerHandler::OnPasswordFormSubmitted(
    const PasswordForm& password_form) {
  password_manager_.OnPasswordFormSubmitted(password_form);
}

void AwPasswordManagerHandler::OnPasswordFormsRendered(
    const std::vector<PasswordForm>& visible_forms) {
  password_manager_.OnPasswordFormsRendered(visible_forms);
}

void AwPasswordManagerHandler::PromptUserToSavePassword(
    PasswordFormManager* form_to_save) {
  if (ShouldPromptToSavePassword()) {
    AwContentsClientBridgeBase* bridge =
      AwContentsClientBridgeBase::FromWebContents(web_contents_);

    scoped_refptr<PromptUserToSavePasswordTask> task =
      new PromptUserToSavePasswordTask(web_contents_, form_to_save);
    base::Callback<void(int)> cb =
      base::Bind(&PromptUserToSavePasswordTask::onRememberPasswordResult, task);

    bridge->PromptUserToSavePassword(cb);
  }
}

bool AwPasswordManagerHandler::ShouldPromptToSavePassword() {
  AwSettings* aw_settings = AwSettings::FromWebContents(web_contents_);
  bool savePassword = false;
  if (aw_settings)
    savePassword = aw_settings->GetSavePassword();
  return savePassword;
}

void AwPasswordManagerHandler::PasswordWasAutofilled(
    const autofill::PasswordFormMap& password_form_map) const {
}

void AwPasswordManagerHandler::AuthenticateAutofillAndFillForm(
      scoped_ptr<autofill::PasswordFormFillData> fill_data) {
}

PasswordManagerDriver* AwPasswordManagerHandler::GetDriver() {
  return this;
}

PrefService* AwPasswordManagerHandler::GetPrefs() {
  AwBrowserContext::FromWebContents(web_contents())->
      CreateUserPrefServiceIfNecessary();
  return user_prefs::UserPrefs::Get( AwBrowserContext::FromWebContents(web_contents()));;
}

PasswordStore* AwPasswordManagerHandler::GetPasswordStore() {
  return AwBrowserContext::FromWebContents(web_contents())->password_store();
}


void AwPasswordManagerHandler::FillPasswordForm(
    const autofill::PasswordFormFillData& form_data) {
  DCHECK(web_contents());
  if (ShouldPromptToSavePassword()) {
    web_contents()->GetRenderViewHost()->Send(new AutofillMsg_FillPasswordForm(
      web_contents()->GetRenderViewHost()->GetRoutingID(), form_data));
  }
}

void AwPasswordManagerHandler::AllowPasswordGenerationForForm(
    autofill::PasswordForm* form) {
}

bool AwPasswordManagerHandler::EncryptMatch(autofill::PasswordForm* preferred_match) {
    // Encrypt the password !!
    AwContents* awcontent =
        AwContents::FromWebContents(web_contents_);
    std::string encrypted_str;
    bool result = awcontent->Encrypt(preferred_match->password_value, &encrypted_str);
    if (result && !encrypted_str.empty()) {
      std::string base64_encrypted_str;
      base::Base64Encode(encrypted_str, &base64_encrypted_str);
      preferred_match->password_value = base::UTF8ToUTF16(base64_encrypted_str);
    } else {
      preferred_match->password_value = base::UTF8ToUTF16(std::string());
    }
    return true;
}

bool AwPasswordManagerHandler::DecryptMatch(autofill::PasswordForm* preferred_match) {
  bool result = false;
  // Decrypt the password !!
  AwContents* awcontent =
        AwContents::FromWebContents(web_contents_);
  std::string base64_encrypted_str(base::UTF16ToUTF8(preferred_match->password_value));
  if (!base64_encrypted_str.empty()) {
    std::string encrypted_str;
    base::Base64Decode(base64_encrypted_str, &encrypted_str);
    base::string16 decrypted_str;
    result = awcontent->Decrypt(encrypted_str, &decrypted_str);
    if (result) {
      preferred_match->password_value = decrypted_str;
    } else {
      preferred_match->password_value = base::string16();
    }
    // Zero out clear text password
    decrypted_str.assign(decrypted_str.size(), '\0');
  }
  return true;
}

void AwPasswordManagerHandler::AccountCreationFormsFound(
    const std::vector<autofill::FormData>& forms) {
}

bool AwPasswordManagerHandler::DidLastPageLoadEncounterSSLErrors() {
  DCHECK(web_contents());
  content::NavigationEntry* entry =
      web_contents()->GetController().GetActiveEntry();
  if (!entry) {
    NOTREACHED();
    return false;
  }

  return net::IsCertStatusError(entry->GetSSL().cert_status);
}

bool AwPasswordManagerHandler::IsOffTheRecord() {
  DCHECK(web_contents());
  return web_contents()->GetBrowserContext()->IsOffTheRecord();
}

PasswordGenerationManager*
AwPasswordManagerHandler::GetPasswordGenerationManager() {
  return NULL;
}

PasswordManager* AwPasswordManagerHandler::GetPasswordManager() {
  return &password_manager_;
}


autofill::AutofillManager* AwPasswordManagerHandler::GetAutofillManager() {
  autofill::AutofillDriverImpl* driver =
      autofill::AutofillDriverImpl::FromWebContents(web_contents());
  return driver ? driver->autofill_manager() : NULL;
}


}
