// Copyright (c) 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef MEDIA_BASE_ANDROID_MEDIA_DRM_BRIDGE_H_
#define MEDIA_BASE_ANDROID_MEDIA_DRM_BRIDGE_H_

#include <jni.h>
#include <string>
#include <vector>

#include "base/android/scoped_java_ref.h"
#include "base/callback.h"
#include "media/base/media_export.h"
#include "media/base/media_keys.h"

namespace media {

class MediaPlayerManager;

// This class provides DRM services for android EME implementation.
// TODO(qinmin): implement all the functions in this class.
class MEDIA_EXPORT MediaDrmBridge : public MediaKeys {
 public:
  enum SecurityLevel {
    SECURITY_LEVEL_NONE = 0,
    SECURITY_LEVEL_1 = 1,
    SECURITY_LEVEL_3 = 3,
  };

  virtual ~MediaDrmBridge();

  // Returns a MediaDrmBridge instance if |scheme_uuid| is supported, or a NULL
  // pointer otherwise.
  static MediaDrmBridge* Create(int media_keys_id,
                                const std::vector<uint8>& scheme_uuid,
                                MediaPlayerManager* manager);

  // Checks whether MediaDRM is available.
  static bool IsAvailable();

  static bool RegisterMediaDrmBridge(JNIEnv* env);

  // MediaKeys implementations.
  virtual bool GenerateKeyRequest(const std::string& type,
                                  const uint8* init_data,
                                  int init_data_length) OVERRIDE;
  virtual void AddKey(const uint8* key, int key_length,
                      const uint8* init_data, int init_data_length,
                      const std::string& session_id) OVERRIDE;
  virtual void CancelKeyRequest(const std::string& session_id) OVERRIDE;

  // Returns a MediaCrypto object if it's already created. Returns a null object
  // otherwise.
  base::android::ScopedJavaLocalRef<jobject> GetMediaCrypto();

  // Sets callback which will be called when MediaCrypto is ready.
  // If |closure| is null, previously set callback will be cleared.
  void SetMediaCryptoReadyCB(const base::Closure& closure);

  // Called after a MediaCrypto object is created.
  void OnMediaCryptoReady(JNIEnv* env, jobject);

  // Called after we got the response for GenerateKeyRequest().
  void OnKeyMessage(JNIEnv* env, jobject, jstring j_session_id,
                    jbyteArray message, jstring destination_url);

  // Called when key is added.
  void OnKeyAdded(JNIEnv* env, jobject, jstring j_session_id);

  // Called when error happens.
  void OnKeyError(JNIEnv* env, jobject, jstring j_session_id);

  // Helper function to determine whether a protected surface is needed for the
  // video playback.
  bool IsProtectedSurfaceRequired();

  int media_keys_id() const { return media_keys_id_; }

 private:
  MediaDrmBridge(int media_keys_id,
                 const std::vector<uint8>& scheme_uuid,
                 MediaPlayerManager* manager);

  // Get the security level of the media.
  SecurityLevel GetSecurityLevel();

  // ID of the MediaKeys object.
  int media_keys_id_;

  // UUID of the key system.
  std::vector<uint8> scheme_uuid_;

  // Java MediaDrm instance.
  base::android::ScopedJavaGlobalRef<jobject> j_media_drm_;

  // Non-owned pointer.
  MediaPlayerManager* manager_;

  base::Closure media_crypto_ready_cb_;

  DISALLOW_COPY_AND_ASSIGN(MediaDrmBridge);
};

}  // namespace media

#endif  // MEDIA_BASE_ANDROID_MEDIA_DRM_BRIDGE_H_
