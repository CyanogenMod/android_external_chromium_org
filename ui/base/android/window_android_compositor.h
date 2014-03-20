// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef UI_BASE_ANDROID_WINDOW_ANDROID_COMPOSITOR_H_
#define UI_BASE_ANDROID_WINDOW_ANDROID_COMPOSITOR_H_

#include "ui/base/ui_base_export.h"

namespace cc {
class Layer;
}

namespace ui {

// Android interface for compositor-related tasks.
class UI_BASE_EXPORT WindowAndroidCompositor {
 public:
  virtual ~WindowAndroidCompositor() {}

  virtual void AttachLayerForReadback(scoped_refptr<cc::Layer> layer) = 0;
};

}  // namespace ui

#endif  // UI_BASE_ANDROID_WINDOW_ANDROID_COMPOSITOR_H_
