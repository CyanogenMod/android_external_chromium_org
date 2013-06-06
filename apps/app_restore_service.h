// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef APPS_APP_RESTORE_SERVICE_H_
#define APPS_APP_RESTORE_SERVICE_H_

#include <string>
#include <vector>

#include "apps/app_lifetime_monitor.h"
#include "chrome/browser/extensions/shell_window_registry.h"
#include "components/browser_context_keyed_service/browser_context_keyed_service.h"
#include "content/public/browser/notification_observer.h"
#include "content/public/browser/notification_registrar.h"

namespace extensions {
class Extension;
}

class Profile;

namespace apps {

// Tracks what apps need to be restarted when the browser restarts.
class AppRestoreService : public BrowserContextKeyedService,
                          public content::NotificationObserver,
                          public AppLifetimeMonitor::Observer {
 public:
  // Returns true if apps should be restored on the current platform, given
  // whether this new browser process launched due to a restart.
  static bool ShouldRestoreApps(bool is_browser_restart);

  explicit AppRestoreService(Profile* profile);

  // Restart apps that need to be restarted and clear the "running" preference
  // from apps to prevent them being restarted in subsequent restarts.
  void HandleStartup(bool should_restore_apps);

 private:
  // content::NotificationObserver.
  virtual void Observe(int type,
                       const content::NotificationSource& source,
                       const content::NotificationDetails& details) OVERRIDE;

  // AppLifetimeMonitor::Observer.
  virtual void OnAppStart(Profile* profile, const std::string& app_id) OVERRIDE;
  virtual void OnAppActivated(Profile* profile,
                              const std::string& app_id) OVERRIDE;
  virtual void OnAppDeactivated(Profile* profile,
                                const std::string& app_id) OVERRIDE;
  virtual void OnAppStop(Profile* profile, const std::string& app_id) OVERRIDE;

  // BrowserContextKeyedService.
  virtual void Shutdown() OVERRIDE;

  void RecordAppStart(const std::string& extension_id);
  void RecordAppStop(const std::string& extension_id);
  void RecordAppActiveState(const std::string& id, bool is_active);

  void RestoreApp(const extensions::Extension* extension);

  void StartObservingAppLifetime();
  void StopObservingAppLifetime();

  content::NotificationRegistrar registrar_;
  Profile* profile_;

  DISALLOW_COPY_AND_ASSIGN(AppRestoreService);
};

}  // namespace apps

#endif  // APPS_APP_RESTORE_SERVICE_H_
