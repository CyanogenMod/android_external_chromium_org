// Copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/ui/webui/flags_ui.h"

#include <string>

#include "base/memory/singleton.h"
#include "base/utf_string_conversions.h"
#include "base/values.h"
#include "chrome/browser/about_flags.h"
#include "chrome/browser/browser_process.h"
#include "chrome/browser/prefs/pref_service.h"
#include "chrome/browser/profiles/profile.h"
#include "chrome/browser/ui/browser_list.h"
#include "chrome/browser/ui/webui/chrome_web_ui_data_source.h"
#include "chrome/common/jstemplate_builder.h"
#include "chrome/common/pref_names.h"
#include "chrome/common/url_constants.h"
#include "content/browser/browser_thread.h"
#include "content/browser/tab_contents/tab_contents.h"
#include "grit/browser_resources.h"
#include "grit/chromium_strings.h"
#include "grit/generated_resources.h"
#include "grit/theme_resources.h"
#include "ui/base/l10n/l10n_util.h"
#include "ui/base/resource/resource_bundle.h"

#if defined(OS_CHROMEOS)
#include "chrome/browser/chromeos/user_cros_settings_provider.h"
#include "chrome/browser/chromeos/login/user_manager.h"
#endif

static const char kFlagsJs[] = "flags.js";
static const char kStringsJs[] = "strings.js";

///////////////////////////////////////////////////////////////////////////////
//
// FlagsUIHTMLSource
//
///////////////////////////////////////////////////////////////////////////////

class FlagsUIHTMLSource : public ChromeWebUIDataSource {
 public:
  FlagsUIHTMLSource();

  // Called when the network layer has requested a resource underneath
  // the path we registered.
  virtual void StartDataRequest(const std::string& path,
                                bool is_incognito,
                                int request_id);
  virtual std::string GetMimeType(const std::string&) const;

 private:
  ~FlagsUIHTMLSource() {}
  DISALLOW_COPY_AND_ASSIGN(FlagsUIHTMLSource);
};

FlagsUIHTMLSource::FlagsUIHTMLSource()
    : ChromeWebUIDataSource(chrome::kChromeUIFlagsHost) {
  AddLocalizedString("flagsLongTitle", IDS_FLAGS_LONG_TITLE);
  AddLocalizedString("flagsTableTitle", IDS_FLAGS_TABLE_TITLE);
  AddLocalizedString("flagsNoExperimentsAvailable",
                     IDS_FLAGS_NO_EXPERIMENTS_AVAILABLE);
  AddLocalizedString("flagsWarningHeader", IDS_FLAGS_WARNING_HEADER);
  AddLocalizedString("flagsBlurb", IDS_FLAGS_WARNING_TEXT);
#if defined(OS_CHROMEOS)
  int ids = IDS_PRODUCT_OS_NAME;
#else
  int ids = IDS_PRODUCT_NAME;
#endif
  AddString("flagsRestartNotice",
            l10n_util::GetStringFUTF16(IDS_FLAGS_RELAUNCH_NOTICE,
                                       l10n_util::GetStringUTF16(ids)));
  AddLocalizedString("flagsRestartButton", IDS_FLAGS_RELAUNCH_BUTTON);
  AddLocalizedString("disable", IDS_FLAGS_DISABLE);
  AddLocalizedString("enable", IDS_FLAGS_ENABLE);
#if defined(OS_CHROMEOS)
  // Set the strings to show which user can actually change the flags
  AddLocalizedString("ownerOnly", IDS_OPTIONS_ACCOUNTS_OWNER_ONLY);
  AddString("ownerUserId",
            UTF8ToUTF16(chromeos::UserCrosSettingsProvider::cached_owner()));
#endif
}

void FlagsUIHTMLSource::StartDataRequest(const std::string& path,
                                        bool is_incognito,
                                        int request_id) {
  if (path == kStringsJs) {
    SendLocalizedStringsAsJSON(request_id);
  } else {
    int idr;
    if (path == kFlagsJs)
      idr = IDR_FLAGS_JS;
#if defined (OS_CHROMEOS)
    else if (!chromeos::UserManager::Get()->current_user_is_owner())
      idr = IDR_FLAGS_HTML_WARNING;
#endif
    else
      idr = IDR_FLAGS_HTML;

    SendFromResourceBundle(request_id, idr);
  }
}

std::string FlagsUIHTMLSource::GetMimeType(const std::string& path) const {
  if (path == kStringsJs || path == kFlagsJs)
    return "application/javascript";

  return "text/html";
}

namespace {

////////////////////////////////////////////////////////////////////////////////
//
// FlagsDOMHandler
//
////////////////////////////////////////////////////////////////////////////////

// The handler for Javascript messages for the about:flags page.
class FlagsDOMHandler : public WebUIMessageHandler {
 public:
  FlagsDOMHandler() {}
  virtual ~FlagsDOMHandler() {}

  // WebUIMessageHandler implementation.
  virtual void RegisterMessages();

  // Callback for the "requestFlagsExperiments" message.
  void HandleRequestFlagsExperiments(const ListValue* args);

  // Callback for the "enableFlagsExperiment" message.
  void HandleEnableFlagsExperimentMessage(const ListValue* args);

  // Callback for the "restartBrowser" message. Restores all tabs on restart.
  void HandleRestartBrowser(const ListValue* args);

 private:
  DISALLOW_COPY_AND_ASSIGN(FlagsDOMHandler);
};

void FlagsDOMHandler::RegisterMessages() {
  web_ui_->RegisterMessageCallback("requestFlagsExperiments",
      NewCallback(this, &FlagsDOMHandler::HandleRequestFlagsExperiments));
  web_ui_->RegisterMessageCallback("enableFlagsExperiment",
      NewCallback(this, &FlagsDOMHandler::HandleEnableFlagsExperimentMessage));
  web_ui_->RegisterMessageCallback("restartBrowser",
      NewCallback(this, &FlagsDOMHandler::HandleRestartBrowser));
}

void FlagsDOMHandler::HandleRequestFlagsExperiments(const ListValue* args) {
  DictionaryValue results;
  results.Set("flagsExperiments",
              about_flags::GetFlagsExperimentsData(
                  g_browser_process->local_state()));
  results.SetBoolean("needsRestart",
                     about_flags::IsRestartNeededToCommitChanges());
  web_ui_->CallJavascriptFunction("returnFlagsExperiments", results);
}

void FlagsDOMHandler::HandleEnableFlagsExperimentMessage(
    const ListValue* args) {
  DCHECK_EQ(2u, args->GetSize());
  if (args->GetSize() != 2)
    return;

  std::string experiment_internal_name;
  std::string enable_str;
  if (!args->GetString(0, &experiment_internal_name) ||
      !args->GetString(1, &enable_str))
    return;

  about_flags::SetExperimentEnabled(
      g_browser_process->local_state(),
      experiment_internal_name,
      enable_str == "true");
}

void FlagsDOMHandler::HandleRestartBrowser(const ListValue* args) {
#if !defined(OS_CHROMEOS)
  // Set the flag to restore state after the restart.
  PrefService* pref_service = g_browser_process->local_state();
  pref_service->SetBoolean(prefs::kRestartLastSessionOnShutdown, true);
  BrowserList::CloseAllBrowsersAndExit();
#else
  // For CrOS instead of browser restart (which is not supported) perform a full
  // sign out. Session will be only restored is user has that setting set.
  // Same session restore behavior happens in case of full restart after update.
  BrowserList::GetLastActive()->Exit();
#endif
}

}  // namespace

///////////////////////////////////////////////////////////////////////////////
//
// FlagsUI
//
///////////////////////////////////////////////////////////////////////////////

FlagsUI::FlagsUI(TabContents* contents) : ChromeWebUI(contents) {
  AddMessageHandler((new FlagsDOMHandler())->Attach(this));

  FlagsUIHTMLSource* html_source = new FlagsUIHTMLSource();

  // Set up the about:flags source.
  contents->profile()->GetChromeURLDataManager()->AddDataSource(html_source);
}

// static
RefCountedMemory* FlagsUI::GetFaviconResourceBytes() {
  return ResourceBundle::GetSharedInstance().
      LoadDataResourceBytes(IDR_FLAGS);
}

// static
void FlagsUI::RegisterPrefs(PrefService* prefs) {
  prefs->RegisterListPref(prefs::kEnabledLabsExperiments);
}
