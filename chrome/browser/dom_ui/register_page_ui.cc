// Copyright (c) 2010 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/dom_ui/register_page_ui.h"

#include <string>

#include "app/resource_bundle.h"
#include "base/callback.h"
#include "base/logging.h"
#include "base/string_piece.h"
#include "base/utf_string_conversions.h"
#include "base/values.h"
#include "base/weak_ptr.h"
#include "chrome/browser/browser_process.h"
#include "chrome/browser/chrome_thread.h"
#include "chrome/browser/dom_ui/chrome_url_data_manager.h"
#include "chrome/browser/tab_contents/tab_contents.h"
#include "chrome/common/url_constants.h"
#include "grit/browser_resources.h"

#if defined(OS_CHROMEOS)
#include "chrome/browser/chromeos/cros/cros_library.h"
#include "chrome/browser/chromeos/customization_document.h"
#include "chrome/browser/chromeos/login/wizard_controller.h"
#include "chrome/browser/chromeos/version_loader.h"
#endif

namespace {
// Constant value for os_name sent in setUserInfo.
const char kOSName[] = "ChromeOS";
}  // namespace

class RegisterPageUIHTMLSource : public ChromeURLDataManager::DataSource {
 public:
  RegisterPageUIHTMLSource();

  // Called when the network layer has requested a resource underneath
  // the path we registered.
  virtual void StartDataRequest(const std::string& path,
                                bool is_off_the_record,
                                int request_id);
  virtual std::string GetMimeType(const std::string&) const {
    return "text/html";
  }

 private:
  ~RegisterPageUIHTMLSource() {}

  DISALLOW_COPY_AND_ASSIGN(RegisterPageUIHTMLSource);
};

// The handler for Javascript messages related to the "register" view.
class RegisterPageHandler : public DOMMessageHandler,
                            public base::SupportsWeakPtr<RegisterPageHandler> {
 public:
  RegisterPageHandler();
  virtual ~RegisterPageHandler();

  // Init work after Attach.
  void Init();

  // DOMMessageHandler implementation.
  virtual DOMMessageHandler* Attach(DOMUI* dom_ui);
  virtual void RegisterMessages();

 private:
  // Handlers for JS DOMUI messages.
  void HandleGetRegistrationUrl(const ListValue* args);
  void HandleGetUserInfo(const ListValue* args);

#if defined(OS_CHROMEOS)
  // Callback from chromeos::VersionLoader giving the version.
  void OnVersion(chromeos::VersionLoader::Handle handle, std::string version);
#endif

  // Sends message to host registration page with system/user info data.
  void SendUserInfo();

#if defined(OS_CHROMEOS)
  // Handles asynchronously loading the version.
  chromeos::VersionLoader version_loader_;
#endif

  // Used to request the version.
  CancelableRequestConsumer version_consumer_;

  std::string version_;

  DISALLOW_COPY_AND_ASSIGN(RegisterPageHandler);
};

////////////////////////////////////////////////////////////////////////////////
//
// RegisterPageUIHTMLSource
//
////////////////////////////////////////////////////////////////////////////////

RegisterPageUIHTMLSource::RegisterPageUIHTMLSource()
    : DataSource(chrome::kChromeUIRegisterPageHost, MessageLoop::current()) {
}

void RegisterPageUIHTMLSource::StartDataRequest(const std::string& path,
                                                bool is_off_the_record,
                                                int request_id) {
  // Make sure that chrome://register is available only during
  // OOBE wizard lifetime and when device has not been registered yet.
#if defined(OS_CHROMEOS)
  if (!WizardController::default_controller() ||
      WizardController::IsDeviceRegistered()) {
    scoped_refptr<RefCountedBytes> empty_bytes(new RefCountedBytes);
    SendResponse(request_id, empty_bytes);
    return;
  }

  static const base::StringPiece register_html(
      ResourceBundle::GetSharedInstance().GetRawDataResource(
          IDR_HOST_REGISTRATION_PAGE_HTML));

  scoped_refptr<RefCountedBytes> html_bytes(new RefCountedBytes);
  html_bytes->data.resize(register_html.size());
  std::copy(register_html.begin(),
            register_html.end(),
            html_bytes->data.begin());

  SendResponse(request_id, html_bytes);
#else
  scoped_refptr<RefCountedBytes> empty_bytes(new RefCountedBytes);
  SendResponse(request_id, empty_bytes);
#endif
}

////////////////////////////////////////////////////////////////////////////////
//
// RegisterPageHandler
//
////////////////////////////////////////////////////////////////////////////////
RegisterPageHandler::RegisterPageHandler() {
}

RegisterPageHandler::~RegisterPageHandler() {
}

DOMMessageHandler* RegisterPageHandler::Attach(DOMUI* dom_ui) {
  return DOMMessageHandler::Attach(dom_ui);
}

void RegisterPageHandler::Init() {
}

void RegisterPageHandler::RegisterMessages() {
#if defined(OS_CHROMEOS)
  dom_ui_->RegisterMessageCallback("getRegistrationUrl",
      NewCallback(this, &RegisterPageHandler::HandleGetRegistrationUrl));
  dom_ui_->RegisterMessageCallback("getUserInfo",
      NewCallback(this, &RegisterPageHandler::HandleGetUserInfo));
#endif
}

void RegisterPageHandler::HandleGetRegistrationUrl(const ListValue* args) {
#if defined(OS_CHROMEOS)
  if (WizardController::default_controller() &&
      WizardController::default_controller()->GetCustomization()) {
    const std::string& url = WizardController::default_controller()->
        GetCustomization()->registration_url();
    LOG(INFO) << "Loading registration form with URL: " << url;
    StringValue url_value(url);
    dom_ui_->CallJavascriptFunction(L"setRegistrationUrl", url_value);
  } else {
    LOG(ERROR) << "Startup manifest not defined.";
    dom_ui_->CallJavascriptFunction(L"skipRegistration");
  }
#endif
}

void RegisterPageHandler::HandleGetUserInfo(const ListValue* args) {
#if defined(OS_CHROMEOS)
  if (chromeos::CrosLibrary::Get()->EnsureLoaded()) {
     version_loader_.GetVersion(
         &version_consumer_, NewCallback(this,
                                         &RegisterPageHandler::OnVersion));
  } else {
    LOG(ERROR) << "Error loading cros library.";
    dom_ui_->CallJavascriptFunction(L"skipRegistration");
  }
#endif
}

#if defined(OS_CHROMEOS)
void RegisterPageHandler::OnVersion(chromeos::VersionLoader::Handle handle,
                                    std::string version) {
  version_ = version;
  SendUserInfo();
}
#endif

void RegisterPageHandler::SendUserInfo() {
#if defined(OS_CHROMEOS)
  DictionaryValue value;
  // TODO(nkostylev): Extract all available system/user info.
  // http://crosbug.com/4813

  std::string system_sku;
  if (WizardController::default_controller() &&
      WizardController::default_controller()->GetCustomization()) {
    system_sku = WizardController::default_controller()->
        GetCustomization()->product_sku();
  } else {
    LOG(ERROR) << "Startup manifest not defined.";
    dom_ui_->CallJavascriptFunction(L"skipRegistration");
    return;
  }

  // Required info.
  value.SetString("system_hwqual", "hardware qual identifier");
  value.SetString("system_sku", system_sku);
  value.SetString("system_serial", "serial number");
  value.SetString("os_language", g_browser_process->GetApplicationLocale());
  value.SetString("os_name", kOSName);
  value.SetString("os_version", version_);
  value.SetString("os_connection", "connection type");
  value.SetString("user_email", "");

  // Optional info.
  value.SetString("user_first_name", "");
  value.SetString("user_last_name", "");

  LOG(INFO) << "Sending user info to host page";
  dom_ui_->CallJavascriptFunction(L"setUserInfo", value);
#endif
}

////////////////////////////////////////////////////////////////////////////////
//
// RegisterPageUI
//
////////////////////////////////////////////////////////////////////////////////

RegisterPageUI::RegisterPageUI(TabContents* contents) : DOMUI(contents){
  RegisterPageHandler* handler = new RegisterPageHandler();
  AddMessageHandler((handler)->Attach(this));
  handler->Init();
  RegisterPageUIHTMLSource* html_source = new RegisterPageUIHTMLSource();

  // Set up the chrome://register/ source.
  ChromeThread::PostTask(
      ChromeThread::IO, FROM_HERE,
      NewRunnableMethod(
          Singleton<ChromeURLDataManager>::get(),
          &ChromeURLDataManager::AddDataSource,
          make_scoped_refptr(html_source)));
}
