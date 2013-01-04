// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chromeos/network/network_util.h"

#include "testing/gtest/include/gtest/gtest.h"

using chromeos::network_util::NetmaskToPrefixLength;
using chromeos::network_util::PrefixLengthToNetmask;

typedef testing::Test NetworkUtilTest;

TEST_F(NetworkUtilTest, NetmaskToPrefixLength) {
  // Valid netmasks
  EXPECT_EQ(32, NetmaskToPrefixLength("255.255.255.255"));
  EXPECT_EQ(31, NetmaskToPrefixLength("255.255.255.254"));
  EXPECT_EQ(30, NetmaskToPrefixLength("255.255.255.252"));
  EXPECT_EQ(29, NetmaskToPrefixLength("255.255.255.248"));
  EXPECT_EQ(28, NetmaskToPrefixLength("255.255.255.240"));
  EXPECT_EQ(27, NetmaskToPrefixLength("255.255.255.224"));
  EXPECT_EQ(26, NetmaskToPrefixLength("255.255.255.192"));
  EXPECT_EQ(25, NetmaskToPrefixLength("255.255.255.128"));
  EXPECT_EQ(24, NetmaskToPrefixLength("255.255.255.0"));
  EXPECT_EQ(23, NetmaskToPrefixLength("255.255.254.0"));
  EXPECT_EQ(22, NetmaskToPrefixLength("255.255.252.0"));
  EXPECT_EQ(21, NetmaskToPrefixLength("255.255.248.0"));
  EXPECT_EQ(20, NetmaskToPrefixLength("255.255.240.0"));
  EXPECT_EQ(19, NetmaskToPrefixLength("255.255.224.0"));
  EXPECT_EQ(18, NetmaskToPrefixLength("255.255.192.0"));
  EXPECT_EQ(17, NetmaskToPrefixLength("255.255.128.0"));
  EXPECT_EQ(16, NetmaskToPrefixLength("255.255.0.0"));
  EXPECT_EQ(15, NetmaskToPrefixLength("255.254.0.0"));
  EXPECT_EQ(14, NetmaskToPrefixLength("255.252.0.0"));
  EXPECT_EQ(13, NetmaskToPrefixLength("255.248.0.0"));
  EXPECT_EQ(12, NetmaskToPrefixLength("255.240.0.0"));
  EXPECT_EQ(11, NetmaskToPrefixLength("255.224.0.0"));
  EXPECT_EQ(10, NetmaskToPrefixLength("255.192.0.0"));
  EXPECT_EQ(9, NetmaskToPrefixLength("255.128.0.0"));
  EXPECT_EQ(8, NetmaskToPrefixLength("255.0.0.0"));
  EXPECT_EQ(7, NetmaskToPrefixLength("254.0.0.0"));
  EXPECT_EQ(6, NetmaskToPrefixLength("252.0.0.0"));
  EXPECT_EQ(5, NetmaskToPrefixLength("248.0.0.0"));
  EXPECT_EQ(4, NetmaskToPrefixLength("240.0.0.0"));
  EXPECT_EQ(3, NetmaskToPrefixLength("224.0.0.0"));
  EXPECT_EQ(2, NetmaskToPrefixLength("192.0.0.0"));
  EXPECT_EQ(1, NetmaskToPrefixLength("128.0.0.0"));
  EXPECT_EQ(0, NetmaskToPrefixLength("0.0.0.0"));
  // Invalid netmasks
  EXPECT_EQ(-1, NetmaskToPrefixLength("255.255.255"));
  EXPECT_EQ(-1, NetmaskToPrefixLength("255.255.255.255.255"));
  EXPECT_EQ(-1, NetmaskToPrefixLength("255.255.255.255.0"));
  EXPECT_EQ(-1, NetmaskToPrefixLength("255.255.255.256"));
  EXPECT_EQ(-1, NetmaskToPrefixLength("255.255.255.1"));
  EXPECT_EQ(-1, NetmaskToPrefixLength("255.255.240.255"));
  EXPECT_EQ(-1, NetmaskToPrefixLength("255.0.0.255"));
  EXPECT_EQ(-1, NetmaskToPrefixLength("255.255.255.FF"));
  EXPECT_EQ(-1, NetmaskToPrefixLength("255,255,255,255"));
  EXPECT_EQ(-1, NetmaskToPrefixLength("255 255 255 255"));
}

TEST_F(NetworkUtilTest, PrefixLengthToNetmask) {
  // Valid Prefix Lengths
  EXPECT_EQ("255.255.255.255", PrefixLengthToNetmask(32));
  EXPECT_EQ("255.255.255.254", PrefixLengthToNetmask(31));
  EXPECT_EQ("255.255.255.252", PrefixLengthToNetmask(30));
  EXPECT_EQ("255.255.255.248", PrefixLengthToNetmask(29));
  EXPECT_EQ("255.255.255.240", PrefixLengthToNetmask(28));
  EXPECT_EQ("255.255.255.224", PrefixLengthToNetmask(27));
  EXPECT_EQ("255.255.255.192", PrefixLengthToNetmask(26));
  EXPECT_EQ("255.255.255.128", PrefixLengthToNetmask(25));
  EXPECT_EQ("255.255.255.0", PrefixLengthToNetmask(24));
  EXPECT_EQ("255.255.254.0", PrefixLengthToNetmask(23));
  EXPECT_EQ("255.255.252.0", PrefixLengthToNetmask(22));
  EXPECT_EQ("255.255.248.0", PrefixLengthToNetmask(21));
  EXPECT_EQ("255.255.240.0", PrefixLengthToNetmask(20));
  EXPECT_EQ("255.255.224.0", PrefixLengthToNetmask(19));
  EXPECT_EQ("255.255.192.0", PrefixLengthToNetmask(18));
  EXPECT_EQ("255.255.128.0", PrefixLengthToNetmask(17));
  EXPECT_EQ("255.255.0.0", PrefixLengthToNetmask(16));
  EXPECT_EQ("255.254.0.0", PrefixLengthToNetmask(15));
  EXPECT_EQ("255.252.0.0", PrefixLengthToNetmask(14));
  EXPECT_EQ("255.248.0.0", PrefixLengthToNetmask(13));
  EXPECT_EQ("255.240.0.0", PrefixLengthToNetmask(12));
  EXPECT_EQ("255.224.0.0", PrefixLengthToNetmask(11));
  EXPECT_EQ("255.192.0.0", PrefixLengthToNetmask(10));
  EXPECT_EQ("255.128.0.0", PrefixLengthToNetmask(9));
  EXPECT_EQ("255.0.0.0", PrefixLengthToNetmask(8));
  EXPECT_EQ("254.0.0.0", PrefixLengthToNetmask(7));
  EXPECT_EQ("252.0.0.0", PrefixLengthToNetmask(6));
  EXPECT_EQ("248.0.0.0", PrefixLengthToNetmask(5));
  EXPECT_EQ("240.0.0.0", PrefixLengthToNetmask(4));
  EXPECT_EQ("224.0.0.0", PrefixLengthToNetmask(3));
  EXPECT_EQ("192.0.0.0", PrefixLengthToNetmask(2));
  EXPECT_EQ("128.0.0.0", PrefixLengthToNetmask(1));
  EXPECT_EQ("0.0.0.0", PrefixLengthToNetmask(0));
  // Invalid Prefix Lengths
  EXPECT_EQ("", PrefixLengthToNetmask(-1));
  EXPECT_EQ("", PrefixLengthToNetmask(33));
  EXPECT_EQ("", PrefixLengthToNetmask(255));
}
