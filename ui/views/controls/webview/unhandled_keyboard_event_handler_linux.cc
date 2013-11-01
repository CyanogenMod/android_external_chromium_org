// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ui/views/controls/webview/unhandled_keyboard_event_handler.h"

#include "base/logging.h"
#include "content/public/browser/native_web_keyboard_event.h"
#include "ui/events/event.h"
#include "ui/views/focus/focus_manager.h"

using content::NativeWebKeyboardEvent;

namespace views {

UnhandledKeyboardEventHandler::UnhandledKeyboardEventHandler()
    : ignore_next_char_event_(false) {
}

void UnhandledKeyboardEventHandler::HandleKeyboardEvent(
    const NativeWebKeyboardEvent& event,
    FocusManager* focus_manager) {
  if (!focus_manager) {
    NOTREACHED();
    return;
  }

  // Previous calls to TranslateMessage can generate Char events as well as
  // RawKeyDown events, even if the latter triggered an accelerator.  In these
  // cases, we discard the Char events.
  if (event.type == WebKit::WebInputEvent::Char && ignore_next_char_event_) {
    ignore_next_char_event_ = false;
    return;
  }
  // It's necessary to reset this flag, because a RawKeyDown event may not
  // always generate a Char event.
  ignore_next_char_event_ = false;

  if (event.type == WebKit::WebInputEvent::RawKeyDown) {
    ui::Accelerator accelerator(
        static_cast<ui::KeyboardCode>(event.windowsKeyCode),
        content::GetModifiersFromNativeWebKeyboardEvent(event));

    // This is tricky: we want to set ignore_next_char_event_ if
    // ProcessAccelerator returns true. But ProcessAccelerator might delete
    // |this| if the accelerator is a "close tab" one. So we speculatively
    // set the flag and fix it if no event was handled.
    ignore_next_char_event_ = true;

    if (focus_manager->ProcessAccelerator(accelerator)) {
      return;
    }

    // ProcessAccelerator didn't handle the accelerator, so we know both
    // that |this| is still valid, and that we didn't want to set the flag.
    ignore_next_char_event_ = false;
  }

  if (event.os_event && !event.skip_in_browser)
    focus_manager->OnKeyEvent(*static_cast<ui::KeyEvent*>(event.os_event));
}

}  // namespace views
