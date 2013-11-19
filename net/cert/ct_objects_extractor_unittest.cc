// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "net/cert/ct_objects_extractor.h"

#include "base/files/file_path.h"
#include "net/base/test_data_directory.h"
#include "net/cert/ct_log_verifier.h"
#include "net/cert/ct_serialization.h"
#include "net/cert/signed_certificate_timestamp.h"
#include "net/cert/x509_certificate.h"
#include "net/test/cert_test_util.h"
#include "net/test/ct_test_util.h"
#include "testing/gtest/include/gtest/gtest.h"

namespace net {

namespace ct {

class CTObjectsExtractorTest : public ::testing::Test {
 public:
  virtual void SetUp() OVERRIDE {
    precert_chain_ =
        CreateCertificateListFromFile(GetTestCertsDirectory(),
                                      "ct-test-embedded-cert.pem",
                                      X509Certificate::FORMAT_AUTO);
    ASSERT_EQ(2u, precert_chain_.size());

    std::string der_test_cert(ct::GetDerEncodedX509Cert());
    test_cert_ = X509Certificate::CreateFromBytes(der_test_cert.data(),
                                                  der_test_cert.length());

    log_ = CTLogVerifier::Create(ct::GetTestPublicKey(), "testlog").Pass();
    ASSERT_TRUE(log_);
  }

  void ExtractEmbeddedSCT(scoped_refptr<X509Certificate> cert,
                          SignedCertificateTimestamp* sct) {
    std::string sct_list;
    EXPECT_TRUE(ExtractEmbeddedSCTList(cert->os_cert_handle(), &sct_list));

    std::vector<base::StringPiece> parsed_scts;
    base::StringPiece sct_list_sp(sct_list);
    // Make sure the SCT list can be decoded properly
    EXPECT_TRUE(DecodeSCTList(&sct_list_sp, &parsed_scts));

    EXPECT_TRUE(DecodeSignedCertificateTimestamp(&parsed_scts[0], sct));
  }

 protected:
  CertificateList precert_chain_;
  scoped_refptr<X509Certificate> test_cert_;
  scoped_ptr<CTLogVerifier> log_;
};

// Test that an SCT can be extracted and the extracted SCT contains the
// expected data.
TEST_F(CTObjectsExtractorTest, ExtractEmbeddedSCT) {
  ct::SignedCertificateTimestamp sct;
  ExtractEmbeddedSCT(precert_chain_[0], &sct);

  EXPECT_EQ(sct.version, SignedCertificateTimestamp::SCT_VERSION_1);
  EXPECT_EQ(ct::GetTestPublicKeyId(), sct.log_id);

  base::Time expected_timestamp =
      base::Time::UnixEpoch() +
      base::TimeDelta::FromMilliseconds(1365181456275);
  EXPECT_EQ(expected_timestamp, sct.timestamp);
}

TEST_F(CTObjectsExtractorTest, ExtractPrecert) {
  LogEntry entry;
  ASSERT_TRUE(GetPrecertLogEntry(precert_chain_[0]->os_cert_handle(),
                                 precert_chain_[1]->os_cert_handle(),
                                 &entry));

  ASSERT_EQ(ct::LogEntry::LOG_ENTRY_TYPE_PRECERT, entry.type);
  // Should have empty leaf cert for this log entry type.
  ASSERT_TRUE(entry.leaf_certificate.empty());
  // Compare hash values of issuer spki.
  SHA256HashValue expected_issuer_key_hash;
  memcpy(expected_issuer_key_hash.data, GetDefaultIssuerKeyHash().data(), 32);
  ASSERT_TRUE(expected_issuer_key_hash.Equals(entry.issuer_key_hash));
}

TEST_F(CTObjectsExtractorTest, ExtractOrdinaryX509Cert) {
  LogEntry entry;
  ASSERT_TRUE(GetX509LogEntry(test_cert_->os_cert_handle(), &entry));

  ASSERT_EQ(ct::LogEntry::LOG_ENTRY_TYPE_X509, entry.type);
  // Should have empty tbs_certificate for this log entry type.
  ASSERT_TRUE(entry.tbs_certificate.empty());
  // Length of leaf_certificate should be 718, see the CT Serialization tests.
  ASSERT_EQ(718U, entry.leaf_certificate.size());
}

// Test that the embedded SCT verifies
TEST_F(CTObjectsExtractorTest, ExtractedSCTVerifies) {
  ct::SignedCertificateTimestamp sct;
  ExtractEmbeddedSCT(precert_chain_[0], &sct);

  LogEntry entry;
  ASSERT_TRUE(GetPrecertLogEntry(precert_chain_[0]->os_cert_handle(),
                                 precert_chain_[1]->os_cert_handle(),
                                 &entry));

  EXPECT_TRUE(log_->Verify(entry, sct));
}

// Test that an externally-provided SCT verifies over the LogEntry
// of a regular X.509 Certificate
TEST_F(CTObjectsExtractorTest, ComplementarySCTVerifies) {
  ct::SignedCertificateTimestamp sct;
  GetX509CertSCT(&sct);

  LogEntry entry;
  ASSERT_TRUE(GetX509LogEntry(test_cert_->os_cert_handle(), &entry));

  EXPECT_TRUE(log_->Verify(entry, sct));
}

}  // namespace ct

}  // namespace net
