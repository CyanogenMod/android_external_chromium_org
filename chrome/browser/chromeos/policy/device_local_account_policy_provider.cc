// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/chromeos/policy/device_local_account_policy_provider.h"

#include "base/bind.h"
#include "base/command_line.h"
#include "base/values.h"
#include "chrome/browser/chromeos/policy/device_local_account.h"
#include "chrome/browser/chromeos/policy/device_local_account_external_data_manager.h"
#include "chromeos/dbus/power_policy_controller.h"
#include "components/policy/core/common/cloud/cloud_policy_core.h"
#include "components/policy/core/common/cloud/cloud_policy_service.h"
#include "components/policy/core/common/policy_bundle.h"
#include "components/policy/core/common/policy_map.h"
#include "components/policy/core/common/policy_namespace.h"
#include "components/policy/core/common/policy_switches.h"
#include "content/public/browser/browser_thread.h"
#include "net/url_request/url_request_context_getter.h"
#include "policy/policy_constants.h"

namespace policy {

DeviceLocalAccountPolicyProvider::DeviceLocalAccountPolicyProvider(
    const std::string& user_id,
    DeviceLocalAccountPolicyService* service,
    scoped_ptr<PolicyMap> chrome_policy_overrides)
    : user_id_(user_id),
      service_(service),
      chrome_policy_overrides_(chrome_policy_overrides.Pass()),
      store_initialized_(false),
      waiting_for_policy_refresh_(false),
      weak_factory_(this) {
  service_->AddObserver(this);
  UpdateFromBroker();
}

DeviceLocalAccountPolicyProvider::~DeviceLocalAccountPolicyProvider() {
  service_->RemoveObserver(this);
}

// static
scoped_ptr<DeviceLocalAccountPolicyProvider>
DeviceLocalAccountPolicyProvider::Create(
    const std::string& user_id,
    DeviceLocalAccountPolicyService* device_local_account_policy_service) {
  DeviceLocalAccount::Type type;
  if (!device_local_account_policy_service ||
      !IsDeviceLocalAccountUser(user_id, &type)) {
    return scoped_ptr<DeviceLocalAccountPolicyProvider>();
  }

  scoped_ptr<PolicyMap> chrome_policy_overrides;
  if (type == DeviceLocalAccount::TYPE_PUBLIC_SESSION) {
    chrome_policy_overrides.reset(new PolicyMap());

    // Exit the session when the lid is closed. The default behavior is to
    // suspend while leaving the session running, which is not desirable for
    // public sessions.
    chrome_policy_overrides->Set(
        key::kLidCloseAction,
        POLICY_LEVEL_MANDATORY,
        POLICY_SCOPE_MACHINE,
        new base::FundamentalValue(
            chromeos::PowerPolicyController::ACTION_STOP_SESSION),
        NULL);
    // Force the |ShelfAutoHideBehavior| policy to |Never|, ensuring that the
    // ash shelf does not auto-hide.
    chrome_policy_overrides->Set(
        key::kShelfAutoHideBehavior,
        POLICY_LEVEL_MANDATORY,
        POLICY_SCOPE_MACHINE,
        new base::StringValue("Never"),
        NULL);
    // Force the |ShowLogoutButtonInTray| policy to |true|, ensuring that a big,
    // red logout button is shown in the ash system tray.
    chrome_policy_overrides->Set(
        key::kShowLogoutButtonInTray,
        POLICY_LEVEL_MANDATORY,
        POLICY_SCOPE_MACHINE,
        new base::FundamentalValue(true),
        NULL);
    // Force the |FullscreenAllowed| policy to |false|, ensuring that the ash
    // shelf cannot be hidden by entering fullscreen mode.
    chrome_policy_overrides->Set(
        key::kFullscreenAllowed,
        POLICY_LEVEL_MANDATORY,
        POLICY_SCOPE_MACHINE,
        new base::FundamentalValue(false),
        NULL);
  }

  scoped_ptr<DeviceLocalAccountPolicyProvider> provider(
      new DeviceLocalAccountPolicyProvider(user_id,
                                           device_local_account_policy_service,
                                           chrome_policy_overrides.Pass()));
  return provider.Pass();
}

void DeviceLocalAccountPolicyProvider::Init(SchemaRegistry* schema_registry) {
  ConfigurationPolicyProvider::Init(schema_registry);
  MaybeCreateComponentPolicyService();
}

bool DeviceLocalAccountPolicyProvider::IsInitializationComplete(
    PolicyDomain domain) const {
  if (domain == POLICY_DOMAIN_CHROME)
    return store_initialized_;
  if (ComponentCloudPolicyService::SupportsDomain(domain) &&
      component_policy_service_) {
    return component_policy_service_->is_initialized();
  }
  return true;
}

void DeviceLocalAccountPolicyProvider::RefreshPolicies() {
  DeviceLocalAccountPolicyBroker* broker = GetBroker();
  if (broker && broker->core()->service()) {
    waiting_for_policy_refresh_ = true;
    broker->core()->service()->RefreshPolicy(
        base::Bind(&DeviceLocalAccountPolicyProvider::ReportPolicyRefresh,
                   weak_factory_.GetWeakPtr()));
  } else {
    UpdateFromBroker();
  }
}

void DeviceLocalAccountPolicyProvider::Shutdown() {
  component_policy_service_.reset();
  ConfigurationPolicyProvider::Shutdown();
}

void DeviceLocalAccountPolicyProvider::OnPolicyUpdated(
    const std::string& user_id) {
  if (user_id == user_id_) {
    MaybeCreateComponentPolicyService();
    UpdateFromBroker();
  }
}

void DeviceLocalAccountPolicyProvider::OnDeviceLocalAccountsChanged() {
  MaybeCreateComponentPolicyService();
  UpdateFromBroker();
}

void DeviceLocalAccountPolicyProvider::OnBrokerShutdown(
    DeviceLocalAccountPolicyBroker* broker) {
  if (broker->user_id() == user_id_) {
    // The |component_policy_service_| relies on the broker's CloudPolicyCore,
    // so destroy it if the broker is going away.
    component_policy_service_.reset();
  }
}

void DeviceLocalAccountPolicyProvider::OnComponentCloudPolicyUpdated() {
  UpdateFromBroker();
}

DeviceLocalAccountPolicyBroker* DeviceLocalAccountPolicyProvider::GetBroker() {
  return service_->GetBrokerForUser(user_id_);
}

void DeviceLocalAccountPolicyProvider::ReportPolicyRefresh(bool success) {
  waiting_for_policy_refresh_ = false;
  UpdateFromBroker();
}

void DeviceLocalAccountPolicyProvider::UpdateFromBroker() {
  DeviceLocalAccountPolicyBroker* broker = GetBroker();
  scoped_ptr<PolicyBundle> bundle(new PolicyBundle());
  if (broker) {
    store_initialized_ |= broker->core()->store()->is_initialized();
    if (!waiting_for_policy_refresh_) {
      // Copy policy from the broker.
      bundle->Get(PolicyNamespace(POLICY_DOMAIN_CHROME, std::string()))
          .CopyFrom(broker->core()->store()->policy_map());
      external_data_manager_ = broker->external_data_manager();
    } else {
      // Wait for the refresh to finish.
      return;
    }
  } else {
    // Keep existing policy, but do send an update.
    waiting_for_policy_refresh_ = false;
    weak_factory_.InvalidateWeakPtrs();
    bundle->CopyFrom(policies());
  }

  if (component_policy_service_)
    bundle->MergeFrom(component_policy_service_->policy());

  // Apply overrides.
  if (chrome_policy_overrides_) {
    PolicyMap& chrome_policy =
        bundle->Get(PolicyNamespace(POLICY_DOMAIN_CHROME, std::string()));
    for (PolicyMap::const_iterator it(chrome_policy_overrides_->begin());
         it != chrome_policy_overrides_->end();
         ++it) {
      const PolicyMap::Entry& entry = it->second;
      chrome_policy.Set(
          it->first, entry.level, entry.scope, entry.value->DeepCopy(), NULL);
    }
  }

  UpdatePolicy(bundle.Pass());
}

void DeviceLocalAccountPolicyProvider::MaybeCreateComponentPolicyService() {
  if (component_policy_service_)
    return;  // Already started.

  if (CommandLine::ForCurrentProcess()->HasSwitch(
          switches::kDisableComponentCloudPolicy)) {
    // Disabled via the command line.
    return;
  }

  DeviceLocalAccountPolicyBroker* broker = GetBroker();
  if (!broker || !schema_registry())
    return;  // Missing broker or not initialized yet.

  scoped_ptr<ResourceCache> resource_cache(
      new ResourceCache(broker->GetComponentPolicyCachePath(),
                        content::BrowserThread::GetMessageLoopProxyForThread(
                            content::BrowserThread::FILE)));

  component_policy_service_.reset(new ComponentCloudPolicyService(
      this,
      schema_registry(),
      broker->core(),
      resource_cache.Pass(),
      service_->request_context(),
      content::BrowserThread::GetMessageLoopProxyForThread(
          content::BrowserThread::FILE),
      content::BrowserThread::GetMessageLoopProxyForThread(
          content::BrowserThread::IO)));
}

}  // namespace policy
