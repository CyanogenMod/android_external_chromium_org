// Copyright (c) 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/omnibox/omnibox_field_trial.h"

#include "base/basictypes.h"
#include "base/metrics/field_trial.h"
#include "base/strings/string16.h"
#include "chrome/common/metrics/entropy_provider.h"
#include "chrome/common/metrics/variations/variations_util.h"
#include "testing/gtest/include/gtest/gtest.h"

class OmniboxFieldTrialTest : public testing::Test {
 public:
  OmniboxFieldTrialTest() {}

  static void SetUpTestCase() {
    ResetFieldTrialList();
  }

  static void TearDownTestCase() {
    delete field_trial_list_;
    field_trial_list_ = NULL;
  }

  static void ResetFieldTrialList() {
    // It's important to delete the old pointer first which sets
    // FieldTrialList::global_ to NULL.
    if (field_trial_list_)
      delete field_trial_list_;
    field_trial_list_ = new base::FieldTrialList(
        new metrics::SHA1EntropyProvider("foo"));
    OmniboxFieldTrial::ActivateDynamicTrials();
  }

  // Creates and activates a field trial.
  static base::FieldTrial* CreateTestTrial(const std::string& name,
                                           const std::string& group_name) {
    base::FieldTrial* trial = base::FieldTrialList::CreateFieldTrial(
        name, group_name);
    trial->group();
    return trial;
  }

 private:
  // Needed for Activate{Static/Dynamic}Trials().
  static base::FieldTrialList* field_trial_list_;

  DISALLOW_COPY_AND_ASSIGN(OmniboxFieldTrialTest);
};

// static
base::FieldTrialList* OmniboxFieldTrialTest::field_trial_list_ = NULL;

// Test if GetDisabledProviderTypes() properly parses various field trial
// group names.
TEST_F(OmniboxFieldTrialTest, GetDisabledProviderTypes) {
  EXPECT_EQ(0, OmniboxFieldTrial::GetDisabledProviderTypes());

  {
    SCOPED_TRACE("Invalid groups");
    CreateTestTrial("AutocompleteDynamicTrial_0", "DisabledProviders_");
    EXPECT_EQ(0, OmniboxFieldTrial::GetDisabledProviderTypes());
    ResetFieldTrialList();
    CreateTestTrial("AutocompleteDynamicTrial_1", "DisabledProviders_XXX");
    EXPECT_EQ(0, OmniboxFieldTrial::GetDisabledProviderTypes());
    ResetFieldTrialList();
    CreateTestTrial("AutocompleteDynamicTrial_1", "DisabledProviders_12abc");
    EXPECT_EQ(0, OmniboxFieldTrial::GetDisabledProviderTypes());
  }

  {
    SCOPED_TRACE("Valid group name, unsupported trial name.");
    ResetFieldTrialList();
    CreateTestTrial("UnsupportedTrialName", "DisabledProviders_20");
    EXPECT_EQ(0, OmniboxFieldTrial::GetDisabledProviderTypes());
  }

  {
    SCOPED_TRACE("Valid field and group name.");
    ResetFieldTrialList();
    CreateTestTrial("AutocompleteDynamicTrial_2", "DisabledProviders_3");
    EXPECT_EQ(3, OmniboxFieldTrial::GetDisabledProviderTypes());
    // Two groups should be OR-ed together.
    CreateTestTrial("AutocompleteDynamicTrial_3", "DisabledProviders_6");
    EXPECT_EQ(7, OmniboxFieldTrial::GetDisabledProviderTypes());
  }
}

// Test if InZeroSuggestFieldTrial() properly parses various field trial
// group names.
TEST_F(OmniboxFieldTrialTest, ZeroSuggestFieldTrial) {
  EXPECT_FALSE(OmniboxFieldTrial::InZeroSuggestFieldTrial());

  {
    SCOPED_TRACE("Valid group name, unsupported trial name.");
    ResetFieldTrialList();
    CreateTestTrial("UnsupportedTrialName", "EnableZeroSuggest");
    EXPECT_FALSE(OmniboxFieldTrial::InZeroSuggestFieldTrial());

    ResetFieldTrialList();
    CreateTestTrial("UnsupportedTrialName", "EnableZeroSuggest_Queries");
    EXPECT_FALSE(OmniboxFieldTrial::InZeroSuggestFieldTrial());

    ResetFieldTrialList();
    CreateTestTrial("UnsupportedTrialName", "EnableZeroSuggest_URLS");
    EXPECT_FALSE(OmniboxFieldTrial::InZeroSuggestFieldTrial());
  }

  {
    SCOPED_TRACE("Valid trial name, unsupported group name.");
    ResetFieldTrialList();
    CreateTestTrial("AutocompleteDynamicTrial_2", "UnrelatedGroup");
    EXPECT_FALSE(OmniboxFieldTrial::InZeroSuggestFieldTrial());
  }

  {
    SCOPED_TRACE("Valid field and group name.");
    ResetFieldTrialList();
    CreateTestTrial("AutocompleteDynamicTrial_2", "EnableZeroSuggest");
    EXPECT_TRUE(OmniboxFieldTrial::InZeroSuggestFieldTrial());

    ResetFieldTrialList();
    CreateTestTrial("AutocompleteDynamicTrial_2", "EnableZeroSuggest_Queries");
    EXPECT_TRUE(OmniboxFieldTrial::InZeroSuggestFieldTrial());

    ResetFieldTrialList();
    CreateTestTrial("AutocompleteDynamicTrial_3", "EnableZeroSuggest_URLs");
    EXPECT_TRUE(OmniboxFieldTrial::InZeroSuggestFieldTrial());
  }
}

TEST_F(OmniboxFieldTrialTest, GetValueForRuleInContext) {
  // Must be the same as kBundledExperimentFieldTrialName
  // defined in omnibox_field_trial.cc.
  const std::string kTrialName = "OmniboxBundledExperimentV1";
  {
    std::map<std::string, std::string> params;
    // Rule 1 has some exact matches and a global fallback.
    params["rule1:1"] = "rule1-1-value";  // NEW_TAB_PAGE
    params["rule1:3"] = "rule1-3-value";  // HOMEPAGE
    params["rule1:*"] = "rule1-*-value";  // global
    // Rule 2 has no exact matches but has a global fallback.
    params["rule2:*"] = "rule2-*-value";  // global
    // Rule 3 has an exact match but no global fallback.
    params["rule3:4"] = "rule3-4-value";  // OTHER
    // Add a malformed rule to make sure it doesn't screw things up.
    params["unrecognized"] = "unrecognized-value";
    ASSERT_TRUE(chrome_variations::AssociateVariationParams(
        kTrialName, "A", params));
  }

  base::FieldTrialList::CreateFieldTrial(kTrialName, "A");

  // Tests for rule 1.
  EXPECT_EQ(
      "rule1-1-value",
      OmniboxFieldTrial::GetValueForRuleInContext(
          "rule1", AutocompleteInput::NEW_TAB_PAGE));  // exact match
  EXPECT_EQ(
      "rule1-*-value",
      OmniboxFieldTrial::GetValueForRuleInContext(
          "rule1", AutocompleteInput::BLANK));  // fallback to global
  EXPECT_EQ(
      "rule1-3-value",
      OmniboxFieldTrial::GetValueForRuleInContext(
          "rule1", AutocompleteInput::HOMEPAGE));  // exact match
  EXPECT_EQ(
      "rule1-*-value",
      OmniboxFieldTrial::GetValueForRuleInContext(
          "rule1", AutocompleteInput::OTHER));  // fallback to global

  // Tests for rule 2.
  EXPECT_EQ(
      "rule2-*-value",
      OmniboxFieldTrial::GetValueForRuleInContext(
          "rule2", AutocompleteInput::HOMEPAGE));  // fallback to global
  EXPECT_EQ(
      "rule2-*-value",
      OmniboxFieldTrial::GetValueForRuleInContext(
          "rule2", AutocompleteInput::OTHER));  // fallback to global

  // Tests for rule 3.
  EXPECT_EQ(
      "",
      OmniboxFieldTrial::GetValueForRuleInContext(
          "rule3", AutocompleteInput::BLANK));  // no global fallback
  EXPECT_EQ(
      "",
      OmniboxFieldTrial::GetValueForRuleInContext(
          "rule3", AutocompleteInput::HOMEPAGE));  // no global fallback
  EXPECT_EQ(
      "rule3-4-value",
      OmniboxFieldTrial::GetValueForRuleInContext(
          "rule3", AutocompleteInput::OTHER));  // exact match

  // Tests for rule 4 (a missing rule).
  EXPECT_EQ(
      "",
      OmniboxFieldTrial::GetValueForRuleInContext(
          "rule4", AutocompleteInput::OTHER));  // no rule at all
}
