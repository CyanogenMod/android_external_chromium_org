// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_UI_TOOLBAR_TOOLBAR_MODEL_H_
#define CHROME_BROWSER_UI_TOOLBAR_TOOLBAR_MODEL_H_

#include <string>

#include "base/basictypes.h"
#include "base/string16.h"
#include "googleurl/src/gurl.h"

class Profile;
class ToolbarModelDelegate;

namespace content {
class NavigationController;
}

namespace net {
class X509Certificate;
}

// This class is the model used by the toolbar, location bar and autocomplete
// edit.  It populates its states from the current navigation entry retrieved
// from the navigation controller returned by GetNavigationController().
class ToolbarModel {
 public:
  // TODO(wtc): unify ToolbarModel::SecurityLevel with SecurityStyle.  We
  // don't need two sets of security UI levels.  SECURITY_STYLE_AUTHENTICATED
  // needs to be refined into three levels: warning, standard, and EV.
  enum SecurityLevel {
    NONE = 0,          // HTTP/no URL/user is editing
    EV_SECURE,         // HTTPS with valid EV cert
    SECURE,            // HTTPS (non-EV)
    SECURITY_WARNING,  // HTTPS, but unable to check certificate revocation
                       // status or with insecure content on the page
    SECURITY_ERROR,    // Attempted HTTPS and failed, page not authenticated
    NUM_SECURITY_LEVELS,
  };

  explicit ToolbarModel(ToolbarModelDelegate* delegate);
  ~ToolbarModel();

  // Returns the text for the current page's URL. This will have been formatted
  // for display to the user:
  //   - Some characters may be unescaped.
  //   - The scheme and/or trailing slash may be dropped.
  //   - if |display_search_urls_as_search_terms| is true, the query will be
  //   extracted from search URLs for the user's default search engine and those
  //   will be displayed in place of the URL.
  string16 GetText(bool display_search_urls_as_search_terms) const;

  // Returns the URL of the current navigation entry.
  GURL GetURL() const;

  // Returns true if a call to GetText(true) would successfully replace the URL
  // with search terms.
  bool WouldReplaceSearchURLWithSearchTerms() const;

  // Returns the security level that the toolbar should display.
  SecurityLevel GetSecurityLevel() const;

  // Returns the resource_id of the icon to show to the left of the address,
  // based on the current URL.  This doesn't cover specialized icons while the
  // user is editing; see OmniboxView::GetIcon().
  int GetIcon() const;

  // Returns the name of the EV cert holder.  Only call this when the security
  // level is EV_SECURE.
  string16 GetEVCertName() const;

  // Returns whether the URL for the current navigation entry should be
  // in the location bar.
  bool ShouldDisplayURL() const;

  // Getter/setter of whether the text in location bar is currently being
  // edited.
  void set_input_in_progress(bool value) { input_in_progress_ = value; }
  bool input_in_progress() const { return input_in_progress_; }

  // Returns "<organization_name> [<country>]".
  static string16 GetEVCertName(const net::X509Certificate& cert);

 private:
  // Returns the navigation controller used to retrieve the navigation entry
  // from which the states are retrieved.
  // If this returns NULL, default values are used.
  content::NavigationController* GetNavigationController() const;

  // Attempt to extract search terms from |url|. Called by GetText if
  // |display_search_urls_as_search_terms| is true and by
  // WouldReplaceSearchURLWithSearchTerms.
  string16 TryToExtractSearchTermsFromURL(const GURL& url) const;

  // Helper method to extract the profile from the navigation controller.
  Profile* GetProfile() const;

  ToolbarModelDelegate* delegate_;

  // Whether the text in the location bar is currently being edited.
  bool input_in_progress_;

  DISALLOW_IMPLICIT_CONSTRUCTORS(ToolbarModel);
};

#endif  // CHROME_BROWSER_UI_TOOLBAR_TOOLBAR_MODEL_H_
