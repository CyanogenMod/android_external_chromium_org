// Copyright (c) 2006-2009 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <gtk/gtk.h>

#include "views/focus/focus_manager.h"

#include "base/logging.h"
#include "views/widget/widget_gtk.h"

namespace views {

void FocusManager::ClearNativeFocus() {
  gtk_widget_grab_focus(widget_->GetNativeView());
}

void FocusManager::FocusNativeView(gfx::NativeView native_view) {
  if (native_view && !gtk_widget_is_focus(native_view))
    gtk_widget_grab_focus(native_view);
}

 // static
FocusManager* FocusManager::GetFocusManagerForNativeView(
    gfx::NativeView native_view) {
  GtkWidget* root = gtk_widget_get_toplevel(native_view);
  if (!root || !GTK_WIDGET_TOPLEVEL(root))
    return NULL;

  WidgetGtk* widget = WidgetGtk::GetViewForNative(root);
  if (!widget) {
    NOTREACHED();
    return NULL;
  }
  FocusManager* focus_manager = widget->GetFocusManager();
  DCHECK(focus_manager) << "no FocusManager for top level Widget";
  return focus_manager;
}

}  // namespace views
