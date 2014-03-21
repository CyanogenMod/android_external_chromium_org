// Copyright (c) 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "base/files/file_path.h"
#include "net/base/net_errors.h"
#include "net/base/test_completion_callback.h"
#include "net/base/test_data_directory.h"
#include "net/cert/cert_status_flags.h"
#include "net/cert/cert_verify_result.h"
#include "net/cert/x509_certificate.h"
#include "net/quic/crypto/proof_source.h"
#include "net/quic/crypto/proof_verifier.h"
#include "net/quic/test_tools/crypto_test_utils.h"
#include "net/test/cert_test_util.h"
#include "testing/gtest/include/gtest/gtest.h"

#if defined(OS_WIN)
#include "base/win/windows_version.h"
#endif

using std::string;
using std::vector;

namespace net {
namespace test {

TEST(ProofTest, Verify) {
  // TODO(rtenneti): Enable testing of ProofVerifier.
#if 0
  scoped_ptr<ProofSource> source(CryptoTestUtils::ProofSourceForTesting());
  scoped_ptr<ProofVerifier> verifier(
      CryptoTestUtils::ProofVerifierForTesting());

  const string server_config = "server config bytes";
  const string hostname = "test.example.com";
  const vector<string>* certs;
  const vector<string>* first_certs;
  string error_details, signature, first_signature;
  CertVerifyResult cert_verify_result;

  ASSERT_TRUE(source->GetProof(hostname, server_config, false /* no ECDSA */,
                               &first_certs, &first_signature));
  ASSERT_TRUE(source->GetProof(hostname, server_config, false /* no ECDSA */,
                               &certs, &signature));

  // Check that the proof source is caching correctly:
  ASSERT_EQ(first_certs, certs);
  ASSERT_EQ(signature, first_signature);

  int rv;
  TestCompletionCallback callback;
  rv = verifier->VerifyProof(hostname, server_config, *certs, signature,
                             &error_details, &cert_verify_result,
                             callback.callback());
  rv = callback.GetResult(rv);
  ASSERT_EQ(OK, rv);
  ASSERT_EQ("", error_details);
  ASSERT_FALSE(IsCertStatusError(cert_verify_result.cert_status));

  rv = verifier->VerifyProof("foo.com", server_config, *certs, signature,
                             &error_details, &cert_verify_result,
                             callback.callback());
  rv = callback.GetResult(rv);
  ASSERT_EQ(ERR_FAILED, rv);
  ASSERT_NE("", error_details);

  rv = verifier->VerifyProof(hostname, server_config.substr(1, string::npos),
                             *certs, signature, &error_details,
                             &cert_verify_result, callback.callback());
  rv = callback.GetResult(rv);
  ASSERT_EQ(ERR_FAILED, rv);
  ASSERT_NE("", error_details);

  const string corrupt_signature = "1" + signature;
  rv = verifier->VerifyProof(hostname, server_config, *certs,
                             corrupt_signature, &error_details,
                             &cert_verify_result, callback.callback());
  rv = callback.GetResult(rv);
  ASSERT_EQ(ERR_FAILED, rv);
  ASSERT_NE("", error_details);

  vector<string> wrong_certs;
  for (size_t i = 1; i < certs->size(); i++) {
    wrong_certs.push_back((*certs)[i]);
  }
  rv = verifier->VerifyProof("foo.com", server_config, wrong_certs, signature,
                             &error_details, &cert_verify_result,
                             callback.callback());
  rv = callback.GetResult(rv);
  ASSERT_EQ(ERR_FAILED, rv);
  ASSERT_NE("", error_details);
#endif  // 0
}

// TestProofVerifierCallback is a simple callback for a ProofVerifier that
// signals a TestCompletionCallback when called and stores the results from the
// ProofVerifier in pointers passed to the constructor.
class TestProofVerifierCallback : public ProofVerifierCallback {
 public:
  TestProofVerifierCallback(TestCompletionCallback* comp_callback,
                            bool* ok,
                            string* error_details)
      : comp_callback_(comp_callback),
        ok_(ok),
        error_details_(error_details) {}

  virtual void Run(bool ok,
                   const string& error_details,
                   scoped_ptr<ProofVerifyDetails>* details) OVERRIDE {
    *ok_ = ok;
    *error_details_ = error_details;

    comp_callback_->callback().Run(0);
  }

 private:
  TestCompletionCallback* const comp_callback_;
  bool* const ok_;
  string* const error_details_;
};

// RunVerification runs |verifier->VerifyProof| and asserts that the result
// matches |expected_ok|.
static void RunVerification(ProofVerifier* verifier,
                            const string& hostname,
                            const string& server_config,
                            const vector<string>& certs,
                            const string& proof,
                            bool expected_ok) {
  scoped_ptr<ProofVerifyDetails> details;
  TestCompletionCallback comp_callback;
  bool ok;
  string error_details;
  TestProofVerifierCallback* callback =
      new TestProofVerifierCallback(&comp_callback, &ok, &error_details);

  ProofVerifier::Status status = verifier->VerifyProof(
      hostname, server_config, certs, proof, &error_details, &details,
      callback);

  switch (status) {
    case ProofVerifier::FAILURE:
      ASSERT_FALSE(expected_ok);
      ASSERT_NE("", error_details);
      return;
    case ProofVerifier::SUCCESS:
      ASSERT_TRUE(expected_ok);
      ASSERT_EQ("", error_details);
      return;
    case ProofVerifier::PENDING:
      comp_callback.WaitForResult();
      ASSERT_EQ(expected_ok, ok);
      break;
  }
}

static string PEMCertFileToDER(const string& file_name) {
  base::FilePath certs_dir = GetTestCertsDirectory();
  scoped_refptr<X509Certificate> cert =
      ImportCertFromFile(certs_dir, file_name);
  CHECK_NE(static_cast<X509Certificate*>(NULL), cert);

  string der_bytes;
  CHECK(X509Certificate::GetDEREncoded(cert->os_cert_handle(), &der_bytes));
  return der_bytes;
}

// A known answer test that allows us to test ProofVerifier without a working
// ProofSource.
TEST(ProofTest, VerifyRSAKnownAnswerTest) {
  // These sample signatures were generated by running the Proof.Verify test
  // and dumping the bytes of the |signature| output of ProofSource::GetProof().
  // sLen = special value -2 used by OpenSSL.
  static const unsigned char signature_data_0[] = {
    0x31, 0xd5, 0xfb, 0x40, 0x30, 0x75, 0xd2, 0x7d, 0x61, 0xf9, 0xd7, 0x54,
    0x30, 0x06, 0xaf, 0x54, 0x0d, 0xb0, 0x0a, 0xda, 0x63, 0xca, 0x7e, 0x9e,
    0xce, 0xba, 0x10, 0x05, 0x1b, 0xa6, 0x7f, 0xef, 0x2b, 0xa3, 0xff, 0x3c,
    0xbb, 0x9a, 0xe4, 0xbf, 0xb8, 0x0c, 0xc1, 0xbd, 0xed, 0xc2, 0x90, 0x68,
    0xeb, 0x45, 0x48, 0xea, 0x3c, 0x95, 0xf8, 0xa2, 0xb9, 0xe7, 0x62, 0x29,
    0x00, 0xc3, 0x18, 0xb4, 0x16, 0x6f, 0x5e, 0xb0, 0xc1, 0x26, 0xc0, 0x4b,
    0x84, 0xf5, 0x97, 0xfc, 0x17, 0xf9, 0x1c, 0x43, 0xb8, 0xf2, 0x3f, 0x38,
    0x32, 0xad, 0x36, 0x52, 0x2c, 0x26, 0x92, 0x7a, 0xea, 0x2c, 0xa2, 0xf4,
    0x28, 0x2f, 0x19, 0x4d, 0x1f, 0x11, 0x46, 0x82, 0xd0, 0xc4, 0x86, 0x56,
    0x5c, 0x97, 0x9e, 0xc6, 0x37, 0x8e, 0xaf, 0x9d, 0x69, 0xe9, 0x4f, 0x5a,
    0x6d, 0x70, 0x75, 0xc7, 0x41, 0x95, 0x68, 0x53, 0x94, 0xca, 0x31, 0x63,
    0x61, 0x9f, 0xb8, 0x8c, 0x3b, 0x75, 0x36, 0x8b, 0x69, 0xa2, 0x35, 0xc0,
    0x4b, 0x77, 0x55, 0x08, 0xc2, 0xb4, 0x56, 0xd2, 0x81, 0xce, 0x9e, 0x25,
    0xdb, 0x50, 0x74, 0xb3, 0x8a, 0xd9, 0x20, 0x42, 0x3f, 0x85, 0x2d, 0xaa,
    0xfd, 0x66, 0xfa, 0xd6, 0x95, 0x55, 0x6b, 0x63, 0x63, 0x04, 0xf8, 0x6c,
    0x3e, 0x08, 0x22, 0x39, 0xb9, 0x9a, 0xe0, 0xd7, 0x01, 0xff, 0xeb, 0x8a,
    0xb9, 0xe2, 0x34, 0xa5, 0xa0, 0x51, 0xe9, 0xbe, 0x15, 0x12, 0xbf, 0xbe,
    0x64, 0x3d, 0x3f, 0x98, 0xce, 0xc1, 0xa6, 0x33, 0x32, 0xd3, 0x5c, 0xa8,
    0x39, 0x93, 0xdc, 0x1c, 0xb9, 0xab, 0x3c, 0x80, 0x62, 0xb3, 0x76, 0x21,
    0xdf, 0x47, 0x1e, 0xa9, 0x0e, 0x5e, 0x8a, 0xbe, 0x66, 0x5b, 0x7c, 0x21,
    0xfa, 0x78, 0x2d, 0xd1, 0x1d, 0x5c, 0x35, 0x8a, 0x34, 0xb2, 0x1a, 0xc2,
    0xc4, 0x4b, 0x53, 0x54,
  };
  static const unsigned char signature_data_1[] = {
    0x01, 0x7b, 0x52, 0x35, 0xe3, 0x51, 0xdd, 0xf1, 0x67, 0x8d, 0x31, 0x5e,
    0xa3, 0x75, 0x1f, 0x68, 0x6c, 0xdd, 0x41, 0x7a, 0x18, 0x25, 0xe0, 0x12,
    0x6e, 0x84, 0x46, 0x5e, 0xb2, 0x98, 0xd7, 0x84, 0xe1, 0x62, 0xe0, 0xc1,
    0xc4, 0xd7, 0x4f, 0x4f, 0x80, 0xc1, 0x92, 0xd6, 0x02, 0xaf, 0xca, 0x28,
    0x9f, 0xe0, 0xf3, 0x74, 0xd7, 0xf1, 0x44, 0x67, 0x59, 0x27, 0xc8, 0xc2,
    0x8b, 0xd4, 0xe5, 0x4a, 0x07, 0xfd, 0x00, 0xd6, 0x8a, 0xbf, 0x8b, 0xcd,
    0x6a, 0xe0, 0x1d, 0xf6, 0x4b, 0x68, 0x0f, 0xcf, 0xb9, 0xd0, 0xa1, 0xbc,
    0x2e, 0xcf, 0x7c, 0x03, 0x47, 0x11, 0xe4, 0x4c, 0xbc, 0x1b, 0x6b, 0xa5,
    0x2a, 0x82, 0x86, 0xa4, 0x7f, 0x1d, 0x85, 0x64, 0x21, 0x10, 0xd2, 0xb2,
    0xa0, 0x31, 0xa2, 0x78, 0xe6, 0xf2, 0xea, 0x96, 0x38, 0x8c, 0x9a, 0xe1,
    0x01, 0xab, 0x8e, 0x95, 0x66, 0xc8, 0xe5, 0xcc, 0x80, 0xa3, 0xbd, 0x16,
    0xa7, 0x79, 0x19, 0x39, 0x61, 0x3d, 0xff, 0x37, 0xca, 0x9f, 0x97, 0x05,
    0xc7, 0xcb, 0xf0, 0xea, 0xaf, 0x64, 0x07, 0xc0, 0xed, 0x2a, 0x98, 0xa4,
    0xaf, 0x04, 0x6f, 0xf2, 0xc9, 0xb2, 0x73, 0x9a, 0x56, 0x85, 0x43, 0x64,
    0x5f, 0xaa, 0xb7, 0xff, 0x31, 0x4c, 0x2e, 0x6c, 0x17, 0xcf, 0xe5, 0xbe,
    0x7f, 0x7e, 0xad, 0xf5, 0x6f, 0x84, 0x50, 0x20, 0x29, 0xb3, 0x57, 0xe7,
    0xb1, 0xdc, 0x2c, 0x95, 0x48, 0xfe, 0xb0, 0xc1, 0x92, 0xda, 0xc5, 0x58,
    0x95, 0xb0, 0x1a, 0x3a, 0x05, 0x71, 0x3c, 0x6d, 0x20, 0x01, 0x4c, 0xa9,
    0xe4, 0x38, 0x08, 0x65, 0xb4, 0xbd, 0x86, 0x76, 0xbd, 0xad, 0x25, 0x06,
    0x74, 0x0b, 0xca, 0x95, 0x27, 0x0c, 0x13, 0x08, 0x7e, 0x30, 0xcf, 0xf6,
    0xb5, 0xc1, 0x2a, 0x08, 0xfc, 0x4b, 0xc6, 0xb5, 0x2f, 0x23, 0x27, 0x32,
    0x89, 0xdb, 0x0e, 0x4a,
  };
  static const unsigned char signature_data_2[] = {
    0x6d, 0x7d, 0x22, 0x8c, 0x85, 0xc4, 0x8a, 0x80, 0x05, 0xe4, 0x3c, 0xaf,
    0x10, 0x3b, 0xe3, 0x51, 0xb1, 0x86, 0x52, 0x63, 0xb6, 0x17, 0x33, 0xbd,
    0x1b, 0x1e, 0xc4, 0x50, 0x10, 0xfc, 0xcc, 0xea, 0x6b, 0x11, 0xeb, 0x6d,
    0x5e, 0x00, 0xe7, 0xf3, 0x67, 0x99, 0x74, 0x53, 0x12, 0x8f, 0xe4, 0x3e,
    0x20, 0x17, 0x8e, 0x83, 0xe6, 0xdc, 0x83, 0x91, 0x0e, 0xf3, 0x69, 0x22,
    0x95, 0x14, 0xdf, 0xc1, 0xda, 0xb5, 0xdb, 0x6a, 0x1a, 0xb4, 0x4f, 0x26,
    0xd0, 0x32, 0x1d, 0x73, 0x95, 0x1f, 0x39, 0x1d, 0x00, 0xcb, 0xc3, 0x92,
    0x49, 0x53, 0xcb, 0x5c, 0x36, 0x70, 0x19, 0xd9, 0x64, 0x36, 0xda, 0xfb,
    0x20, 0xe5, 0x47, 0xd9, 0x08, 0xc6, 0x5a, 0x9e, 0x87, 0x1a, 0xdb, 0x11,
    0x7b, 0x17, 0xfc, 0x53, 0x7b, 0xc1, 0xa0, 0xc0, 0x33, 0xcf, 0x96, 0xba,
    0x03, 0x79, 0x8e, 0xc6, 0x05, 0xd2, 0xb7, 0xa2, 0xe2, 0xc1, 0x67, 0xb7,
    0x6a, 0xeb, 0xb1, 0x40, 0xbb, 0x7d, 0x57, 0xcb, 0xc2, 0x60, 0x9f, 0xf1,
    0x72, 0xe5, 0xad, 0xce, 0x95, 0x45, 0x7c, 0xbc, 0x75, 0x81, 0x45, 0x19,
    0xe1, 0xa7, 0x2f, 0x05, 0x52, 0xeb, 0xed, 0xdd, 0x19, 0xd9, 0x1a, 0xc9,
    0x5a, 0x06, 0x8e, 0x29, 0x54, 0xb5, 0x4f, 0x80, 0xaa, 0x36, 0x36, 0xc0,
    0xff, 0x64, 0xac, 0xe8, 0x0f, 0x99, 0x35, 0x5e, 0xc6, 0x72, 0x1f, 0x8c,
    0xc4, 0x2b, 0x7d, 0xc1, 0xfb, 0xf0, 0x12, 0x61, 0xb1, 0x18, 0x65, 0xdd,
    0xc2, 0x38, 0x92, 0xba, 0x84, 0xf8, 0xc8, 0x5e, 0x17, 0x63, 0xe0, 0x9c,
    0x2c, 0xe6, 0x70, 0x71, 0xdc, 0xe5, 0xc1, 0xea, 0xb3, 0x9a, 0xb6, 0x91,
    0xdc, 0xc5, 0x56, 0x84, 0x8a, 0x31, 0x31, 0x23, 0x61, 0x94, 0x7e, 0x01,
    0x22, 0x49, 0xf3, 0xcb, 0x0e, 0x31, 0x03, 0x04, 0x1b, 0x14, 0x43, 0x7c,
    0xad, 0x42, 0xe5, 0x55,
  };

  scoped_ptr<ProofVerifier> verifier(
      CryptoTestUtils::ProofVerifierForTesting());

  const string server_config = "server config bytes";
  const string hostname = "test.example.com";
  CertVerifyResult cert_verify_result;

  vector<string> certs(2);
  certs[0] = PEMCertFileToDER("quic_test.example.com.crt");
  certs[1] = PEMCertFileToDER("quic_intermediate.crt");

  // Signatures are nondeterministic, so we test multiple signatures on the
  // same server_config.
  vector<string> signatures(3);
  signatures[0].assign(reinterpret_cast<const char*>(signature_data_0),
                       sizeof(signature_data_0));
  signatures[1].assign(reinterpret_cast<const char*>(signature_data_1),
                       sizeof(signature_data_1));
  signatures[2].assign(reinterpret_cast<const char*>(signature_data_2),
                       sizeof(signature_data_2));

  for (size_t i = 0; i < signatures.size(); i++) {
    const string& signature = signatures[i];

    RunVerification(
        verifier.get(), hostname, server_config, certs, signature, true);
    RunVerification(
        verifier.get(), "foo.com", server_config, certs, signature, false);
    RunVerification(
        verifier.get(), hostname, server_config.substr(1, string::npos),
        certs, signature, false);

    const string corrupt_signature = "1" + signature;
    RunVerification(
        verifier.get(), hostname, server_config, certs, corrupt_signature,
        false);

    vector<string> wrong_certs;
    for (size_t i = 1; i < certs.size(); i++) {
      wrong_certs.push_back(certs[i]);
    }
    RunVerification(verifier.get(), hostname, server_config, wrong_certs,
                    signature, false);
  }
}

// A known answer test that allows us to test ProofVerifier without a working
// ProofSource.
TEST(ProofTest, VerifyECDSAKnownAnswerTest) {
  // Disable this test on platforms that do not support ECDSA certificates.
#if defined(OS_WIN)
  if (base::win::GetVersion() < base::win::VERSION_VISTA)
    return;
#endif

  // These sample signatures were generated by running the Proof.Verify test
  // (modified to use ECDSA for signing proofs) and dumping the bytes of the
  // |signature| output of ProofSource::GetProof().
  static const unsigned char signature_data_0[] = {
    0x30, 0x45, 0x02, 0x21, 0x00, 0x89, 0xc4, 0x7d, 0x08, 0xd1, 0x49, 0x19,
    0x6c, 0xd1, 0x7c, 0xb9, 0x25, 0xe0, 0xe3, 0xbd, 0x6a, 0x5c, 0xd7, 0xaa,
    0x0c, 0xdc, 0x4f, 0x8e, 0xeb, 0xde, 0xbf, 0x32, 0xf8, 0xd1, 0x84, 0x95,
    0x97, 0x02, 0x20, 0x29, 0x3d, 0x49, 0x22, 0x73, 0xed, 0x8b, 0xde, 0x3d,
    0xc2, 0xa4, 0x20, 0xcc, 0xe7, 0xc8, 0x2a, 0x85, 0x20, 0x9b, 0x5b, 0xda,
    0xcd, 0x58, 0x23, 0xbe, 0x89, 0x73, 0x31, 0x87, 0x51, 0xd1, 0x01,
  };
  static const unsigned char signature_data_1[] = {
    0x30, 0x46, 0x02, 0x21, 0x00, 0xec, 0xdf, 0x69, 0xc8, 0x24, 0x59, 0x93,
    0xda, 0x49, 0xee, 0x37, 0x28, 0xaf, 0xeb, 0x0e, 0x2f, 0x80, 0x17, 0x4b,
    0x3b, 0xf6, 0x54, 0xcd, 0x3b, 0x86, 0xc5, 0x98, 0x0d, 0xff, 0xc6, 0xb1,
    0xe7, 0x02, 0x21, 0x00, 0xe1, 0x36, 0x8c, 0xc0, 0xf4, 0x50, 0x5f, 0xba,
    0xfb, 0xe2, 0xff, 0x1d, 0x5d, 0x64, 0xe4, 0x07, 0xbb, 0x5a, 0x4b, 0x19,
    0xb6, 0x39, 0x7a, 0xc4, 0x12, 0xc6, 0xe5, 0x42, 0xc8, 0x78, 0x33, 0xcd,
  };
  static const unsigned char signature_data_2[] = {
    0x30, 0x45, 0x02, 0x20, 0x09, 0x51, 0xe9, 0xde, 0xdb, 0x01, 0xfd, 0xb4,
    0xd8, 0x20, 0xbb, 0xad, 0x41, 0xe3, 0xaa, 0xe7, 0xa3, 0xc3, 0x32, 0x10,
    0x9d, 0xfa, 0x37, 0xce, 0x17, 0xd1, 0x29, 0xf9, 0xd4, 0x1d, 0x0d, 0x19,
    0x02, 0x21, 0x00, 0xc6, 0x20, 0xd4, 0x28, 0xf9, 0x70, 0xb5, 0xb4, 0xff,
    0x4a, 0x35, 0xba, 0xa0, 0xf2, 0x8e, 0x00, 0xf7, 0xcb, 0x43, 0xaf, 0x2d,
    0x1f, 0xce, 0x92, 0x05, 0xca, 0x29, 0xfe, 0xd2, 0x8f, 0xd9, 0x31,
  };

  scoped_ptr<ProofVerifier> verifier(
      CryptoTestUtils::ProofVerifierForTesting());

  const string server_config = "server config bytes";
  const string hostname = "test.example.com";
  CertVerifyResult cert_verify_result;

  vector<string> certs(2);
  certs[0] = PEMCertFileToDER("quic_test_ecc.example.com.crt");
  certs[1] = PEMCertFileToDER("quic_intermediate.crt");

  // Signatures are nondeterministic, so we test multiple signatures on the
  // same server_config.
  vector<string> signatures(3);
  signatures[0].assign(reinterpret_cast<const char*>(signature_data_0),
                       sizeof(signature_data_0));
  signatures[1].assign(reinterpret_cast<const char*>(signature_data_1),
                       sizeof(signature_data_1));
  signatures[2].assign(reinterpret_cast<const char*>(signature_data_2),
                       sizeof(signature_data_2));

  for (size_t i = 0; i < signatures.size(); i++) {
    const string& signature = signatures[i];

    RunVerification(
        verifier.get(), hostname, server_config, certs, signature, true);
    RunVerification(
        verifier.get(), "foo.com", server_config, certs, signature, false);
    RunVerification(
        verifier.get(), hostname, server_config.substr(1, string::npos),
        certs, signature, false);

    // An ECDSA signature is DER-encoded. Corrupt the last byte so that the
    // signature can still be DER-decoded correctly.
    string corrupt_signature = signature;
    corrupt_signature[corrupt_signature.size() - 1] += 1;
    RunVerification(
        verifier.get(), hostname, server_config, certs, corrupt_signature,
        false);

    // Prepending a "1" makes the DER invalid.
    const string bad_der_signature1 = "1" + signature;
    RunVerification(
        verifier.get(), hostname, server_config, certs, bad_der_signature1,
        false);

    vector<string> wrong_certs;
    for (size_t i = 1; i < certs.size(); i++) {
      wrong_certs.push_back(certs[i]);
    }
    RunVerification(
        verifier.get(), hostname, server_config, wrong_certs, signature,
        false);
  }
}

}  // namespace test
}  // namespace net
