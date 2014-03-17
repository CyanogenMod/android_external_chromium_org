// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ASH_FRAME_HEADER_PAINTER_UTIL_H_
#define ASH_FRAME_HEADER_PAINTER_UTIL_H_

#include "ash/ash_export.h"
#include "base/macros.h"

namespace gfx {
class FontList;
class Rect;
}
namespace views {
class View;
class Widget;
}

namespace ash {

// Static-only helper class for functionality used accross multiple
// implementations of HeaderPainter.
class ASH_EXPORT HeaderPainterUtil {
 public:
  // Returns the radius of the header's corners when the window is restored.
  static int GetTopCornerRadiusWhenRestored();

  // Returns the distance between the left edge of the window and the header
  // icon.
  static int GetIconXOffset();

  // Returns the size of the header icon.
  static int GetIconSize();

  // Returns the amount that the frame background is inset from the left edge of
  // the window.
  static int GetThemeBackgroundXInset();

  // Returns the bounds for the header's title given the header icon, the
  // caption button container and the font used.
  // |icon| should be NULL if the header does not use an icon.
  static gfx::Rect GetTitleBounds(const views::View* icon,
                                  const views::View* caption_button_container,
                                  const gfx::FontList& title_font_list);

  // Returns true if the header for |widget| can animate to new visuals when the
  // widget's activation changes. Returns false if the header should switch to
  // new visuals instantaneously.
  static bool CanAnimateActivation(views::Widget* widget);

 private:
  DISALLOW_IMPLICIT_CONSTRUCTORS(HeaderPainterUtil);
};

}  // namespace ash

#endif  // ASH_FRAME_HEADER_PAINTER_UTIL_H_
