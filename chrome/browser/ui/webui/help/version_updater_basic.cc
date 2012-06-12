// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/ui/webui/help/version_updater_basic.h"

#include "base/string16.h"
#include "chrome/browser/lifetime/application_lifetime.h"
#include "chrome/browser/upgrade_detector.h"

void VersionUpdaterBasic::CheckForUpdate(
    const StatusCallback& status_callback) {
  if (UpgradeDetector::GetInstance()->notify_upgrade())
    status_callback.Run(NEARLY_UPDATED, 0, string16());
  else
    status_callback.Run(DISABLED, 0, string16());
}

void VersionUpdaterBasic::RelaunchBrowser() const {
  browser::AttemptRestart();
}

VersionUpdater* VersionUpdater::Create() {
  return new VersionUpdaterBasic;
}
