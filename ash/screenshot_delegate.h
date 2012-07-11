// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ASH_SCREENSHOT_DELEGATE_H_
#define ASH_SCREENSHOT_DELEGATE_H_

namespace aura {
class Window;
}  // namespace aura

namespace gfx {
class Rect;
}  // namespace gfx

namespace ash {

// Delegate for taking screenshots.
class ScreenshotDelegate {
 public:
  virtual ~ScreenshotDelegate() {}

  // The actual task of taking a screenshot for the given window.
  // This method is called when the user wants to take a screenshot
  // manually.
  virtual void HandleTakeScreenshot(aura::Window* window) = 0;

  // The actual task of taking a partial screenshot for the given
  // window.
  virtual void HandleTakePartialScreenshot(
      aura::Window* window, const gfx::Rect& rect) = 0;
};
}  // namespace ash

#endif  // ASH_SCREENSHOT_DELEGATE_H_
