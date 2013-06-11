// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// A test application for the FinancialPing class.
//
// These tests should not be executed on the build server:
// - They modify machine state (registry).
//
// These tests require write access to HKCU and HKLM.
//
// The "GGLA" brand is used to test the normal code flow of the code, and the
// "TEST" brand is used to test the supplementary brand code code flow.  In one
// case below, the brand "GOOG" is used because the code wants to use a brand
// that is neither of the two mentioned above.

#include "rlz/lib/financial_ping.h"

#include "base/basictypes.h"
#include "base/logging.h"
#include "base/strings/string_util.h"
#include "base/strings/stringprintf.h"
#include "base/strings/utf_string_conversions.h"
#include "rlz/lib/lib_values.h"
#include "rlz/lib/machine_id.h"
#include "rlz/lib/rlz_lib.h"
#include "rlz/lib/rlz_value_store.h"
#include "rlz/test/rlz_test_helpers.h"
#include "testing/gmock/include/gmock/gmock.h"
#include "testing/gtest/include/gtest/gtest.h"

#if defined(OS_WIN)
#include "rlz/win/lib/machine_deal.h"
#else
#include "base/time.h"
#endif

namespace {

// Must match the implementation in file_time.cc.
int64 GetSystemTimeAsInt64() {
#if defined(OS_WIN)
  FILETIME now_as_file_time;
  GetSystemTimeAsFileTime(&now_as_file_time);
  LARGE_INTEGER integer;
  integer.HighPart = now_as_file_time.dwHighDateTime;
  integer.LowPart = now_as_file_time.dwLowDateTime;
  return integer.QuadPart;
#else
  double now_seconds = base::Time::Now().ToDoubleT();
  return static_cast<int64>(now_seconds * 1000 * 1000 * 10);
#endif
}

// Ping times in 100-nanosecond intervals.
const int64 k1MinuteInterval = 60LL * 10000000LL;  // 1 minute

}  // namespace anonymous

class FinancialPingTest : public RlzLibTestBase {
};

TEST_F(FinancialPingTest, FormRequest) {
  std::string brand_string = rlz_lib::SupplementaryBranding::GetBrand();
  const char* brand = brand_string.empty() ? "GGLA" : brand_string.c_str();

#if defined(OS_WIN)
  EXPECT_TRUE(rlz_lib::MachineDealCode::Set("dcc_value"));
#define DCC_PARAM "&dcc=dcc_value"
#else
#define DCC_PARAM ""
#endif

  EXPECT_TRUE(rlz_lib::SetAccessPointRlz(rlz_lib::IETB_SEARCH_BOX,
      "TbRlzValue"));

  EXPECT_TRUE(rlz_lib::ClearAllProductEvents(rlz_lib::TOOLBAR_NOTIFIER));
  EXPECT_TRUE(rlz_lib::RecordProductEvent(rlz_lib::TOOLBAR_NOTIFIER,
      rlz_lib::IE_DEFAULT_SEARCH, rlz_lib::SET_TO_GOOGLE));
  EXPECT_TRUE(rlz_lib::RecordProductEvent(rlz_lib::TOOLBAR_NOTIFIER,
      rlz_lib::IE_HOME_PAGE, rlz_lib::INSTALL));

  rlz_lib::AccessPoint points[] =
    {rlz_lib::IETB_SEARCH_BOX, rlz_lib::NO_ACCESS_POINT,
     rlz_lib::NO_ACCESS_POINT};

  std::string machine_id;
  bool got_machine_id = rlz_lib::GetMachineId(&machine_id);

  std::string request;
  EXPECT_TRUE(rlz_lib::FinancialPing::FormRequest(rlz_lib::TOOLBAR_NOTIFIER,
      points, "swg", brand, NULL, "en", false, &request));
  std::string expected_response;
  base::StringAppendF(&expected_response,
      "/tools/pso/ping?as=swg&brand=%s&hl=en&"
      "events=I7S,W1I&rep=2&rlz=T4:TbRlzValue" DCC_PARAM
, brand);

  if (got_machine_id)
    base::StringAppendF(&expected_response, "&id=%s", machine_id.c_str());
  EXPECT_EQ(expected_response, request);

  EXPECT_TRUE(rlz_lib::SetAccessPointRlz(rlz_lib::IETB_SEARCH_BOX, ""));
  EXPECT_TRUE(rlz_lib::FinancialPing::FormRequest(rlz_lib::TOOLBAR_NOTIFIER,
      points, "swg", brand, "IdOk2", NULL, false, &request));
  expected_response.clear();
  base::StringAppendF(&expected_response,
      "/tools/pso/ping?as=swg&brand=%s&pid=IdOk2&"
      "events=I7S,W1I&rep=2&rlz=T4:" DCC_PARAM, brand);

  if (got_machine_id)
    base::StringAppendF(&expected_response, "&id=%s", machine_id.c_str());
  EXPECT_EQ(expected_response, request);

  EXPECT_TRUE(rlz_lib::FinancialPing::FormRequest(rlz_lib::TOOLBAR_NOTIFIER,
      points, "swg", brand, "IdOk", NULL, true, &request));
  expected_response.clear();
  base::StringAppendF(&expected_response,
      "/tools/pso/ping?as=swg&brand=%s&pid=IdOk&"
      "events=I7S,W1I&rep=2&rlz=T4:" DCC_PARAM, brand);
  EXPECT_EQ(expected_response, request);

  EXPECT_TRUE(rlz_lib::FinancialPing::FormRequest(rlz_lib::TOOLBAR_NOTIFIER,
      points, "swg", brand, NULL, NULL, true, &request));
  expected_response.clear();
  base::StringAppendF(&expected_response,
      "/tools/pso/ping?as=swg&brand=%s&events=I7S,W1I&rep=2"
      "&rlz=T4:" DCC_PARAM, brand);
  EXPECT_EQ(expected_response, request);


  // Clear all events.
  EXPECT_TRUE(rlz_lib::ClearAllProductEvents(rlz_lib::TOOLBAR_NOTIFIER));

  // Clear all RLZs.
  char rlz[rlz_lib::kMaxRlzLength + 1];
  for (int ap = rlz_lib::NO_ACCESS_POINT + 1;
       ap < rlz_lib::LAST_ACCESS_POINT; ap++) {
    rlz[0] = 0;
    rlz_lib::AccessPoint point = static_cast<rlz_lib::AccessPoint>(ap);
    if (rlz_lib::GetAccessPointRlz(point, rlz, arraysize(rlz)) && rlz[0]) {
      rlz_lib::SetAccessPointRlz(point, "");
    }
  }

  EXPECT_TRUE(rlz_lib::SetAccessPointRlz(rlz_lib::IETB_SEARCH_BOX,
      "TbRlzValue"));
  EXPECT_TRUE(rlz_lib::SetAccessPointRlz(rlz_lib::QUICK_SEARCH_BOX,
      "QsbRlzValue"));
  EXPECT_TRUE(rlz_lib::FinancialPing::FormRequest(rlz_lib::TOOLBAR_NOTIFIER,
      points, "swg", brand, NULL, NULL, false, &request));
  expected_response.clear();
  base::StringAppendF(&expected_response,
      "/tools/pso/ping?as=swg&brand=%s&rep=2&rlz=T4:TbRlzValue,"
      "Q1:QsbRlzValue" DCC_PARAM, brand);
  EXPECT_STREQ(expected_response.c_str(), request.c_str());

  if (!GetAccessPointRlz(rlz_lib::IE_HOME_PAGE, rlz, arraysize(rlz))) {
    points[2] = rlz_lib::IE_HOME_PAGE;
    EXPECT_TRUE(rlz_lib::FinancialPing::FormRequest(rlz_lib::TOOLBAR_NOTIFIER,
        points, "swg", brand, "MyId", "en-US", true, &request));
    expected_response.clear();
    base::StringAppendF(&expected_response,
        "/tools/pso/ping?as=swg&brand=%s&hl=en-US&pid=MyId&rep=2"
        "&rlz=T4:TbRlzValue,Q1:QsbRlzValue" DCC_PARAM, brand);
  EXPECT_STREQ(expected_response.c_str(), request.c_str());
  }
}

TEST_F(FinancialPingTest, FormRequestBadBrand) {
  rlz_lib::AccessPoint points[] =
    {rlz_lib::IETB_SEARCH_BOX, rlz_lib::NO_ACCESS_POINT,
     rlz_lib::NO_ACCESS_POINT};

  std::string request;
  bool ok = rlz_lib::FinancialPing::FormRequest(rlz_lib::TOOLBAR_NOTIFIER,
      points, "swg", "GOOG", NULL, "en", false, &request);
  EXPECT_EQ(rlz_lib::SupplementaryBranding::GetBrand().empty(), ok);
}


static void SetLastPingTime(int64 time, rlz_lib::Product product) {
  rlz_lib::ScopedRlzValueStoreLock lock;
  rlz_lib::RlzValueStore* store = lock.GetStore();
  ASSERT_TRUE(store);
  ASSERT_TRUE(store->HasAccess(rlz_lib::RlzValueStore::kWriteAccess));
  store->WritePingTime(product, time);
}

TEST_F(FinancialPingTest, IsPingTime) {
  int64 now = GetSystemTimeAsInt64();
  int64 last_ping = now - rlz_lib::kEventsPingInterval - k1MinuteInterval;
  SetLastPingTime(last_ping, rlz_lib::TOOLBAR_NOTIFIER);

  // No events, last ping just over a day ago.
  EXPECT_TRUE(rlz_lib::ClearAllProductEvents(rlz_lib::TOOLBAR_NOTIFIER));
  EXPECT_FALSE(rlz_lib::FinancialPing::IsPingTime(rlz_lib::TOOLBAR_NOTIFIER,
                                                  false));

  // Has events, last ping just over a day ago.
  EXPECT_TRUE(rlz_lib::RecordProductEvent(rlz_lib::TOOLBAR_NOTIFIER,
      rlz_lib::IE_DEFAULT_SEARCH, rlz_lib::SET_TO_GOOGLE));
  EXPECT_TRUE(rlz_lib::FinancialPing::IsPingTime(rlz_lib::TOOLBAR_NOTIFIER,
                                                 false));

  // Has events, last ping just under a day ago.
  last_ping = now - rlz_lib::kEventsPingInterval + k1MinuteInterval;
  SetLastPingTime(last_ping, rlz_lib::TOOLBAR_NOTIFIER);
  EXPECT_FALSE(rlz_lib::FinancialPing::IsPingTime(rlz_lib::TOOLBAR_NOTIFIER,
                                                  false));

  EXPECT_TRUE(rlz_lib::ClearAllProductEvents(rlz_lib::TOOLBAR_NOTIFIER));

  // No events, last ping just under a week ago.
  last_ping = now - rlz_lib::kNoEventsPingInterval + k1MinuteInterval;
  SetLastPingTime(last_ping, rlz_lib::TOOLBAR_NOTIFIER);
  EXPECT_FALSE(rlz_lib::FinancialPing::IsPingTime(rlz_lib::TOOLBAR_NOTIFIER,
                                                  false));

  // No events, last ping just over a week ago.
  last_ping = now - rlz_lib::kNoEventsPingInterval - k1MinuteInterval;
  SetLastPingTime(last_ping, rlz_lib::TOOLBAR_NOTIFIER);
  EXPECT_TRUE(rlz_lib::FinancialPing::IsPingTime(rlz_lib::TOOLBAR_NOTIFIER,
                                                 false));

  // Last ping was in future (invalid).
  last_ping = now + k1MinuteInterval;
  SetLastPingTime(last_ping, rlz_lib::TOOLBAR_NOTIFIER);
  EXPECT_TRUE(rlz_lib::FinancialPing::IsPingTime(rlz_lib::TOOLBAR_NOTIFIER,
                                                 false));
  EXPECT_TRUE(rlz_lib::RecordProductEvent(rlz_lib::TOOLBAR_NOTIFIER,
      rlz_lib::IE_DEFAULT_SEARCH, rlz_lib::SET_TO_GOOGLE));
  EXPECT_TRUE(rlz_lib::FinancialPing::IsPingTime(rlz_lib::TOOLBAR_NOTIFIER,
                                                 false));
}

TEST_F(FinancialPingTest, BrandingIsPingTime) {
  // Don't run these tests if a supplementary brand is already in place.  That
  // way we can control the branding.
  if (!rlz_lib::SupplementaryBranding::GetBrand().empty())
    return;

  int64 now = GetSystemTimeAsInt64();
  int64 last_ping = now - rlz_lib::kEventsPingInterval - k1MinuteInterval;
  SetLastPingTime(last_ping, rlz_lib::TOOLBAR_NOTIFIER);

  // Has events, last ping just over a day ago.
  EXPECT_TRUE(rlz_lib::RecordProductEvent(rlz_lib::TOOLBAR_NOTIFIER,
      rlz_lib::IE_DEFAULT_SEARCH, rlz_lib::SET_TO_GOOGLE));
  EXPECT_TRUE(rlz_lib::FinancialPing::IsPingTime(rlz_lib::TOOLBAR_NOTIFIER,
                                                 false));

  {
    rlz_lib::SupplementaryBranding branding("TEST");
    SetLastPingTime(last_ping, rlz_lib::TOOLBAR_NOTIFIER);

    // Has events, last ping just over a day ago.
    EXPECT_TRUE(rlz_lib::RecordProductEvent(rlz_lib::TOOLBAR_NOTIFIER,
        rlz_lib::IE_DEFAULT_SEARCH, rlz_lib::SET_TO_GOOGLE));
    EXPECT_TRUE(rlz_lib::FinancialPing::IsPingTime(rlz_lib::TOOLBAR_NOTIFIER,
                                                   false));
  }

  last_ping = now - k1MinuteInterval;
  SetLastPingTime(last_ping, rlz_lib::TOOLBAR_NOTIFIER);

  EXPECT_FALSE(rlz_lib::FinancialPing::IsPingTime(rlz_lib::TOOLBAR_NOTIFIER,
                                                  false));

  {
    rlz_lib::SupplementaryBranding branding("TEST");
    EXPECT_TRUE(rlz_lib::FinancialPing::IsPingTime(rlz_lib::TOOLBAR_NOTIFIER,
                                                   false));
  }
}

TEST_F(FinancialPingTest, ClearLastPingTime) {
  int64 now = GetSystemTimeAsInt64();
  int64 last_ping = now - rlz_lib::kEventsPingInterval + k1MinuteInterval;
  SetLastPingTime(last_ping, rlz_lib::TOOLBAR_NOTIFIER);

  // Has events, last ping just under a day ago.
  EXPECT_TRUE(rlz_lib::ClearAllProductEvents(rlz_lib::TOOLBAR_NOTIFIER));
  EXPECT_TRUE(rlz_lib::RecordProductEvent(rlz_lib::TOOLBAR_NOTIFIER,
      rlz_lib::IE_DEFAULT_SEARCH, rlz_lib::SET_TO_GOOGLE));
  EXPECT_FALSE(rlz_lib::FinancialPing::IsPingTime(rlz_lib::TOOLBAR_NOTIFIER,
                                                  false));

  EXPECT_TRUE(rlz_lib::FinancialPing::ClearLastPingTime(
      rlz_lib::TOOLBAR_NOTIFIER));
  EXPECT_TRUE(rlz_lib::FinancialPing::IsPingTime(rlz_lib::TOOLBAR_NOTIFIER,
                                                 false));
}
