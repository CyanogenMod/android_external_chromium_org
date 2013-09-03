// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <string>
#include <vector>

#include "base/basictypes.h"
#include "base/bind.h"
#include "media/base/decoder_buffer.h"
#include "media/base/decrypt_config.h"
#include "media/base/mock_filters.h"
#include "media/cdm/aes_decryptor.h"
#include "media/webm/webm_constants.h"
#include "testing/gmock/include/gmock/gmock.h"
#include "testing/gtest/include/gtest/gtest.h"

using ::testing::_;
using ::testing::Gt;
using ::testing::IsNull;
using ::testing::NotNull;
using ::testing::SaveArg;
using ::testing::StrNe;

MATCHER(IsEmpty, "") { return arg.empty(); }

namespace media {

static const char kClearKeySystem[] = "org.w3.clearkey";

static const uint8 kOriginalData[] = "Original subsample data.";
static const int kOriginalDataSize = 24;

static const uint8 kKeyId[] = { 0x00, 0x01, 0x02, 0x03 };

static const uint8 kKey[] = {
  0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b,
  0x0c, 0x0d, 0x0e, 0x0f, 0x10, 0x11, 0x12, 0x13
};

static const uint8 kWrongKey[] = {
  0xee, 0xee, 0xee, 0xee, 0xee, 0xee, 0xee, 0xee,
  0xee, 0xee, 0xee, 0xee, 0xee, 0xee, 0xee, 0xee
};

static const uint8 kIv[] = {
  0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

// kOriginalData encrypted with kKey and kIv but without any subsamples (or
// equivalently using kSubsampleEntriesCypherOnly).
static const uint8 kEncryptedData[] = {
  0x2f, 0x03, 0x09, 0xef, 0x71, 0xaf, 0x31, 0x16,
  0xfa, 0x9d, 0x18, 0x43, 0x1e, 0x96, 0x71, 0xb5,
  0xbf, 0xf5, 0x30, 0x53, 0x9a, 0x20, 0xdf, 0x95
};

// kOriginalData encrypted with kSubsampleKey and kSubsampleIv using
// kSubsampleEntriesNormal.
static const uint8 kSubsampleEncryptedData[] = {
  0x4f, 0x72, 0x09, 0x16, 0x09, 0xe6, 0x79, 0xad,
  0x70, 0x73, 0x75, 0x62, 0x09, 0xbb, 0x83, 0x1d,
  0x4d, 0x08, 0xd7, 0x78, 0xa4, 0xa7, 0xf1, 0x2e
};

static const uint8 kOriginalData2[] = "Changed Original data.";

static const uint8 kIv2[] = {
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

static const uint8 kKeyId2[] = {
  0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
  0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
  0x10, 0x11, 0x12, 0x13
};

static const uint8 kKey2[] = {
  0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1a, 0x1b,
  0x1c, 0x1d, 0x1e, 0x1f, 0x20, 0x21, 0x22, 0x23
};

static const uint8 kEncryptedData2[] = {
  0x57, 0x66, 0xf4, 0x12, 0x1a, 0xed, 0xb5, 0x79,
  0x1c, 0x8e, 0x25, 0xd7, 0x17, 0xe7, 0x5e, 0x16,
  0xe3, 0x40, 0x08, 0x27, 0x11, 0xe9
};

// Subsample entries for testing. The sum of |cypher_bytes| and |clear_bytes| of
// all entries must be equal to kOriginalDataSize to make the subsample entries
// valid.

static const SubsampleEntry kSubsampleEntriesNormal[] = {
  { 2, 7 },
  { 3, 11 },
  { 1, 0 }
};

static const SubsampleEntry kSubsampleEntriesWrongSize[] = {
  { 3, 6 }, // This entry doesn't match the correct entry.
  { 3, 11 },
  { 1, 0 }
};

static const SubsampleEntry kSubsampleEntriesInvalidTotalSize[] = {
  { 1, 1000 }, // This entry is too large.
  { 3, 11 },
  { 1, 0 }
};

static const SubsampleEntry kSubsampleEntriesClearOnly[] = {
  { 7, 0 },
  { 8, 0 },
  { 9, 0 }
};

static const SubsampleEntry kSubsampleEntriesCypherOnly[] = {
  { 0, 6 },
  { 0, 8 },
  { 0, 10 }
};

static scoped_refptr<DecoderBuffer> CreateEncryptedBuffer(
    const std::vector<uint8>& data,
    const std::vector<uint8>& key_id,
    const std::vector<uint8>& iv,
    int offset,
    const std::vector<SubsampleEntry>& subsample_entries) {
  DCHECK(!data.empty());
  int padded_size = offset + data.size();
  scoped_refptr<DecoderBuffer> encrypted_buffer(new DecoderBuffer(padded_size));
  memcpy(encrypted_buffer->writable_data() + offset, &data[0], data.size());
  CHECK(encrypted_buffer.get());
  std::string key_id_string(
      reinterpret_cast<const char*>(key_id.empty() ? NULL : &key_id[0]),
      key_id.size());
  std::string iv_string(
      reinterpret_cast<const char*>(iv.empty() ? NULL : &iv[0]), iv.size());
  encrypted_buffer->set_decrypt_config(scoped_ptr<DecryptConfig>(
      new DecryptConfig(key_id_string, iv_string, offset, subsample_entries)));
  return encrypted_buffer;
}

class AesDecryptorTest : public testing::Test {
 public:
  AesDecryptorTest()
      : decryptor_(
            base::Bind(&AesDecryptorTest::KeyAdded, base::Unretained(this)),
            base::Bind(&AesDecryptorTest::KeyError, base::Unretained(this)),
            base::Bind(&AesDecryptorTest::KeyMessage, base::Unretained(this))),
        decrypt_cb_(base::Bind(&AesDecryptorTest::BufferDecrypted,
                               base::Unretained(this))),
        original_data_(kOriginalData, kOriginalData + kOriginalDataSize),
        encrypted_data_(kEncryptedData,
                        kEncryptedData + arraysize(kEncryptedData)),
        subsample_encrypted_data_(
            kSubsampleEncryptedData,
            kSubsampleEncryptedData + arraysize(kSubsampleEncryptedData)),
        key_id_(kKeyId, kKeyId + arraysize(kKeyId)),
        key_(kKey, kKey + arraysize(kKey)),
        iv_(kIv, kIv + arraysize(kIv)),
        normal_subsample_entries_(
            kSubsampleEntriesNormal,
            kSubsampleEntriesNormal + arraysize(kSubsampleEntriesNormal)) {
  }

 protected:
  void GenerateKeyRequest(const std::vector<uint8>& key_id) {
    DCHECK(!key_id.empty());
    EXPECT_CALL(*this, KeyMessage(StrNe(std::string()), key_id, ""))
        .WillOnce(SaveArg<0>(&session_id_string_));
    EXPECT_TRUE(decryptor_.GenerateKeyRequest(
        std::string(), &key_id[0], key_id.size()));
  }

  enum AddKeyExpectation {
    KEY_ADDED,
    KEY_ERROR
  };

  void AddKeyAndExpect(const std::vector<uint8>& key_id,
                       const std::vector<uint8>& key,
                       AddKeyExpectation result) {
    DCHECK(!key_id.empty());
    DCHECK(!key.empty());

    if (result == KEY_ADDED) {
      EXPECT_CALL(*this, KeyAdded(session_id_string_));
    } else if (result == KEY_ERROR) {
      EXPECT_CALL(*this, KeyError(session_id_string_,
                                  MediaKeys::kUnknownError, 0));
    } else {
      NOTREACHED();
    }

    decryptor_.AddKey(&key[0], key.size(), &key_id[0], key_id.size(),
                      session_id_string_);
  }

  MOCK_METHOD2(BufferDecrypted, void(Decryptor::Status,
                                     const scoped_refptr<DecoderBuffer>&));

  enum DecryptExpectation {
    SUCCESS,
    DATA_MISMATCH,
    DATA_AND_SIZE_MISMATCH,
    DECRYPT_ERROR
  };

  void DecryptAndExpect(const scoped_refptr<DecoderBuffer>& encrypted,
                        const std::vector<uint8>& plain_text,
                        DecryptExpectation result) {
    scoped_refptr<DecoderBuffer> decrypted;

    if (result != DECRYPT_ERROR) {
      EXPECT_CALL(*this, BufferDecrypted(Decryptor::kSuccess, NotNull()))
          .WillOnce(SaveArg<1>(&decrypted));
    } else {
      EXPECT_CALL(*this, BufferDecrypted(Decryptor::kError, IsNull()))
          .WillOnce(SaveArg<1>(&decrypted));
    }

    decryptor_.Decrypt(Decryptor::kVideo, encrypted, decrypt_cb_);

    std::vector<uint8> decrypted_text;
    if (decrypted && decrypted->data_size()) {
      decrypted_text.assign(
        decrypted->data(), decrypted->data() + decrypted->data_size());
    }

    switch (result) {
      case SUCCESS:
        EXPECT_EQ(plain_text, decrypted_text);
        break;
      case DATA_MISMATCH:
        EXPECT_EQ(plain_text.size(), decrypted_text.size());
        EXPECT_NE(plain_text, decrypted_text);
        break;
      case DATA_AND_SIZE_MISMATCH:
        EXPECT_NE(plain_text.size(), decrypted_text.size());
        break;
      case DECRYPT_ERROR:
        EXPECT_TRUE(decrypted_text.empty());
        break;
    }
  }

  MOCK_METHOD1(KeyAdded, void(const std::string&));
  MOCK_METHOD3(KeyError, void(const std::string&,
                              MediaKeys::KeyError, int));
  MOCK_METHOD3(KeyMessage, void(const std::string& session_id,
                                const std::vector<uint8>& message,
                                const std::string& default_url));

  AesDecryptor decryptor_;
  std::string session_id_string_;
  AesDecryptor::DecryptCB decrypt_cb_;

  // Constants for testing.
  const std::vector<uint8> original_data_;
  const std::vector<uint8> encrypted_data_;
  const std::vector<uint8> subsample_encrypted_data_;
  const std::vector<uint8> key_id_;
  const std::vector<uint8> key_;
  const std::vector<uint8> iv_;
  const std::vector<SubsampleEntry> normal_subsample_entries_;
  const std::vector<SubsampleEntry> no_subsample_entries_;
};

TEST_F(AesDecryptorTest, GenerateKeyRequestWithNullInitData) {
  EXPECT_CALL(*this, KeyMessage(StrNe(std::string()), IsEmpty(), ""));
  EXPECT_TRUE(decryptor_.GenerateKeyRequest(std::string(), NULL, 0));
}

TEST_F(AesDecryptorTest, NormalDecryption) {
  GenerateKeyRequest(key_id_);
  AddKeyAndExpect(key_id_, key_, KEY_ADDED);
  scoped_refptr<DecoderBuffer> encrypted_buffer = CreateEncryptedBuffer(
      encrypted_data_, key_id_, iv_, 0, no_subsample_entries_);
  DecryptAndExpect(encrypted_buffer, original_data_, SUCCESS);
}

TEST_F(AesDecryptorTest, DecryptionWithOffset) {
  GenerateKeyRequest(key_id_);
  AddKeyAndExpect(key_id_, key_, KEY_ADDED);
  scoped_refptr<DecoderBuffer> encrypted_buffer = CreateEncryptedBuffer(
      encrypted_data_, key_id_, iv_, 23, no_subsample_entries_);
  DecryptAndExpect(encrypted_buffer, original_data_, SUCCESS);
}

TEST_F(AesDecryptorTest, UnencryptedFrame) {
  // An empty iv string signals that the frame is unencrypted.
  scoped_refptr<DecoderBuffer> encrypted_buffer = CreateEncryptedBuffer(
      original_data_, key_id_, std::vector<uint8>(), 0, no_subsample_entries_);
  DecryptAndExpect(encrypted_buffer, original_data_, SUCCESS);
}

TEST_F(AesDecryptorTest, WrongKey) {
  GenerateKeyRequest(key_id_);
  std::vector<uint8> wrong_key(kWrongKey, kWrongKey + arraysize(kWrongKey));
  AddKeyAndExpect(key_id_, wrong_key, KEY_ADDED);
  scoped_refptr<DecoderBuffer> encrypted_buffer = CreateEncryptedBuffer(
      encrypted_data_, key_id_, iv_, 0, no_subsample_entries_);
  DecryptAndExpect(encrypted_buffer, original_data_, DATA_MISMATCH);
}

TEST_F(AesDecryptorTest, NoKey) {
  scoped_refptr<DecoderBuffer> encrypted_buffer = CreateEncryptedBuffer(
      encrypted_data_, key_id_, iv_, 0, no_subsample_entries_);
  EXPECT_CALL(*this, BufferDecrypted(AesDecryptor::kNoKey, IsNull()));
  decryptor_.Decrypt(Decryptor::kVideo, encrypted_buffer, decrypt_cb_);
}

TEST_F(AesDecryptorTest, KeyReplacement) {
  GenerateKeyRequest(key_id_);
  scoped_refptr<DecoderBuffer> encrypted_buffer = CreateEncryptedBuffer(
      encrypted_data_, key_id_, iv_, 0, no_subsample_entries_);

  std::vector<uint8> wrong_key(kWrongKey, kWrongKey + arraysize(kWrongKey));
  AddKeyAndExpect(key_id_, wrong_key, KEY_ADDED);
  ASSERT_NO_FATAL_FAILURE(DecryptAndExpect(
      encrypted_buffer, original_data_, DATA_MISMATCH));

  AddKeyAndExpect(key_id_, key_, KEY_ADDED);
  ASSERT_NO_FATAL_FAILURE(
      DecryptAndExpect(encrypted_buffer, original_data_, SUCCESS));
}

TEST_F(AesDecryptorTest, WrongSizedKey) {
  GenerateKeyRequest(key_id_);
  // Use "-1" to create a wrong sized key.
  std::vector<uint8> wrong_sized_key(kKey, kKey + arraysize(kKey) - 1);
  AddKeyAndExpect(key_id_, wrong_sized_key, KEY_ERROR);
}

TEST_F(AesDecryptorTest, MultipleKeysAndFrames) {
  GenerateKeyRequest(key_id_);
  AddKeyAndExpect(key_id_, key_, KEY_ADDED);
  scoped_refptr<DecoderBuffer> encrypted_buffer = CreateEncryptedBuffer(
      encrypted_data_, key_id_, iv_, 10, no_subsample_entries_);
  ASSERT_NO_FATAL_FAILURE(
      DecryptAndExpect(encrypted_buffer, original_data_, SUCCESS));

  std::vector<uint8> key_id_2 =
      std::vector<uint8>(kKeyId2, kKeyId2 + arraysize(kKeyId2));
  AddKeyAndExpect(
      key_id_2, std::vector<uint8>(kKey2, kKey2 + arraysize(kKey2)), KEY_ADDED);

  // The first key is still available after we added a second key.
  ASSERT_NO_FATAL_FAILURE(
      DecryptAndExpect(encrypted_buffer, original_data_, SUCCESS));

  // The second key is also available.
  encrypted_buffer = CreateEncryptedBuffer(
      std::vector<uint8>(kEncryptedData2,
                         kEncryptedData2 + arraysize(kEncryptedData2)),
      key_id_2,
      std::vector<uint8>(kIv2, kIv2 + arraysize(kIv2)),
      30,
      no_subsample_entries_);
  ASSERT_NO_FATAL_FAILURE(DecryptAndExpect(
      encrypted_buffer,
      std::vector<uint8>(kOriginalData2,
                         kOriginalData2 + arraysize(kOriginalData2) - 1),
      SUCCESS));
}

TEST_F(AesDecryptorTest, CorruptedIv) {
  GenerateKeyRequest(key_id_);
  AddKeyAndExpect(key_id_, key_, KEY_ADDED);

  std::vector<uint8> bad_iv = iv_;
  bad_iv[1]++;

  scoped_refptr<DecoderBuffer> encrypted_buffer = CreateEncryptedBuffer(
      encrypted_data_, key_id_, bad_iv, 0, no_subsample_entries_);

  DecryptAndExpect(encrypted_buffer, original_data_, DATA_MISMATCH);
}

TEST_F(AesDecryptorTest, CorruptedData) {
  GenerateKeyRequest(key_id_);
  AddKeyAndExpect(key_id_, key_, KEY_ADDED);

  std::vector<uint8> bad_data = encrypted_data_;
  bad_data[1]++;

  scoped_refptr<DecoderBuffer> encrypted_buffer = CreateEncryptedBuffer(
      bad_data, key_id_, iv_, 0, no_subsample_entries_);
  DecryptAndExpect(encrypted_buffer, original_data_, DATA_MISMATCH);
}

TEST_F(AesDecryptorTest, EncryptedAsUnencryptedFailure) {
  GenerateKeyRequest(key_id_);
  AddKeyAndExpect(key_id_, key_, KEY_ADDED);
  scoped_refptr<DecoderBuffer> encrypted_buffer = CreateEncryptedBuffer(
      encrypted_data_, key_id_, std::vector<uint8>(), 0, no_subsample_entries_);
  DecryptAndExpect(encrypted_buffer, original_data_, DATA_MISMATCH);
}

TEST_F(AesDecryptorTest, SubsampleDecryption) {
  GenerateKeyRequest(key_id_);
  AddKeyAndExpect(key_id_, key_, KEY_ADDED);
  scoped_refptr<DecoderBuffer> encrypted_buffer = CreateEncryptedBuffer(
      subsample_encrypted_data_, key_id_, iv_, 0, normal_subsample_entries_);
  DecryptAndExpect(encrypted_buffer, original_data_, SUCCESS);
}

// Ensures noninterference of data offset and subsample mechanisms. We never
// expect to encounter this in the wild, but since the DecryptConfig doesn't
// disallow such a configuration, it should be covered.
TEST_F(AesDecryptorTest, SubsampleDecryptionWithOffset) {
  GenerateKeyRequest(key_id_);
  AddKeyAndExpect(key_id_, key_, KEY_ADDED);
  scoped_refptr<DecoderBuffer> encrypted_buffer = CreateEncryptedBuffer(
      subsample_encrypted_data_, key_id_, iv_, 23, normal_subsample_entries_);
  DecryptAndExpect(encrypted_buffer, original_data_, SUCCESS);
}

TEST_F(AesDecryptorTest, SubsampleWrongSize) {
  GenerateKeyRequest(key_id_);
  AddKeyAndExpect(key_id_, key_, KEY_ADDED);

  std::vector<SubsampleEntry> subsample_entries_wrong_size(
      kSubsampleEntriesWrongSize,
      kSubsampleEntriesWrongSize + arraysize(kSubsampleEntriesWrongSize));

  scoped_refptr<DecoderBuffer> encrypted_buffer = CreateEncryptedBuffer(
      subsample_encrypted_data_, key_id_, iv_, 0, subsample_entries_wrong_size);
  DecryptAndExpect(encrypted_buffer, original_data_, DATA_MISMATCH);
}

TEST_F(AesDecryptorTest, SubsampleInvalidTotalSize) {
  GenerateKeyRequest(key_id_);
  AddKeyAndExpect(key_id_, key_, KEY_ADDED);

  std::vector<SubsampleEntry> subsample_entries_invalid_total_size(
      kSubsampleEntriesInvalidTotalSize,
      kSubsampleEntriesInvalidTotalSize +
          arraysize(kSubsampleEntriesInvalidTotalSize));

  scoped_refptr<DecoderBuffer> encrypted_buffer = CreateEncryptedBuffer(
      subsample_encrypted_data_, key_id_, iv_, 0,
      subsample_entries_invalid_total_size);
  DecryptAndExpect(encrypted_buffer, original_data_, DECRYPT_ERROR);
}

// No cypher bytes in any of the subsamples.
TEST_F(AesDecryptorTest, SubsampleClearBytesOnly) {
  GenerateKeyRequest(key_id_);
  AddKeyAndExpect(key_id_, key_, KEY_ADDED);

  std::vector<SubsampleEntry> clear_only_subsample_entries(
      kSubsampleEntriesClearOnly,
      kSubsampleEntriesClearOnly + arraysize(kSubsampleEntriesClearOnly));

  scoped_refptr<DecoderBuffer> encrypted_buffer = CreateEncryptedBuffer(
      original_data_, key_id_, iv_, 0, clear_only_subsample_entries);
  DecryptAndExpect(encrypted_buffer, original_data_, SUCCESS);
}

// No clear bytes in any of the subsamples.
TEST_F(AesDecryptorTest, SubsampleCypherBytesOnly) {
  GenerateKeyRequest(key_id_);
  AddKeyAndExpect(key_id_, key_, KEY_ADDED);

  std::vector<SubsampleEntry> cypher_only_subsample_entries(
      kSubsampleEntriesCypherOnly,
      kSubsampleEntriesCypherOnly + arraysize(kSubsampleEntriesCypherOnly));

  scoped_refptr<DecoderBuffer> encrypted_buffer = CreateEncryptedBuffer(
      encrypted_data_, key_id_, iv_, 0, cypher_only_subsample_entries);
  DecryptAndExpect(encrypted_buffer, original_data_, SUCCESS);
}

}  // namespace media
