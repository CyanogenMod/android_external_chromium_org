// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "remoting/protocol/input_event_tracker.h"

#include "base/logging.h"
#include "remoting/proto/event.pb.h"

namespace remoting {
namespace protocol {

InputEventTracker::InputEventTracker(InputStub* input_stub)
    : input_stub_(input_stub),
      mouse_pos_(SkIPoint::Make(0, 0)),
      mouse_button_state_(0) {
}

InputEventTracker::~InputEventTracker() {
}

bool InputEventTracker::IsKeyPressed(uint32 usb_keycode) const {
  return pressed_keys_.find(usb_keycode) != pressed_keys_.end();
}

int InputEventTracker::PressedKeyCount() const {
  return pressed_keys_.size() + pressed_vkeys_.size();
}

void InputEventTracker::ReleaseAll() {
  std::set<uint32>::iterator i;
  for (i = pressed_keys_.begin(); i != pressed_keys_.end(); ++i) {
    KeyEvent event;
    event.set_pressed(false);
    event.set_usb_keycode(*i);
    input_stub_->InjectKeyEvent(event);
  }
  pressed_keys_.clear();

  std::set<int>::iterator j;
  for (j = pressed_vkeys_.begin(); j != pressed_vkeys_.end(); ++j) {
    KeyEvent event;
    event.set_pressed(false);
    event.set_keycode(*j);
    input_stub_->InjectKeyEvent(event);
  }
  pressed_vkeys_.clear();

  for (int i = MouseEvent::BUTTON_UNDEFINED + 1;
       i < MouseEvent::BUTTON_MAX; ++i) {
    if (mouse_button_state_ & (1 << (i - 1))) {
      MouseEvent mouse;

      // TODO(wez): EventInjectors should cope with positionless events by
      // using the current cursor position, and we wouldn't set position here.
      mouse.set_x(mouse_pos_.x());
      mouse.set_y(mouse_pos_.y());

      mouse.set_button((MouseEvent::MouseButton)i);
      mouse.set_button_down(false);
      input_stub_->InjectMouseEvent(mouse);
    }
  }
  mouse_button_state_ = 0;
}

void InputEventTracker::InjectKeyEvent(const KeyEvent& event) {
  if (event.has_pressed()) {
    if (event.has_usb_keycode()) {
      if (event.pressed()) {
        pressed_keys_.insert(event.usb_keycode());
      } else {
        pressed_keys_.erase(event.usb_keycode());
      }
    } else if (event.has_keycode()) {
      if (event.pressed()) {
        pressed_vkeys_.insert(event.keycode());
      } else {
        pressed_vkeys_.erase(event.keycode());
      }
    }
  }
  input_stub_->InjectKeyEvent(event);
}

void InputEventTracker::InjectMouseEvent(const MouseEvent& event) {
  if (event.has_x() && event.has_y()) {
    mouse_pos_ = SkIPoint::Make(event.x(), event.y());
  }
  if (event.has_button() && event.has_button_down()) {
    // Button values are defined in remoting/proto/event.proto.
    if (event.button() >= 1 && event.button() < MouseEvent::BUTTON_MAX) {
      uint32 button_change = 1 << (event.button() - 1);
      if (event.button_down()) {
        mouse_button_state_ |= button_change;
      } else {
        mouse_button_state_ &= ~button_change;
      }
    }
  }
  input_stub_->InjectMouseEvent(event);
}

}  // namespace protocol
}  // namespace remoting
