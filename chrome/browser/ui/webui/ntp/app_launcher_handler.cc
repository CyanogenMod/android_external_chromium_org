// Copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/ui/webui/ntp/app_launcher_handler.h"

#include <string>
#include <vector>

#include "base/auto_reset.h"
#include "base/callback.h"
#include "base/command_line.h"
#include "base/metrics/histogram.h"
#include "base/string_number_conversions.h"
#include "base/string_split.h"
#include "base/string_util.h"
#include "base/utf_string_conversions.h"
#include "base/values.h"
#include "chrome/browser/extensions/apps_promo.h"
#include "chrome/browser/extensions/crx_installer.h"
#include "chrome/browser/extensions/extension_prefs.h"
#include "chrome/browser/extensions/extension_service.h"
#include "chrome/browser/platform_util.h"
#include "chrome/browser/prefs/pref_service.h"
#include "chrome/browser/prefs/scoped_user_pref_update.h"
#include "chrome/browser/profiles/profile.h"
#include "chrome/browser/ui/browser.h"
#include "chrome/browser/ui/browser_list.h"
#include "chrome/browser/ui/browser_window.h"
#include "chrome/browser/ui/webui/extension_icon_source.h"
#include "chrome/browser/ui/webui/ntp/shown_sections_handler.h"
#include "chrome/common/chrome_notification_types.h"
#include "chrome/common/chrome_switches.h"
#include "chrome/common/extensions/extension.h"
#include "chrome/common/extensions/extension_constants.h"
#include "chrome/common/extensions/extension_icon_set.h"
#include "chrome/common/extensions/extension_resource.h"
#include "chrome/common/favicon_url.h"
#include "chrome/common/pref_names.h"
#include "chrome/common/url_constants.h"
#include "chrome/common/web_apps.h"
#include "content/browser/disposition_utils.h"
#include "content/browser/tab_contents/tab_contents.h"
#include "content/common/notification_service.h"
#include "googleurl/src/gurl.h"
#include "grit/browser_resources.h"
#include "grit/generated_resources.h"
#include "net/base/escape.h"
#include "ui/base/animation/animation.h"
#include "ui/gfx/codec/png_codec.h"
#include "webkit/glue/window_open_disposition.h"

namespace {

// The URL prefixes used by the NTP to signal when the web store or an app
// has launched so we can record the proper histogram.
const char* kPingLaunchAppByID = "record-app-launch-by-id";
const char* kPingLaunchWebStore = "record-webstore-launch";
const char* kPingLaunchAppByURL = "record-app-launch-by-url";

const UnescapeRule::Type kUnescapeRules =
    UnescapeRule::NORMAL | UnescapeRule::URL_SPECIAL_CHARS;

extension_misc::AppLaunchBucket ParseLaunchSource(
    const std::string& launch_source) {
  int bucket_num = extension_misc::APP_LAUNCH_BUCKET_INVALID;
  base::StringToInt(launch_source, &bucket_num);
  extension_misc::AppLaunchBucket bucket =
      static_cast<extension_misc::AppLaunchBucket>(bucket_num);
  CHECK(bucket < extension_misc::APP_LAUNCH_BUCKET_BOUNDARY);
  return bucket;
}

}  // namespace

AppLauncherHandler::AppLauncherHandler(ExtensionService* extension_service)
    : extension_service_(extension_service),
      promo_active_(false),
      ignore_changes_(false) {
}

AppLauncherHandler::~AppLauncherHandler() {}

// Serializes |notification| into a new DictionaryValue which the caller then
// owns.
static DictionaryValue* SerializeNotification(
    const AppNotification& notification) {
  DictionaryValue* dictionary = new DictionaryValue();
  dictionary->SetString("title", notification.title);
  dictionary->SetString("body", notification.body);
  if (!notification.linkUrl.is_empty()) {
    dictionary->SetString("linkUrl", notification.linkUrl.spec());
    dictionary->SetString("linkText", notification.linkText);
  }
  return dictionary;
}

// static
DictionaryValue* AppLauncherHandler::CreateAppInfo(
    const Extension* extension,
    const AppNotification* notification,
    ExtensionService* service) {
  // Don't include the WebStore and the Cloud Print app.
  // The WebStore launcher gets special treatment in ntp/apps.js.
  // The Cloud Print app should never be displayed in the NTP.
  bool ntp3 =
      !CommandLine::ForCurrentProcess()->HasSwitch(switches::kNewTabPage4);
  if (!extension->is_app() ||
      (ntp3 && extension->id() == extension_misc::kWebStoreAppId) ||
      (extension->id() == extension_misc::kCloudPrintAppId)) {
    return NULL;
  }

  bool enabled = service->IsExtensionEnabled(extension->id()) &&
      !service->GetTerminatedExtension(extension->id());
  GURL icon_big =
      ExtensionIconSource::GetIconURL(extension,
                                      Extension::EXTENSION_ICON_LARGE,
                                      ExtensionIconSet::MATCH_EXACTLY,
                                      !enabled);
  GURL icon_small =
      ExtensionIconSource::GetIconURL(extension,
                                      Extension::EXTENSION_ICON_BITTY,
                                      ExtensionIconSet::MATCH_BIGGER,
                                      !enabled);

  DictionaryValue* value = new DictionaryValue();
  value->Clear();
  value->SetString("id", extension->id());
  value->SetString("name", extension->name());
  value->SetString("description", extension->description());
  value->SetString("launch_url", extension->GetFullLaunchURL().spec());
  if (enabled)
    value->SetString("options_url", extension->options_url().spec());
  value->SetBoolean("can_uninstall",
                    Extension::UserMayDisable(extension->location()));
  value->SetString("icon_big", icon_big.spec());
  value->SetString("icon_small", icon_small.spec());
  value->SetInteger("launch_container", extension->launch_container());
  ExtensionPrefs* prefs = service->extension_prefs();
  value->SetInteger("launch_type",
      prefs->GetLaunchType(extension->id(),
                           ExtensionPrefs::LAUNCH_DEFAULT));
  value->SetBoolean("is_component",
      extension->location() == Extension::COMPONENT);

  if (notification)
    value->Set("notification", SerializeNotification(*notification));

  int app_launch_index = prefs->GetAppLaunchIndex(extension->id());
  if (app_launch_index == -1) {
    // Make sure every app has a launch index (some predate the launch index).
    app_launch_index = prefs->GetNextAppLaunchIndex();
    prefs->SetAppLaunchIndex(extension->id(), app_launch_index);
  }
  value->SetInteger("app_launch_index", app_launch_index);

  int page_index = prefs->GetPageIndex(extension->id());
  if (page_index >= 0) {
    // Only provide a value if one is stored
    value->SetInteger("page_index", page_index);
  }
  return value;
}

// static
bool AppLauncherHandler::HandlePing(Profile* profile, const std::string& path) {
  std::vector<std::string> params;
  base::SplitString(path, '+', &params);

  // Check if the user launched an app from the most visited or recently
  // closed sections.
  if (kPingLaunchAppByURL == params.at(0)) {
    CHECK(params.size() == 3);
    RecordAppLaunchByURL(
        profile, params.at(1), ParseLaunchSource(params.at(2)));
    return true;
  }

  bool is_web_store_ping = kPingLaunchWebStore == params.at(0);
  bool is_app_launch_ping = kPingLaunchAppByID == params.at(0);

  if (!is_web_store_ping && !is_app_launch_ping)
    return false;

  CHECK(params.size() >= 2);

  bool is_promo_active = params.at(1) == "true";

  // At this point, the user must have used the app launcher, so we hide the
  // promo if its still displayed.
  if (is_promo_active) {
    DCHECK(profile->GetExtensionService());
    profile->GetExtensionService()->apps_promo()->ExpireDefaultApps();
  }

  if (is_web_store_ping) {
    RecordWebStoreLaunch(is_promo_active);
  }  else {
    CHECK(params.size() == 3);
    RecordAppLaunchByID(is_promo_active, ParseLaunchSource(params.at(2)));
  }

  return true;
}

WebUIMessageHandler* AppLauncherHandler::Attach(WebUI* web_ui) {
  // TODO(arv): Add initialization code to the Apps store etc.
  return WebUIMessageHandler::Attach(web_ui);
}

void AppLauncherHandler::RegisterMessages() {
  web_ui_->RegisterMessageCallback("getApps",
      NewCallback(this, &AppLauncherHandler::HandleGetApps));
  web_ui_->RegisterMessageCallback("launchApp",
      NewCallback(this, &AppLauncherHandler::HandleLaunchApp));
  web_ui_->RegisterMessageCallback("setLaunchType",
      NewCallback(this, &AppLauncherHandler::HandleSetLaunchType));
  web_ui_->RegisterMessageCallback("uninstallApp",
      NewCallback(this, &AppLauncherHandler::HandleUninstallApp));
  web_ui_->RegisterMessageCallback("hideAppsPromo",
      NewCallback(this, &AppLauncherHandler::HandleHideAppsPromo));
  web_ui_->RegisterMessageCallback("createAppShortcut",
      NewCallback(this, &AppLauncherHandler::HandleCreateAppShortcut));
  web_ui_->RegisterMessageCallback("reorderApps",
      NewCallback(this, &AppLauncherHandler::HandleReorderApps));
  web_ui_->RegisterMessageCallback("setPageIndex",
      NewCallback(this, &AppLauncherHandler::HandleSetPageIndex));
  web_ui_->RegisterMessageCallback("promoSeen",
      NewCallback(this, &AppLauncherHandler::HandlePromoSeen));
  web_ui_->RegisterMessageCallback("saveAppPageName",
      NewCallback(this, &AppLauncherHandler::HandleSaveAppPageName));
  web_ui_->RegisterMessageCallback("generateAppForLink",
      NewCallback(this, &AppLauncherHandler::HandleGenerateAppForLink));
}

void AppLauncherHandler::Observe(int type,
                                 const NotificationSource& source,
                                 const NotificationDetails& details) {
  if (ignore_changes_)
    return;

  switch (type) {
    case chrome::NOTIFICATION_APP_NOTIFICATION_STATE_CHANGED: {
      const std::string& id = *Details<const std::string>(details).ptr();
      const AppNotification* notification =
          extension_service_->app_notification_manager()->GetLast(id);
      ListValue args;
      args.Append(new StringValue(id));
      if (notification)
        args.Append(SerializeNotification(*notification));
      web_ui_->CallJavascriptFunction("appNotificationChanged", args);
      break;
    }
    case chrome::NOTIFICATION_EXTENSION_LOADED:
    case chrome::NOTIFICATION_EXTENSION_UNLOADED: {
      const Extension* extension =
          type == chrome::NOTIFICATION_EXTENSION_LOADED ?
              Details<const Extension>(details).ptr() :
              Details<UnloadedExtensionInfo>(details)->extension;
      if (!extension->is_app())
        break;

      if (CommandLine::ForCurrentProcess()->HasSwitch(switches::kNewTabPage4)) {
        scoped_ptr<DictionaryValue> app_info(GetAppInfo(extension));
        if (app_info.get()) {
          std::string function =
              type == chrome::NOTIFICATION_EXTENSION_LOADED ?
                  "ntp4.appAdded" : "ntp4.appRemoved";
          web_ui_->CallJavascriptFunction(function, *app_info);
        }
      } else if (web_ui_->tab_contents()) {
        HandleGetApps(NULL);
      }

      break;
    }
    case chrome::NOTIFICATION_EXTENSION_LAUNCHER_REORDERED:
    // The promo may not load until a couple seconds after the first NTP view,
    // so we listen for the load notification and notify the NTP when ready.
    case chrome::NOTIFICATION_WEB_STORE_PROMO_LOADED:
      if (web_ui_->tab_contents())
        HandleGetApps(NULL);
      break;
    case chrome::NOTIFICATION_PREF_CHANGED: {
      if (!web_ui_->tab_contents())
        break;
      // Handle app page renames.
      std::string* pref_name = Details<std::string>(details).ptr();
      if (*pref_name == prefs::kNTPAppPageNames) {
        // TODO(estade): this doesn't need to regenerate the entire page.
        HandleGetApps(NULL);
      } else {
        // Default prefs change handling.
        DictionaryValue dictionary;
        FillAppDictionary(&dictionary);
        web_ui_->CallJavascriptFunction("appsPrefChangeCallback", dictionary);
      }
      break;
    }
    default:
      NOTREACHED();
  }
}

void AppLauncherHandler::FillAppDictionary(DictionaryValue* dictionary) {
  ListValue* list = new ListValue();
  const ExtensionList* extensions = extension_service_->extensions();
  ExtensionList::const_iterator it;
  for (it = extensions->begin(); it != extensions->end(); ++it) {
    DictionaryValue* app_info = GetAppInfo(*it);
    if (app_info)
      list->Append(app_info);
  }

  extensions = extension_service_->disabled_extensions();
  for (it = extensions->begin(); it != extensions->end(); ++it) {
    DictionaryValue* app_info = CreateAppInfo(*it,
                                              NULL,
                                              extension_service_);
    if (app_info)
      list->Append(app_info);
  }

  extensions = extension_service_->terminated_extensions();
  for (it = extensions->begin(); it != extensions->end(); ++it) {
    DictionaryValue* app_info = CreateAppInfo(*it,
                                              NULL,
                                              extension_service_);
    if (app_info)
      list->Append(app_info);
  }

  dictionary->Set("apps", list);

  // TODO(estade): remove these settings when the old NTP is removed. The new
  // NTP does it in js.
#if defined(OS_MACOSX)
  // App windows are not yet implemented on mac.
  dictionary->SetBoolean("disableAppWindowLaunch", true);
  dictionary->SetBoolean("disableCreateAppShortcut", true);
#endif

#if defined(OS_CHROMEOS)
  // Making shortcut does not make sense on ChromeOS because it does not have
  // a desktop.
  dictionary->SetBoolean("disableCreateAppShortcut", true);
#endif

  dictionary->SetBoolean(
      "showLauncher",
      extension_service_->apps_promo()->ShouldShowAppLauncher(
          extension_service_->GetAppIds()));

  if (CommandLine::ForCurrentProcess()->HasSwitch(switches::kNewTabPage4)) {
    PrefService* prefs = web_ui_->GetProfile()->GetPrefs();
    const ListValue* app_page_names = prefs->GetList(prefs::kNTPAppPageNames);
    if (app_page_names && app_page_names->GetSize()) {
      dictionary->Set("appPageNames",
                      static_cast<ListValue*>(app_page_names->DeepCopy()));
    }
  }
}

DictionaryValue* AppLauncherHandler::GetAppInfo(const Extension* extension) {
  AppNotificationManager* notification_manager =
      extension_service_->app_notification_manager();
  return CreateAppInfo(extension,
                       notification_manager->GetLast(extension->id()),
                       extension_service_);
}

void AppLauncherHandler::FillPromoDictionary(DictionaryValue* dictionary) {
  dictionary->SetString("promoHeader", AppsPromo::GetPromoHeaderText());
  dictionary->SetString("promoButton", AppsPromo::GetPromoButtonText());
  dictionary->SetString("promoLink", AppsPromo::GetPromoLink().spec());
  dictionary->SetString("promoLogo", AppsPromo::GetPromoLogo().spec());
  dictionary->SetString("promoExpire", AppsPromo::GetPromoExpireText());
}

void AppLauncherHandler::HandleGetApps(const ListValue* args) {
  DictionaryValue dictionary;

  // Tell the client whether to show the promo for this view. We don't do this
  // in the case of PREF_CHANGED because:
  //
  // a) At that point in time, depending on the pref that changed, it can look
  //    like the set of apps installed has changed, and we will mark the promo
  //    expired.
  // b) Conceptually, it doesn't really make sense to count a
  //    prefchange-triggered refresh as a promo 'view'.
  AppsPromo* apps_promo = extension_service_->apps_promo();
  PrefService* prefs = web_ui_->GetProfile()->GetPrefs();
  bool apps_promo_just_expired = false;
  if (apps_promo->ShouldShowPromo(extension_service_->GetAppIds(),
                                  &apps_promo_just_expired)) {
    apps_promo->MaximizeAppsIfNecessary();
    dictionary.SetBoolean("showPromo", true);
    FillPromoDictionary(&dictionary);
    promo_active_ = true;
  } else {
    dictionary.SetBoolean("showPromo", false);
    promo_active_ = false;
  }

  // If the default apps have just expired (user viewed them too many times with
  // no interaction), then we uninstall them and focus the recent sites section.
  if (apps_promo_just_expired) {
    ignore_changes_ = true;
    UninstallDefaultApps();
    ignore_changes_ = false;
    ShownSectionsHandler::SetShownSection(prefs, THUMB);
  }

  FillAppDictionary(&dictionary);
  web_ui_->CallJavascriptFunction("getAppsCallback", dictionary);

  // First time we get here we set up the observer so that we can tell update
  // the apps as they change.
  if (registrar_.IsEmpty()) {
    registrar_.Add(this, chrome::NOTIFICATION_APP_NOTIFICATION_STATE_CHANGED,
        NotificationService::AllSources());
    registrar_.Add(this, chrome::NOTIFICATION_EXTENSION_LOADED,
        NotificationService::AllSources());
    registrar_.Add(this, chrome::NOTIFICATION_EXTENSION_UNLOADED,
        NotificationService::AllSources());
    registrar_.Add(this, chrome::NOTIFICATION_EXTENSION_LAUNCHER_REORDERED,
        NotificationService::AllSources());
    registrar_.Add(this, chrome::NOTIFICATION_WEB_STORE_PROMO_LOADED,
        NotificationService::AllSources());
  }
  if (pref_change_registrar_.IsEmpty()) {
    pref_change_registrar_.Init(
        extension_service_->extension_prefs()->pref_service());
    pref_change_registrar_.Add(ExtensionPrefs::kExtensionsPref, this);
    pref_change_registrar_.Add(prefs::kNTPAppPageNames, this);
  }
}

void AppLauncherHandler::HandleLaunchApp(const ListValue* args) {
  std::string extension_id;
  double source = -1.0;
  bool alt_key = false;
  bool ctrl_key = false;
  bool meta_key = false;
  bool shift_key = false;
  double button = 0.0;

  CHECK(args->GetString(0, &extension_id));
  CHECK(args->GetDouble(1, &source));
  if (args->GetSize() > 2) {
    CHECK(args->GetBoolean(2, &alt_key));
    CHECK(args->GetBoolean(3, &ctrl_key));
    CHECK(args->GetBoolean(4, &meta_key));
    CHECK(args->GetBoolean(5, &shift_key));
    CHECK(args->GetDouble(6, &button));
  }

  extension_misc::AppLaunchBucket launch_bucket =
      static_cast<extension_misc::AppLaunchBucket>(
          static_cast<int>(source));
  CHECK(launch_bucket >= 0 &&
        launch_bucket < extension_misc::APP_LAUNCH_BUCKET_BOUNDARY);

  const Extension* extension =
      extension_service_->GetExtensionById(extension_id, false);

  // Prompt the user to re-enable the application if disabled.
  if (!extension) {
    PromptToEnableApp(extension_id);
    return;
  }

  Profile* profile = extension_service_->profile();

  // If the user pressed special keys when clicking, override the saved
  // preference for launch container.
  bool middle_button = (button == 1.0);
  WindowOpenDisposition disposition =
        disposition_utils::DispositionFromClick(middle_button, alt_key,
                                                ctrl_key, meta_key, shift_key);

  if (extension_id != extension_misc::kWebStoreAppId) {
    RecordAppLaunchByID(promo_active_, launch_bucket);
    extension_service_->apps_promo()->ExpireDefaultApps();
  }

  if (disposition == NEW_FOREGROUND_TAB || disposition == NEW_BACKGROUND_TAB) {
    // TODO(jamescook): Proper support for background tabs.
    Browser::OpenApplication(
        profile, extension, extension_misc::LAUNCH_TAB, disposition);
  } else if (disposition == NEW_WINDOW) {
    // Force a new window open.
    Browser::OpenApplication(
            profile, extension, extension_misc::LAUNCH_WINDOW, disposition);
  } else {
    // Look at preference to find the right launch container.  If no preference
    // is set, launch as a regular tab.
    extension_misc::LaunchContainer launch_container =
        extension_service_->extension_prefs()->GetLaunchContainer(
            extension, ExtensionPrefs::LAUNCH_REGULAR);

    // To give a more "launchy" experience when using the NTP launcher, we close
    // it automatically.
    Browser* browser = BrowserList::GetLastActive();
    TabContents* old_contents = NULL;
    if (browser)
      old_contents = browser->GetSelectedTabContents();

    TabContents* new_contents = Browser::OpenApplication(
        profile, extension, launch_container,
        old_contents ? CURRENT_TAB : NEW_FOREGROUND_TAB);

    // This will also destroy the handler, so do not perform any actions after.
    if (new_contents != old_contents && browser && browser->tab_count() > 1)
      browser->CloseTabContents(old_contents);
  }
}

void AppLauncherHandler::HandleSetLaunchType(const ListValue* args) {
  std::string extension_id;
  double launch_type;
  CHECK(args->GetString(0, &extension_id));
  CHECK(args->GetDouble(1, &launch_type));

  const Extension* extension =
      extension_service_->GetExtensionById(extension_id, true);
  CHECK(extension);

  // Don't update the page; it already knows about the launch type change.
  scoped_ptr<AutoReset<bool> > auto_reset;
  if (CommandLine::ForCurrentProcess()->HasSwitch(switches::kNewTabPage4))
    auto_reset.reset(new AutoReset<bool>(&ignore_changes_, true));

  extension_service_->extension_prefs()->SetLaunchType(
      extension_id,
      static_cast<ExtensionPrefs::LaunchType>(
          static_cast<int>(launch_type)));
}

void AppLauncherHandler::HandleUninstallApp(const ListValue* args) {
  std::string extension_id;
  CHECK(args->GetString(0, &extension_id));

  const Extension* extension = extension_service_->GetExtensionById(
      extension_id, true);
  if (!extension)
    return;

  if (!Extension::UserMayDisable(extension->location())) {
    LOG(ERROR) << "Attempt to uninstall an extension that is non-usermanagable "
               << "was made. Extension id : " << extension->id();
    return;
  }
  if (!extension_id_prompting_.empty())
    return;  // Only one prompt at a time.

  extension_id_prompting_ = extension_id;

  bool dont_confirm = false;
  if (args->GetBoolean(1, &dont_confirm) && dont_confirm) {
    scoped_ptr<AutoReset<bool> > auto_reset;
    if (CommandLine::ForCurrentProcess()->HasSwitch(switches::kNewTabPage4))
      auto_reset.reset(new AutoReset<bool>(&ignore_changes_, true));
    ExtensionDialogAccepted();
  } else {
    GetExtensionUninstallDialog()->ConfirmUninstall(this, extension);
  }
}

void AppLauncherHandler::HandleHideAppsPromo(const ListValue* args) {
  // If the user has intentionally hidden the promotion, we'll uninstall all the
  // default apps (we know the user hasn't installed any apps on their own at
  // this point, or the promotion wouldn't have been shown).
  ignore_changes_ = true;
  UninstallDefaultApps();
  extension_service_->apps_promo()->HidePromo();
  ignore_changes_ = false;
  HandleGetApps(NULL);
}

void AppLauncherHandler::HandleCreateAppShortcut(const ListValue* args) {
  std::string extension_id;
  if (!args->GetString(0, &extension_id)) {
    NOTREACHED();
    return;
  }

  const Extension* extension =
      extension_service_->GetExtensionById(extension_id, true);
  CHECK(extension);

  Browser* browser = BrowserList::GetLastActive();
  if (!browser)
    return;
  browser->window()->ShowCreateChromeAppShortcutsDialog(
      browser->profile(), extension);
}

void AppLauncherHandler::HandleReorderApps(const ListValue* args) {
  CHECK(args->GetSize() == 2);

  std::string dragged_app_id;
  ListValue* app_order;
  CHECK(args->GetString(0, &dragged_app_id));
  CHECK(args->GetList(1, &app_order));

  std::vector<std::string> extension_ids;
  for (size_t i = 0; i < app_order->GetSize(); ++i) {
    std::string value;
    if (app_order->GetString(i, &value))
      extension_ids.push_back(value);
  }

  // Don't update the page; it already knows the apps have been reordered.
  scoped_ptr<AutoReset<bool> > auto_reset;
  if (CommandLine::ForCurrentProcess()->HasSwitch(switches::kNewTabPage4))
    auto_reset.reset(new AutoReset<bool>(&ignore_changes_, true));

  extension_service_->extension_prefs()->SetAppDraggedByUser(dragged_app_id);
  extension_service_->extension_prefs()->SetAppLauncherOrder(extension_ids);
}

void AppLauncherHandler::HandleSetPageIndex(const ListValue* args) {
  std::string extension_id;
  double page_index;
  CHECK(args->GetString(0, &extension_id));
  CHECK(args->GetDouble(1, &page_index));

  // Don't update the page; it already knows the apps have been reordered.
  scoped_ptr<AutoReset<bool> > auto_reset;
  if (CommandLine::ForCurrentProcess()->HasSwitch(switches::kNewTabPage4))
    auto_reset.reset(new AutoReset<bool>(&ignore_changes_, true));

  extension_service_->extension_prefs()->SetPageIndex(extension_id,
      static_cast<int>(page_index));
}

void AppLauncherHandler::HandlePromoSeen(const ListValue* args) {
  UMA_HISTOGRAM_ENUMERATION(extension_misc::kAppsPromoHistogram,
                            extension_misc::PROMO_SEEN,
                            extension_misc::PROMO_BUCKET_BOUNDARY);
}

void AppLauncherHandler::HandleSaveAppPageName(const ListValue* args) {
  string16 name;
  CHECK(args->GetString(0, &name));

  double page_index;
  CHECK(args->GetDouble(1, &page_index));

  AutoReset<bool> auto_reset(&ignore_changes_, true);
  PrefService* prefs = web_ui_->GetProfile()->GetPrefs();
  ListPrefUpdate update(prefs, prefs::kNTPAppPageNames);
  ListValue* list = update.Get();
  list->Set(static_cast<size_t>(page_index), Value::CreateStringValue(name));
}

void AppLauncherHandler::HandleGenerateAppForLink(const ListValue* args) {
  string16 url;
  CHECK(args->GetString(0, &url));

  string16 title;
  CHECK(args->GetString(1, &title));

  GURL launch_url(url);

  scoped_ptr<WebApplicationInfo> web_app(new WebApplicationInfo);
  web_app->is_bookmark_app = true;
  web_app->title = title;
  web_app->app_url = launch_url;
  WebApplicationInfo::IconInfo icon;
  icon.url = GURL();
  icon.width = icon.height = 16;
  web_app->icons.push_back(icon);

  Profile* profile = web_ui_->GetProfile();
  FaviconService* favicon_service =
      profile->GetFaviconService(Profile::EXPLICIT_ACCESS);
  if (!favicon_service) {
    LOG(ERROR) << "No favicon service";
    scoped_refptr<CrxInstaller> installer(
        extension_service_->MakeCrxInstaller(NULL));
    installer->InstallWebApp(*web_app);
    return;
  }

  // TODO(gbillock): get page thumb from thumbnail db/history svc?
  FaviconService::Handle h = favicon_service->GetFaviconForURL(
      launch_url, history::FAVICON, &favicon_consumer_,
      NewCallback(this, &AppLauncherHandler::OnFaviconForApp));
  favicon_consumer_.SetClientData(favicon_service, h, web_app.release());
}

void AppLauncherHandler::OnFaviconForApp(FaviconService::Handle handle,
                                         history::FaviconData data) {
  scoped_ptr<WebApplicationInfo> web_app(
      favicon_consumer_.GetClientDataForCurrentRequest());
  CHECK(!web_app->icons.empty());
  if (data.is_valid() && gfx::PNGCodec::Decode(data.image_data->front(),
                                               data.image_data->size(),
                                               &(web_app->icons[0].data))) {
    web_app->icons[0].url = GURL();
    web_app->icons[0].width = web_app->icons[0].data.width();
    web_app->icons[0].height = web_app->icons[0].data.height();
  }

  scoped_refptr<CrxInstaller> installer(
      extension_service_->MakeCrxInstaller(NULL));
  installer->InstallWebApp(*web_app);
}

// static
void AppLauncherHandler::RegisterUserPrefs(PrefService* pref_service) {
  // TODO(csilv): We will want this to be a syncable preference instead.
  pref_service->RegisterListPref(prefs::kNTPAppPageNames,
                                 PrefService::UNSYNCABLE_PREF);
}

// static
void AppLauncherHandler::RecordWebStoreLaunch(bool promo_active) {
  UMA_HISTOGRAM_ENUMERATION(extension_misc::kAppLaunchHistogram,
                            extension_misc::APP_LAUNCH_NTP_WEBSTORE,
                            extension_misc::APP_LAUNCH_BUCKET_BOUNDARY);

  if (!promo_active) return;

  UMA_HISTOGRAM_ENUMERATION(extension_misc::kAppsPromoHistogram,
                            extension_misc::PROMO_LAUNCH_WEB_STORE,
                            extension_misc::PROMO_BUCKET_BOUNDARY);
}

// static
void AppLauncherHandler::RecordAppLaunchByID(
    bool promo_active, extension_misc::AppLaunchBucket bucket) {
  CHECK(bucket != extension_misc::APP_LAUNCH_BUCKET_INVALID);

  UMA_HISTOGRAM_ENUMERATION(extension_misc::kAppLaunchHistogram, bucket,
                            extension_misc::APP_LAUNCH_BUCKET_BOUNDARY);

  if (!promo_active) return;

  UMA_HISTOGRAM_ENUMERATION(extension_misc::kAppsPromoHistogram,
                            extension_misc::PROMO_LAUNCH_APP,
                            extension_misc::PROMO_BUCKET_BOUNDARY);
}

// static
void AppLauncherHandler::RecordAppLaunchByURL(
    Profile* profile,
    std::string escaped_url,
    extension_misc::AppLaunchBucket bucket) {
  CHECK(bucket != extension_misc::APP_LAUNCH_BUCKET_INVALID);

  GURL url(UnescapeURLComponent(escaped_url, kUnescapeRules));
  DCHECK(profile->GetExtensionService());
  if (!profile->GetExtensionService()->IsInstalledApp(url))
    return;

  UMA_HISTOGRAM_ENUMERATION(extension_misc::kAppLaunchHistogram, bucket,
                            extension_misc::APP_LAUNCH_BUCKET_BOUNDARY);
}

void AppLauncherHandler::PromptToEnableApp(const std::string& extension_id) {
  const Extension* extension =
      extension_service_->GetExtensionById(extension_id, true);
  if (!extension) {
    extension = extension_service_->GetTerminatedExtension(extension_id);
    CHECK(extension);
    // If the app was terminated, reload it first. (This reallocates the
    // Extension object.)
    extension_service_->ReloadExtension(extension_id);
    extension = extension_service_->GetExtensionById(extension_id, true);
  }

  ExtensionPrefs* extension_prefs = extension_service_->extension_prefs();
  if (!extension_prefs->DidExtensionEscalatePermissions(extension_id)) {
    // Enable the extension immediately if its privileges weren't escalated.
    // This is a no-op if the extension was previously terminated.
    extension_service_->EnableExtension(extension_id);

    // Launch app asynchronously so the image will update.
    StringValue* app_id = Value::CreateStringValue(extension_id);
    web_ui_->CallJavascriptFunction("launchAppAfterEnable", *app_id);
    return;
  }

  if (!extension_id_prompting_.empty())
    return;  // Only one prompt at a time.

  extension_id_prompting_ = extension_id;
  GetExtensionInstallUI()->ConfirmReEnable(this, extension);
}

void AppLauncherHandler::ExtensionDialogAccepted() {
  // Do the uninstall work here.
  DCHECK(!extension_id_prompting_.empty());

  // The extension can be uninstalled in another window while the UI was
  // showing. Do nothing in that case.
  const Extension* extension =
      extension_service_->GetExtensionById(extension_id_prompting_, true);
  if (!extension)
    return;

  extension_service_->UninstallExtension(extension_id_prompting_,
                                         false /* external_uninstall */, NULL);

  extension_id_prompting_ = "";
}

void AppLauncherHandler::ExtensionDialogCanceled() {
  extension_id_prompting_ = "";
}

void AppLauncherHandler::InstallUIProceed() {
  // Do the re-enable work here.
  DCHECK(!extension_id_prompting_.empty());

  // The extension can be uninstalled in another window while the UI was
  // showing. Do nothing in that case.
  const Extension* extension =
      extension_service_->GetExtensionById(extension_id_prompting_, true);
  if (!extension)
    return;

  extension_service_->GrantPermissionsAndEnableExtension(extension);

  // We bounce this off the NTP so the browser can update the apps icon.
  // If we don't launch the app asynchronously, then the app's disabled
  // icon disappears but isn't replaced by the enabled icon, making a poor
  // visual experience.
  StringValue* app_id = Value::CreateStringValue(extension->id());
  web_ui_->CallJavascriptFunction("launchAppAfterEnable", *app_id);

  extension_id_prompting_ = "";
}

void AppLauncherHandler::InstallUIAbort(bool user_initiated) {
  // We record the histograms here because ExtensionDialogCanceled is also
  // called when the extension uninstall dialog is canceled.
  const Extension* extension =
      extension_service_->GetExtensionById(extension_id_prompting_, true);
  std::string histogram_name = user_initiated ?
      "Extensions.Permissions_ReEnableCancel" :
      "Extensions.Permissions_ReEnableAbort";
  ExtensionService::RecordPermissionMessagesHistogram(
      extension, histogram_name.c_str());

  ExtensionDialogCanceled();
}

ExtensionUninstallDialog* AppLauncherHandler::GetExtensionUninstallDialog() {
  if (!extension_uninstall_dialog_.get()) {
    extension_uninstall_dialog_.reset(
        new ExtensionUninstallDialog(web_ui_->GetProfile()));
  }
  return extension_uninstall_dialog_.get();
}

ExtensionInstallUI* AppLauncherHandler::GetExtensionInstallUI() {
  if (!extension_install_ui_.get()) {
    extension_install_ui_.reset(
        new ExtensionInstallUI(web_ui_->GetProfile()));
  }
  return extension_install_ui_.get();
}

void AppLauncherHandler::UninstallDefaultApps() {
  AppsPromo* apps_promo = extension_service_->apps_promo();
  const ExtensionIdSet& app_ids = apps_promo->old_default_apps();
  for (ExtensionIdSet::const_iterator iter = app_ids.begin();
       iter != app_ids.end(); ++iter) {
    if (extension_service_->GetExtensionById(*iter, true))
      extension_service_->UninstallExtension(*iter, false, NULL);
  }
}
