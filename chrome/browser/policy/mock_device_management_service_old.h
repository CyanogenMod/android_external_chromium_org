// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_POLICY_MOCK_DEVICE_MANAGEMENT_SERVICE_OLD_H_
#define CHROME_BROWSER_POLICY_MOCK_DEVICE_MANAGEMENT_SERVICE_OLD_H_
#pragma once

#include <string>

#include "base/basictypes.h"
#include "chrome/browser/policy/device_management_backend.h"
#include "chrome/browser/policy/device_management_service.h"
#include "chrome/browser/policy/proto/device_management_backend.pb.h"
#include "testing/gmock/include/gmock/gmock.h"

namespace policy {

// This proxy class is used so that expectations can be defined for a single
// persistent instance of DMBackend while the DeviceTokenFetcher under test
// merrily creates and destroys proxies.
class ProxyDeviceManagementBackend : public DeviceManagementBackend {
 public:
  explicit ProxyDeviceManagementBackend(DeviceManagementBackend* backend);
  virtual ~ProxyDeviceManagementBackend();

  // DeviceRegisterRequest implementation:
  virtual void ProcessRegisterRequest(
      const std::string& gaia_auth_token,
      const std::string& oauth_token,
      const std::string& device_id,
      const enterprise_management::DeviceRegisterRequest& request,
      DeviceRegisterResponseDelegate* delegate) OVERRIDE;
  virtual void ProcessUnregisterRequest(
      const std::string& device_management_token,
      const std::string& device_id,
      const enterprise_management::DeviceUnregisterRequest& request,
      DeviceUnregisterResponseDelegate* delegate) OVERRIDE;
  virtual void ProcessPolicyRequest(
      const std::string& device_management_token,
      const std::string& device_id,
      CloudPolicyDataStore::UserAffiliation affiliation,
      const enterprise_management::DevicePolicyRequest& request,
      const enterprise_management::DeviceStatusReportRequest* device_status,
      DevicePolicyResponseDelegate* delegate) OVERRIDE;
  virtual void ProcessAutoEnrollmentRequest(
      const std::string& device_id,
      const enterprise_management::DeviceAutoEnrollmentRequest& request,
      DeviceAutoEnrollmentResponseDelegate* delegate) OVERRIDE;

 private:
  DeviceManagementBackend* backend_;  // weak
  DISALLOW_COPY_AND_ASSIGN(ProxyDeviceManagementBackend);
};

// TODO(mnissler): Deprecated, remove (http://crbug.com/108928).
// Use the new callback-style MockDeviceManagementService in
// mock_device_management_service.h instead.
class MockDeviceManagementServiceOld : public DeviceManagementService {
 public:
  MockDeviceManagementServiceOld();
  virtual ~MockDeviceManagementServiceOld();

  MOCK_METHOD0(CreateBackend, DeviceManagementBackend*());

  // This method bypasses the mocked version and calls the superclass'
  // CreateBackend(), returning a "real" backend.
  DeviceManagementBackend* CreateBackendNotMocked() {
    return DeviceManagementService::CreateBackend();
  }

 private:
  DISALLOW_COPY_AND_ASSIGN(MockDeviceManagementServiceOld);
};

// Use this to make CreateBackend return a ProxyDeviceManagementBackend for
// the given |backend|.
ACTION_P(MockDeviceManagementServiceProxyBackend, backend) {
  return new ProxyDeviceManagementBackend(backend);
}

// Use this to make StartJob send a faked response to the job immediately.
ACTION_P4(MockDeviceManagementServiceRespondToJob, status, response_code,
          cookies, data) {
  arg0->HandleResponse(status, response_code, cookies, data);
}

}  // namespace policy

#endif  // CHROME_BROWSER_POLICY_MOCK_DEVICE_MANAGEMENT_SERVICE_OLD_H_
