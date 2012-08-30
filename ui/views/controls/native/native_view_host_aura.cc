// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ui/views/controls/native/native_view_host_aura.h"

#include "base/logging.h"
#include "ui/aura/focus_manager.h"
#include "ui/aura/window.h"
#include "ui/views/controls/native/native_view_host.h"
#include "ui/views/widget/widget.h"

namespace views {

NativeViewHostAura::NativeViewHostAura(NativeViewHost* host)
    : host_(host),
      installed_clip_(false) {
}

NativeViewHostAura::~NativeViewHostAura() {
}

////////////////////////////////////////////////////////////////////////////////
// NativeViewHostAura, NativeViewHostWrapper implementation:
void NativeViewHostAura::NativeViewAttached() {
  if (host_->native_view()->parent())
    host_->native_view()->parent()->RemoveChild(host_->native_view());
  host_->GetWidget()->GetNativeView()->AddChild(host_->native_view());
  host_->Layout();

  host_->native_view()->AddObserver(this);
}

void NativeViewHostAura::NativeViewDetaching(bool destroyed) {
  if (!destroyed) {
    host_->native_view()->RemoveObserver(this);
    host_->native_view()->Hide();
    if (host_->native_view()->parent())
      host_->native_view()->parent()->RemoveChild(host_->native_view());
  }
}

void NativeViewHostAura::AddedToWidget() {
  if (!host_->native_view())
    return;

  aura::Window* widget_window = host_->GetWidget()->GetNativeView();
  if (host_->native_view()->parent() != widget_window)
    widget_window->AddChild(host_->native_view());
  if (host_->IsDrawn())
    host_->native_view()->Show();
  else
    host_->native_view()->Hide();
  host_->Layout();
}

void NativeViewHostAura::RemovedFromWidget() {
  if (host_->native_view()) {
    host_->native_view()->Hide();
    if (host_->native_view()->parent())
      host_->native_view()->parent()->RemoveChild(host_->native_view());
  }
}

void NativeViewHostAura::InstallClip(int x, int y, int w, int h) {
  NOTIMPLEMENTED();
}

bool NativeViewHostAura::HasInstalledClip() {
  return installed_clip_;
}

void NativeViewHostAura::UninstallClip() {
  installed_clip_ = false;
}

void NativeViewHostAura::ShowWidget(int x, int y, int w, int h) {
  // TODO: need to support fast resize.
  host_->native_view()->SetBounds(gfx::Rect(x, y, w, h));
  host_->native_view()->Show();
}

void NativeViewHostAura::HideWidget() {
  host_->native_view()->Hide();
}

void NativeViewHostAura::SetFocus() {
  aura::Window* window = host_->native_view();
  if (window->GetFocusManager())
    window->GetFocusManager()->SetFocusedWindow(window, NULL);
}

gfx::NativeViewAccessible NativeViewHostAura::GetNativeViewAccessible() {
  return NULL;
}

void NativeViewHostAura::OnWindowDestroyed(aura::Window* window) {
  DCHECK(window == host_->native_view());
  host_->NativeViewDestroyed();
}

// static
NativeViewHostWrapper* NativeViewHostWrapper::CreateWrapper(
    NativeViewHost* host) {
  return new NativeViewHostAura(host);
}

}  // namespace views
