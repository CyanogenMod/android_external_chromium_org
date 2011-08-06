// Copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef UI_BASE_ANIMATION_ANIMATION_CONTAINER_ELEMENT_H_
#define UI_BASE_ANIMATION_ANIMATION_CONTAINER_ELEMENT_H_
#pragma once

#include "base/time.h"
#include "ui/base/ui_export.h"

namespace ui {

// Interface for the elements the AnimationContainer contains. This is
// implemented by Animation.
class UI_EXPORT AnimationContainerElement {
 public:
  // Sets the start of the animation. This is invoked from
  // AnimationContainer::Start.
  virtual void SetStartTime(base::TimeTicks start_time) = 0;

  // Invoked when the animation is to progress.
  virtual void Step(base::TimeTicks time_now) = 0;

  // Returns the time interval of the animation. If an Element needs to change
  // this it should first invoke Stop, then Start.
  virtual base::TimeDelta GetTimerInterval() const = 0;

 protected:
  virtual ~AnimationContainerElement() {}
};

}  // namespace ui

#endif  // UI_BASE_ANIMATION_ANIMATION_CONTAINER_ELEMENT_H_
