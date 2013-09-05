// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_MANAGED_MODE_MANAGED_MODE_INTERSTITIAL_H_
#define CHROME_BROWSER_MANAGED_MODE_MANAGED_MODE_INTERSTITIAL_H_

#include <string>

#include "base/callback.h"
#include "base/compiler_specific.h"
#include "base/prefs/pref_change_registrar.h"
#include "content/public/browser/interstitial_page_delegate.h"
#include "url/gurl.h"

namespace content {
class InterstitialPage;
class WebContents;
}

// Delegate for an interstitial page when a page is blocked in managed mode
// because it is not on any whitelist. It lets an authorized user preview the
// blocked page, to decide later whether to manually allow it.
class ManagedModeInterstitial : public content::InterstitialPageDelegate {
 public:
  ManagedModeInterstitial(content::WebContents* web_contents,
                          const GURL& url,
                          const base::Callback<void(bool)>& callback);
  virtual ~ManagedModeInterstitial();

 private:
  // InterstitialPageDelegate implementation.
  virtual std::string GetHTMLContents() OVERRIDE;
  virtual void CommandReceived(const std::string& command) OVERRIDE;
  virtual void OnProceed() OVERRIDE;
  virtual void OnDontProceed() OVERRIDE;

  // Returns whether the blocked URL is now allowed. Called initially before the
  // interstitial is shown (to catch race conditions), or when the URL filtering
  // prefs change.
  bool ShouldProceed();

  void OnFilteringPrefsChanged();
  void DispatchContinueRequest(bool continue_request);

  // Owns the interstitial, which owns us.
  content::WebContents* web_contents_;

  content::InterstitialPage* interstitial_page_;  // Owns us.

  PrefChangeRegistrar pref_change_registrar_;

  // The UI language. Used for formatting the URL for display.
  std::string languages_;
  GURL url_;

  base::Callback<void(bool)> callback_;

  DISALLOW_COPY_AND_ASSIGN(ManagedModeInterstitial);
};

#endif  // CHROME_BROWSER_MANAGED_MODE_MANAGED_MODE_INTERSTITIAL_H_
