// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef UI_VIEWS_CONTROLS_BUTTON_BLUE_BUTTON_H_
#define UI_VIEWS_CONTROLS_BUTTON_BLUE_BUTTON_H_

#include "base/compiler_specific.h"
#include "base/strings/string16.h"
#include "ui/views/controls/button/label_button.h"

namespace views {

// A class representing a blue button.
class VIEWS_EXPORT BlueButton : public LabelButton {
 public:
  static const char kViewClassName[];

  BlueButton(ButtonListener* listener, const base::string16& text);
  virtual ~BlueButton();

 private:
  // Overridden from LabelButton:
  virtual const char* GetClassName() const OVERRIDE;
  virtual const ui::Animation* GetThemeAnimation() const OVERRIDE;

  DISALLOW_COPY_AND_ASSIGN(BlueButton);
};

}  // namespace views

#endif  // UI_VIEWS_CONTROLS_BUTTON_BLUE_BUTTON_H_
