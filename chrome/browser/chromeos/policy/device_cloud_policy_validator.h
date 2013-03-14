// Copyright (c) 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_CHROMEOS_POLICY_DEVICE_CLOUD_POLICY_VALIDATOR_H_
#define CHROME_BROWSER_CHROMEOS_POLICY_DEVICE_CLOUD_POLICY_VALIDATOR_H_

#include "chrome/browser/policy/cloud/cloud_policy_validator.h"

namespace enterprise_management {
class ChromeDeviceSettingsProto;
}

namespace policy {

typedef CloudPolicyValidator<enterprise_management::ChromeDeviceSettingsProto>
    DeviceCloudPolicyValidator;

}  // namespace policy

#endif  // CHROME_BROWSER_CHROMEOS_POLICY_DEVICE_CLOUD_POLICY_VALIDATOR_H_
