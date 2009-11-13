// Copyright (c) 2009 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_DOWNLOAD_DOWNLOAD_STARTED_ANIMATION_H_
#define CHROME_BROWSER_DOWNLOAD_DOWNLOAD_STARTED_ANIMATION_H_

class TabContents;

class DownloadStartedAnimation {
 public:
  static void Show(TabContents* tab_contents);

 private:
  DownloadStartedAnimation() { }
};

#endif  // CHROME_BROWSER_DOWNLOAD_DOWNLOAD_STARTED_ANIMATION_H_
