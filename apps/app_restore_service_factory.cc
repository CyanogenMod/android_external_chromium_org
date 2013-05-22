// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "apps/app_restore_service_factory.h"

#include "apps/app_restore_service.h"
#include "chrome/browser/extensions/shell_window_registry.h"
#include "chrome/browser/profiles/profile.h"
#include "components/browser_context_keyed_service/browser_context_dependency_manager.h"

namespace apps {

// static
AppRestoreService* AppRestoreServiceFactory::GetForProfile(Profile* profile) {
  return static_cast<AppRestoreService*>(
      GetInstance()->GetServiceForBrowserContext(profile, true));
}

// static
void AppRestoreServiceFactory::ResetForProfile(Profile* profile) {
  AppRestoreServiceFactory* factory = GetInstance();
  factory->BrowserContextShutdown(profile);
  factory->BrowserContextDestroyed(profile);
}

AppRestoreServiceFactory* AppRestoreServiceFactory::GetInstance() {
  return Singleton<AppRestoreServiceFactory>::get();
}

AppRestoreServiceFactory::AppRestoreServiceFactory()
    : BrowserContextKeyedServiceFactory(
        "AppRestoreService",
        BrowserContextDependencyManager::GetInstance()) {
  DependsOn(extensions::ShellWindowRegistry::Factory::GetInstance());
}

AppRestoreServiceFactory::~AppRestoreServiceFactory() {
}

BrowserContextKeyedService* AppRestoreServiceFactory::BuildServiceInstanceFor(
    content::BrowserContext* profile) const {
  return new AppRestoreService(static_cast<Profile*>(profile));
}

bool AppRestoreServiceFactory::ServiceIsCreatedWithBrowserContext() const {
  return true;
}

}  // namespace apps
