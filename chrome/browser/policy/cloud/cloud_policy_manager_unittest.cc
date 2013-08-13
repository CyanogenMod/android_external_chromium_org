// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/policy/cloud/cloud_policy_manager.h"

#include "base/basictypes.h"
#include "base/bind.h"
#include "base/callback.h"
#include "base/compiler_specific.h"
#include "base/memory/scoped_ptr.h"
#include "base/message_loop/message_loop.h"
#include "base/test/test_simple_task_runner.h"
#include "chrome/browser/invalidation/fake_invalidation_service.h"
#include "chrome/browser/policy/cloud/cloud_policy_constants.h"
#include "chrome/browser/policy/cloud/cloud_policy_invalidator.h"
#include "chrome/browser/policy/cloud/mock_cloud_policy_client.h"
#include "chrome/browser/policy/cloud/mock_cloud_policy_store.h"
#include "chrome/browser/policy/cloud/policy_builder.h"
#include "chrome/browser/policy/configuration_policy_provider_test.h"
#include "chrome/browser/policy/external_data_fetcher.h"
#include "chrome/browser/policy/mock_configuration_policy_provider.h"
#include "sync/notifier/invalidation_util.h"
#include "testing/gmock/include/gmock/gmock.h"
#include "testing/gtest/include/gtest/gtest.h"

using testing::Mock;
using testing::_;

namespace em = enterprise_management;

namespace policy {
namespace {

class TestHarness : public PolicyProviderTestHarness {
 public:
  explicit TestHarness(PolicyLevel level);
  virtual ~TestHarness();

  virtual void SetUp() OVERRIDE;

  virtual ConfigurationPolicyProvider* CreateProvider(
      scoped_refptr<base::SequencedTaskRunner> task_runner,
      const PolicyDefinitionList* policy_definition_list) OVERRIDE;

  virtual void InstallEmptyPolicy() OVERRIDE;
  virtual void InstallStringPolicy(const std::string& policy_name,
                                   const std::string& policy_value) OVERRIDE;
  virtual void InstallIntegerPolicy(const std::string& policy_name,
                                    int policy_value) OVERRIDE;
  virtual void InstallBooleanPolicy(const std::string& policy_name,
                                    bool policy_value) OVERRIDE;
  virtual void InstallStringListPolicy(
      const std::string& policy_name,
      const base::ListValue* policy_value) OVERRIDE;
  virtual void InstallDictionaryPolicy(
      const std::string& policy_name,
      const base::DictionaryValue* policy_value) OVERRIDE;

  // Creates harnesses for mandatory and recommended levels, respectively.
  static PolicyProviderTestHarness* CreateMandatory();
  static PolicyProviderTestHarness* CreateRecommended();

 private:
  MockCloudPolicyStore store_;

  DISALLOW_COPY_AND_ASSIGN(TestHarness);
};

TestHarness::TestHarness(PolicyLevel level)
    : PolicyProviderTestHarness(level, POLICY_SCOPE_USER) {}

TestHarness::~TestHarness() {}

void TestHarness::SetUp() {}

ConfigurationPolicyProvider* TestHarness::CreateProvider(
    scoped_refptr<base::SequencedTaskRunner> task_runner,
    const PolicyDefinitionList* policy_definition_list) {
  // Create and initialize the store.
  store_.NotifyStoreLoaded();
  ConfigurationPolicyProvider* provider = new CloudPolicyManager(
      PolicyNamespaceKey(dm_protocol::kChromeUserPolicyType, std::string()),
      &store_);
  Mock::VerifyAndClearExpectations(&store_);
  return provider;
}

void TestHarness::InstallEmptyPolicy() {}

void TestHarness::InstallStringPolicy(const std::string& policy_name,
                                      const std::string& policy_value) {
  store_.policy_map_.Set(policy_name, policy_level(), policy_scope(),
                         base::Value::CreateStringValue(policy_value), NULL);
}

void TestHarness::InstallIntegerPolicy(const std::string& policy_name,
                                       int policy_value) {
  store_.policy_map_.Set(policy_name, policy_level(), policy_scope(),
                         base::Value::CreateIntegerValue(policy_value), NULL);
}

void TestHarness::InstallBooleanPolicy(const std::string& policy_name,
                                       bool policy_value) {
  store_.policy_map_.Set(policy_name, policy_level(), policy_scope(),
                         base::Value::CreateBooleanValue(policy_value), NULL);
}

void TestHarness::InstallStringListPolicy(const std::string& policy_name,
                                          const base::ListValue* policy_value) {
  store_.policy_map_.Set(policy_name, policy_level(), policy_scope(),
                         policy_value->DeepCopy(), NULL);
}

void TestHarness::InstallDictionaryPolicy(
    const std::string& policy_name,
    const base::DictionaryValue* policy_value) {
  store_.policy_map_.Set(policy_name, policy_level(), policy_scope(),
                         policy_value->DeepCopy(), NULL);
}

// static
PolicyProviderTestHarness* TestHarness::CreateMandatory() {
  return new TestHarness(POLICY_LEVEL_MANDATORY);
}

// static
PolicyProviderTestHarness* TestHarness::CreateRecommended() {
  return new TestHarness(POLICY_LEVEL_RECOMMENDED);
}

// Instantiate abstract test case for basic policy reading tests.
INSTANTIATE_TEST_CASE_P(
    UserCloudPolicyManagerProviderTest,
    ConfigurationPolicyProviderTest,
    testing::Values(TestHarness::CreateMandatory,
                    TestHarness::CreateRecommended));

class TestCloudPolicyManager : public CloudPolicyManager {
 public:
  explicit TestCloudPolicyManager(CloudPolicyStore* store)
      : CloudPolicyManager(PolicyNamespaceKey(
                               dm_protocol::kChromeUserPolicyType,
                               std::string()),
                           store) {}
  virtual ~TestCloudPolicyManager() {}

  // Publish the protected members for testing.
  using CloudPolicyManager::client;
  using CloudPolicyManager::store;
  using CloudPolicyManager::service;
  using CloudPolicyManager::CheckAndPublishPolicy;
  using CloudPolicyManager::StartRefreshScheduler;

 private:
  DISALLOW_COPY_AND_ASSIGN(TestCloudPolicyManager);
};

MATCHER_P(ProtoMatches, proto, "") {
  return arg.SerializePartialAsString() == proto.SerializePartialAsString();
}

class CloudPolicyManagerTest : public testing::Test {
 protected:
  CloudPolicyManagerTest()
      : policy_ns_key_(dm_protocol::kChromeUserPolicyType, std::string()) {}

  virtual void SetUp() OVERRIDE {
    // Set up a policy map for testing.
    policy_map_.Set("key", POLICY_LEVEL_MANDATORY, POLICY_SCOPE_USER,
                    base::Value::CreateStringValue("value"), NULL);
    expected_bundle_.Get(PolicyNamespace(POLICY_DOMAIN_CHROME, std::string()))
        .CopyFrom(policy_map_);

    policy_.payload().mutable_passwordmanagerenabled()->set_value(false);
    policy_.Build();

    EXPECT_CALL(store_, Load());
    manager_.reset(new TestCloudPolicyManager(&store_));
    manager_->Init();
    Mock::VerifyAndClearExpectations(&store_);
    manager_->AddObserver(&observer_);
  }

  virtual void TearDown() OVERRIDE {
    manager_->RemoveObserver(&observer_);
    manager_->Shutdown();
  }

  // Sets up for an invalidations test.
  void CreateInvalidator() {
    // Add the invalidation registration info to the policy data.
    em::PolicyData* policy_data = new em::PolicyData(policy_.policy_data());
    policy_data->set_invalidation_source(12345);
    policy_data->set_invalidation_name("12345");
    store_.policy_.reset(policy_data);

    // Connect the core.
    MockCloudPolicyClient* client = new MockCloudPolicyClient();
    EXPECT_CALL(*client, SetupRegistration(_, _));
    manager_->core()->Connect(scoped_ptr<CloudPolicyClient>(client));

    // Create invalidation objects.
    task_runner_ = new base::TestSimpleTaskRunner();
    invalidation_service_.reset(new invalidation::FakeInvalidationService());
    invalidator_.reset(new CloudPolicyInvalidator(
        manager_.get(),
        manager_->core()->store(),
        task_runner_));
  }

  void ShutdownInvalidator() {
    invalidator_->Shutdown();
  }

  // Call EnableInvalidations on the manager.
  void EnableInvalidations() {
    manager_->EnableInvalidations(
        base::Bind(
            &CloudPolicyInvalidator::InitializeWithService,
            base::Unretained(invalidator_.get()),
            base::Unretained(invalidation_service_.get())));
  }

  // Determine if the invalidator has registered with the invalidation service.
  bool IsInvalidatorRegistered() {
    syncer::ObjectIdSet object_ids =
        invalidation_service_->invalidator_registrar().GetAllRegisteredIds();
    return object_ids.size() == 1 &&
        object_ids.begin()->source() == 12345 &&
        object_ids.begin()->name() == "12345";
  }

  // Determine if the invalidator is unregistered with the invalidation service.
  bool IsInvalidatorUnregistered() {
    syncer::ObjectIdSet object_ids =
        invalidation_service_->invalidator_registrar().GetAllRegisteredIds();
    return object_ids.empty();
  }

  // Required by the refresh scheduler that's created by the manager.
  base::MessageLoop loop_;

  // Testing policy.
  const PolicyNamespaceKey policy_ns_key_;
  UserPolicyBuilder policy_;
  PolicyMap policy_map_;
  PolicyBundle expected_bundle_;

  // Policy infrastructure.
  MockConfigurationPolicyObserver observer_;
  MockCloudPolicyStore store_;
  scoped_ptr<TestCloudPolicyManager> manager_;

  // Invalidation objects.
  scoped_refptr<base::TestSimpleTaskRunner> task_runner_;
  scoped_ptr<invalidation::FakeInvalidationService> invalidation_service_;
  scoped_ptr<CloudPolicyInvalidator> invalidator_;

 private:
  DISALLOW_COPY_AND_ASSIGN(CloudPolicyManagerTest);
};

TEST_F(CloudPolicyManagerTest, InitAndShutdown) {
  PolicyBundle empty_bundle;
  EXPECT_TRUE(empty_bundle.Equals(manager_->policies()));
  EXPECT_FALSE(manager_->IsInitializationComplete(POLICY_DOMAIN_CHROME));

  EXPECT_CALL(observer_, OnUpdatePolicy(_)).Times(0);
  manager_->CheckAndPublishPolicy();
  Mock::VerifyAndClearExpectations(&observer_);

  store_.policy_map_.CopyFrom(policy_map_);
  store_.policy_.reset(new em::PolicyData(policy_.policy_data()));
  EXPECT_CALL(observer_, OnUpdatePolicy(manager_.get()));
  store_.NotifyStoreLoaded();
  Mock::VerifyAndClearExpectations(&observer_);
  EXPECT_TRUE(expected_bundle_.Equals(manager_->policies()));
  EXPECT_TRUE(manager_->IsInitializationComplete(POLICY_DOMAIN_CHROME));

  MockCloudPolicyClient* client = new MockCloudPolicyClient();
  EXPECT_CALL(*client, SetupRegistration(_, _));
  manager_->core()->Connect(scoped_ptr<CloudPolicyClient>(client));
  Mock::VerifyAndClearExpectations(client);
  EXPECT_TRUE(manager_->client());
  EXPECT_TRUE(manager_->service());

  EXPECT_CALL(observer_, OnUpdatePolicy(manager_.get()));
  manager_->CheckAndPublishPolicy();
  Mock::VerifyAndClearExpectations(&observer_);

  manager_->core()->Disconnect();
  EXPECT_FALSE(manager_->client());
  EXPECT_FALSE(manager_->service());
}

TEST_F(CloudPolicyManagerTest, RegistrationAndFetch) {
  EXPECT_CALL(observer_, OnUpdatePolicy(manager_.get()));
  store_.NotifyStoreLoaded();
  Mock::VerifyAndClearExpectations(&observer_);
  EXPECT_TRUE(manager_->IsInitializationComplete(POLICY_DOMAIN_CHROME));

  MockCloudPolicyClient* client = new MockCloudPolicyClient();
  manager_->core()->Connect(scoped_ptr<CloudPolicyClient>(client));

  client->SetDMToken(policy_.policy_data().request_token());
  client->NotifyRegistrationStateChanged();

  client->SetPolicy(policy_ns_key_, policy_.policy());
  EXPECT_CALL(store_, Store(ProtoMatches(policy_.policy())));
  client->NotifyPolicyFetched();
  Mock::VerifyAndClearExpectations(&store_);

  store_.policy_map_.CopyFrom(policy_map_);
  EXPECT_CALL(observer_, OnUpdatePolicy(manager_.get()));
  store_.NotifyStoreLoaded();
  Mock::VerifyAndClearExpectations(&observer_);
  EXPECT_TRUE(expected_bundle_.Equals(manager_->policies()));
}

TEST_F(CloudPolicyManagerTest, Update) {
  EXPECT_CALL(observer_, OnUpdatePolicy(manager_.get()));
  store_.NotifyStoreLoaded();
  Mock::VerifyAndClearExpectations(&observer_);
  EXPECT_TRUE(manager_->IsInitializationComplete(POLICY_DOMAIN_CHROME));
  PolicyBundle empty_bundle;
  EXPECT_TRUE(empty_bundle.Equals(manager_->policies()));

  store_.policy_map_.CopyFrom(policy_map_);
  EXPECT_CALL(observer_, OnUpdatePolicy(manager_.get()));
  store_.NotifyStoreLoaded();
  Mock::VerifyAndClearExpectations(&observer_);
  EXPECT_TRUE(expected_bundle_.Equals(manager_->policies()));
  EXPECT_TRUE(manager_->IsInitializationComplete(POLICY_DOMAIN_CHROME));
}

TEST_F(CloudPolicyManagerTest, RefreshNotRegistered) {
  MockCloudPolicyClient* client = new MockCloudPolicyClient();
  manager_->core()->Connect(scoped_ptr<CloudPolicyClient>(client));

  EXPECT_CALL(observer_, OnUpdatePolicy(manager_.get()));
  store_.NotifyStoreLoaded();
  Mock::VerifyAndClearExpectations(&observer_);

  // A refresh on a non-registered store should not block.
  EXPECT_CALL(observer_, OnUpdatePolicy(manager_.get()));
  manager_->RefreshPolicies();
  Mock::VerifyAndClearExpectations(&observer_);
}

TEST_F(CloudPolicyManagerTest, RefreshSuccessful) {
  MockCloudPolicyClient* client = new MockCloudPolicyClient();
  manager_->core()->Connect(scoped_ptr<CloudPolicyClient>(client));

  // Simulate a store load.
  store_.policy_.reset(new em::PolicyData(policy_.policy_data()));
  EXPECT_CALL(observer_, OnUpdatePolicy(manager_.get()));
  EXPECT_CALL(*client, SetupRegistration(_, _));
  store_.NotifyStoreLoaded();
  Mock::VerifyAndClearExpectations(client);
  Mock::VerifyAndClearExpectations(&observer_);

  // Acknowledge registration.
  client->SetDMToken(policy_.policy_data().request_token());

  // Start a refresh.
  EXPECT_CALL(observer_, OnUpdatePolicy(_)).Times(0);
  EXPECT_CALL(*client, FetchPolicy());
  manager_->RefreshPolicies();
  Mock::VerifyAndClearExpectations(client);
  Mock::VerifyAndClearExpectations(&observer_);
  store_.policy_map_.CopyFrom(policy_map_);

  // A stray reload should be suppressed until the refresh completes.
  EXPECT_CALL(observer_, OnUpdatePolicy(_)).Times(0);
  store_.NotifyStoreLoaded();
  Mock::VerifyAndClearExpectations(&observer_);

  // Respond to the policy fetch, which should trigger a write to |store_|.
  EXPECT_CALL(observer_, OnUpdatePolicy(_)).Times(0);
  EXPECT_CALL(store_, Store(_));
  client->SetPolicy(policy_ns_key_, policy_.policy());
  client->NotifyPolicyFetched();
  Mock::VerifyAndClearExpectations(&observer_);
  Mock::VerifyAndClearExpectations(&store_);

  // The load notification from |store_| should trigger the policy update.
  EXPECT_CALL(observer_, OnUpdatePolicy(manager_.get()));
  store_.NotifyStoreLoaded();
  EXPECT_TRUE(expected_bundle_.Equals(manager_->policies()));
  Mock::VerifyAndClearExpectations(&observer_);
}

TEST_F(CloudPolicyManagerTest, SignalOnError) {
  // Simulate a failed load and verify that it triggers OnUpdatePolicy().
  store_.policy_.reset(new em::PolicyData(policy_.policy_data()));
  EXPECT_CALL(observer_, OnUpdatePolicy(manager_.get()));
  store_.NotifyStoreError();
  Mock::VerifyAndClearExpectations(&observer_);

  EXPECT_TRUE(manager_->IsInitializationComplete(POLICY_DOMAIN_CHROME));
}

TEST_F(CloudPolicyManagerTest, EnableInvalidationsBeforeRefreshScheduler) {
  CreateInvalidator();
  EXPECT_TRUE(IsInvalidatorUnregistered());
  EnableInvalidations();
  EXPECT_TRUE(IsInvalidatorUnregistered());
  manager_->StartRefreshScheduler();
  EXPECT_TRUE(IsInvalidatorRegistered());
  ShutdownInvalidator();
}

TEST_F(CloudPolicyManagerTest, EnableInvalidationsAfterRefreshScheduler) {
  CreateInvalidator();
  EXPECT_TRUE(IsInvalidatorUnregistered());
  manager_->StartRefreshScheduler();
  EXPECT_TRUE(IsInvalidatorUnregistered());
  EnableInvalidations();
  EXPECT_TRUE(IsInvalidatorRegistered());
  ShutdownInvalidator();
}

}  // namespace
}  // namespace policy
