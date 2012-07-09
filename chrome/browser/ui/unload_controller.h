// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_UI_UNLOAD_CONTROLLER_H_
#define CHROME_BROWSER_UI_UNLOAD_CONTROLLER_H_
#pragma once

#include <set>

#include "base/memory/weak_ptr.h"
#include "chrome/browser/ui/tabs/tab_strip_model_observer.h"
#include "content/public/browser/notification_observer.h"
#include "content/public/browser/notification_registrar.h"

class Browser;
class TabContents;
class TabStripModel;

namespace content {
class NotificationSource;
class NotifictaionDetails;
class WebContents;
}

namespace chrome {

class UnloadController : public content::NotificationObserver,
                         public TabStripModelObserver {
 public:
  explicit UnloadController(Browser* browser);
  virtual ~UnloadController();

  // Returns true if |contents| can be cleanly closed. When |browser_| is being
  // closed, this function will return false to indicate |contents| should not
  // be cleanly closed, since the fast shutdown path will just kill its
  // renderer.
  bool CanCloseContents(content::WebContents* contents);

  // Called when a BeforeUnload handler is fired for |contents|. |proceed|
  // indicates the user's response to the Y/N BeforeUnload handler dialog. If
  // this parameter is false, any pending attempt to close the whole browser
  // will be canceled. Returns true if Unload handlers should be fired. When the
  // |browser_| is being closed, Unload handlers for any particular WebContents
  // will not be run until every WebContents being closed has a chance to run
  // its BeforeUnloadHandler.
  bool BeforeUnloadFired(content::WebContents* contents, bool proceed);

  bool is_attempting_to_close_browser() const {
    return is_attempting_to_close_browser_;
  }

  // Called in response to a request to close |browser_|'s window. Returns true
  // when there are no remaining unload handlers to be run.
  bool ShouldCloseWindow();

  // Returns true if |browser_| has any tabs that have BeforeUnload handlers
  // that have not been fired. This method is non-const because it builds a list
  // of tabs that need their BeforeUnloadHandlers fired.
  // TODO(beng): This seems like it could be private but it is used by
  //             AreAllBrowsersCloseable() in application_lifetime.cc. It seems
  //             very similar to ShouldCloseWindow() and some consolidation
  //             could be pursued.
  bool TabsNeedBeforeUnloadFired();

 private:
  typedef std::set<content::WebContents*> UnloadListenerSet;

  // Overridden from content::NotificationObserver:
  virtual void Observe(int type,
                       const content::NotificationSource& source,
                       const content::NotificationDetails& details) OVERRIDE;

  // Overridden from TabStripModelObserver:
  virtual void TabInsertedAt(TabContents* contents,
                             int index,
                             bool foreground) OVERRIDE;
  virtual void TabDetachedAt(TabContents* contents, int index) OVERRIDE;
  virtual void TabReplacedAt(TabStripModel* tab_strip_model,
                             TabContents* old_contents,
                             TabContents* new_contents,
                             int index) OVERRIDE;
  virtual void TabStripEmpty() OVERRIDE;

  void TabAttachedImpl(TabContents* contents);
  void TabDetachedImpl(TabContents* contents);

  // Processes the next tab that needs it's beforeunload/unload event fired.
  void ProcessPendingTabs();

  // Whether we've completed firing all the tabs' beforeunload/unload events.
  bool HasCompletedUnloadProcessing() const;

  // Clears all the state associated with processing tabs' beforeunload/unload
  // events since the user cancelled closing the window.
  void CancelWindowClose();

  // Removes |web_contents| from the passed |set|.
  // Returns whether the tab was in the set in the first place.
  bool RemoveFromSet(UnloadListenerSet* set,
                     content::WebContents* web_contents);

  // Cleans up state appropriately when we are trying to close the browser and
  // the tab has finished firing its unload handler. We also use this in the
  // cases where a tab crashes or hangs even if the beforeunload/unload haven't
  // successfully fired. If |process_now| is true |ProcessPendingTabs| is
  // invoked immediately, otherwise it is invoked after a delay (PostTask).
  //
  // Typically you'll want to pass in true for |process_now|. Passing in true
  // may result in deleting |tab|. If you know that shouldn't happen (because of
  // the state of the stack), pass in false.
  void ClearUnloadState(content::WebContents* web_contents, bool process_now);

  Browser* browser_;

  content::NotificationRegistrar registrar_;

  // Tracks tabs that need there beforeunload event fired before we can
  // close the browser. Only gets populated when we try to close the browser.
  UnloadListenerSet tabs_needing_before_unload_fired_;

  // Tracks tabs that need there unload event fired before we can
  // close the browser. Only gets populated when we try to close the browser.
  UnloadListenerSet tabs_needing_unload_fired_;

  // Whether we are processing the beforeunload and unload events of each tab
  // in preparation for closing the browser. UnloadController owns this state
  // rather than Browser because unload handlers are the only reason that a
  // Browser window isn't just immediately closed.
  bool is_attempting_to_close_browser_;

  base::WeakPtrFactory<UnloadController> weak_factory_;

  DISALLOW_COPY_AND_ASSIGN(UnloadController);
};

}  // namespace chrome

#endif  // CHROME_BROWSER_UI_UNLOAD_CONTROLLER_H_
