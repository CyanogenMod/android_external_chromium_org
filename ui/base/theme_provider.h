// Copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef UI_BASE_THEME_PROVIDER_H_
#define UI_BASE_THEME_PROVIDER_H_
#pragma once

#include "base/basictypes.h"
#include "third_party/skia/include/core/SkColor.h"
#include "ui/base/ui_export.h"

#if defined(OS_MACOSX)
#ifdef __OBJC__
@class NSColor;
@class NSGradient;
@class NSImage;
#else
class NSColor;
class NSGradient;
class NSImage;
#endif  // __OBJC__
#elif !defined(OS_WIN)
typedef struct _GdkColor GdkColor;
typedef struct _GdkPixbuf GdkPixbuf;
#endif  // OS_*

class Profile;
class RefCountedMemory;
class SkBitmap;

namespace ui {

////////////////////////////////////////////////////////////////////////////////
//
// ThemeProvider
//
//   ThemeProvider is an abstract class that defines the API that should be
//   implemented to provide bitmaps and color information for a given theme.
//
////////////////////////////////////////////////////////////////////////////////

class UI_EXPORT ThemeProvider {
 public:
  virtual ~ThemeProvider();

  // TODO(beng): This dependency is horrible!
  // Initialize the provider with the passed in profile.
  virtual void Init(Profile* profile) = 0;

  // Get the bitmap specified by |id|. An implementation of ThemeProvider should
  // have its own source of ids (e.g. an enum, or external resource bundle).
  virtual SkBitmap* GetBitmapNamed(int id) const = 0;

  // Get the color specified by |id|.
  virtual SkColor GetColor(int id) const = 0;

  // Get the property (e.g. an alignment expressed in an enum, or a width or
  // height) specified by |id|.
  virtual bool GetDisplayProperty(int id, int* result) const = 0;

  // Whether we should use the native system frame (typically Aero glass) or
  // a custom frame.
  virtual bool ShouldUseNativeFrame() const = 0;

  // Whether or not we have a certain image. Used for when the default theme
  // doesn't provide a certain image, but custom themes might (badges, etc).
  virtual bool HasCustomImage(int id) const = 0;

  // Reads the image data from the theme file into the specified vector. Only
  // valid for un-themed resources and the themed IDR_THEME_NTP_* in most
  // implementations of ThemeProvider. Returns NULL on error.
  virtual RefCountedMemory* GetRawData(int id) const = 0;

#if defined(OS_MACOSX)
  // Gets the NSImage with the specified |id|.
  //
  // The bitmap is not assumed to exist. If a theme does not provide an image,
  // if |allow_default| is true, then the default image will be returned, else
  // this function will return nil.
  virtual NSImage* GetNSImageNamed(int id, bool allow_default) const = 0;

  // Gets the NSImage that GetNSImageNamed (above) would return, but returns it
  // as a pattern color.
  virtual NSColor* GetNSImageColorNamed(int id, bool allow_default) const = 0;

  // Gets the NSColor with the specified |id|.
  //
  // The color is not assumed to exist. If a theme does not provide an color, if
  // |allow_default| is true, then the default color will be returned, else this
  // function will return nil.
  virtual NSColor* GetNSColor(int id, bool allow_default) const = 0;

  // Gets the NSColor for tinting with the specified |id|.
  //
  // The tint is not assumed to exist. If a theme does not provide a tint with
  // that id, if |allow_default| is true, then the default tint will be
  // returned, else this function will return nil.
  virtual NSColor* GetNSColorTint(int id, bool allow_default) const = 0;

  // Gets the NSGradient with the specified |id|.
  virtual NSGradient* GetNSGradient(int id) const = 0;
#elif defined(OS_POSIX) && !defined(TOOLKIT_VIEWS)
  // Gets the GdkPixbuf with the specified |id|.  Returns a pointer to a shared
  // instance of the GdkPixbuf.  This shared GdkPixbuf is owned by the theme
  // provider and should not be freed.
  //
  // The bitmap is assumed to exist. This function will log in release, and
  // assert in debug mode if it does not. On failure, this will return a
  // pointer to a shared empty placeholder bitmap so it will be visible what
  // is missing.
  virtual GdkPixbuf* GetPixbufNamed(int id) const = 0;

  // As above, but flips it in RTL locales.
  virtual GdkPixbuf* GetRTLEnabledPixbufNamed(int id) const = 0;
#endif
};

}  // namespace ui

#endif  // UI_BASE_THEME_PROVIDER_H_
