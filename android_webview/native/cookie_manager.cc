// Copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "android_webview/native/cookie_manager.h"

#include "android_webview/browser/aw_browser_context.h"
#include "android_webview/browser/aw_cookie_access_policy.h"
#include "android_webview/browser/scoped_allow_wait_for_legacy_web_view_api.h"
#include "android_webview/native/aw_browser_dependency_factory.h"
#include "base/android/jni_string.h"
#include "base/android/path_utils.h"
#include "base/bind.h"
#include "base/bind_helpers.h"
#include "base/file_util.h"
#include "base/files/file_path.h"
#include "base/lazy_instance.h"
#include "base/message_loop/message_loop.h"
#include "base/message_loop/message_loop_proxy.h"
#include "base/path_service.h"
#include "base/synchronization/lock.h"
#include "base/synchronization/waitable_event.h"
#include "base/threading/sequenced_worker_pool.h"
#include "base/threading/thread.h"
#include "base/threading/thread_restrictions.h"
#include "content/public/browser/browser_context.h"
#include "content/public/browser/browser_thread.h"
#include "content/public/browser/cookie_store_factory.h"
#include "content/public/common/url_constants.h"
#include "jni/AwCookieManager_jni.h"
#include "net/cookies/cookie_monster.h"
#include "net/cookies/cookie_options.h"
#include "net/url_request/url_request_context.h"

using base::FilePath;
using base::android::ConvertJavaStringToUTF8;
using base::android::ConvertJavaStringToUTF16;
using content::BrowserThread;
using net::CookieList;
using net::CookieMonster;

// In the future, we may instead want to inject an explicit CookieStore
// dependency into this object during process initialization to avoid
// depending on the URLRequestContext.
// See issue http://crbug.com/157683

// All functions on the CookieManager can be called from any thread, including
// threads without a message loop. BrowserThread::IO is used to call methods
// on CookieMonster that needs to be called, and called back, on a chrome
// thread.

namespace android_webview {

namespace {

// Are cookies allowed for file:// URLs by default?
const bool kDefaultFileSchemeAllowed = false;

void ImportLegacyCookieStore(const FilePath& cookie_store_path) {
  // We use the old cookie store to create the new cookie store only if the
  // new cookie store does not exist.
  if (base::PathExists(cookie_store_path))
    return;

  // WebViewClassic gets the database path from Context and appends a
  // hardcoded name. (see https://android.googlesource.com/platform/frameworks/base/+/bf6f6f9de72c9fd15e6bd/core/java/android/webkit/JniUtil.java and
  // https://android.googlesource.com/platform/external/webkit/+/7151ed0c74599/Source/WebKit/android/WebCoreSupport/WebCookieJar.cpp)
  FilePath old_cookie_store_path;
  base::android::GetDatabaseDirectory(&old_cookie_store_path);
  old_cookie_store_path = old_cookie_store_path.Append(
      FILE_PATH_LITERAL("webviewCookiesChromium.db"));
  if (base::PathExists(old_cookie_store_path) &&
      !base::Move(old_cookie_store_path, cookie_store_path)) {
         LOG(WARNING) << "Failed to move old cookie store path from "
                      << old_cookie_store_path.AsUTF8Unsafe() << " to "
                      << cookie_store_path.AsUTF8Unsafe();
  }
}

class CookieManager {
 public:
  static CookieManager* GetInstance();

  scoped_refptr<net::CookieStore> CreateCookieStore(
      AwBrowserContext* browser_context);

  void SetAcceptCookie(bool accept);
  bool AcceptCookie();
  void SetCookie(const GURL& host, const std::string& cookie_value);
  std::string GetCookie(const GURL& host);
  void RemoveSessionCookie();
  void RemoveAllCookie();
  void RemoveExpiredCookie();
  void FlushCookieStore();
  bool HasCookies();
  int CountCookies();
  bool AllowFileSchemeCookies();
  void SetAcceptFileSchemeCookies(bool accept);

 private:
  friend struct base::DefaultLazyInstanceTraits<CookieManager>;

  CookieManager();
  ~CookieManager();

  typedef base::Callback<void(base::WaitableEvent*)> CookieTask;
  void ExecCookieTask(const CookieTask& task,
                      const bool wait_for_completion);

  void SetCookieAsyncHelper(
      const GURL& host,
      const std::string& value,
      base::WaitableEvent* completion);
  void SetCookieCompleted(bool success);

  void GetCookieValueAsyncHelper(
      const GURL& host,
      std::string* result,
      base::WaitableEvent* completion);
  void GetCookieValueCompleted(base::WaitableEvent* completion,
                               std::string* result,
                               const std::string& value);

  void RemoveSessionCookieAsyncHelper(base::WaitableEvent* completion);
  void RemoveAllCookieAsyncHelper(base::WaitableEvent* completion);
  void RemoveCookiesCompleted(int num_deleted);

  void FlushCookieStoreAsyncHelper(base::WaitableEvent* completion);

  void CountCookiesAsyncHelper(int* result,
                             base::WaitableEvent* completion);
  void CountCookiesCompleted(base::WaitableEvent* completion,
                           int* result,
                           const CookieList& cookies);

  void CreateCookieMonster(
    const FilePath& user_data_dir,
    const scoped_refptr<base::SequencedTaskRunner>& client_task_runner,
    const scoped_refptr<base::SequencedTaskRunner>& background_task_runner);
  void EnsureCookieMonsterExistsLocked();
  bool AllowFileSchemeCookiesLocked();
  void SetAcceptFileSchemeCookiesLocked(bool accept);

  scoped_refptr<net::CookieMonster> cookie_monster_;
  scoped_refptr<base::MessageLoopProxy> cookie_monster_proxy_;
  base::Lock cookie_monster_lock_;

  // Both these threads are normally NULL. They only exist when the temporary
  // cookie monster is in use.
  scoped_ptr<base::Thread> cookie_monster_client_thread_;
  scoped_ptr<base::Thread> cookie_monster_backend_thread_;

  DISALLOW_COPY_AND_ASSIGN(CookieManager);
};

base::LazyInstance<CookieManager>::Leaky g_lazy_instance;

// static
CookieManager* CookieManager::GetInstance() {
  return g_lazy_instance.Pointer();
}

CookieManager::CookieManager() {
}

CookieManager::~CookieManager() {
}

void CookieManager::CreateCookieMonster(
    const FilePath& user_data_dir,
    const scoped_refptr<base::SequencedTaskRunner>& client_task_runner,
    const scoped_refptr<base::SequencedTaskRunner>& background_task_runner) {
  FilePath cookie_store_path =
      user_data_dir.Append(FILE_PATH_LITERAL("Cookies"));

  background_task_runner->PostTask(
      FROM_HERE,
      base::Bind(ImportLegacyCookieStore, cookie_store_path));

  net::CookieStore* cookie_store = content::CreatePersistentCookieStore(
    cookie_store_path,
    true,
    NULL,
    NULL,
    client_task_runner,
    background_task_runner);
  cookie_monster_ = cookie_store->GetCookieMonster();
  cookie_monster_->SetPersistSessionCookies(true);
}

void CookieManager::EnsureCookieMonsterExistsLocked() {
  cookie_monster_lock_.AssertAcquired();
  if (cookie_monster_.get()) {
    return;
  }

  // Create temporary cookie monster.
  FilePath user_data_dir;
  if (!PathService::Get(base::DIR_ANDROID_APP_DATA, &user_data_dir)) {
    NOTREACHED() << "Failed to get app data directory for Android WebView";
  }
  cookie_monster_client_thread_.reset(
      new base::Thread("TempCookieMonsterClient"));
  cookie_monster_client_thread_->Start();
  cookie_monster_proxy_ = cookie_monster_client_thread_->message_loop_proxy();
  cookie_monster_backend_thread_.reset(
      new base::Thread("TempCookieMonsterBackend"));
  cookie_monster_backend_thread_->Start();

  CreateCookieMonster(user_data_dir,
                      cookie_monster_proxy_,
                      cookie_monster_backend_thread_->message_loop_proxy());
  SetAcceptFileSchemeCookiesLocked(kDefaultFileSchemeAllowed);
}

// Executes the |task| on the FILE thread. |wait_for_completion| should only be
// true if the Java API method returns a value or is explicitly stated to be
// synchronous.
void CookieManager::ExecCookieTask(const CookieTask& task,
                                   const bool wait_for_completion) {
  base::WaitableEvent completion(false, false);
  base::AutoLock lock(cookie_monster_lock_);

  EnsureCookieMonsterExistsLocked();

  cookie_monster_proxy_->PostTask(FROM_HERE,
      base::Bind(task, wait_for_completion ? &completion : NULL));

  if (wait_for_completion) {
    ScopedAllowWaitForLegacyWebViewApi wait;
    completion.Wait();
  }
}

scoped_refptr<net::CookieStore> CookieManager::CreateCookieStore(
    AwBrowserContext* browser_context) {
  base::AutoLock lock(cookie_monster_lock_);
  bool accept_file_scheme_cookies = kDefaultFileSchemeAllowed;

  if (cookie_monster_client_thread_) {
    accept_file_scheme_cookies = AllowFileSchemeCookiesLocked();
    // We created a temporary cookie monster on its own threads, and now need to
    // carefully shut everything down in the right order before starting the
    // 'real' one.

    // 1. Flush any pending commits to the store and wait for this to complete.
    base::WaitableEvent flush_completion(false, false);
    base::Closure flush_callback =
        base::Bind(&base::WaitableEvent::Signal,
                   base::Unretained(&flush_completion));
    cookie_monster_proxy_->PostTask(FROM_HERE,
        base::Bind(&CookieMonster::FlushStore,
                   cookie_monster_,
                   flush_callback));
    flush_completion.Wait();

    // 2. Delete the cookie monster.
    cookie_monster_ = NULL;

    // 3. Shut down the threads.
    cookie_monster_client_thread_.reset();
    cookie_monster_backend_thread_.reset();
  }

  // Now go ahead and create the real cookie monster.
  DCHECK(!cookie_monster_.get());

  cookie_monster_proxy_ =
      BrowserThread::GetMessageLoopProxyForThread(BrowserThread::IO);
  scoped_refptr<base::SequencedTaskRunner> background_task_runner =
      BrowserThread::GetBlockingPool()->GetSequencedTaskRunner(
          BrowserThread::GetBlockingPool()->GetSequenceToken());

  CreateCookieMonster(browser_context->GetPath(), NULL, background_task_runner);
  SetAcceptFileSchemeCookiesLocked(accept_file_scheme_cookies);
  return cookie_monster_;
}

void CookieManager::SetAcceptCookie(bool accept) {
  AwCookieAccessPolicy::GetInstance()->SetGlobalAllowAccess(accept);
}

bool CookieManager::AcceptCookie() {
  return AwCookieAccessPolicy::GetInstance()->GetGlobalAllowAccess();
}

void CookieManager::SetCookie(const GURL& host,
                              const std::string& cookie_value) {
  ExecCookieTask(base::Bind(&CookieManager::SetCookieAsyncHelper,
                            base::Unretained(this),
                            host,
                            cookie_value), false);
}

void CookieManager::SetCookieAsyncHelper(
    const GURL& host,
    const std::string& value,
    base::WaitableEvent* completion) {
  DCHECK(!completion);
  net::CookieOptions options;
  options.set_include_httponly();

  cookie_monster_->SetCookieWithOptionsAsync(
      host, value, options,
      base::Bind(&CookieManager::SetCookieCompleted, base::Unretained(this)));
}

void CookieManager::SetCookieCompleted(bool success) {
  // The CookieManager API does not return a value for SetCookie,
  // so we don't need to propagate the |success| value back to the caller.
}

std::string CookieManager::GetCookie(const GURL& host) {
  std::string cookie_value;
  ExecCookieTask(base::Bind(&CookieManager::GetCookieValueAsyncHelper,
                            base::Unretained(this),
                            host,
                            &cookie_value), true);

  return cookie_value;
}

void CookieManager::GetCookieValueAsyncHelper(
    const GURL& host,
    std::string* result,
    base::WaitableEvent* completion) {
  net::CookieOptions options;
  options.set_include_httponly();

  cookie_monster_->GetCookiesWithOptionsAsync(
      host,
      options,
      base::Bind(&CookieManager::GetCookieValueCompleted,
                 base::Unretained(this),
                 completion,
                 result));
}

void CookieManager::GetCookieValueCompleted(base::WaitableEvent* completion,
                                            std::string* result,
                                            const std::string& value) {
  *result = value;
  DCHECK(completion);
  completion->Signal();
}

void CookieManager::RemoveSessionCookie() {
  ExecCookieTask(base::Bind(&CookieManager::RemoveSessionCookieAsyncHelper,
                            base::Unretained(this)), false);
}

void CookieManager::RemoveSessionCookieAsyncHelper(
    base::WaitableEvent* completion) {
  DCHECK(!completion);
  cookie_monster_->DeleteSessionCookiesAsync(
      base::Bind(&CookieManager::RemoveCookiesCompleted,
                 base::Unretained(this)));
}

void CookieManager::RemoveCookiesCompleted(int num_deleted) {
  // The CookieManager API does not return a value for removeSessionCookie or
  // removeAllCookie, so we don't need to propagate the |num_deleted| value back
  // to the caller.
}

void CookieManager::RemoveAllCookie() {
  ExecCookieTask(base::Bind(&CookieManager::RemoveAllCookieAsyncHelper,
                            base::Unretained(this)), false);
}

// TODO(kristianm): Pass a null callback so it will not be invoked
// across threads.
void CookieManager::RemoveAllCookieAsyncHelper(
    base::WaitableEvent* completion) {
  DCHECK(!completion);
  cookie_monster_->DeleteAllAsync(
      base::Bind(&CookieManager::RemoveCookiesCompleted,
                 base::Unretained(this)));
}

void CookieManager::RemoveExpiredCookie() {
  // HasCookies will call GetAllCookiesAsync, which in turn will force a GC.
  HasCookies();
}

void CookieManager::FlushCookieStoreAsyncHelper(
    base::WaitableEvent* completion) {
  DCHECK(!completion);
  cookie_monster_->FlushStore(base::Bind(&base::DoNothing));
}

void CookieManager::FlushCookieStore() {
  ExecCookieTask(base::Bind(&CookieManager::FlushCookieStoreAsyncHelper,
                            base::Unretained(this)), false);
}

bool CookieManager::HasCookies() {
  int count;
  ExecCookieTask(base::Bind(&CookieManager::CountCookiesAsyncHelper,
                            base::Unretained(this),
                            &count), true);
  return count != 0;
}

int CookieManager::CountCookies() {
  int count;
  ExecCookieTask(base::Bind(&CookieManager::CountCookiesAsyncHelper,
                            base::Unretained(this),
                            &count), true);
  return count;
}

// TODO(kristianm): Simplify this, copying the entire list around
// should not be needed.
void CookieManager::CountCookiesAsyncHelper(int* result,
                                  base::WaitableEvent* completion) {
  cookie_monster_->GetAllCookiesAsync(
      base::Bind(&CookieManager::CountCookiesCompleted,
                 base::Unretained(this),
                 completion,
                 result));
}

void CookieManager::CountCookiesCompleted(base::WaitableEvent* completion,
                                        int* result,
                                        const CookieList& cookies) {
  *result = cookies.size();
  DCHECK(completion);
  completion->Signal();
}

bool CookieManager::AllowFileSchemeCookies() {
  base::AutoLock lock(cookie_monster_lock_);
  EnsureCookieMonsterExistsLocked();
  return AllowFileSchemeCookiesLocked();
}

bool CookieManager::AllowFileSchemeCookiesLocked() {
  return cookie_monster_->IsCookieableScheme(chrome::kFileScheme);
}

void CookieManager::SetAcceptFileSchemeCookies(bool accept) {
  base::AutoLock lock(cookie_monster_lock_);
  EnsureCookieMonsterExistsLocked();
  SetAcceptFileSchemeCookiesLocked(accept);
}

void CookieManager::SetAcceptFileSchemeCookiesLocked(bool accept) {
  // The docs on CookieManager base class state the API must not be called after
  // creating a CookieManager instance (which contradicts its own internal
  // implementation) but this code does rely on the essence of that comment, as
  // the monster will DCHECK here if it has already been lazy initialized (i.e.
  // if cookies have been read or written from the store). If that turns out to
  // be a problemin future, it looks like it maybe possible to relax the DCHECK.
  cookie_monster_->SetEnableFileScheme(accept);
}

}  // namespace

static void SetAcceptCookie(JNIEnv* env, jobject obj, jboolean accept) {
  CookieManager::GetInstance()->SetAcceptCookie(accept);
}

static jboolean AcceptCookie(JNIEnv* env, jobject obj) {
  return CookieManager::GetInstance()->AcceptCookie();
}

static void SetCookie(JNIEnv* env, jobject obj, jstring url, jstring value) {
  GURL host(ConvertJavaStringToUTF16(env, url));
  std::string cookie_value(ConvertJavaStringToUTF8(env, value));

  CookieManager::GetInstance()->SetCookie(host, cookie_value);
}

static jstring GetCookie(JNIEnv* env, jobject obj, jstring url) {
  GURL host(ConvertJavaStringToUTF16(env, url));

  return base::android::ConvertUTF8ToJavaString(
      env,
      CookieManager::GetInstance()->GetCookie(host)).Release();
}

static void RemoveSessionCookie(JNIEnv* env, jobject obj) {
  CookieManager::GetInstance()->RemoveSessionCookie();
}

static int CountCookies(JNIEnv* env, jobject obj) {
  return CookieManager::GetInstance()->CountCookies();
}

static void RemoveAllCookie(JNIEnv* env, jobject obj) {
  CookieManager::GetInstance()->RemoveAllCookie();
}

static void RemoveExpiredCookie(JNIEnv* env, jobject obj) {
  CookieManager::GetInstance()->RemoveExpiredCookie();
}

static void FlushCookieStore(JNIEnv* env, jobject obj) {
  CookieManager::GetInstance()->FlushCookieStore();
}

static jboolean HasCookies(JNIEnv* env, jobject obj) {
  return CookieManager::GetInstance()->HasCookies();
}

static jboolean AllowFileSchemeCookies(JNIEnv* env, jobject obj) {
  return CookieManager::GetInstance()->AllowFileSchemeCookies();
}

static void SetAcceptFileSchemeCookies(JNIEnv* env, jobject obj,
                                       jboolean accept) {
  return CookieManager::GetInstance()->SetAcceptFileSchemeCookies(accept);
}

scoped_refptr<net::CookieStore> CreateCookieStore(
    AwBrowserContext* browser_context) {
  return CookieManager::GetInstance()->CreateCookieStore(browser_context);
}

bool RegisterCookieManager(JNIEnv* env) {
  return RegisterNativesImpl(env);
}

}  // android_webview namespace
