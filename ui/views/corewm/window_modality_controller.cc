// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ui/views/corewm/window_modality_controller.h"

#include <algorithm>

#include "ui/aura/client/aura_constants.h"
#include "ui/aura/client/capture_client.h"
#include "ui/aura/env.h"
#include "ui/aura/root_window.h"
#include "ui/aura/window.h"
#include "ui/aura/window_property.h"
#include "ui/base/events/event.h"
#include "ui/base/ui_base_types.h"
#include "ui/views/corewm/window_animations.h"
#include "ui/views/corewm/window_util.h"

namespace views {
namespace corewm {

// Transient child's modal parent.
extern const aura::WindowProperty<aura::Window*>* const kModalParentKey;
DEFINE_WINDOW_PROPERTY_KEY(aura::Window*, kModalParentKey, NULL);

namespace {

bool HasAncestor(aura::Window* window, aura::Window* ancestor) {
  if (!window)
    return false;
  if (window == ancestor)
    return true;
  return HasAncestor(window->parent(), ancestor);
}

bool TransientChildIsWindowModal(aura::Window* window) {
  return window->GetProperty(aura::client::kModalKey) == ui::MODAL_TYPE_WINDOW;
}

bool TransientChildIsChildModal(aura::Window* window) {
  return window->GetProperty(aura::client::kModalKey) == ui::MODAL_TYPE_CHILD;
}

aura::Window* GetModalParent(aura::Window* window) {
  return window->GetProperty(kModalParentKey);
}

bool IsModalTransientChild(aura::Window* transient, aura::Window* original) {
  return transient->IsVisible() &&
      (TransientChildIsWindowModal(transient) ||
       (TransientChildIsChildModal(transient) &&
        (HasAncestor(original, GetModalParent(transient)))));
}

aura::Window* GetModalTransientChild(
    aura::Window* activatable,
    aura::Window* original) {
  aura::Window::Windows::const_iterator it;
  for (it = activatable->transient_children().begin();
       it != activatable->transient_children().end();
       ++it) {
    aura::Window* transient = *it;
    if (IsModalTransientChild(transient, original)) {
      return transient->transient_children().empty() ?
          transient : GetModalTransientChild(transient, original);
    }
  }
  return NULL;
}

}  // namespace

void SetModalParent(aura::Window* child, aura::Window* parent) {
  child->SetProperty(kModalParentKey, parent);
}

aura::Window* GetModalTransient(aura::Window* window) {
  if (!window)
    return NULL;

  // We always want to check the for the transient child of the toplevel window.
  aura::Window* toplevel = GetToplevelWindow(window);
  if (!toplevel)
    return NULL;

  return GetModalTransientChild(toplevel, window);
}

////////////////////////////////////////////////////////////////////////////////
// WindowModalityController, public:

WindowModalityController::WindowModalityController() {
  aura::Env::GetInstance()->AddObserver(this);
}

WindowModalityController::~WindowModalityController() {
  aura::Env::GetInstance()->RemoveObserver(this);
  for (size_t i = 0; i < windows_.size(); ++i)
    windows_[i]->RemoveObserver(this);
}

////////////////////////////////////////////////////////////////////////////////
// WindowModalityController, aura::EventFilter implementation:

ui::EventResult WindowModalityController::OnKeyEvent(ui::KeyEvent* event) {
  aura::Window* target = static_cast<aura::Window*>(event->target());
  return GetModalTransient(target) ? ui::ER_CONSUMED : ui::ER_UNHANDLED;
}

ui::EventResult WindowModalityController::OnMouseEvent(ui::MouseEvent* event) {
  aura::Window* target = static_cast<aura::Window*>(event->target());
  return ProcessLocatedEvent(target, event) ? ui::ER_CONSUMED :
                                              ui::ER_UNHANDLED;
}

ui::EventResult WindowModalityController::OnTouchEvent(ui::TouchEvent* event) {
  aura::Window* target = static_cast<aura::Window*>(event->target());
  return ProcessLocatedEvent(target, event) ? ui::ER_CONSUMED :
                                              ui::ER_UNHANDLED;
}

////////////////////////////////////////////////////////////////////////////////
// WindowModalityController, aura::EnvObserver implementation:

void WindowModalityController::OnWindowInitialized(aura::Window* window) {
  windows_.push_back(window);
  window->AddObserver(this);
}

////////////////////////////////////////////////////////////////////////////////
// WindowModalityController, aura::WindowObserver implementation:

void WindowModalityController::OnWindowPropertyChanged(aura::Window* window,
                                                       const void* key,
                                                       intptr_t old) {
  // In tests, we sometimes create the modality relationship after a window is
  // visible.
  if (key == aura::client::kModalKey &&
      window->GetProperty(aura::client::kModalKey) != ui::MODAL_TYPE_NONE &&
      window->IsVisible()) {
    ActivateWindow(window);
  }
}

void WindowModalityController::OnWindowVisibilityChanged(
    aura::Window* window,
    bool visible) {
  if (visible && window->GetProperty(aura::client::kModalKey) ==
      ui::MODAL_TYPE_WINDOW) {
    // Make sure no other window has capture, otherwise |window| won't get mouse
    // events.
    aura::Window* capture_window = aura::client::GetCaptureWindow(window);
    if (capture_window)
      capture_window->ReleaseCapture();
  }
}

void WindowModalityController::OnWindowDestroyed(aura::Window* window) {
  windows_.erase(std::find(windows_.begin(), windows_.end(), window));
  window->RemoveObserver(this);
}

bool WindowModalityController::ProcessLocatedEvent(aura::Window* target,
                                                   ui::LocatedEvent* event) {
  aura::Window* modal_transient_child = GetModalTransient(target);
  if (modal_transient_child && (event->type() == ui::ET_MOUSE_PRESSED ||
                                event->type() == ui::ET_TOUCH_PRESSED)) {
    AnimateWindow(modal_transient_child, WINDOW_ANIMATION_TYPE_BOUNCE);
  }
  return !!modal_transient_child;
}

}  // namespace corewm
}  // namespace views
