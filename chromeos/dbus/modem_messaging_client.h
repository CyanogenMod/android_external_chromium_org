// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROMEOS_DBUS_MODEM_MESSAGING_CLIENT_H_
#define CHROMEOS_DBUS_MODEM_MESSAGING_CLIENT_H_
#pragma once

#include <string>
#include <vector>

#include "base/basictypes.h"
#include "base/callback.h"
#include "chromeos/chromeos_export.h"
#include "chromeos/dbus/dbus_client_implementation_type.h"

namespace dbus {
class Bus;
class ObjectPath;
}

namespace chromeos {

// ModemMessagingClient is used to communicate with the
// org.freedesktop.ModemManager1.Modem.Messaging service.  All methods
// should be called from the origin thread (UI thread) which
// initializes the DBusThreadManager instance.
class CHROMEOS_EXPORT ModemMessagingClient {
 public:
  typedef base::Callback<void()> DeleteCallback;
  typedef base::Callback<void(const dbus::ObjectPath& message_path,
                              bool complete)> SmsReceivedHandler;
  typedef base::Callback<void(const std::vector<dbus::ObjectPath>& paths)>
      ListCallback;

  virtual ~ModemMessagingClient();

  // Factory function, creates a new instance and returns ownership.
  // For normal usage, access the singleton via DBusThreadManager::Get().
  static ModemMessagingClient* Create(DBusClientImplementationType type,
                                      dbus::Bus* bus);

  // Sets SmsReceived signal handler.
  virtual void SetSmsReceivedHandler(const std::string& service_name,
                                     const dbus::ObjectPath& object_path,
                                     const SmsReceivedHandler& handler) = 0;

  // Resets SmsReceived signal handler.
  virtual void ResetSmsReceivedHandler(const std::string& service_name,
                                       const dbus::ObjectPath& object_path) = 0;

  // Calls Delete method.  |callback| is called after the method call succeeds.
  virtual void Delete(const std::string& service_name,
                      const dbus::ObjectPath& object_path,
                      const dbus::ObjectPath& sms_path,
                      const DeleteCallback& callback) = 0;

  // Calls List method.  |callback| is called after the method call succeeds.
  virtual void List(const std::string& service_name,
                    const dbus::ObjectPath& object_path,
                    const ListCallback& callback) = 0;

 protected:
  // Create() should be used instead.
  ModemMessagingClient();

 private:
  DISALLOW_COPY_AND_ASSIGN(ModemMessagingClient);
};

}  // namespace chromeos

#endif  // CHROMEOS_DBUS_MODEM_MESSAGING_CLIENT_H_
