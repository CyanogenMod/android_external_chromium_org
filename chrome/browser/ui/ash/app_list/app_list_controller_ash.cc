// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/ui/ash/app_list/app_list_controller_ash.h"

#include "ash/shell.h"
#include "chrome/browser/extensions/extension_service.h"
#include "chrome/browser/profiles/profile.h"
#include "chrome/browser/ui/ash/extension_utils.h"
#include "chrome/browser/ui/ash/launcher/chrome_launcher_controller.h"
#include "chrome/common/extensions/extension.h"

AppListControllerAsh::AppListControllerAsh() {}

AppListControllerAsh::~AppListControllerAsh() {}

void AppListControllerAsh::CloseView() {
  DCHECK(ash::Shell::HasInstance());
  if (ash::Shell::GetInstance()->GetAppListTargetVisibility())
    ash::Shell::GetInstance()->ToggleAppList();
}

bool AppListControllerAsh::IsAppPinned(const std::string& extension_id) {
  return ChromeLauncherController::instance()->IsAppPinned(extension_id);
}

void AppListControllerAsh::PinApp(const std::string& extension_id) {
  ChromeLauncherController::instance()->PinAppWithID(extension_id);
}

void AppListControllerAsh::UnpinApp(const std::string& extension_id) {
  ChromeLauncherController::instance()->UnpinAppsWithID(extension_id);
}

bool AppListControllerAsh::CanPin() {
  return ChromeLauncherController::instance()->CanPin();
}

void AppListControllerAsh::ActivateApp(Profile* profile,
                                       const std::string& extension_id,
                                       int event_flags) {
  ChromeLauncherController::instance()->OpenAppID(extension_id,
                                                  event_flags);
}

void AppListControllerAsh::OpenApp(
    Profile* profile,
    const std::string& extension_id,
    int event_flags) {
  ExtensionService* service = profile->GetExtensionService();
  const extensions::Extension* extension = service->extensions()->GetByID(
      extension_id);
  DCHECK(extension);
  extension_utils::OpenExtension(profile, extension, event_flags);
}
