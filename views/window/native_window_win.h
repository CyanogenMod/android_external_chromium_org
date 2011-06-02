// Copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef VIEWS_WINDOW_NATIVE_WINDOW_WIN_H_
#define VIEWS_WINDOW_NATIVE_WINDOW_WIN_H_
#pragma once

#include "views/widget/native_widget_win.h"
#include "views/window/native_window.h"
#include "views/window/window.h"

namespace gfx {
class Font;
class Point;
class Size;
};

namespace views {
namespace internal {
class NativeWindowDelegate;

// This is exposed only for testing
// Adjusts the value of |child_rect| if necessary to ensure that it is
// completely visible within |parent_rect|.
void EnsureRectIsVisibleInRect(const gfx::Rect& parent_rect,
                               gfx::Rect* child_rect,
                               int padding);

}  // namespace internal

class Client;
class WindowDelegate;

////////////////////////////////////////////////////////////////////////////////
//
// NativeWindowWin
//
//  A NativeWindowWin is a NativeWidgetWin that encapsulates a window with a
//  frame. The frame may or may not be rendered by the operating system. The
//  window may or may not be top level.
//
////////////////////////////////////////////////////////////////////////////////
class NativeWindowWin : public NativeWidgetWin,
                        public NativeWindow {
 public:
  explicit NativeWindowWin(internal::NativeWindowDelegate* delegate);
  virtual ~NativeWindowWin();

  // Show the window with the specified show command.
  void Show(int show_state);

  // Returns the system set window title font.
  static gfx::Font GetWindowTitleFont();

  // Overridden from NativeWindow:
  virtual Window* GetWindow() OVERRIDE;
  virtual const Window* GetWindow() const OVERRIDE;

 protected:
  friend Window;

  // Returns the insets of the client area relative to the non-client area of
  // the window. Override this function instead of OnNCCalcSize, which is
  // crazily complicated.
  virtual gfx::Insets GetClientAreaInsets() const;

  // Retrieve the show state of the window. This is one of the SW_SHOW* flags
  // passed into Windows' ShowWindow method. For normal windows this defaults
  // to SW_SHOWNORMAL, however windows (e.g. the main window) can override this
  // method to provide different values (e.g. retrieve the user's specified
  // show state from the shortcut starutp info).
  virtual int GetShowState() const;

  // Overridden from NativeWidgetWin:
  virtual void InitNativeWidget(const Widget::InitParams& params) OVERRIDE;
  virtual void OnActivateApp(BOOL active, DWORD thread_id) OVERRIDE;
  virtual void OnDestroy() OVERRIDE;
  virtual LRESULT OnDwmCompositionChanged(UINT msg,
                                          WPARAM w_param,
                                          LPARAM l_param) OVERRIDE;
  virtual void OnEnterSizeMove() OVERRIDE;
  virtual void OnExitSizeMove() OVERRIDE;
  virtual void OnGetMinMaxInfo(MINMAXINFO* minmax_info) OVERRIDE;
  virtual void OnInitMenu(HMENU menu) OVERRIDE;
  virtual LRESULT OnMouseActivate(UINT message, WPARAM w_param, LPARAM l_param)
      OVERRIDE;
  virtual LRESULT OnMouseRange(UINT message,
                               WPARAM w_param,
                               LPARAM l_param) OVERRIDE;
  virtual LRESULT OnNCActivate(BOOL active) OVERRIDE;
  LRESULT OnNCCalcSize(BOOL mode, LPARAM l_param);  // Don't override.
  virtual LRESULT OnNCHitTest(const CPoint& point) OVERRIDE;
  virtual void OnNCPaint(HRGN rgn) OVERRIDE;
  virtual LRESULT OnNCUAHDrawCaption(UINT msg,
                                     WPARAM w_param,
                                     LPARAM l_param) OVERRIDE;
  virtual LRESULT OnNCUAHDrawFrame(UINT msg,
                                   WPARAM w_param,
                                   LPARAM l_param) OVERRIDE;
  virtual LRESULT OnSetCursor(UINT message,
                              WPARAM w_param,
                              LPARAM l_param) OVERRIDE;
  virtual LRESULT OnSetIcon(UINT size_type, HICON new_icon) OVERRIDE;
  virtual LRESULT OnSetText(const wchar_t* text) OVERRIDE;
  virtual void OnSize(UINT size_param, const CSize& new_size) OVERRIDE;
  virtual void OnSysCommand(UINT notification_code, CPoint click) OVERRIDE;
  virtual void OnWindowPosChanging(WINDOWPOS* window_pos) OVERRIDE;

  // Overridden from NativeWindow:
  virtual NativeWidget* AsNativeWidget() OVERRIDE;
  virtual const NativeWidget* AsNativeWidget() const OVERRIDE;
  virtual gfx::Rect GetRestoredBounds() const OVERRIDE;
  virtual void ShowNativeWindow(ShowState state) OVERRIDE;
  virtual void BecomeModal() OVERRIDE;
  virtual void EnableClose(bool enable) OVERRIDE;

  // Overridden from NativeWidgetWin:
  virtual bool IsActive() const OVERRIDE;

 private:
  // If necessary, enables all ancestors.
  void RestoreEnabledIfNecessary();

  // Calculate the appropriate window styles for this window.
  DWORD CalculateWindowStyle();
  DWORD CalculateWindowExStyle();

  // Lock or unlock the window from being able to redraw itself in response to
  // updates to its invalid region.
  class ScopedRedrawLock;
  void LockUpdates();
  void UnlockUpdates();

  // Stops ignoring SetWindowPos() requests (see below).
  void StopIgnoringPosChanges() { ignore_window_pos_changes_ = false; }

  //  Update accessibility information via our WindowDelegate.
  void UpdateAccessibleName(std::wstring& accessible_name);
  void UpdateAccessibleRole();
  void UpdateAccessibleState();

  // Calls the default WM_NCACTIVATE handler with the specified activation
  // value, safely wrapping the call in a ScopedRedrawLock to prevent frame
  // flicker.
  LRESULT CallDefaultNCActivateHandler(BOOL active);

  // A delegate implementation that handles events received here.
  internal::NativeWindowDelegate* delegate_;

  // Whether all ancestors have been enabled. This is only used if is_modal_ is
  // true.
  bool restored_enabled_;

  // True if this window is the active top level window.
  bool is_active_;

  // True if updates to this window are currently locked.
  bool lock_updates_;

  // The window styles of the window before updates were locked.
  DWORD saved_window_style_;

  // When true, this flag makes us discard incoming SetWindowPos() requests that
  // only change our position/size.  (We still allow changes to Z-order,
  // activation, etc.)
  bool ignore_window_pos_changes_;

  // The following factory is used to ignore SetWindowPos() calls for short time
  // periods.
  ScopedRunnableMethodFactory<NativeWindowWin> ignore_pos_changes_factory_;

  // Set to true when the user presses the right mouse button on the caption
  // area. We need this so we can correctly show the context menu on mouse-up.
  bool is_right_mouse_pressed_on_caption_;

  // The last-seen monitor containing us, and its rect and work area.  These are
  // used to catch updates to the rect and work area and react accordingly.
  HMONITOR last_monitor_;
  gfx::Rect last_monitor_rect_, last_work_area_;

  DISALLOW_COPY_AND_ASSIGN(NativeWindowWin);
};

}  // namespace views

#endif  // VIEWS_WINDOW_NATIVE_WINDOW_WIN_H_
