// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef COMPONENTS_TRANSLATE_CORE_BROWSER_TRANSLATE_DRIVER_H_
#define COMPONENTS_TRANSLATE_CORE_BROWSER_TRANSLATE_DRIVER_H_

#include <string>

class GURL;

// Interface that allows Translate core code to interact with its driver (i.e.,
// obtain information from it and give information to it). A concrete
// implementation must be provided by the driver.
class TranslateDriver {
 public:
  // Returns true if the current page was navigated through a link.
  virtual bool IsLinkNavigation() = 0;

  // Called when Translate is enabled or disabled.
  virtual void OnTranslateEnabledChanged() = 0;

  // Called when the page is "translated" state of the page changed.
  virtual void OnIsPageTranslatedChanged() = 0;

  // Translates the page contents from |source_lang| to |target_lang|.
  virtual void TranslatePage(const std::string& translate_script,
                             const std::string& source_lang,
                             const std::string& target_lang) = 0;

  // Reverts the contents of the page to its original language.
  virtual void RevertTranslation() = 0;

  // Returns whether the user is currently operating in off-the-record mode.
  virtual bool IsOffTheRecord() = 0;

  // Returns the mime type of the current page.
  virtual const std::string& GetContentsMimeType() = 0;

  // Returns the last committed URL, or an empty GURL if there is no committed
  // URL.
  virtual const GURL& GetLastCommittedURL() = 0;

  // Returns the active URL, or an empty GURL if there is no active URL.
  virtual const GURL& GetActiveURL() = 0;

  // Returns the visible URL, or an empty GURL if there is no visible URL.
  virtual const GURL& GetVisibleURL() = 0;

  // Returns whether the driver has access to the current page.
  virtual bool HasCurrentPage() = 0;

  // Returns an int identifying the current page. Should only be called if
  // |HasCurrentPage()| is true.
  virtual int GetCurrentPageID() = 0;

  // Opens |url| in a new tab.
  virtual void OpenUrlInNewTab(const GURL& url) = 0;
};

#endif  // COMPONENTS_TRANSLATE_CORE_BROWSER_TRANSLATE_DRIVER_H_
