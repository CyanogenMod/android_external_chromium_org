// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/prerender/prerender_link_manager_factory.h"

#include "chrome/browser/prerender/prerender_link_manager.h"
#include "chrome/browser/prerender/prerender_manager.h"
#include "chrome/browser/prerender/prerender_manager_factory.h"
#include "chrome/browser/profiles/incognito_helpers.h"
#include "chrome/browser/profiles/profile.h"
#include "components/browser_context_keyed_service/browser_context_dependency_manager.h"

namespace prerender {

// static
PrerenderLinkManager* PrerenderLinkManagerFactory::GetForProfile(
    Profile* profile) {
  return static_cast<PrerenderLinkManager*>(
      GetInstance()->GetServiceForBrowserContext(profile, true));
}

// static
PrerenderLinkManagerFactory* PrerenderLinkManagerFactory::GetInstance() {
  return Singleton<PrerenderLinkManagerFactory>::get();
}

PrerenderLinkManagerFactory::PrerenderLinkManagerFactory()
    : BrowserContextKeyedServiceFactory(
        "PrerenderLinkmanager",
        BrowserContextDependencyManager::GetInstance()) {
  DependsOn(prerender::PrerenderManagerFactory::GetInstance());
}

BrowserContextKeyedService*
PrerenderLinkManagerFactory::BuildServiceInstanceFor(
    content::BrowserContext* profile) const {
  PrerenderManager* prerender_manager =
      PrerenderManagerFactory::GetForProfile(static_cast<Profile*>(profile));
  if (!prerender_manager)
    return NULL;
  PrerenderLinkManager* prerender_link_manager =
      new PrerenderLinkManager(prerender_manager);
  return prerender_link_manager;
}

content::BrowserContext* PrerenderLinkManagerFactory::GetBrowserContextToUse(
    content::BrowserContext* context) const {
  return chrome::GetBrowserContextOwnInstanceInIncognito(context);
}

}  // namespace prerender
