// Copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "remoting/host/desktop_environment.h"

#include "base/bind.h"
#include "base/compiler_specific.h"
#include "remoting/host/capturer.h"
#include "remoting/host/chromoting_host.h"
#include "remoting/host/chromoting_host_context.h"
#include "remoting/host/continue_window.h"
#include "remoting/host/curtain.h"
#include "remoting/host/disconnect_window.h"
#include "remoting/host/event_executor.h"
#include "remoting/host/local_input_monitor.h"

// Milliseconds before the continue window is shown.
static const int kContinueWindowShowTimeoutMs = 10 * 60 * 1000;

// Milliseconds before the continue window is automatically dismissed and
// the connection is closed.
static const int kContinueWindowHideTimeoutMs = 60 * 1000;

namespace remoting {

// static
DesktopEnvironment* DesktopEnvironment::Create(ChromotingHostContext* context) {
  scoped_ptr<Capturer> capturer(Capturer::Create());
  scoped_ptr<EventExecutor> event_executor(
      EventExecutor::Create(context->desktop_message_loop(), capturer.get()));
  scoped_ptr<Curtain> curtain(Curtain::Create());
  scoped_ptr<DisconnectWindow> disconnect_window(DisconnectWindow::Create());
  scoped_ptr<ContinueWindow> continue_window(ContinueWindow::Create());
  scoped_ptr<LocalInputMonitor> local_input_monitor(
      LocalInputMonitor::Create());

  if (capturer.get() == NULL || event_executor.get() == NULL ||
      curtain.get() == NULL || disconnect_window.get() == NULL ||
      continue_window.get() == NULL || local_input_monitor.get() == NULL) {
    LOG(ERROR) << "Unable to create DesktopEnvironment";
    return NULL;
  }

  return new DesktopEnvironment(context,
                                capturer.release(),
                                event_executor.release(),
                                curtain.release(),
                                disconnect_window.release(),
                                continue_window.release(),
                                local_input_monitor.release());
}

DesktopEnvironment::DesktopEnvironment(ChromotingHostContext* context,
                                       Capturer* capturer,
                                       EventExecutor* event_executor,
                                       Curtain* curtain,
                                       DisconnectWindow* disconnect_window,
                                       ContinueWindow* continue_window,
                                       LocalInputMonitor* local_input_monitor)
    : host_(NULL),
      context_(context),
      capturer_(capturer),
      event_executor_(event_executor),
      curtain_(curtain),
      disconnect_window_(disconnect_window),
      continue_window_(continue_window),
      local_input_monitor_(local_input_monitor),
      is_monitoring_local_inputs_(false),
      continue_timer_state_(INACTIVE),
      ui_thread_proxy_(context->ui_message_loop()) {
}

DesktopEnvironment::~DesktopEnvironment() {
}

void DesktopEnvironment::Shutdown() {
  DCHECK(context_->ui_message_loop()->BelongsToCurrentThread());

  MonitorLocalInputs(false);
  ShowDisconnectWindow(false, std::string());
  ShowContinueWindow(false);
  StartContinueWindowTimer(false);

  ui_thread_proxy_.Detach();
}

void DesktopEnvironment::OnConnect(const std::string& username) {
  ui_thread_proxy_.PostTask(FROM_HERE, base::Bind(
      &DesktopEnvironment::ProcessOnConnect, base::Unretained(this), username));
}

void DesktopEnvironment::OnLastDisconnect() {
  ui_thread_proxy_.PostTask(FROM_HERE, base::Bind(
      &DesktopEnvironment::ProcessOnLastDisconnect, base::Unretained(this)));
}

void DesktopEnvironment::OnPause(bool pause) {
  ui_thread_proxy_.PostTask(FROM_HERE, base::Bind(
      &DesktopEnvironment::ProcessOnPause, base::Unretained(this), pause));
}

void DesktopEnvironment::ProcessOnConnect(const std::string& username) {
  DCHECK(context_->ui_message_loop()->BelongsToCurrentThread());

  MonitorLocalInputs(true);
  ShowDisconnectWindow(true, username);
  StartContinueWindowTimer(true);
}

void DesktopEnvironment::ProcessOnLastDisconnect() {
  DCHECK(context_->ui_message_loop()->BelongsToCurrentThread());

  MonitorLocalInputs(false);
  ShowDisconnectWindow(false, std::string());
  ShowContinueWindow(false);
  StartContinueWindowTimer(false);
}

void DesktopEnvironment::ProcessOnPause(bool pause) {
  if (!pause) {
    continue_timer_state_ = INACTIVE;
    StartContinueWindowTimer(true);
  }
}

void DesktopEnvironment::MonitorLocalInputs(bool enable) {
  DCHECK(context_->ui_message_loop()->BelongsToCurrentThread());

  if (enable == is_monitoring_local_inputs_)
    return;
  if (enable) {
    local_input_monitor_->Start(host_);
  } else {
    local_input_monitor_->Stop();
  }
  is_monitoring_local_inputs_ = enable;
}

void DesktopEnvironment::ShowDisconnectWindow(bool show,
                                              const std::string& username) {
  DCHECK(context_->ui_message_loop()->BelongsToCurrentThread());

  if (show) {
    disconnect_window_->Show(host_, username);
  } else {
    disconnect_window_->Hide();
  }
}

void DesktopEnvironment::ShowContinueWindow(bool show) {
  DCHECK(context_->ui_message_loop()->BelongsToCurrentThread());

  if (show) {
    continue_window_->Show(host_);
  } else {
    continue_window_->Hide();
  }
}

void DesktopEnvironment::StartContinueWindowTimer(bool start) {
  DCHECK(context_->ui_message_loop()->BelongsToCurrentThread());

  if (start && continue_timer_state_ == INACTIVE) {
    continue_timer_target_time_ = base::Time::Now() +
        base::TimeDelta::FromMilliseconds(kContinueWindowShowTimeoutMs);
    ui_thread_proxy_.PostDelayedTask(
        FROM_HERE, base::Bind(&DesktopEnvironment::ContinueWindowTimerFunc,
                              base::Unretained(this)),
        kContinueWindowShowTimeoutMs);
    continue_timer_state_ = SHOW_DIALOG;
  } else if (!start) {
    continue_timer_state_ = INACTIVE;
  }

}

void DesktopEnvironment::ContinueWindowTimerFunc() {
  DCHECK(context_->ui_message_loop()->BelongsToCurrentThread());

  // This function may be called prematurely if timer was stopped and
  // then started again. In that case we just ignore this call.
  if (continue_timer_target_time_ > base::Time::Now())
    return;

  switch (continue_timer_state_) {
    case INACTIVE:
      // This function will still be called, even if the timeout was cancelled.
      return;
    case SHOW_DIALOG:
      host_->PauseSession(true);
      ShowContinueWindow(true);
      continue_timer_target_time_ = base::Time::Now() +
          base::TimeDelta::FromMilliseconds(kContinueWindowHideTimeoutMs);
      ui_thread_proxy_.PostDelayedTask(
          FROM_HERE, base::Bind(&DesktopEnvironment::ContinueWindowTimerFunc,
                                base::Unretained(this)),
          kContinueWindowHideTimeoutMs);
      continue_timer_state_ = SHUTDOWN_HOST;
      break;
    case SHUTDOWN_HOST:
      continue_timer_state_ = INACTIVE;
      ShowContinueWindow(false);
      host_->Shutdown(NULL);
      break;
  }
}


}  // namespace remoting
