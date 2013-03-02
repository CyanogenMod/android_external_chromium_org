// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef REMOTING_HOST_HOST_EVENT_LOGGER_H_
#define REMOTING_HOST_HOST_EVENT_LOGGER_H_

#include <string>

#include "base/basictypes.h"
#include "base/compiler_specific.h"
#include "base/memory/scoped_ptr.h"
#include "base/memory/weak_ptr.h"

namespace remoting {

class HostStatusMonitor;

class HostEventLogger {
 public:
  virtual ~HostEventLogger() {}

  // Creates an event-logger that monitors host status changes and logs
  // corresponding events to the OS-specific log (syslog/EventLog).
  static scoped_ptr<HostEventLogger> Create(
      base::WeakPtr<HostStatusMonitor> monitor,
      const std::string& application_name);

 protected:
  HostEventLogger() {}

 private:
  DISALLOW_COPY_AND_ASSIGN(HostEventLogger);
};

}

#endif  // REMOTING_HOST_HOST_EVENT_LOGGER_H_
