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

#ifndef ANDROID_WEBVIEW_PASSWORD_MANAGER_HANDLER_H_
#define ANDROID_WEBVIEW_PASSWORD_MANAGER_HANDLER_H_

#include "content/public/browser/web_contents_observer.h"
#include "content/public/browser/web_contents_user_data.h"
#include "components/autofill/core/common/password_form.h"
#include "components/autofill/core/common/password_form_fill_data.h"
#include "components/password_manager/core/browser/password_manager.h"
#include "components/autofill/content/renderer/password_generation_manager.h"
#include "components/password_manager/core/browser/password_manager_client.h"
#include "components/password_manager/core/browser/password_manager_driver.h"

using autofill::PasswordForm;
namespace android_webview {

class AwPasswordStore;

class AwPasswordManagerHandler :  public PasswordManagerClient,
                                 public content::WebContentsObserver,
                                 public PasswordManagerDriver,
                                 public content::WebContentsUserData<AwPasswordManagerHandler> {
 public:

  AwPasswordManagerHandler(content::WebContents* web_contents);
  virtual ~AwPasswordManagerHandler();

  // content::WebContentsObserver overrides.
  virtual bool OnMessageReceived(const IPC::Message& message) OVERRIDE;

  // PasswordManagerClient implementation.
  virtual void PromptUserToSavePassword(PasswordFormManager* form_to_save)
      OVERRIDE;
  virtual void PasswordWasAutofilled(
      const autofill::PasswordFormMap& best_matches) const OVERRIDE;
  virtual void AuthenticateAutofillAndFillForm(
      scoped_ptr<autofill::PasswordFormFillData> fill_data) OVERRIDE;
  virtual PrefService* GetPrefs() OVERRIDE;
  virtual PasswordStore* GetPasswordStore() OVERRIDE;
  virtual PasswordManagerDriver* GetDriver() OVERRIDE;

  // PasswordManagerDriver implementation.
  virtual void FillPasswordForm(const autofill::PasswordFormFillData& form_data)
      OVERRIDE;
  virtual void AllowPasswordGenerationForForm(autofill::PasswordForm* form)
      OVERRIDE;
  virtual void AccountCreationFormsFound(
      const std::vector<autofill::FormData>& forms) OVERRIDE;
  virtual bool DidLastPageLoadEncounterSSLErrors() OVERRIDE;
  virtual bool IsOffTheRecord() OVERRIDE;
  virtual PasswordGenerationManager* GetPasswordGenerationManager() OVERRIDE;
  virtual PasswordManager* GetPasswordManager() OVERRIDE;
  virtual autofill::AutofillManager* GetAutofillManager() OVERRIDE;
  virtual bool DecryptMatch(autofill::PasswordForm* preferred_match) OVERRIDE;
  virtual bool EncryptMatch(autofill::PasswordForm* preferred_match) OVERRIDE;


  void OnPasswordFormsParsed(
      const std::vector<PasswordForm>& forms);
  void OnPasswordFormsRendered(
      const std::vector<PasswordForm>& visible_forms);
  void OnPasswordFormSubmitted(const PasswordForm& password_form);

  bool ShouldPromptToSavePassword();

   // Convenience method to allow //AWC code easy access to a PasswordManager
  // from a WebContents instance.
  static PasswordManager* GetManagerFromWebContents(
      content::WebContents* contents);

 protected:

 private:
  friend class content::WebContentsUserData<AwPasswordManagerHandler>;
  content::WebContents* web_contents_;
  PasswordManager password_manager_;
  DISALLOW_COPY_AND_ASSIGN(AwPasswordManagerHandler);
};

}

#endif  // ANDROID_WEBVIEW_PASSWORD_MANAGER_HANDLER_H_
