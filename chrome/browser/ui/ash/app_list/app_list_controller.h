// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_UI_ASH_APP_LIST_APP_LIST_CONTROLLER_H_
#define CHROME_BROWSER_UI_ASH_APP_LIST_APP_LIST_CONTROLLER_H_

#include <string>

class Profile;

namespace extensions {
class Extension;
}

// Interface to allow the view delegate to call out to whatever is controlling
// the app list. This will have different implementations for different
// platforms.
class AppListController {
 public:
  virtual ~AppListController() {}

  // Handle the controller being closed.
  virtual void CloseView() = 0;

  // Control of pinning apps.
  virtual bool IsAppPinned(const std::string& extension_id) = 0;
  virtual void PinApp(const std::string& extension_id) = 0;
  virtual void UnpinApp(const std::string& extension_id) = 0;
  virtual bool CanPin() = 0;

  // App has been clicked on in the app list.
  virtual void ActivateApp(Profile* profile,
                           const std::string& extension_id,
                           int event_flags) = 0;
  // Open the app (e.g. from search results).
  virtual void OpenApp(Profile* profile,
                       const std::string& extension_id,
                       int event_flags) = 0;
};

#endif  // CHROME_BROWSER_UI_ASH_APP_LIST_APP_LIST_CONTROLLER_H_
