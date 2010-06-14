// Copyright (c) 2010 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_VIEWS_LOCATION_BAR_LOCATION_BAR_VIEW_H_
#define CHROME_BROWSER_VIEWS_LOCATION_BAR_LOCATION_BAR_VIEW_H_

#include <string>
#include <vector>

#include "base/task.h"
#include "chrome/browser/autocomplete/autocomplete_edit.h"
#include "chrome/browser/extensions/extension_context_menu_model.h"
#include "chrome/browser/first_run.h"
#include "chrome/browser/location_bar.h"
#include "chrome/browser/tab_contents/tab_contents.h"
#include "chrome/browser/toolbar_model.h"
#include "chrome/browser/views/extensions/extension_popup.h"
#include "chrome/common/notification_observer.h"
#include "chrome/common/notification_registrar.h"
#include "gfx/font.h"
#include "gfx/rect.h"
#include "views/controls/native/native_view_host.h"

#if defined(OS_WIN)
#include "chrome/browser/autocomplete/autocomplete_edit_view_win.h"
#else
#include "chrome/browser/autocomplete/autocomplete_edit_view_gtk.h"
#endif

class Browser;
class CommandUpdater;
class ContentSettingImageView;
class EVBubbleView;
class ExtensionAction;
class GURL;
class KeywordHintView;
class LocationIconView;
class PageActionWithBadgeView;
class Profile;
class SelectedKeywordView;
class StarView;

namespace views {
class HorizontalPainter;
};

/////////////////////////////////////////////////////////////////////////////
//
// LocationBarView class
//
//   The LocationBarView class is a View subclass that paints the background
//   of the URL bar strip and contains its content.
//
/////////////////////////////////////////////////////////////////////////////
class LocationBarView : public LocationBar,
                        public LocationBarTesting,
                        public views::View,
                        public views::DragController,
                        public AutocompleteEditController {
 public:
  class Delegate {
   public:
    // Should return the current tab contents.
    virtual TabContents* GetTabContents() = 0;

    // Called by the location bar view when the user starts typing in the edit.
    // This forces our security style to be UNKNOWN for the duration of the
    // editing.
    virtual void OnInputInProgress(bool in_progress) = 0;
  };

  enum ColorKind {
    BACKGROUND = 0,
    TEXT,
    SELECTED_TEXT,
    DEEMPHASIZED_TEXT,
    SECURITY_TEXT,
  };

  // The modes reflect the different scenarios where a location bar can be used.
  // The normal mode is the mode used in a regular browser window.
  // In popup mode, the location bar view is read only and has a slightly
  // different presentation (font size / color).
  // In app launcher mode, the location bar is empty and no security states or
  // page/browser actions are displayed.
  enum Mode {
    NORMAL = 0,
    POPUP,
    APP_LAUNCHER
  };

  LocationBarView(Profile* profile,
                  CommandUpdater* command_updater,
                  ToolbarModel* model,
                  Delegate* delegate,
                  Mode mode);
  virtual ~LocationBarView();

  void Init();

  // Returns whether this instance has been initialized by callin Init. Init can
  // only be called when the receiving instance is attached to a view container.
  bool IsInitialized() const;

  // Returns the appropriate color for the desired kind, based on the user's
  // system theme.
  static SkColor GetColor(ToolbarModel::SecurityLevel security_level,
                          ColorKind kind);

  // Updates the location bar.  We also reset the bar's permanent text and
  // security style, and, if |tab_for_state_restoring| is non-NULL, also restore
  // saved state that the tab holds.
  void Update(const TabContents* tab_for_state_restoring);

  void SetProfile(Profile* profile);
  Profile* profile() const { return profile_; }

  // Returns the current TabContents.
  TabContents* GetTabContents() const;

  // Sets |preview_enabled| for the PageAction View associated with this
  // |page_action|. If |preview_enabled| is true, the view will display the
  // PageActions icon even though it has not been activated by the extension.
  // This is used by the ExtensionInstalledBubble to preview what the icon
  // will look like for the user upon installation of the extension.
  void SetPreviewEnabledPageAction(ExtensionAction *page_action,
                                   bool preview_enabled);

  // Retrieves the PageAction View which is associated with |page_action|.
  views::View* GetPageActionView(ExtensionAction* page_action);

  // Toggles the star on or off.
  void SetStarToggled(bool on);

  // Shows the bookmark bubble.
  void ShowStarBubble(const GURL& url, bool newly_bookmarked);

  // Sizing functions
  virtual gfx::Size GetPreferredSize();

  // Layout and Painting functions
  virtual void Layout();
  virtual void Paint(gfx::Canvas* canvas);

  // No focus border for the location bar, the caret is enough.
  virtual void PaintFocusBorder(gfx::Canvas* canvas) { }

  // Called when any ancestor changes its size, asks the AutocompleteEditModel
  // to close its popup.
  virtual void VisibleBoundsInRootChanged();

#if defined(OS_WIN)
  // Event Handlers
  virtual bool OnMousePressed(const views::MouseEvent& event);
  virtual bool OnMouseDragged(const views::MouseEvent& event);
  virtual void OnMouseReleased(const views::MouseEvent& event, bool canceled);
#endif

  // AutocompleteEditController
  virtual void OnAutocompleteAccept(const GURL& url,
                                    WindowOpenDisposition disposition,
                                    PageTransition::Type transition,
                                    const GURL& alternate_nav_url);
  virtual void OnChanged();
  virtual void OnInputInProgress(bool in_progress);
  virtual void OnKillFocus();
  virtual void OnSetFocus();
  virtual SkBitmap GetFavIcon() const;
  virtual std::wstring GetTitle() const;

  // Overridden from views::View:
  virtual bool SkipDefaultKeyEventProcessing(const views::KeyEvent& e);
  virtual bool GetAccessibleRole(AccessibilityTypes::Role* role);

  // Overridden from views::DragController:
  virtual void WriteDragData(View* sender,
                             const gfx::Point& press_pt,
                             OSExchangeData* data);
  virtual int GetDragOperations(View* sender, const gfx::Point& p);
  virtual bool CanStartDrag(View* sender,
                            const gfx::Point& press_pt,
                            const gfx::Point& p);

  // Overridden from LocationBar:
  virtual void ShowFirstRunBubble(FirstRun::BubbleType bubble_type);
  virtual std::wstring GetInputString() const;
  virtual WindowOpenDisposition GetWindowOpenDisposition() const;
  virtual PageTransition::Type GetPageTransition() const;
  virtual void AcceptInput();
  virtual void FocusLocation(bool select_all);
  virtual void FocusSearch();
  virtual void UpdateContentSettingsIcons();
  virtual void UpdatePageActions();
  virtual void InvalidatePageActions();
  virtual void SaveStateToContents(TabContents* contents);
  virtual void Revert();
  virtual const AutocompleteEditView* location_entry() const {
    return location_entry_.get();
  }
  virtual AutocompleteEditView* location_entry() {
    return location_entry_.get();
  }
  virtual void PushForceHidden();
  virtual void PopForceHidden();
  virtual LocationBarTesting* GetLocationBarForTesting() { return this; }

  // Overridden from LocationBarTesting:
  virtual int PageActionCount() { return page_action_views_.size(); }
  virtual int PageActionVisibleCount();
  virtual ExtensionAction* GetPageAction(size_t index);
  virtual ExtensionAction* GetVisiblePageAction(size_t index);
  virtual void TestPageActionPressed(size_t index);

  static const int kVertMargin;     // Space above and below the edit.
  static const int kEdgeThickness;  // Unavailable space at horizontal edges.
  static const int kItemPadding;    // Space between items within the bar.

 protected:
  void Focus();

 private:
  typedef std::vector<ContentSettingImageView*> ContentSettingViews;

  friend class PageActionImageView;
  friend class PageActionWithBadgeView;
  typedef std::vector<PageActionWithBadgeView*> PageActionViews;

  // Returns the height in pixels of the margin at the top of the bar.
  int TopMargin() const;

  // Returns the amount of horizontal space (in pixels) out of
  // |location_bar_width| that is not taken up by the actual text in
  // location_entry_.
  int AvailableWidth(int location_bar_width);

  // Returns whether the |available_width| is large enough to contain a view
  // with preferred width |pref_width| at its preferred size. If this returns
  // true, the preferred size should be used. If this returns false, the
  // minimum size of the view should be used.
  bool UsePref(int pref_width, int available_width);

  // If View fits in the specified region, it is made visible and the
  // bounds are adjusted appropriately. If the View does not fit, it is
  // made invisible.
  void LayoutView(bool leading, views::View* view, int available_width,
                  gfx::Rect* bounds);

  // Update the visibility state of the Content Blocked icons to reflect what is
  // actually blocked on the current page.
  void RefreshContentSettingViews();

  // Delete all page action views that we have created.
  void DeletePageActionViews();

  // Update the views for the Page Actions, to reflect state changes for
  // PageActions.
  void RefreshPageActionViews();

  // Sets the visibility of view to new_vis.
  void ToggleVisibility(bool new_vis, views::View* view);

#if defined(OS_WIN)
  // Helper for the Mouse event handlers that does all the real work.
  void OnMouseEvent(const views::MouseEvent& event, UINT msg);
#endif

  // Helper to show the first run info bubble.
  void ShowFirstRunBubbleInternal(FirstRun::BubbleType bubble_type);

  // Current browser. Not owned by us.
  Browser* browser_;

  // Current profile. Not owned by us.
  Profile* profile_;

  // The Autocomplete Edit field.
#if defined(OS_WIN)
  scoped_ptr<AutocompleteEditViewWin> location_entry_;
#else
  scoped_ptr<AutocompleteEditViewGtk> location_entry_;
#endif

  // The CommandUpdater for the Browser object that corresponds to this View.
  CommandUpdater* command_updater_;

  // The model.
  ToolbarModel* model_;

  // Our delegate.
  Delegate* delegate_;

  // This is the string of text from the autocompletion session that the user
  // entered or selected.
  std::wstring location_input_;

  // The user's desired disposition for how their input should be opened
  WindowOpenDisposition disposition_;

  // The transition type to use for the navigation
  PageTransition::Type transition_;

  // Font used by edit and some of the hints.
  gfx::Font font_;

  // An object used to paint the normal-mode background.
  scoped_ptr<views::HorizontalPainter> painter_;

  // An icon to the left of the edit field.
  LocationIconView* location_icon_view_;

  // A bubble displayed for EV HTTPS sites.
  EVBubbleView* ev_bubble_view_;

  // Location_entry view wrapper
  views::NativeViewHost* location_entry_view_;

  // The following views are used to provide hints and remind the user as to
  // what is going in the edit. They are all added a children of the
  // LocationBarView. At most one is visible at a time. Preference is
  // given to the keyword_view_, then hint_view_.
  // These autocollapse when the edit needs the room.

  // Shown if the user has selected a keyword.
  SelectedKeywordView* selected_keyword_view_;

  // Shown if the selected url has a corresponding keyword.
  KeywordHintView* keyword_hint_view_;

  // The content setting views.
  ContentSettingViews content_setting_views_;

  // The page action icon views.
  PageActionViews page_action_views_;

  // The star.
  StarView* star_view_;

  // The mode that dictates how the bar shows.
  Mode mode_;

  // Counts the number of times consumers have asked us to be hidden.
  // We should actually be hidden iff this is greater than zero.
  int force_hidden_count_;

  // Used schedule a task for the first run info bubble.
  ScopedRunnableMethodFactory<LocationBarView> first_run_bubble_;

  DISALLOW_IMPLICIT_CONSTRUCTORS(LocationBarView);
};

#endif  // CHROME_BROWSER_VIEWS_LOCATION_BAR_LOCATION_BAR_VIEW_H_
