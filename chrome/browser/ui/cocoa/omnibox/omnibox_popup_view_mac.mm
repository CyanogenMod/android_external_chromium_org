// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/ui/cocoa/omnibox/omnibox_popup_view_mac.h"

#include <cmath>

#include "base/stl_util.h"
#include "base/strings/sys_string_conversions.h"
#include "chrome/browser/autocomplete/autocomplete_match.h"
#include "chrome/browser/search/search.h"
#include "chrome/browser/ui/cocoa/browser_window_controller.h"
#import "chrome/browser/ui/cocoa/omnibox/omnibox_popup_cell.h"
#import "chrome/browser/ui/cocoa/omnibox/omnibox_popup_matrix.h"
#include "chrome/browser/ui/cocoa/omnibox/omnibox_view_mac.h"
#include "chrome/browser/ui/omnibox/omnibox_edit_model.h"
#include "chrome/browser/ui/omnibox/omnibox_popup_model.h"
#include "chrome/browser/ui/omnibox/omnibox_popup_non_view.h"
#include "grit/theme_resources.h"
#include "skia/ext/skia_utils_mac.h"
#import "third_party/GTM/AppKit/GTMNSAnimation+Duration.h"
#import "ui/base/cocoa/cocoa_event_utils.h"
#import "ui/base/cocoa/flipped_view.h"
#include "ui/base/cocoa/window_size_constants.h"
#include "ui/base/resource/resource_bundle.h"
#include "ui/base/text/text_elider.h"
#include "ui/gfx/rect.h"
#include "ui/gfx/scoped_ns_graphics_context_save_gstate_mac.h"

namespace {

// How much to adjust the cell sizing up from the default determined
// by the font.
const CGFloat kCellHeightAdjust = 6.0;

// Padding between matrix and the top and bottom of the popup window.
const CGFloat kPopupPaddingVertical = 5.0;

// How far to offset the text column from the left.
const CGFloat kTextXOffset = 28.0;

// Animation duration when animating the popup window smaller.
const NSTimeInterval kShrinkAnimationDuration = 0.1;

// Maximum fraction of the popup width that can be used to display match
// contents.
const CGFloat kMaxContentsFraction = 0.7;

// Background colors for different states of the popup elements.
NSColor* BackgroundColor() {
  return [NSColor controlBackgroundColor];
}

NSColor* ContentTextColor() {
  return [NSColor blackColor];
}
NSColor* DimContentTextColor() {
  return [NSColor darkGrayColor];
}
NSColor* URLTextColor() {
  return [NSColor colorWithCalibratedRed:0.0 green:0.55 blue:0.0 alpha:1.0];
}

}  // namespace

// static
OmniboxPopupView* OmniboxPopupViewMac::Create(OmniboxView* omnibox_view,
                                              OmniboxEditModel* edit_model,
                                              NSTextField* field) {
#if defined(HTML_INSTANT_EXTENDED_POPUP)
  if (chrome::IsInstantExtendedAPIEnabled())
    return new OmniboxPopupNonView(edit_model);
#endif
  return new OmniboxPopupViewMac(omnibox_view, edit_model, field);
}


OmniboxPopupViewMac::OmniboxPopupViewMac(OmniboxView* omnibox_view,
                                         OmniboxEditModel* edit_model,
                                         NSTextField* field)
    : omnibox_view_(omnibox_view),
      model_(new OmniboxPopupModel(this, edit_model)),
      field_(field),
      popup_(nil),
      targetPopupFrame_(NSZeroRect) {
  DCHECK(omnibox_view);
  DCHECK(edit_model);
}

OmniboxPopupViewMac::~OmniboxPopupViewMac() {
  // Destroy the popup model before this object is destroyed, because
  // it can call back to us in the destructor.
  model_.reset();

  // Break references to |this| because the popup may not be
  // deallocated immediately.
  [autocomplete_matrix_ setPopupView:NULL];
}

bool OmniboxPopupViewMac::IsOpen() const {
  return popup_ != nil;
}

void OmniboxPopupViewMac::UpdatePopupAppearance() {
  DCHECK([NSThread isMainThread]);
  const AutocompleteResult& result = model_->result();
  if (result.empty()) {
    [[popup_ parentWindow] removeChildWindow:popup_];
    [popup_ orderOut:nil];

    // Break references to |this| because the popup may not be
    // deallocated immediately.
    [autocomplete_matrix_ setPopupView:NULL];
    autocomplete_matrix_.reset();

    popup_.reset(nil);

    targetPopupFrame_ = NSZeroRect;

    return;
  }

  CreatePopupIfNeeded();

  // The popup's font is a slightly smaller version of the field's.
  NSFont* fieldFont = OmniboxViewMac::GetFieldFont();
  const CGFloat resultFontSize = [fieldFont pointSize];
  gfx::Font resultFont(base::SysNSStringToUTF8([fieldFont fontName]),
                       static_cast<int>(resultFontSize));

  // Calculate the width of the matrix based on backing out the
  // popup's border from the width of the field.
  const CGFloat matrixWidth = NSWidth([field_ bounds]);
  DCHECK_GT(matrixWidth, 0.0);

  // Load the results into the popup's matrix.
  const size_t rows = model_->result().size();
  DCHECK_GT(rows, 0U);
  [autocomplete_matrix_ renewRows:rows columns:1];
  for (size_t ii = 0; ii < rows; ++ii) {
    OmniboxPopupCell* cell = [autocomplete_matrix_ cellAtRow:ii column:0];
    const AutocompleteMatch& match = model_->result().match_at(ii);
    [cell setImage:ImageForMatch(match)];
    [cell setAttributedTitle:MatchText(match, resultFont, matrixWidth)];
  }

  // Set the cell size to fit a line of text in the cell's font.  All
  // cells should use the same font and each should layout in one
  // line, so they should all be about the same height.
  const NSSize cellSize =
      [[autocomplete_matrix_ cellAtRow:0 column:0] cellSize];
  DCHECK_GT(cellSize.height, 0.0);
  const CGFloat cellHeight = cellSize.height + kCellHeightAdjust;
  [autocomplete_matrix_ setCellSize:NSMakeSize(matrixWidth, cellHeight)];

  // Update the selection before placing (and displaying) the window.
  PaintUpdatesNow();

  // Calculate the matrix size manually rather than using -sizeToCells
  // because actually resizing the matrix messed up the popup size
  // animation.
  DCHECK_EQ([autocomplete_matrix_ intercellSpacing].height, 0.0);
  PositionPopup(rows * cellHeight);
}

gfx::Rect OmniboxPopupViewMac::GetTargetBounds() {
  // Flip the coordinate system before returning.
  NSScreen* screen = [[NSScreen screens] objectAtIndex:0];
  NSRect monitorFrame = [screen frame];
  gfx::Rect bounds(NSRectToCGRect(targetPopupFrame_));
  bounds.set_y(monitorFrame.size.height - bounds.y() - bounds.height());
  return bounds;
}

// This is only called by model in SetSelectedLine() after updating
// everything.  Popup should already be visible.
void OmniboxPopupViewMac::PaintUpdatesNow() {
  [autocomplete_matrix_ selectCellAtRow:model_->selected_line() column:0];
}

void OmniboxPopupViewMac::SetSelectedLine(size_t line) {
  model_->SetSelectedLine(line, false, false);
}

void OmniboxPopupViewMac::OpenURLForRow(int row, bool force_background) {
  DCHECK_GE(row, 0);

  WindowOpenDisposition disposition = NEW_BACKGROUND_TAB;
  if (!force_background) {
    disposition = ui::WindowOpenDispositionFromNSEvent([NSApp currentEvent]);
  }

  // OpenMatch() may close the popup, which will clear the result set
  // and, by extension, |match| and its contents.  So copy the
  // relevant match out to make sure it stays alive until the call
  // completes.
  AutocompleteMatch match = model_->result().match_at(row);
  omnibox_view_->OpenMatch(match, disposition, GURL(), row);
}

// Return the text to show for the match, based on the match's
// contents and description.  Result will be in |font|, with the
// boldfaced version used for matches.
NSAttributedString* OmniboxPopupViewMac::MatchText(
    const AutocompleteMatch& match, gfx::Font& font, float cellWidth) {
  NSMutableAttributedString *as =
      DecorateMatchedString(match.contents,
                            match.contents_class,
                            ContentTextColor(),
                            DimContentTextColor(),
                            font);

  // If there is a description, append it, separated from the contents
  // with an en dash, and decorated with a distinct color.
  if (!match.description.empty()) {
    // Make sure the current string fits w/in kMaxContentsFraction of
    // the cell to make sure the description will be at least
    // partially visible.
    // TODO(shess): Consider revising our NSCell subclass to have two
    // bits and just draw them right, rather than truncating here.
    const float textWidth = cellWidth - kTextXOffset;
    as = ElideString(as, match.contents, font,
                     textWidth * kMaxContentsFraction);

    NSDictionary* attributes =
        [NSDictionary dictionaryWithObjectsAndKeys:
             font.GetNativeFont(), NSFontAttributeName,
             ContentTextColor(), NSForegroundColorAttributeName,
             nil];
    NSString* rawEnDash = @" \u2013 ";
    NSAttributedString* enDash =
        [[[NSAttributedString alloc] initWithString:rawEnDash
                                         attributes:attributes] autorelease];

    // In Windows, a boolean force_dim is passed as true for the
    // description.  Here, we pass the dim text color for both normal and dim,
    // to accomplish the same thing.
    NSAttributedString* description =
        DecorateMatchedString(match.description, match.description_class,
                              DimContentTextColor(),
                              DimContentTextColor(),
                              font);

    [as appendAttributedString:enDash];
    [as appendAttributedString:description];
  }

  NSMutableParagraphStyle* style =
      [[[NSMutableParagraphStyle alloc] init] autorelease];
  [style setLineBreakMode:NSLineBreakByTruncatingTail];
  [style setTighteningFactorForTruncation:0.0];
  [as addAttribute:NSParagraphStyleAttributeName value:style
             range:NSMakeRange(0, [as length])];

  return as;
}

// Helper for MatchText() to allow sharing code between the contents
// and description cases.  Returns NSMutableAttributedString as a
// convenience for MatchText().
NSMutableAttributedString* OmniboxPopupViewMac::DecorateMatchedString(
    const string16 &matchString,
    const AutocompleteMatch::ACMatchClassifications &classifications,
    NSColor* textColor, NSColor* dimTextColor, gfx::Font& font) {
  // Cache for on-demand computation of the bold version of |font|.
  NSFont* boldFont = nil;

  // Start out with a string using the default style info.
  NSString* s = base::SysUTF16ToNSString(matchString);
  NSDictionary* attributes = [NSDictionary dictionaryWithObjectsAndKeys:
                                  font.GetNativeFont(), NSFontAttributeName,
                                  textColor, NSForegroundColorAttributeName,
                                  nil];
  NSMutableAttributedString* as =
      [[[NSMutableAttributedString alloc] initWithString:s
                                              attributes:attributes]
        autorelease];

  // As a protective measure, bail if the length of the match string is not
  // the same as the length of the converted NSString. http://crbug.com/121703
  if ([s length] != matchString.size())
    return as;

  // Mark up the runs which differ from the default.
  for (ACMatchClassifications::const_iterator i = classifications.begin();
       i != classifications.end(); ++i) {
    const BOOL isLast = (i+1) == classifications.end();
    const NSInteger nextOffset =
        (isLast ? [s length] : static_cast<NSInteger>((i + 1)->offset));
    const NSInteger location = static_cast<NSInteger>(i->offset);
    const NSInteger length = nextOffset - static_cast<NSInteger>(i->offset);
    // Guard against bad, off-the-end classification ranges.
    if (i->offset >= [s length] || length <= 0)
      break;
    const NSRange range = NSMakeRange(location,
        MIN(length, static_cast<NSInteger>([s length]) - location));

    if (0 != (i->style & ACMatchClassification::URL)) {
      [as addAttribute:NSForegroundColorAttributeName
                 value:URLTextColor() range:range];
    }

    if (0 != (i->style & ACMatchClassification::MATCH)) {
      if (!boldFont) {
        NSFontManager* fontManager = [NSFontManager sharedFontManager];
        boldFont = [fontManager convertFont:font.GetNativeFont()
                                toHaveTrait:NSBoldFontMask];
      }
      [as addAttribute:NSFontAttributeName value:boldFont range:range];
    }

    if (0 != (i->style & ACMatchClassification::DIM)) {
      [as addAttribute:NSForegroundColorAttributeName
                 value:dimTextColor
                 range:range];
    }
  }

  return as;
}

NSMutableAttributedString* OmniboxPopupViewMac::ElideString(
    NSMutableAttributedString* aString,
    const string16 originalString,
    const gfx::Font& font,
    const float width) {
  // If it already fits, nothing to be done.
  if ([aString size].width <= width) {
    return aString;
  }

  // If ElideText() decides to do nothing, nothing to be done.
  const string16 elided =
      ui::ElideText(originalString, font, width, ui::ELIDE_AT_END);
  if (0 == elided.compare(originalString)) {
    return aString;
  }

  // If everything was elided away, clear the string.
  if (elided.empty()) {
    [aString deleteCharactersInRange:NSMakeRange(0, [aString length])];
    return aString;
  }

  // The ellipses should be the last character, and everything before
  // that should match the original string.
  const size_t i(elided.length() - 1);
  DCHECK_NE(0, elided.compare(0, i, originalString));

  // Replace the end of |aString| with the ellipses from |elided|.
  NSString* s = base::SysUTF16ToNSString(elided.substr(i));
  [aString replaceCharactersInRange:NSMakeRange(i, [aString length] - i)
                         withString:s];

  return aString;
}

void OmniboxPopupViewMac::CreatePopupIfNeeded() {
  if (!popup_) {
    popup_.reset(
        [[NSWindow alloc] initWithContentRect:ui::kWindowSizeDeterminedLater
                                    styleMask:NSBorderlessWindowMask
                                      backing:NSBackingStoreBuffered
                                        defer:YES]);
    [popup_ setMovableByWindowBackground:NO];
    // The window shape is determined by the content view.
    [popup_ setAlphaValue:1.0];
    [popup_ setOpaque:YES];
    [popup_ setBackgroundColor:BackgroundColor()];
    [popup_ setHasShadow:YES];
    [popup_ setLevel:NSNormalWindowLevel];
    // Use a flipped view to pin the matrix top the top left. This is needed
    // for animated resize.
    base::scoped_nsobject<FlippedView> contentView(
        [[FlippedView alloc] initWithFrame:NSZeroRect]);
    [popup_ setContentView:contentView];

    autocomplete_matrix_.reset(
        [[OmniboxPopupMatrix alloc] initWithPopupView:this]);
    [contentView addSubview:autocomplete_matrix_];

    // TODO(dtseng): Ignore until we provide NSAccessibility support.
    [popup_ accessibilitySetOverrideValue:NSAccessibilityUnknownRole
        forAttribute:NSAccessibilityRoleAttribute];
  }
}

void OmniboxPopupViewMac::PositionPopup(const CGFloat matrixHeight) {
  BrowserWindowController* controller =
      [BrowserWindowController browserWindowControllerForView:field_];
  NSRect anchorRectBase = [controller omniboxPopupAnchorRect];

  // Calculate the popup's position on the screen.
  NSRect popupFrame = anchorRectBase;
  // Size to fit the matrix and shift down by the size.
  popupFrame.size.height = matrixHeight + kPopupPaddingVertical * 2.0;
  popupFrame.origin.y -= NSHeight(popupFrame);
  // Shift to screen coordinates.
  popupFrame.origin =
      [[controller window] convertBaseToScreen:popupFrame.origin];

  // Do nothing if the popup is already animating to the given |frame|.
  if (NSEqualRects(popupFrame, targetPopupFrame_))
    return;

  NSPoint fieldOriginBase =
      [field_ convertPoint:[field_ bounds].origin toView:nil];
  NSRect matrixFrame = NSZeroRect;
  matrixFrame.origin.x = fieldOriginBase.x - NSMinX(anchorRectBase);
  matrixFrame.origin.y = kPopupPaddingVertical;
  matrixFrame.size.width = [autocomplete_matrix_ cellSize].width;
  matrixFrame.size.height = matrixHeight;
  [autocomplete_matrix_ setFrame:matrixFrame];

  NSRect currentPopupFrame = [popup_ frame];
  targetPopupFrame_ = popupFrame;

  // Animate the frame change if the only change is that the height got smaller.
  // Otherwise, resize immediately.
  bool animate = (NSHeight(popupFrame) < NSHeight(currentPopupFrame) &&
                  NSWidth(popupFrame) == NSWidth(currentPopupFrame));

  base::scoped_nsobject<NSDictionary> savedAnimations;
  if (!animate) {
    // In an ideal world, running a zero-length animation would cancel any
    // running animations and set the new frame value immediately.  In practice,
    // zero-length animations are ignored entirely.  Work around this AppKit bug
    // by explicitly setting an NSNull animation for the "frame" key and then
    // running the animation with a non-zero(!!) duration.  This somehow
    // convinces AppKit to do the right thing.  Save off the current animations
    // dictionary so it can be restored later.
    savedAnimations.reset([[popup_ animations] copy]);
    [popup_ setAnimations:
              [NSDictionary dictionaryWithObjectsAndKeys:[NSNull null],
                                                         @"frame", nil]];
  }

  [NSAnimationContext beginGrouping];
  // Don't use the GTM additon for the "Steve" slowdown because this can happen
  // async from user actions and the effects could be a surprise.
  [[NSAnimationContext currentContext] setDuration:kShrinkAnimationDuration];
  [[popup_ animator] setFrame:popupFrame display:YES];
  [NSAnimationContext endGrouping];

  if (!animate) {
    // Restore the original animations dictionary.  This does not reinstate any
    // previously running animations.
    [popup_ setAnimations:savedAnimations];
  }

  if (![popup_ isVisible])
    [[field_ window] addChildWindow:popup_ ordered:NSWindowAbove];
}

NSImage* OmniboxPopupViewMac::ImageForMatch(const AutocompleteMatch& match) {
  gfx::Image image = model_->GetIconIfExtensionMatch(match);
  if (!image.IsEmpty())
    return image.AsNSImage();

  const int resource_id = match.starred ?
      IDR_OMNIBOX_STAR : AutocompleteMatch::TypeToIcon(match.type);
  return OmniboxViewMac::ImageForResource(resource_id);
}
