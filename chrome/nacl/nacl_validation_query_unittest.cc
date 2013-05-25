// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/nacl/nacl_validation_db.h"
#include "chrome/nacl/nacl_validation_query.h"
#include "testing/gtest/include/gtest/gtest.h"

// This test makes sure that validation signature generation is performed
// correctly.  In effect, this means that we are checking all of the data
// (and no other data) we are passing the signature generator affects the final
// signature.  To avoid tying the tests to a particular implementation, each
// test generates two signatures and compares them rather than trying to compare
// against a specified signature.

namespace {

const char kKey[] = "bogus key for HMAC...";
const char kKeyAlt[] = "bogus key for HMAC!!!";

const char kVersion[] = "bogus version";
const char kVersionAlt[] = "bogus!version";


const char kShortData[] = "Short data 1234567890";
const char kAltShortData[] = "Short!data 1234567890";

const char kLongData[] = "Long data."
    "1234567890123456789012345678901234567890123456789012345678901234567890"
    "1234567890123456789012345678901234567890123456789012345678901234567890"
    "1234567890123456789012345678901234567890123456789012345678901234567890"
    "1234567890123456789012345678901234567890123456789012345678901234567890";

class MockValidationDB : public NaClValidationDB {
 public:
  MockValidationDB()
    : did_query_(false),
      did_set_(false),
      status_(true) {
  }

  virtual bool QueryKnownToValidate(const std::string& signature) OVERRIDE {
    // The typecast is needed to work around gtest trying to take the address
    // of a constant.
    EXPECT_EQ((int) NaClValidationQuery::kDigestLength,
              (int) signature.length());
    EXPECT_FALSE(did_query_);
    EXPECT_FALSE(did_set_);
    did_query_ = true;
    memcpy(query_signature_, signature.data(),
           NaClValidationQuery::kDigestLength);
    return status_;
  }

  virtual void SetKnownToValidate(const std::string& signature) OVERRIDE {
    // The typecast is needed to work around gtest trying to take the address
    // of a constant.
    ASSERT_EQ((int) NaClValidationQuery::kDigestLength,
              (int) signature.length());
    EXPECT_TRUE(did_query_);
    EXPECT_FALSE(did_set_);
    did_set_ = true;
    memcpy(set_signature_, signature.data(),
           NaClValidationQuery::kDigestLength);
    // Signatures should be the same.
    EXPECT_EQ(0, memcmp(query_signature_, set_signature_,
                        NaClValidationQuery::kDigestLength));
  }

  virtual bool ResolveFileToken(struct NaClFileToken* file_token, int32* fd,
                                std::string* path) OVERRIDE {
    *fd = -1;
    *path = "";
    return false;
  }

  bool did_query_;
  bool did_set_;
  bool status_;

  uint8 query_signature_[NaClValidationQuery::kDigestLength];
  uint8 set_signature_[NaClValidationQuery::kDigestLength];
};

class TestQuery {
 public:
  TestQuery(const char* key, const char* version) {
    db.reset(new MockValidationDB());
    context.reset(new NaClValidationQueryContext(db.get(), key, version));
    query.reset(context->CreateQuery());
  }

  scoped_ptr<MockValidationDB> db;
  scoped_ptr<NaClValidationQueryContext> context;
  scoped_ptr<NaClValidationQuery> query;
};

class NaClValidationQueryTest : public ::testing::Test {
 protected:
  scoped_ptr<TestQuery> query1;
  scoped_ptr<TestQuery> query2;

  virtual void SetUp() {
    query1.reset(new TestQuery(kKey, kVersion));
    query2.reset(new TestQuery(kKey, kVersion));
  }

  void AssertQuerySame() {
    ASSERT_TRUE(query1->db->did_query_);
    ASSERT_TRUE(query2->db->did_query_);
    ASSERT_EQ(0, memcmp(query1->db->query_signature_,
                        query2->db->query_signature_,
                        NaClValidationQuery::kDigestLength));
  }

  void AssertQueryDifferent() {
    ASSERT_TRUE(query1->db->did_query_);
    ASSERT_TRUE(query2->db->did_query_);
    ASSERT_NE(0, memcmp(query1->db->query_signature_,
                        query2->db->query_signature_,
                        NaClValidationQuery::kDigestLength));
  }
};

TEST_F(NaClValidationQueryTest, Sanity) {
  query1->query->AddData(kShortData, sizeof(kShortData));
  ASSERT_FALSE(query1->db->did_query_);
  ASSERT_FALSE(query1->db->did_set_);
  ASSERT_EQ(1, query1->query->QueryKnownToValidate());
  ASSERT_TRUE(query1->db->did_query_);
  ASSERT_FALSE(query1->db->did_set_);
  query1->query->SetKnownToValidate();
  ASSERT_TRUE(query1->db->did_query_);
  ASSERT_TRUE(query1->db->did_set_);
}

TEST_F(NaClValidationQueryTest, ConsistentShort) {
  query1->query->AddData(kShortData, sizeof(kShortData));
  query1->query->QueryKnownToValidate();

  query2->query->AddData(kShortData, sizeof(kShortData));
  query2->query->QueryKnownToValidate();

  AssertQuerySame();
}

TEST_F(NaClValidationQueryTest, InconsistentShort) {
  query1->query->AddData(kShortData, sizeof(kShortData));
  query1->query->QueryKnownToValidate();

  query2->query->AddData(kAltShortData, sizeof(kAltShortData));
  query2->query->QueryKnownToValidate();

  AssertQueryDifferent();
}

// Test for a bug caught during development where AddData would accidently
// overwrite previously written data and add uninitialzied memory to the hash.
TEST_F(NaClValidationQueryTest, ConsistentShortBug) {
  query1->query->AddData(kShortData, sizeof(kShortData));
  query1->query->AddData(kShortData, sizeof(kShortData));
  query1->query->QueryKnownToValidate();

  query2->query->AddData(kShortData, sizeof(kShortData));
  query2->query->AddData(kShortData, sizeof(kShortData));
  query2->query->QueryKnownToValidate();

  AssertQuerySame();
}

// Test for a bug caught during development where AddData would accidently
// overwrite previously written data and add uninitialzed memory to the hash.
TEST_F(NaClValidationQueryTest, InconsistentShortBug1) {
  query1->query->AddData(kShortData, sizeof(kShortData));
  query1->query->AddData(kShortData, sizeof(kShortData));
  query1->query->QueryKnownToValidate();

  query2->query->AddData(kAltShortData, sizeof(kAltShortData));
  query2->query->AddData(kShortData, sizeof(kShortData));
  query2->query->QueryKnownToValidate();

  AssertQueryDifferent();
}

// Make sure we don't ignore the second bit of data.
TEST_F(NaClValidationQueryTest, InconsistentShort2) {
  query1->query->AddData(kShortData, sizeof(kShortData));
  query1->query->AddData(kShortData, sizeof(kShortData));
  query1->query->QueryKnownToValidate();

  query2->query->AddData(kShortData, sizeof(kShortData));
  query2->query->AddData(kAltShortData, sizeof(kAltShortData));
  query2->query->QueryKnownToValidate();

  AssertQueryDifferent();
}

TEST_F(NaClValidationQueryTest, InconsistentZeroSizedAdd) {
  query1->query->AddData(kShortData, sizeof(kShortData));
  query1->query->QueryKnownToValidate();

  query2->query->AddData(kShortData, sizeof(kShortData));
  query2->query->AddData(kShortData, 0);
  query2->query->QueryKnownToValidate();

  AssertQueryDifferent();
}

TEST_F(NaClValidationQueryTest, ConsistentZeroSizedAdd) {
  query1->query->AddData(kShortData, sizeof(kShortData));
  query1->query->AddData("a", 0);
  query1->query->QueryKnownToValidate();

  query2->query->AddData(kShortData, sizeof(kShortData));
  query2->query->AddData("b", 0);
  query2->query->QueryKnownToValidate();

  AssertQuerySame();
}

TEST_F(NaClValidationQueryTest, ConsistentRepeatedShort) {
  for (int i = 0; i < 30; i++) {
    query1->query->AddData(kShortData, sizeof(kShortData));
  }
  query1->query->QueryKnownToValidate();

  for (int i = 0; i < 30; i++) {
    query2->query->AddData(kShortData, sizeof(kShortData));
  }
  query2->query->QueryKnownToValidate();

  AssertQuerySame();
}

TEST_F(NaClValidationQueryTest, ConsistentLong) {
  query1->query->AddData(kLongData, sizeof(kLongData));
  query1->query->QueryKnownToValidate();

  query2->query->AddData(kLongData, sizeof(kLongData));
  query2->query->QueryKnownToValidate();

  AssertQuerySame();
}

TEST_F(NaClValidationQueryTest, ConsistentRepeatedLong) {
  for (int i = 0; i < 30; i++) {
    query1->query->AddData(kLongData, sizeof(kLongData));
  }
  query1->query->QueryKnownToValidate();

  for (int i = 0; i < 30; i++) {
    query2->query->AddData(kLongData, sizeof(kLongData));
  }
  query2->query->QueryKnownToValidate();

  AssertQuerySame();
}

TEST_F(NaClValidationQueryTest, PerturbKey) {
  query2.reset(new TestQuery(kKeyAlt, kVersion));

  query1->query->AddData(kShortData, sizeof(kShortData));
  query1->query->QueryKnownToValidate();

  query2->query->AddData(kShortData, sizeof(kShortData));
  query2->query->QueryKnownToValidate();

  AssertQueryDifferent();
}

TEST_F(NaClValidationQueryTest, PerturbVersion) {
  query2.reset(new TestQuery(kKey, kVersionAlt));

  query1->query->AddData(kShortData, sizeof(kShortData));
  query1->query->QueryKnownToValidate();

  query2->query->AddData(kShortData, sizeof(kShortData));
  query2->query->QueryKnownToValidate();

  AssertQueryDifferent();
}

}
