// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_POLICY_CLOUD_CLOUD_POLICY_CLIENT_H_
#define CHROME_BROWSER_POLICY_CLOUD_CLOUD_POLICY_CLIENT_H_

#include <map>
#include <set>
#include <string>

#include "base/basictypes.h"
#include "base/memory/scoped_ptr.h"
#include "base/observer_list.h"
#include "base/time.h"
#include "chrome/browser/policy/cloud/cloud_policy_constants.h"
#include "chrome/browser/policy/cloud/proto/device_management_backend.pb.h"

namespace policy {

class DeviceManagementRequestJob;
class DeviceManagementService;

// Implements the core logic required to talk to the device management service.
// Also keeps track of the current state of the association with the service,
// such as whether there is a valid registration (DMToken is present in that
// case) and whether and what errors occurred in the latest request.
//
// Note that CloudPolicyClient doesn't do any validation of policy responses
// such as signature and time stamp checks. These happen once the policy gets
// installed in the cloud policy cache.
class CloudPolicyClient {
 public:
  // Maps a PolicyNamespaceKey to its corresponding PolicyFetchResponse.
  typedef std::map<PolicyNamespaceKey,
                   enterprise_management::PolicyFetchResponse*> ResponseMap;

  // Observer interface for state and policy changes.
  class Observer {
   public:
    virtual ~Observer();

    // Called when a policy fetch completes successfully. If a policy fetch
    // triggers an error, OnClientError() will fire.
    virtual void OnPolicyFetched(CloudPolicyClient* client) = 0;

    // Called upon registration state changes. This callback is invoked for
    // successful completion of registration and unregistration requests.
    virtual void OnRegistrationStateChanged(CloudPolicyClient* client) = 0;

    // Indicates there's been an error in a previously-issued request.
    virtual void OnClientError(CloudPolicyClient* client) = 0;
  };

  // Delegate interface for supplying status information to upload to the server
  // as part of the policy fetch request.
  class StatusProvider {
   public:
    virtual ~StatusProvider();

    // Retrieves status information to send with the next policy fetch.
    // Implementations must return true if status information was filled in.
    virtual bool GetDeviceStatus(
        enterprise_management::DeviceStatusReportRequest* status) = 0;
    virtual bool GetSessionStatus(
        enterprise_management::SessionStatusReportRequest* status) = 0;

    // Called after the status information has successfully been submitted to
    // the server.
    virtual void OnSubmittedSuccessfully() = 0;
  };

  // |provider| and |service| are weak pointers and it's the caller's
  // responsibility to keep them valid for the lifetime of CloudPolicyClient.
  CloudPolicyClient(const std::string& machine_id,
                    const std::string& machine_model,
                    UserAffiliation user_affiliation,
                    StatusProvider* provider,
                    DeviceManagementService* service);
  virtual ~CloudPolicyClient();

  // Sets the DMToken, thereby establishing a registration with the server. A
  // policy fetch is not automatically issued but can be requested by calling
  // FetchPolicy().
  virtual void SetupRegistration(const std::string& dm_token,
                                 const std::string& client_id);

  // Attempts to register with the device management service. Results in a
  // registration change or error notification.
  virtual void Register(
      enterprise_management::DeviceRegisterRequest::Type registration_type,
      const std::string& auth_token,
      const std::string& client_id,
      bool is_auto_enrollment);

  // Requests a policy fetch. The client being registered is a prerequisite to
  // this operation and this call will CHECK if the client is not in registered
  // state. FetchPolicy() triggers a policy fetch from the cloud. A policy
  // change notification is reported to the observers and the new policy blob
  // can be retrieved once the policy fetch operation completes. In case of
  // multiple requests to fetch policy, new requests will cancel any pending
  // requests and the latest request will eventually trigger notifications.
  virtual void FetchPolicy();

  // Sends an unregistration request to the server.
  virtual void Unregister();

  // Adds an observer to be called back upon policy and state changes.
  void AddObserver(Observer* observer);

  // Removes the specified observer.
  void RemoveObserver(Observer* observer);

  void set_submit_machine_id(bool submit_machine_id) {
    submit_machine_id_ = submit_machine_id;
  }

  void set_last_policy_timestamp(const base::Time& timestamp) {
    last_policy_timestamp_ = timestamp;
  }

  void set_public_key_version(int public_key_version) {
    public_key_version_ = public_key_version;
    public_key_version_valid_ = true;
  }

  void clear_public_key_version() {
    public_key_version_valid_ = false;
  }

  // FetchPolicy() calls will request this policy namespace.
  void AddNamespaceToFetch(const PolicyNamespaceKey& policy_ns_key);

  // FetchPolicy() calls won't request the given policy namespace anymore.
  void RemoveNamespaceToFetch(const PolicyNamespaceKey& policy_ns_key);

  // Whether the client is registered with the device management service.
  bool is_registered() const { return !dm_token_.empty(); }

  const std::string& dm_token() const { return dm_token_; }

  // The device mode as received in the registration request.
  DeviceMode device_mode() const { return device_mode_; }

  // The policy responses as obtained by the last request to the cloud. These
  // policies haven't gone through verification, so their contents cannot be
  // trusted. Use CloudPolicyStore::policy() and CloudPolicyStore::policy_map()
  // instead for making policy decisions.
  const ResponseMap& responses() const {
    return responses_;
  }

  // Returns the policy response for |policy_ns_key|, if found in |responses()|;
  // otherwise returns NULL.
  const enterprise_management::PolicyFetchResponse* GetPolicyFor(
      const PolicyNamespaceKey& policy_ns_key) const;

  DeviceManagementStatus status() const {
    return status_;
  }

 protected:
  // A set of PolicyNamespaceKeys to fetch.
  typedef std::set<PolicyNamespaceKey> NamespaceSet;

  // Callback for retries of registration requests.
  void OnRetryRegister(DeviceManagementRequestJob* job);

  // Callback for registration requests.
  void OnRegisterCompleted(
      DeviceManagementStatus status,
      const enterprise_management::DeviceManagementResponse& response);

  // Callback for policy fetch requests.
  void OnPolicyFetchCompleted(
      DeviceManagementStatus status,
      const enterprise_management::DeviceManagementResponse& response);

  // Callback for unregistration requests.
  void OnUnregisterCompleted(
      DeviceManagementStatus status,
      const enterprise_management::DeviceManagementResponse& response);

  // Observer notification helpers.
  void NotifyPolicyFetched();
  void NotifyRegistrationStateChanged();
  void NotifyClientError();

  // Data necessary for constructing policy requests.
  const std::string machine_id_;
  const std::string machine_model_;
  const UserAffiliation user_affiliation_;
  NamespaceSet namespaces_to_fetch_;

  std::string dm_token_;
  DeviceMode device_mode_;
  std::string client_id_;
  bool submit_machine_id_;
  base::Time last_policy_timestamp_;
  int public_key_version_;
  bool public_key_version_valid_;

  // Used for issuing requests to the cloud.
  DeviceManagementService* service_;
  scoped_ptr<DeviceManagementRequestJob> request_job_;

  // Status upload data is produced by |status_provider_|.
  StatusProvider* status_provider_;

  // The policy responses returned by the last policy fetch operation.
  ResponseMap responses_;
  DeviceManagementStatus status_;

  ObserverList<Observer, true> observers_;

 private:
  DISALLOW_COPY_AND_ASSIGN(CloudPolicyClient);
};

}  // namespace policy

#endif  // CHROME_BROWSER_POLICY_CLOUD_CLOUD_POLICY_CLIENT_H_
