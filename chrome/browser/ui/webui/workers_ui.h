// Copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_UI_WEBUI_WORKERS_UI_H_
#define CHROME_BROWSER_UI_WEBUI_WORKERS_UI_H_
#pragma once

#include "base/memory/ref_counted.h"
#include "content/browser/webui/web_ui.h"

class WorkersUI : public WebUI {
 public:
  explicit WorkersUI(content::WebContents* contents);
  virtual ~WorkersUI();

 private:
  class WorkerCreationDestructionListener;
  scoped_refptr<WorkerCreationDestructionListener> observer_;

  DISALLOW_COPY_AND_ASSIGN(WorkersUI);
};

#endif  // CHROME_BROWSER_UI_WEBUI_WORKERS_UI_H_
