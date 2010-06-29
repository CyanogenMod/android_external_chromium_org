// Copyright (c) 2006-2008 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "base/file_path.h"
#include "chrome/browser/dummy_pref_store.h"
#include "chrome/browser/pref_member.h"
#include "chrome/browser/pref_service.h"
#include "chrome/browser/pref_value_store.h"
#include "chrome/common/notification_service.h"
#include "testing/gtest/include/gtest/gtest.h"

namespace {

static const wchar_t kBoolPref[] = L"bool";
static const wchar_t kIntPref[] = L"int";
static const wchar_t kRealPref[] = L"real";
static const wchar_t kStringPref[] = L"string";

void RegisterTestPrefs(PrefService* prefs) {
  prefs->RegisterBooleanPref(kBoolPref, false);
  prefs->RegisterIntegerPref(kIntPref, 0);
  prefs->RegisterRealPref(kRealPref, 0.0);
  prefs->RegisterStringPref(kStringPref, "default");
}

class PrefMemberTestClass : public NotificationObserver {
 public:
  explicit PrefMemberTestClass(PrefService* prefs)
      : observe_cnt_(0), prefs_(prefs) {
    str_.Init(kStringPref, prefs, this);
  }

  virtual void Observe(NotificationType type,
                       const NotificationSource& source,
                       const NotificationDetails& details) {
    DCHECK(NotificationType::PREF_CHANGED == type);
    PrefService* prefs_in = Source<PrefService>(source).ptr();
    EXPECT_EQ(prefs_in, prefs_);
    std::wstring* pref_name_in = Details<std::wstring>(details).ptr();
    EXPECT_EQ(*pref_name_in, kStringPref);
    EXPECT_EQ(str_.GetValue(), prefs_->GetString(kStringPref));
    ++observe_cnt_;
  }

  StringPrefMember str_;
  int observe_cnt_;

 private:
  PrefService* prefs_;
};

}  // anonymous namespace

TEST(PrefMemberTest, BasicGetAndSet) {
  PrefService prefs(new PrefValueStore(NULL, new DummyPrefStore(), NULL));
  RegisterTestPrefs(&prefs);

  // Test bool
  BooleanPrefMember boolean;
  boolean.Init(kBoolPref, &prefs, NULL);

  // Check the defaults
  EXPECT_FALSE(prefs.GetBoolean(kBoolPref));
  EXPECT_FALSE(boolean.GetValue());
  EXPECT_FALSE(*boolean);

  // Try changing through the member variable.
  boolean.SetValue(true);
  EXPECT_TRUE(boolean.GetValue());
  EXPECT_TRUE(prefs.GetBoolean(kBoolPref));
  EXPECT_TRUE(*boolean);

  // Try changing back through the pref.
  prefs.SetBoolean(kBoolPref, false);
  EXPECT_FALSE(prefs.GetBoolean(kBoolPref));
  EXPECT_FALSE(boolean.GetValue());
  EXPECT_FALSE(*boolean);

  // Test int
  IntegerPrefMember integer;
  integer.Init(kIntPref, &prefs, NULL);

  // Check the defaults
  EXPECT_EQ(0, prefs.GetInteger(kIntPref));
  EXPECT_EQ(0, integer.GetValue());
  EXPECT_EQ(0, *integer);

  // Try changing through the member variable.
  integer.SetValue(5);
  EXPECT_EQ(5, integer.GetValue());
  EXPECT_EQ(5, prefs.GetInteger(kIntPref));
  EXPECT_EQ(5, *integer);

  // Try changing back through the pref.
  prefs.SetInteger(kIntPref, 2);
  EXPECT_EQ(2, prefs.GetInteger(kIntPref));
  EXPECT_EQ(2, integer.GetValue());
  EXPECT_EQ(2, *integer);

  // Test real (double)
  RealPrefMember real;
  real.Init(kRealPref, &prefs, NULL);

  // Check the defaults
  EXPECT_EQ(0.0, prefs.GetReal(kRealPref));
  EXPECT_EQ(0.0, real.GetValue());
  EXPECT_EQ(0.0, *real);

  // Try changing through the member variable.
  real.SetValue(1.0);
  EXPECT_EQ(1.0, real.GetValue());
  EXPECT_EQ(1.0, prefs.GetReal(kRealPref));
  EXPECT_EQ(1.0, *real);

  // Try changing back through the pref.
  prefs.SetReal(kRealPref, 3.0);
  EXPECT_EQ(3.0, prefs.GetReal(kRealPref));
  EXPECT_EQ(3.0, real.GetValue());
  EXPECT_EQ(3.0, *real);

  // Test string
  StringPrefMember string;
  string.Init(kStringPref, &prefs, NULL);

  // Check the defaults
  EXPECT_EQ("default", prefs.GetString(kStringPref));
  EXPECT_EQ("default", string.GetValue());
  EXPECT_EQ("default", *string);

  // Try changing through the member variable.
  string.SetValue("foo");
  EXPECT_EQ("foo", string.GetValue());
  EXPECT_EQ("foo", prefs.GetString(kStringPref));
  EXPECT_EQ("foo", *string);

  // Try changing back through the pref.
  prefs.SetString(kStringPref, "bar");
  EXPECT_EQ("bar", prefs.GetString(kStringPref));
  EXPECT_EQ("bar", string.GetValue());
  EXPECT_EQ("bar", *string);
}

TEST(PrefMemberTest, TwoPrefs) {
  // Make sure two RealPrefMembers stay in sync.
  PrefService prefs(new PrefValueStore(NULL, new DummyPrefStore(), NULL));
  RegisterTestPrefs(&prefs);

  RealPrefMember pref1;
  pref1.Init(kRealPref, &prefs, NULL);
  RealPrefMember pref2;
  pref2.Init(kRealPref, &prefs, NULL);

  pref1.SetValue(2.3);
  EXPECT_EQ(2.3, *pref2);

  pref2.SetValue(3.5);
  EXPECT_EQ(3.5, *pref1);

  prefs.SetReal(kRealPref, 4.2);
  EXPECT_EQ(4.2, *pref1);
  EXPECT_EQ(4.2, *pref2);
}

TEST(PrefMemberTest, Observer) {
  PrefService prefs(new PrefValueStore(NULL, new DummyPrefStore(), NULL));
  RegisterTestPrefs(&prefs);

  PrefMemberTestClass test_obj(&prefs);
  EXPECT_EQ("default", *test_obj.str_);

  // Calling SetValue should not fire the observer.
  test_obj.str_.SetValue("hello");
  EXPECT_EQ(0, test_obj.observe_cnt_);
  EXPECT_EQ("hello", prefs.GetString(kStringPref));

  // Changing the pref does fire the observer.
  prefs.SetString(kStringPref, "world");
  EXPECT_EQ(1, test_obj.observe_cnt_);
  EXPECT_EQ("world", *(test_obj.str_));

  // Not changing the value should not fire the observer.
  prefs.SetString(kStringPref, "world");
  EXPECT_EQ(1, test_obj.observe_cnt_);
  EXPECT_EQ("world", *(test_obj.str_));

  prefs.SetString(kStringPref, "hello");
  EXPECT_EQ(2, test_obj.observe_cnt_);
  EXPECT_EQ("hello", prefs.GetString(kStringPref));
}

TEST(PrefMemberTest, NoInit) {
  // Make sure not calling Init on a PrefMember doesn't cause problems.
  IntegerPrefMember pref;
}
