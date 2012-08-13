// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ui/views/widget/drop_helper.h"

#include "ui/base/dragdrop/drag_drop_types.h"
#include "ui/views/view.h"
#include "ui/views/widget/widget.h"

namespace views {

DropHelper::DropHelper(View* root_view)
    : root_view_(root_view),
      target_view_(NULL),
      deepest_view_(NULL) {
}

DropHelper::~DropHelper() {
}

void DropHelper::ResetTargetViewIfEquals(View* view) {
  if (target_view_ == view)
    target_view_ = NULL;
  if (deepest_view_ == view)
    deepest_view_ = NULL;
}

int DropHelper::OnDragOver(const OSExchangeData& data,
                           const gfx::Point& root_view_location,
                           int drag_operation) {
  View* view = CalculateTargetViewImpl(root_view_location, data, true,
                                       &deepest_view_);

  if (view != target_view_) {
    // Target changed notify old drag exited, then new drag entered.
    NotifyDragExit();
    target_view_ = view;
    NotifyDragEntered(data, root_view_location, drag_operation);
  }

  return NotifyDragOver(data, root_view_location, drag_operation);
}

void DropHelper::OnDragExit() {
  NotifyDragExit();
  deepest_view_ = target_view_ = NULL;
}

int DropHelper::OnDrop(const OSExchangeData& data,
                       const gfx::Point& root_view_location,
                       int drag_operation) {
  View* drop_view = target_view_;
  deepest_view_ = target_view_ = NULL;
  if (!drop_view)
    return ui::DragDropTypes::DRAG_NONE;

  if (drag_operation == ui::DragDropTypes::DRAG_NONE) {
    drop_view->OnDragExited();
    return ui::DragDropTypes::DRAG_NONE;
  }

  gfx::Point view_location(root_view_location);
  View* root_view = drop_view->GetWidget()->GetRootView();
  View::ConvertPointToTarget(root_view, drop_view, &view_location);
  DropTargetEvent drop_event(data, view_location.x(), view_location.y(),
                             drag_operation);
  return drop_view->OnPerformDrop(drop_event);
}

View* DropHelper::CalculateTargetView(
    const gfx::Point& root_view_location,
    const OSExchangeData& data,
    bool check_can_drop) {
  return CalculateTargetViewImpl(root_view_location, data, check_can_drop,
                                 NULL);
}

View* DropHelper::CalculateTargetViewImpl(
    const gfx::Point& root_view_location,
    const OSExchangeData& data,
    bool check_can_drop,
    View** deepest_view) {
  View* view = root_view_->GetEventHandlerForPoint(root_view_location);
  if (view == deepest_view_) {
    // The view the mouse is over hasn't changed; reuse the target.
    return target_view_;
  }
  if (deepest_view)
    *deepest_view = view;
  // TODO(sky): for the time being these are separate. Once I port chrome menu
  // I can switch to the #else implementation and nuke the OS_WIN
  // implementation.
#if defined(OS_WIN)
  // View under mouse changed, which means a new view may want the drop.
  // Walk the tree, stopping at target_view_ as we know it'll accept the
  // drop.
  while (view && view != target_view_ &&
         (!view->enabled() || !view->CanDrop(data))) {
    view = view->parent();
  }
#elif !defined(OS_MACOSX)
  int formats = 0;
  std::set<OSExchangeData::CustomFormat> custom_formats;
  while (view && view != target_view_) {
    if (view->enabled() &&
        view->GetDropFormats(&formats, &custom_formats) &&
        data.HasAnyFormat(formats, custom_formats) &&
        (!check_can_drop || view->CanDrop(data))) {
      // Found the view.
      return view;
    }
    formats = 0;
    custom_formats.clear();
    view = view->parent();
  }
#endif
  return view;
}

void DropHelper::NotifyDragEntered(const OSExchangeData& data,
                                   const gfx::Point& root_view_location,
                                   int drag_operation) {
  if (!target_view_)
    return;

  gfx::Point target_view_location(root_view_location);
  View::ConvertPointToTarget(root_view_, target_view_, &target_view_location);
  DropTargetEvent enter_event(data,
                              target_view_location.x(),
                              target_view_location.y(),
                              drag_operation);
  target_view_->OnDragEntered(enter_event);
}

int DropHelper::NotifyDragOver(const OSExchangeData& data,
                               const gfx::Point& root_view_location,
                               int drag_operation) {
  if (!target_view_)
    return ui::DragDropTypes::DRAG_NONE;

  gfx::Point target_view_location(root_view_location);
  View::ConvertPointToTarget(root_view_, target_view_, &target_view_location);
  DropTargetEvent enter_event(data,
                              target_view_location.x(),
                              target_view_location.y(),
                              drag_operation);
  return target_view_->OnDragUpdated(enter_event);
}

void DropHelper::NotifyDragExit() {
  if (target_view_)
    target_view_->OnDragExited();
}

}  // namespace views
