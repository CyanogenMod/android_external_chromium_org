// Copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_UI_SNAPSHOT_TAB_HELPER_H_
#define CHROME_BROWSER_UI_SNAPSHOT_TAB_HELPER_H_
#pragma once

#include "content/browser/tab_contents/tab_contents_observer.h"

class SkBitmap;
class TabContentsWrapper;

// Per-tab class to handle snapshot functionality.
class SnapshotTabHelper : public TabContentsObserver {
 public:
  explicit SnapshotTabHelper(TabContentsWrapper* wrapper);
  virtual ~SnapshotTabHelper();

  // Captures a snapshot of the page.
  void CaptureSnapshot();

 private:
  // TabContentsObserver overrides:
  virtual bool OnMessageReceived(const IPC::Message& message) OVERRIDE;

  // Internal helpers ----------------------------------------------------------

  // Message handler.
  void OnSnapshot(const SkBitmap& bitmap);

  // Our owning TabContentsWrapper.
  TabContentsWrapper* wrapper_;

  DISALLOW_COPY_AND_ASSIGN(SnapshotTabHelper);
};

#endif  // CHROME_BROWSER_UI_SNAPSHOT_TAB_HELPER_H_
