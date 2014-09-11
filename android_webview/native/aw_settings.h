// Copyright (c) 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ANDROID_WEBVIEW_NATIVE_AW_SETTINGS_H_
#define ANDROID_WEBVIEW_NATIVE_AW_SETTINGS_H_

#include <jni.h>

#include "base/android/jni_weak_ref.h"
#include "base/android/scoped_java_ref.h"
#include "base/memory/scoped_ptr.h"
#include "content/public/browser/web_contents_observer.h"

namespace content{
struct WebPreferences;
}

using base::android::ScopedJavaLocalRef;

namespace content {
  class RenderViewHost;
}

namespace android_webview {

class AwRenderViewHostExt;

class AwSettings : public content::WebContentsObserver {
 public:
  static AwSettings* FromWebContents(content::WebContents* web_contents);

  AwSettings(JNIEnv* env, jobject obj, jlong web_contents);
  virtual ~AwSettings();

  // Called from Java. Methods with "Locked" suffix require that the settings
  // access lock is held during their execution.
  void Destroy(JNIEnv* env, jobject obj);
  void PopulateWebPreferencesLocked(JNIEnv* env, jobject obj, jlong web_prefs);
  void ResetScrollAndScaleState(JNIEnv* env, jobject obj);
  void UpdateEverythingLocked(JNIEnv* env, jobject obj);
  void UpdateInitialPageScaleLocked(JNIEnv* env, jobject obj);
  void UpdateUserAgentLocked(JNIEnv* env, jobject obj);
  void UpdateWebkitPreferencesLocked(JNIEnv* env, jobject obj);
  void UpdateFormDataPreferencesLocked(JNIEnv* env, jobject obj);
  void UpdateRendererPreferencesLocked(JNIEnv* env, jobject obj);
  void UpdateDoNotTrackLocked(JNIEnv* env, jobject obj, jboolean flag);
  void RemoveAutoFillProfile(JNIEnv* env, jobject obj, jstring uniqueId);
  ScopedJavaLocalRef<jstring> AddorUpdateAutoFillProfile(JNIEnv* env, jobject obj, jstring uniqueId,
          jstring fullName, jstring emailAddress,
          jstring companyName, jstring addressLine1, jstring addressLine2,
          jstring city, jstring state, jstring zipCode, jstring country,
          jstring phoneNumber );
  void RemoveAllAutoFillProfiles(JNIEnv* env, jobject obj);
  ScopedJavaLocalRef<jobjectArray> GetAllAutoFillProfiles(JNIEnv* env, jobject obj);
  ScopedJavaLocalRef<jobject> GetAutoFillProfile(JNIEnv* env, jobject obj, jstring uniqueId);

  void PopulateWebPreferences(content::WebPreferences* web_prefs);
  void SetSavePassword(JNIEnv* env, jobject obj, jboolean save);
  bool GetSavePassword();
  void ClearPasswords(JNIEnv* env, jobject obj);

 private:
  AwRenderViewHostExt* GetAwRenderViewHostExt();
  void UpdateEverything();

  // WebContentsObserver overrides:
  virtual void RenderViewCreated(
      content::RenderViewHost* render_view_host) OVERRIDE;
  virtual void WebContentsDestroyed() OVERRIDE;

  bool renderer_prefs_initialized_;

  JavaObjectWeakGlobalRef aw_settings_;
  content::RenderViewHost* render_view_host_;
};

bool RegisterAwSettings(JNIEnv* env);

}  // namespace android_webview

#endif  // ANDROID_WEBVIEW_NATIVE_AW_SETTINGS_H_
