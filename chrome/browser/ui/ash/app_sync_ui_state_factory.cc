// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/ui/ash/app_sync_ui_state_factory.h"

#include "chrome/browser/profiles/profile.h"
#include "chrome/browser/sync/profile_sync_service_factory.h"
#include "chrome/browser/ui/ash/app_sync_ui_state.h"
#include "components/browser_context_keyed_service/browser_context_dependency_manager.h"

// static
AppSyncUIState* AppSyncUIStateFactory::GetForProfile(Profile* profile) {
  if (!AppSyncUIState::ShouldObserveAppSyncForProfile(profile))
    return NULL;

  return static_cast<AppSyncUIState*>(
      GetInstance()->GetServiceForBrowserContext(profile, true));
}

// static
AppSyncUIStateFactory* AppSyncUIStateFactory::GetInstance() {
  return Singleton<AppSyncUIStateFactory>::get();
}

AppSyncUIStateFactory::AppSyncUIStateFactory()
    : BrowserContextKeyedServiceFactory(
        "AppSyncUIState",
        BrowserContextDependencyManager::GetInstance()) {
  DependsOn(ProfileSyncServiceFactory::GetInstance());
}

AppSyncUIStateFactory::~AppSyncUIStateFactory() {
}

BrowserContextKeyedService* AppSyncUIStateFactory::BuildServiceInstanceFor(
    content::BrowserContext* context) const {
  Profile* profile = static_cast<Profile*>(context);
  DCHECK(AppSyncUIState::ShouldObserveAppSyncForProfile(profile));
  return new AppSyncUIState(profile);
}
