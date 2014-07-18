// Copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef UI_VIEWS_CONTROLS_NATIVE_NATIVE_VIEW_HOST_AURA_H_
#define UI_VIEWS_CONTROLS_NATIVE_NATIVE_VIEW_HOST_AURA_H_

#include "base/basictypes.h"
#include "base/compiler_specific.h"
#include "ui/aura/window.h"
#include "ui/aura/window_observer.h"
#include "ui/views/controls/native/native_view_host_wrapper.h"
#include "ui/views/views_export.h"

namespace views {

class NativeViewHost;

// Aura implementation of NativeViewHostWrapper.
class VIEWS_EXPORT NativeViewHostAura : public NativeViewHostWrapper,
                                        public aura::WindowObserver {
 public:
  explicit NativeViewHostAura(NativeViewHost* host);
  virtual ~NativeViewHostAura();

  // Overridden from NativeViewHostWrapper:
  virtual void AttachNativeView() OVERRIDE;
  virtual void NativeViewDetaching(bool destroyed) OVERRIDE;
  virtual void AddedToWidget() OVERRIDE;
  virtual void RemovedFromWidget() OVERRIDE;
  virtual void InstallClip(int x, int y, int w, int h) OVERRIDE;
  virtual bool HasInstalledClip() OVERRIDE;
  virtual void UninstallClip() OVERRIDE;
  virtual void ShowWidget(int x, int y, int w, int h) OVERRIDE;
  virtual void HideWidget() OVERRIDE;
  virtual void SetFocus() OVERRIDE;
  virtual gfx::NativeViewAccessible GetNativeViewAccessible() OVERRIDE;
  virtual gfx::NativeCursor GetCursor(int x, int y) OVERRIDE;

 private:
  friend class NativeViewHostAuraTest;

  class ClippingWindowDelegate;

  // Overridden from aura::WindowObserver:
  virtual void OnWindowDestroyed(aura::Window* window) OVERRIDE;

  // Reparents the native view with the clipping window existing between it and
  // its old parent, so that the fast resize path works.
  void AddClippingWindow();

  // If the native view has been reparented via AddClippingWindow, this call
  // undoes it.
  void RemoveClippingWindow();

  // Our associated NativeViewHost.
  NativeViewHost* host_;

  scoped_ptr<ClippingWindowDelegate> clipping_window_delegate_;

  // Window that exists between the native view and the parent that allows for
  // clipping to occur. This is positioned in the coordinate space of
  // host_->GetWidget().
  aura::Window clipping_window_;
  scoped_ptr<gfx::Rect> clip_rect_;

  DISALLOW_COPY_AND_ASSIGN(NativeViewHostAura);
};

}  // namespace views

#endif  // UI_VIEWS_CONTROLS_NATIVE_NATIVE_VIEW_HOST_AURA_H_
