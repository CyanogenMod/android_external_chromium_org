// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef UI_VIEWS_CONTROLS_LABEL_H_
#define UI_VIEWS_CONTROLS_LABEL_H_

#include <string>
#include <vector>

#include "base/compiler_specific.h"
#include "base/gtest_prod_util.h"
#include "base/strings/string16.h"
#include "third_party/skia/include/core/SkColor.h"
#include "ui/gfx/font_list.h"
#include "ui/gfx/text_constants.h"
#include "ui/views/view.h"

namespace views {

/////////////////////////////////////////////////////////////////////////////
//
// Label class
//
// A label is a view subclass that can display a string.
//
/////////////////////////////////////////////////////////////////////////////
class VIEWS_EXPORT Label : public View {
 public:
  // The following enum is used to indicate whether using the Chrome UI's
  // directionality as the label's directionality, or auto-detecting the label's
  // directionality.
  //
  // If the label text originates from the Chrome UI, we should use the Chrome
  // UI's directionality as the label's directionality.
  //
  // If the text originates from a web page, its directionality is determined
  // based on its first character with strong directionality, disregarding what
  // directionality the Chrome UI is.
  enum DirectionalityMode {
    USE_UI_DIRECTIONALITY = 0,
    AUTO_DETECT_DIRECTIONALITY
  };

  // Internal class name.
  static const char kViewClassName[];

  // The padding for the focus border when rendering focused text.
  static const int kFocusBorderPadding;

  Label();
  explicit Label(const base::string16& text);
  Label(const base::string16& text, const gfx::FontList& font_list);
  virtual ~Label();

  // Gets or sets the fonts used by this label.
  const gfx::FontList& font_list() const { return font_list_; }
  virtual void SetFontList(const gfx::FontList& font_list);

  // Get or set the label text.
  const base::string16& text() const { return text_; }
  virtual void SetText(const base::string16& text);

  // Enables or disables auto-color-readability (enabled by default).  If this
  // is enabled, then calls to set any foreground or background color will
  // trigger an automatic mapper that uses color_utils::GetReadableColor() to
  // ensure that the foreground colors are readable over the background color.
  void SetAutoColorReadabilityEnabled(bool enabled);

  // Sets the color.  This will automatically force the color to be readable
  // over the current background color.
  virtual void SetEnabledColor(SkColor color);
  void SetDisabledColor(SkColor color);

  SkColor enabled_color() const { return actual_enabled_color_; }

  // Sets the background color.  This won't be explicitly drawn, but the label
  // will force the text color to be readable over it.
  void SetBackgroundColor(SkColor color);
  SkColor background_color() const { return background_color_; }

  // Enables a drop shadow underneath the text.
  void SetShadowColors(SkColor enabled_color, SkColor disabled_color);

  // Sets the drop shadow's offset from the text.
  void SetShadowOffset(int x, int y);

  // Sets the shadow blur. Default is zero.
  void set_shadow_blur(double shadow_blur) { shadow_blur_ = shadow_blur; }

  // Disables shadows.
  void ClearEmbellishing();

  // Set the color of a halo on the painted text (use transparent for none).
  void set_halo_color(SkColor halo_color) { halo_color_ = halo_color; }

  // Sets horizontal alignment. If the locale is RTL, and the directionality
  // mode is USE_UI_DIRECTIONALITY, the alignment is flipped around.
  //
  // Caveat: for labels originating from a web page, the directionality mode
  // should be reset to AUTO_DETECT_DIRECTIONALITY before the horizontal
  // alignment is set. Otherwise, the label's alignment specified as a parameter
  // will be flipped in RTL locales.
  void SetHorizontalAlignment(gfx::HorizontalAlignment alignment);

  gfx::HorizontalAlignment horizontal_alignment() const {
    return horizontal_alignment_;
  }

  // Sets the directionality mode. The directionality mode is initialized to
  // USE_UI_DIRECTIONALITY when the label is constructed. USE_UI_DIRECTIONALITY
  // applies to every label that originates from the Chrome UI. However, if the
  // label originates from a web page, its directionality is auto-detected.
  void set_directionality_mode(DirectionalityMode mode) {
    directionality_mode_ = mode;
  }

  DirectionalityMode directionality_mode() const {
    return directionality_mode_;
  }

  // Get or set the distance in pixels between baselines of multi-line text.
  // Default is 0, indicating the distance between lines should be the standard
  // one for the label's text, font list, and platform.
  int line_height() const { return line_height_; }
  void SetLineHeight(int height);

  // Get or set if the label text can wrap on multiple lines; default is false.
  bool is_multi_line() const { return is_multi_line_; }
  void SetMultiLine(bool multi_line);

  // Get or set if the label text should be obscured before rendering (e.g.
  // should "Password!" display as "*********"); default is false.
  bool is_obscured() const { return is_obscured_; }
  void SetObscured(bool obscured);

  // Get the text as displayed to the user, respecting the 'obscured' flag.
  const base::string16& layout_text() const { return layout_text_; }

  // Sets whether the label text can be split on words.
  // Default is false. This only works when is_multi_line is true.
  void SetAllowCharacterBreak(bool allow_character_break);

  // Sets the eliding or fading behavior, applied as necessary. The default is
  // to elide at the end. Eliding is not well supported for multi-line labels.
  void SetElideBehavior(gfx::ElideBehavior elide_behavior);

  // Sets the tooltip text.  Default behavior for a label (single-line) is to
  // show the full text if it is wider than its bounds.  Calling this overrides
  // the default behavior and lets you set a custom tooltip.  To revert to
  // default behavior, call this with an empty string.
  void SetTooltipText(const base::string16& tooltip_text);

  // Resizes the label so its width is set to the width of the longest line and
  // its height deduced accordingly.
  // This is only intended for multi-line labels and is useful when the label's
  // text contains several lines separated with \n.
  // |max_width| is the maximum width that will be used (longer lines will be
  // wrapped).  If 0, no maximum width is enforced.
  void SizeToFit(int max_width);

  // Gets/sets the flag to determine whether the label should be collapsed when
  // it's hidden (not visible). If this flag is true, the label will return a
  // preferred size of (0, 0) when it's not visible.
  void set_collapse_when_hidden(bool value) { collapse_when_hidden_ = value; }
  bool collapse_when_hidden() const { return collapse_when_hidden_; }

  // View:
  virtual gfx::Insets GetInsets() const OVERRIDE;
  virtual int GetBaseline() const OVERRIDE;
  // Overridden to compute the size required to display this label.
  virtual gfx::Size GetPreferredSize() const OVERRIDE;
  // Returns the width of an ellipsis if the label is non-empty, or 0 otherwise.
  virtual gfx::Size GetMinimumSize() const OVERRIDE;
  // Returns the height necessary to display this label with the provided width.
  // This method is used to layout multi-line labels. It is equivalent to
  // GetPreferredSize().height() if the receiver is not multi-line.
  virtual int GetHeightForWidth(int w) const OVERRIDE;
  virtual const char* GetClassName() const OVERRIDE;
  virtual View* GetTooltipHandlerForPoint(const gfx::Point& point) OVERRIDE;
  virtual bool CanProcessEventsWithinSubtree() const OVERRIDE;
  virtual void GetAccessibleState(ui::AXViewState* state) OVERRIDE;
  // Gets the tooltip text for labels that are wider than their bounds, except
  // when the label is multiline, in which case it just returns false (no
  // tooltip).  If a custom tooltip has been specified with SetTooltipText()
  // it is returned instead.
  virtual bool GetTooltipText(const gfx::Point& p,
                              base::string16* tooltip) const OVERRIDE;

 protected:
  // Called by Paint to paint the text.  Override this to change how
  // text is painted.
  virtual void PaintText(gfx::Canvas* canvas,
                         const base::string16& text,
                         const gfx::Rect& text_bounds,
                         int flags);

  virtual gfx::Size GetTextSize() const;

  SkColor disabled_color() const { return actual_disabled_color_; }

  // Overridden from View:
  // Overridden to dirty our text bounds if we're multi-line.
  virtual void OnBoundsChanged(const gfx::Rect& previous_bounds) OVERRIDE;
  virtual void OnPaint(gfx::Canvas* canvas) OVERRIDE;
  virtual void OnNativeThemeChanged(const ui::NativeTheme* theme) OVERRIDE;

 private:
  // These tests call CalculateDrawStringParams in order to verify the
  // calculations done for drawing text.
  FRIEND_TEST_ALL_PREFIXES(LabelTest, DrawSingleLineString);
  FRIEND_TEST_ALL_PREFIXES(LabelTest, DrawMultiLineString);
  FRIEND_TEST_ALL_PREFIXES(LabelTest, DrawSingleLineStringInRTL);
  FRIEND_TEST_ALL_PREFIXES(LabelTest, DrawMultiLineStringInRTL);
  FRIEND_TEST_ALL_PREFIXES(LabelTest, AutoDetectDirectionality);

  // Calls ComputeDrawStringFlags().
  FRIEND_TEST_ALL_PREFIXES(LabelTest, DisableSubpixelRendering);

  // Sets both |text_| and |layout_text_| to appropriate values, taking
  // the label's 'obscured' status into account.
  void SetTextInternal(const base::string16& text);

  void Init(const base::string16& text, const gfx::FontList& font_list);

  void RecalculateColors();

  // Returns where the text is drawn, in the receivers coordinate system.
  gfx::Rect GetTextBounds() const;

  int ComputeDrawStringFlags() const;

  gfx::Rect GetAvailableRect() const;

  // Returns parameters to be used for the DrawString call.
  void CalculateDrawStringParams(base::string16* paint_text,
                                 gfx::Rect* text_bounds,
                                 int* flags) const;

  // Updates any colors that have not been explicitly set from the theme.
  void UpdateColorsFromTheme(const ui::NativeTheme* theme);

  // Resets |cached_heights_| and |cached_heights_cursor_| and mark
  // |text_size_valid_| as false.
  void ResetCachedSize();

  bool ShouldShowDefaultTooltip() const;

  base::string16 text_;
  base::string16 layout_text_;
  gfx::FontList font_list_;
  SkColor requested_enabled_color_;
  SkColor actual_enabled_color_;
  SkColor requested_disabled_color_;
  SkColor actual_disabled_color_;
  SkColor background_color_;

  // Set to true once the corresponding setter is invoked.
  bool enabled_color_set_;
  bool disabled_color_set_;
  bool background_color_set_;

  bool auto_color_readability_;
  mutable gfx::Size text_size_;
  mutable bool text_size_valid_;
  int line_height_;
  bool is_multi_line_;
  bool is_obscured_;
  bool allow_character_break_;
  gfx::ElideBehavior elide_behavior_;
  gfx::HorizontalAlignment horizontal_alignment_;
  base::string16 tooltip_text_;
  // Whether to collapse the label when it's not visible.
  bool collapse_when_hidden_;
  // The following member variable is used to control whether the
  // directionality is auto-detected based on first strong directionality
  // character or is determined by chrome UI's locale.
  DirectionalityMode directionality_mode_;

  // Colors for shadow.
  SkColor enabled_shadow_color_;
  SkColor disabled_shadow_color_;

  // Space between text and shadow.
  gfx::Point shadow_offset_;

  // Should a shadow be drawn behind the text?
  bool has_shadow_;

  // Indicates the level of shadow blurring. Default is zero.
  double shadow_blur_;

  // The halo color drawn around the text if it is not transparent.
  SkColor halo_color_;

  // The cached heights to avoid recalculation in GetHeightForWidth().
  mutable std::vector<gfx::Size> cached_heights_;
  mutable int cached_heights_cursor_;

  DISALLOW_COPY_AND_ASSIGN(Label);
};

}  // namespace views

#endif  // UI_VIEWS_CONTROLS_LABEL_H_
