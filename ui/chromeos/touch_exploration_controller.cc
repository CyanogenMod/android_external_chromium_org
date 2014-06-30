// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ui/chromeos/touch_exploration_controller.h"

#include "base/logging.h"
#include "base/strings/string_number_conversions.h"
#include "ui/aura/client/cursor_client.h"
#include "ui/aura/window.h"
#include "ui/aura/window_event_dispatcher.h"
#include "ui/aura/window_tree_host.h"
#include "ui/events/event.h"
#include "ui/events/event_processor.h"

#define VLOG_STATE() if (VLOG_IS_ON(0)) VlogState(__func__)
#define VLOG_EVENT(event) if (VLOG_IS_ON(0)) VlogEvent(event, __func__)

namespace ui {

TouchExplorationController::TouchExplorationController(
    aura::Window* root_window)
    : root_window_(root_window),
      state_(NO_FINGERS_DOWN),
      event_handler_for_testing_(NULL),
      prev_state_(NO_FINGERS_DOWN) {
  CHECK(root_window);
  root_window->GetHost()->GetEventSource()->AddEventRewriter(this);
}


TouchExplorationController::~TouchExplorationController() {
  root_window_->GetHost()->GetEventSource()->RemoveEventRewriter(this);
}

void TouchExplorationController::CallTapTimerNowForTesting() {
  DCHECK(tap_timer_.IsRunning());
  tap_timer_.Stop();
  OnTapTimerFired();
}

void TouchExplorationController::SetEventHandlerForTesting(
    ui::EventHandler* event_handler_for_testing) {
  event_handler_for_testing_ = event_handler_for_testing;
}

bool TouchExplorationController::IsInNoFingersDownStateForTesting() const {
  return state_ == NO_FINGERS_DOWN;
}

ui::EventRewriteStatus TouchExplorationController::RewriteEvent(
    const ui::Event& event,
    scoped_ptr<ui::Event>* rewritten_event) {
  if (!event.IsTouchEvent()) {
    if (event.IsKeyEvent()) {
      const ui::KeyEvent& key_event = static_cast<const ui::KeyEvent&>(event);
      VLOG(0) << "\nKeyboard event: " << key_event.name() << "\n"
              << " Key code: " << key_event.key_code()
              << ", Flags: " << key_event.flags()
              << ", Is char: " << key_event.is_char();
    }
    if(event.IsGestureEvent()){
      VLOG(0) << "\n Gesture event " << event.name();
    }
    return ui::EVENT_REWRITE_CONTINUE;
  }
  const ui::TouchEvent& touch_event = static_cast<const ui::TouchEvent&>(event);

  // If the tap timer should have fired by now but hasn't, run it now and
  // stop the timer. This is important so that behavior is consistent with
  // the timestamps of the events, and not dependent on the granularity of
  // the timer.
  if (tap_timer_.IsRunning() &&
      touch_event.time_stamp() - initial_press_->time_stamp() >
          gesture_detector_config_.double_tap_timeout) {
    tap_timer_.Stop();
    OnTapTimerFired();
    // Note: this may change the state. We should now continue and process
    // this event under this new state.
  }

  const ui::EventType type = touch_event.type();
  const gfx::PointF& location = touch_event.location_f();
  const int touch_id = touch_event.touch_id();

  // Always update touch ids and touch locations, so we can use those
  // no matter what state we're in.
  if (type == ui::ET_TOUCH_PRESSED) {
    current_touch_ids_.push_back(touch_id);
    touch_locations_.insert(std::pair<int, gfx::PointF>(touch_id, location));
  } else if (type == ui::ET_TOUCH_RELEASED || type == ui::ET_TOUCH_CANCELLED) {
    std::vector<int>::iterator it = std::find(
        current_touch_ids_.begin(), current_touch_ids_.end(), touch_id);

    // Can happen if touch exploration is enabled while fingers were down.
    if (it == current_touch_ids_.end())
      return ui::EVENT_REWRITE_CONTINUE;

    current_touch_ids_.erase(it);
    touch_locations_.erase(touch_id);
  } else if (type == ui::ET_TOUCH_MOVED) {
    std::vector<int>::iterator it = std::find(
        current_touch_ids_.begin(), current_touch_ids_.end(), touch_id);

    // Can happen if touch exploration is enabled while fingers were down.
    if (it == current_touch_ids_.end())
      return ui::EVENT_REWRITE_CONTINUE;

    touch_locations_[*it] = location;
  }
  VLOG_STATE();
  VLOG_EVENT(touch_event);
  // The rest of the processing depends on what state we're in.
  switch(state_) {
    case NO_FINGERS_DOWN:
      return InNoFingersDown(touch_event, rewritten_event);
    case SINGLE_TAP_PRESSED:
      return InSingleTapPressed(touch_event, rewritten_event);
    case SINGLE_TAP_RELEASED:
    case TOUCH_EXPLORE_RELEASED:
      return InSingleTapOrTouchExploreReleased(touch_event, rewritten_event);
    case DOUBLE_TAP_PRESSED:
      return InDoubleTapPressed(touch_event, rewritten_event);
    case TOUCH_EXPLORATION:
      return InTouchExploration(touch_event, rewritten_event);
    case TOUCH_EXPLORE_SECOND_PRESS:
      return InTouchExploreSecondPress(touch_event, rewritten_event);
    case TWO_TO_ONE_FINGER:
      return InTwoToOneFinger(touch_event, rewritten_event);
    case PASSTHROUGH:
      return InPassthrough(touch_event, rewritten_event);
    case WAIT_FOR_RELEASE:
      return InWaitForRelease(touch_event, rewritten_event);
  }
  NOTREACHED();
  return ui::EVENT_REWRITE_CONTINUE;
}

ui::EventRewriteStatus TouchExplorationController::NextDispatchEvent(
    const ui::Event& last_event, scoped_ptr<ui::Event>* new_event) {
  NOTREACHED();
  return ui::EVENT_REWRITE_CONTINUE;
}

ui::EventRewriteStatus TouchExplorationController::InNoFingersDown(
    const ui::TouchEvent& event, scoped_ptr<ui::Event>* rewritten_event) {
  const ui::EventType type = event.type();
  if (type == ui::ET_TOUCH_PRESSED) {
    initial_press_.reset(new TouchEvent(event));
    last_unused_finger_event_.reset(new TouchEvent(event));
    tap_timer_.Start(FROM_HERE,
                     gesture_detector_config_.double_tap_timeout,
                     this,
                     &TouchExplorationController::OnTapTimerFired);
    state_ = SINGLE_TAP_PRESSED;
    VLOG_STATE();
    return ui::EVENT_REWRITE_DISCARD;
  }
  NOTREACHED() << "Unexpected event type received.";
  return ui::EVENT_REWRITE_CONTINUE;
}

ui::EventRewriteStatus TouchExplorationController::InSingleTapPressed(
    const ui::TouchEvent& event, scoped_ptr<ui::Event>* rewritten_event) {
  const ui::EventType type = event.type();

  if (type == ui::ET_TOUCH_PRESSED) {
    // Adding a second finger within the timeout period switches to
    // passing through every event from the second finger and none form the
    // first. The event from the first finger is still saved in initial_press_.
    state_ = TWO_TO_ONE_FINGER;
    last_two_to_one_.reset(new TouchEvent(event));
    rewritten_event->reset(new ui::TouchEvent(ui::ET_TOUCH_PRESSED,
                                              event.location(),
                                              event.touch_id(),
                                              event.time_stamp()));
    (*rewritten_event)->set_flags(event.flags());
    return EVENT_REWRITE_REWRITTEN;
  } else if (type == ui::ET_TOUCH_RELEASED || type == ui::ET_TOUCH_CANCELLED) {
    DCHECK_EQ(0U, current_touch_ids_.size());
    state_ = SINGLE_TAP_RELEASED;
    VLOG_STATE();
    return EVENT_REWRITE_DISCARD;
  } else if (type == ui::ET_TOUCH_MOVED) {
    // If the user moves far enough from the initial touch location (outside
    // the "slop" region, jump to the touch exploration mode early.
    // TODO(evy, lisayin): Add gesture recognition here instead -
    // we should probably jump to gesture mode here if the velocity is
    // high enough, and touch exploration if the velocity is lower.
    float delta = (event.location() - initial_press_->location()).Length();
    if (delta > gesture_detector_config_.touch_slop) {
      EnterTouchToMouseMode();
      state_ = TOUCH_EXPLORATION;
      VLOG_STATE();
      return InTouchExploration(event, rewritten_event);
    }
    return EVENT_REWRITE_DISCARD;
  }
  NOTREACHED() << "Unexpected event type received.";
  return ui::EVENT_REWRITE_CONTINUE;
}

ui::EventRewriteStatus
TouchExplorationController::InSingleTapOrTouchExploreReleased(
    const ui::TouchEvent& event,
    scoped_ptr<ui::Event>* rewritten_event) {
  const ui::EventType type = event.type();
  if (type == ui::ET_TOUCH_PRESSED) {
    // This is the second tap in a double-tap (or double tap-hold).
    // Rewrite at location of last touch exploration.
    // If there is no touch exploration yet, discard instead.
    if (!last_touch_exploration_) {
      return ui::EVENT_REWRITE_DISCARD;
    }
    rewritten_event->reset(
        new ui::TouchEvent(ui::ET_TOUCH_PRESSED,
                           last_touch_exploration_->location(),
                           event.touch_id(),
                           event.time_stamp()));
    (*rewritten_event)->set_flags(event.flags());
    state_ = DOUBLE_TAP_PRESSED;
    VLOG_STATE();
    return ui::EVENT_REWRITE_REWRITTEN;
  } else if (type == ui::ET_TOUCH_RELEASED && !last_touch_exploration_) {
    // If the previous press was discarded, we need to also handle its
    // release.
    if (current_touch_ids_.size() == 0) {
      ResetToNoFingersDown();
    }
    return ui::EVENT_REWRITE_DISCARD;
  }
  NOTREACHED() << "Unexpected event type received.";
  return ui::EVENT_REWRITE_CONTINUE;
}

ui::EventRewriteStatus TouchExplorationController::InDoubleTapPressed(
    const ui::TouchEvent& event, scoped_ptr<ui::Event>* rewritten_event) {
  const ui::EventType type = event.type();
  if (type == ui::ET_TOUCH_PRESSED) {
    return ui::EVENT_REWRITE_DISCARD;
  } else if (type == ui::ET_TOUCH_RELEASED || type == ui::ET_TOUCH_CANCELLED) {
    if (current_touch_ids_.size() != 0)
      return EVENT_REWRITE_DISCARD;

    // Rewrite release at location of last touch exploration with the same
    // id as the prevoius press.
    rewritten_event->reset(
        new ui::TouchEvent(ui::ET_TOUCH_RELEASED,
                           last_touch_exploration_->location(),
                           initial_press_->touch_id(),
                           event.time_stamp()));
    (*rewritten_event)->set_flags(event.flags());
    ResetToNoFingersDown();
    return ui::EVENT_REWRITE_REWRITTEN;
  } else if (type == ui::ET_TOUCH_MOVED) {
    return ui::EVENT_REWRITE_DISCARD;
  }
  NOTREACHED() << "Unexpected event type received.";
  return ui::EVENT_REWRITE_CONTINUE;
}

ui::EventRewriteStatus TouchExplorationController::InTouchExploration(
    const ui::TouchEvent& event,
    scoped_ptr<ui::Event>* rewritten_event) {
  const ui::EventType type = event.type();
  if (type == ui::ET_TOUCH_PRESSED) {
    // Handle split-tap.
    initial_press_.reset(new TouchEvent(event));
    if (tap_timer_.IsRunning())
      tap_timer_.Stop();
    rewritten_event->reset(
        new ui::TouchEvent(ui::ET_TOUCH_PRESSED,
                           last_touch_exploration_->location(),
                           event.touch_id(),
                           event.time_stamp()));
    (*rewritten_event)->set_flags(event.flags());
    state_ = TOUCH_EXPLORE_SECOND_PRESS;
    VLOG_STATE();
    return ui::EVENT_REWRITE_REWRITTEN;
  } else if (type == ui::ET_TOUCH_RELEASED || type == ui::ET_TOUCH_CANCELLED) {
    initial_press_.reset(new TouchEvent(event));
    tap_timer_.Start(FROM_HERE,
                     gesture_detector_config_.double_tap_timeout,
                     this,
                     &TouchExplorationController::OnTapTimerFired);
    state_ = TOUCH_EXPLORE_RELEASED;
    VLOG_STATE();
  } else if (type != ui::ET_TOUCH_MOVED) {
    NOTREACHED() << "Unexpected event type received.";
    return ui::EVENT_REWRITE_CONTINUE;
  }

  // Rewrite as a mouse-move event.
  *rewritten_event = CreateMouseMoveEvent(event.location(), event.flags());
  last_touch_exploration_.reset(new TouchEvent(event));
  return ui::EVENT_REWRITE_REWRITTEN;
}


ui::EventRewriteStatus TouchExplorationController::InTwoToOneFinger(
    const ui::TouchEvent& event,
    scoped_ptr<ui::Event>* rewritten_event) {
  // The user should only ever be in TWO_TO_ONE_FINGER with two fingers down.
  // If the user added or removed a finger, the state is changed.
  ui::EventType type = event.type();
  if (type == ui::ET_TOUCH_RELEASED || type == ui::ET_TOUCH_CANCELLED) {
    DCHECK(current_touch_ids_.size() == 1);
    // Stop passing through the second finger and go to the wait state.
    if (current_touch_ids_.size() == 1) {
      rewritten_event->reset(new ui::TouchEvent(ui::ET_TOUCH_RELEASED,
                                                last_two_to_one_->location(),
                                                last_two_to_one_->touch_id(),
                                                event.time_stamp()));
      (*rewritten_event)->set_flags(event.flags());
      state_ = WAIT_FOR_RELEASE;
      return ui::EVENT_REWRITE_REWRITTEN;
    }
  } else if (type == ui::ET_TOUCH_PRESSED) {
    DCHECK(current_touch_ids_.size() == 3);
    // If a third finger is pressed, we are now going into passthrough mode
    // and now need to dispatch the first finger into a press, as well as the
    // recent press.
    if (current_touch_ids_.size() == 3){
      state_ = PASSTHROUGH;
      DispatchEvent(new ui::TouchEvent(ui::ET_TOUCH_PRESSED,
                                       last_unused_finger_event_->location(),
                                       last_unused_finger_event_->touch_id(),
                                       event.time_stamp()));
      rewritten_event->reset(new ui::TouchEvent(ui::ET_TOUCH_PRESSED,
                                                event.location(),
                                                event.touch_id(),
                                                event.time_stamp()));
      (*rewritten_event)->set_flags(event.flags());
      return ui::EVENT_REWRITE_REWRITTEN;
    }
  } else if (type == ui::ET_TOUCH_MOVED) {
    DCHECK(current_touch_ids_.size() == 2);
    // The first finger should have no events pass through, but for a proper
    // conversion to passthrough, the press of the initial finger should
    // be updated.
    if (event.touch_id() == last_unused_finger_event_->touch_id()) {
      last_unused_finger_event_.reset(new TouchEvent(event));
      return ui::EVENT_REWRITE_DISCARD;
    }
    if (event.touch_id() == last_two_to_one_->touch_id()) {
      last_two_to_one_.reset(new TouchEvent(event));
      rewritten_event->reset(new ui::TouchEvent(ui::ET_TOUCH_MOVED,
                                                event.location(),
                                                event.touch_id(),
                                                event.time_stamp()));
      (*rewritten_event)->set_flags(event.flags());
      return ui::EVENT_REWRITE_REWRITTEN;
    }
  }
  NOTREACHED() << "Unexpected event type received";
  return ui::EVENT_REWRITE_CONTINUE;
}

ui::EventRewriteStatus TouchExplorationController::InPassthrough(
    const ui::TouchEvent& event,
    scoped_ptr<ui::Event>* rewritten_event) {
  ui::EventType type = event.type();

  if (!(type == ui::ET_TOUCH_RELEASED || type == ui::ET_TOUCH_CANCELLED ||
        type == ui::ET_TOUCH_MOVED || type == ui::ET_TOUCH_PRESSED)) {
    NOTREACHED() << "Unexpected event type received.";
    return ui::EVENT_REWRITE_CONTINUE;
  }

  rewritten_event->reset(new ui::TouchEvent(
      type, event.location(), event.touch_id(), event.time_stamp()));
  (*rewritten_event)->set_flags(event.flags());

  if (current_touch_ids_.size() == 0) {
    ResetToNoFingersDown();
  }

  return ui::EVENT_REWRITE_REWRITTEN;
}

ui::EventRewriteStatus TouchExplorationController::InTouchExploreSecondPress(
    const ui::TouchEvent& event,
    scoped_ptr<ui::Event>* rewritten_event) {
  ui::EventType type = event.type();
  gfx::PointF location = event.location_f();
  if (type == ui::ET_TOUCH_PRESSED) {
    return ui::EVENT_REWRITE_DISCARD;
  } else if (type == ui::ET_TOUCH_MOVED) {
    // Currently this is a discard, but could be something like rotor
    // in the future.
    return ui::EVENT_REWRITE_DISCARD;
  } else if (type == ui::ET_TOUCH_RELEASED || type == ui::ET_TOUCH_CANCELLED) {
    // If the touch exploration finger is lifted, there is no option to return
    // to touch explore anymore. The remaining finger acts as a pending
    // tap or long tap for the last touch explore location.
    if (event.touch_id() == last_touch_exploration_->touch_id()){
      state_ = DOUBLE_TAP_PRESSED;
      VLOG_STATE();
      return EVENT_REWRITE_DISCARD;
    }

    // Continue to release the touch only if the touch explore finger is the
    // only finger remaining.
    if (current_touch_ids_.size() != 1)
      return EVENT_REWRITE_DISCARD;

    // Rewrite at location of last touch exploration.
    rewritten_event->reset(
        new ui::TouchEvent(ui::ET_TOUCH_RELEASED,
                           last_touch_exploration_->location(),
                           initial_press_->touch_id(),
                           event.time_stamp()));
    (*rewritten_event)->set_flags(event.flags());
    state_ = TOUCH_EXPLORATION;
    VLOG_STATE();
    return ui::EVENT_REWRITE_REWRITTEN;
  }
  NOTREACHED() << "Unexpected event type received.";
  return ui::EVENT_REWRITE_CONTINUE;
}

ui::EventRewriteStatus TouchExplorationController::InWaitForRelease(
    const ui::TouchEvent& event,
    scoped_ptr<ui::Event>* rewritten_event) {
  ui::EventType type = event.type();
  if (!(type == ui::ET_TOUCH_PRESSED || type == ui::ET_TOUCH_MOVED ||
        type == ui::ET_TOUCH_RELEASED || type == ui::ET_TOUCH_CANCELLED)) {
    NOTREACHED() << "Unexpected event type received.";
    return ui::EVENT_REWRITE_CONTINUE;
  }
  if (current_touch_ids_.size() == 0) {
    state_ = NO_FINGERS_DOWN;
    ResetToNoFingersDown();
  }
  return EVENT_REWRITE_DISCARD;
}

void TouchExplorationController::OnTapTimerFired() {
  switch (state_) {
    case SINGLE_TAP_RELEASED:
      ResetToNoFingersDown();
      break;
    case TOUCH_EXPLORE_RELEASED:
      ResetToNoFingersDown();
      last_touch_exploration_.reset(new TouchEvent(*initial_press_));
      return;
    case SINGLE_TAP_PRESSED:
      EnterTouchToMouseMode();
      state_ = TOUCH_EXPLORATION;
      VLOG_STATE();
      break;
    default:
      return;
  }

  scoped_ptr<ui::Event> mouse_move = CreateMouseMoveEvent(
      initial_press_->location(), initial_press_->flags());
  DispatchEvent(mouse_move.get());
  last_touch_exploration_.reset(new TouchEvent(*initial_press_));
}

void TouchExplorationController::DispatchEvent(ui::Event* event) {
  if (event_handler_for_testing_) {
    event_handler_for_testing_->OnEvent(event);
    return;
  }
  ui::EventDispatchDetails result ALLOW_UNUSED =
      root_window_->GetHost()->dispatcher()->OnEventFromSource(event);
}

scoped_ptr<ui::Event> TouchExplorationController::CreateMouseMoveEvent(
    const gfx::PointF& location,
    int flags) {
  return scoped_ptr<ui::Event>(
      new ui::MouseEvent(
          ui::ET_MOUSE_MOVED,
          location,
          location,
          flags | ui::EF_IS_SYNTHESIZED | ui::EF_TOUCH_ACCESSIBILITY,
          0));
}

void TouchExplorationController::EnterTouchToMouseMode() {
  aura::client::CursorClient* cursor_client =
      aura::client::GetCursorClient(root_window_);
  if (cursor_client && !cursor_client->IsMouseEventsEnabled())
    cursor_client->EnableMouseEvents();
  if (cursor_client && cursor_client->IsCursorVisible())
    cursor_client->HideCursor();
}

void TouchExplorationController::ResetToNoFingersDown() {
  state_ = NO_FINGERS_DOWN;
  VLOG_STATE();
  if (tap_timer_.IsRunning())
    tap_timer_.Stop();
}

void TouchExplorationController::VlogState(const char* function_name) {
  if (prev_state_ == state_)
    return;
  prev_state_ = state_;
  const char* state_string = EnumStateToString(state_);
  VLOG(0) << "\n Function name: " << function_name
          << "\n State: " << state_string;
}

void TouchExplorationController::VlogEvent(const ui::TouchEvent& touch_event,
                                           const char* function_name) {
  CHECK(touch_event.IsTouchEvent());
  if (prev_event_ != NULL &&
      prev_event_->type() == touch_event.type() &&
      prev_event_->touch_id() == touch_event.touch_id()){
    return;
  }
  // The above statement prevents events of the same type and id from being
  // printed in a row. However, if two fingers are down, they would both be
  // moving and alternating printing move events unless we check for this.
  if (prev_event_ != NULL &&
      prev_event_->type() == ET_TOUCH_MOVED &&
      touch_event.type() == ET_TOUCH_MOVED){
    return;
  }
  const std::string& type = touch_event.name();
  const gfx::PointF& location = touch_event.location_f();
  const int touch_id = touch_event.touch_id();

  VLOG(0) << "\n Function name: " << function_name
          << "\n Event Type: " << type
          << "\n Location: " << location.ToString()
          << "\n Touch ID: " << touch_id;
  prev_event_.reset(new TouchEvent(touch_event));
}

const char* TouchExplorationController::EnumStateToString(State state) {
  switch (state) {
    case NO_FINGERS_DOWN:
      return "NO_FINGERS_DOWN";
    case SINGLE_TAP_PRESSED:
      return "SINGLE_TAP_PRESSED";
    case SINGLE_TAP_RELEASED:
      return "SINGLE_TAP_RELEASED";
    case TOUCH_EXPLORE_RELEASED:
      return "TOUCH_EXPLORE_RELEASED";
    case DOUBLE_TAP_PRESSED:
      return "DOUBLE_TAP_PRESSED";
    case TOUCH_EXPLORATION:
      return "TOUCH_EXPLORATION";
    case TOUCH_EXPLORE_SECOND_PRESS:
      return "TOUCH_EXPLORE_SECOND_PRESS";
    case TWO_TO_ONE_FINGER:
      return "TWO_TO_ONE_FINGER";
    case PASSTHROUGH:
      return "PASSTHROUGH";
    case WAIT_FOR_RELEASE:
      return "WAIT_FOR_RELEASE";
  }
  return "Not a state";
}

}  // namespace ui
