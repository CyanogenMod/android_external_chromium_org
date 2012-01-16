// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_CHROMEOS_DBUS_IMAGE_BURNER_CLIENT_H_
#define CHROME_BROWSER_CHROMEOS_DBUS_IMAGE_BURNER_CLIENT_H_
#pragma once

#include <string>

#include "base/basictypes.h"
#include "base/callback.h"

namespace dbus {
class Bus;
}

namespace chromeos {

// ImageBurnerClient is used to communicate with the image burner.
// All method should be called from the origin thread (UI thread) which
// initializes the DBusThreadManager instance.
class ImageBurnerClient {
 public:
  virtual ~ImageBurnerClient();

  // A callback to be called when DBus method call fails.
  typedef base::Callback<void()> ErrorCallback;

  // A callback to handle burn_finished signal.
  typedef base::Callback<void(const std::string& target_path,
                              bool success,
                              const std::string& error)> BurnFinishedHandler;

  // A callback to handle burn_progress_update signal.
  typedef base::Callback<void(const std::string& target_path,
                              int64 num_bytes_burnt,
                              int64 total_size)> BurnProgressUpdateHandler;

  // Burns the image |from_path| to the disk |to_path|.
  virtual void BurnImage(const std::string& from_path,
                         const std::string& to_path,
                         ErrorCallback error_callback) = 0;

  // Sets callbacks as event handlers.
  // |burn_finished_handler| is called when burn_finished signal is received.
  // |burn_progress_update_handler| is called when burn_progress_update signal
  // is received.
  virtual void SetEventHandlers(
      BurnFinishedHandler burn_finished_handler,
      BurnProgressUpdateHandler burn_progress_update_handler) = 0;

  // Resets event handlers. After calling this method, nothing is done when
  // signals are received.
  virtual void ResetEventHandlers() = 0;

  // Factory function, creates a new instance and returns ownership.
  // For normal usage, access the singleton via DBusThreadManager::Get().
  static ImageBurnerClient* Create(dbus::Bus* bus);

 protected:
  // Create() should be used instead.
  ImageBurnerClient();

 private:
  DISALLOW_COPY_AND_ASSIGN(ImageBurnerClient);
};

}  // namespace chromeos

#endif  // CHROME_BROWSER_CHROMEOS_DBUS_IMAGE_BURNER_CLIENT_H_
