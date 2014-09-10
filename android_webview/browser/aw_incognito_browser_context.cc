// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Copyright (c) 2013-2014 The Linux Foundation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "android_webview/browser/aw_incognito_browser_context.h"

#include "android_webview/browser/aw_form_database_service.h"
#include "android_webview/browser/aw_pref_store.h"
#include "android_webview/browser/aw_quota_manager_bridge.h"
#include "android_webview/browser/aw_resource_context.h"
#include "android_webview/browser/jni_dependency_factory.h"
#include "android_webview/browser/net/aw_url_request_incognito_context_getter.h"
#include "android_webview/browser/net/init_native_callback.h"
#include "base/bind.h"
#include "base/prefs/pref_registry_simple.h"
#include "base/prefs/pref_service.h"
#include "base/prefs/pref_service_factory.h"
#include "components/autofill/core/common/autofill_pref_names.h"
#include "components/data_reduction_proxy/browser/data_reduction_proxy_config_service.h"
#include "components/data_reduction_proxy/browser/data_reduction_proxy_params.h"
#include "components/data_reduction_proxy/browser/data_reduction_proxy_prefs.h"
#include "components/data_reduction_proxy/browser/data_reduction_proxy_settings.h"
#include "components/user_prefs/user_prefs.h"
#include "content/public/browser/browser_thread.h"
#include "content/public/browser/ssl_host_state_delegate.h"
#include "content/public/browser/storage_partition.h"
#include "content/public/browser/web_contents.h"
#include "net/cookies/cookie_monster.h"
#include "net/cookies/cookie_store.h"
#include "net/proxy/proxy_service.h"

using base::FilePath;
using content::BrowserThread;
using data_reduction_proxy::DataReductionProxyConfigService;
using data_reduction_proxy::DataReductionProxySettings;

namespace android_webview {

namespace {

// Shows notifications which correspond to PersistentPrefStore's reading errors.
void HandleReadError(PersistentPrefStore::PrefReadError error) {
}

AwIncognitoBrowserContext* g_browser_context_incognito = NULL;

}  // namespace

// SWE-feature-incognito: For incognito data reduction proxy is disabled
bool AwIncognitoBrowserContext::data_reduction_proxy_enabled_ = false;


AwIncognitoBrowserContext::AwIncognitoBrowserContext(
    const FilePath path,
    JniDependencyFactory* native_factory)
    : AwBrowserContext(),
      context_storage_path_(path),
      native_factory_(native_factory),
      number_of_refs(0) {
  DCHECK(g_browser_context_incognito == NULL);
  g_browser_context_incognito = this;

  // This constructor is entered during the creation of ContentBrowserClient,
  // before browser threads are created. Therefore any checks to enforce
  // threading (such as BrowserThread::CurrentlyOn()) will fail here.
}

AwIncognitoBrowserContext::~AwIncognitoBrowserContext() {
  DCHECK(g_browser_context_incognito == this);
  g_browser_context_incognito = NULL;
}

// static
AwIncognitoBrowserContext* AwIncognitoBrowserContext::GetDefault() {
  // TODO(joth): rather than store in a global here, lookup this instance
  // from the Java-side peer.
  return g_browser_context_incognito;
}

// static
AwIncognitoBrowserContext* AwIncognitoBrowserContext::FromWebContents(
    content::WebContents* web_contents) {
  // This is safe; this is the only implementation of the browser context.
  return static_cast<AwIncognitoBrowserContext*>(static_cast<BrowserContext*>(web_contents->GetBrowserContext()));
}

AwURLRequestIncognitoContextGetter* AwIncognitoBrowserContext::getURLRequestContextGetter() {
  CHECK(url_request_context_getter_.get());
  return url_request_context_getter_.get();
}

void AwIncognitoBrowserContext::PreMainMessageLoopRun() {
  cookie_store_ = new net::CookieMonster(NULL, NULL);
#if defined(SPDY_PROXY_AUTH_ORIGIN)
  data_reduction_proxy_settings_.reset(
      new DataReductionProxySettings(
          new data_reduction_proxy::DataReductionProxyParams(
              data_reduction_proxy::DataReductionProxyParams::kAllowed)));
#endif
  scoped_ptr<DataReductionProxyConfigService>
      data_reduction_proxy_config_service(
          new DataReductionProxyConfigService(
              scoped_ptr<net::ProxyConfigService>(
                  net::ProxyService::CreateSystemProxyConfigService(
                      BrowserThread::GetMessageLoopProxyForThread(
                          BrowserThread::IO),
                          NULL /* Ignored on Android */)).Pass()));
  if (data_reduction_proxy_settings_.get()) {
      data_reduction_proxy_configurator_.reset(
          new data_reduction_proxy::DataReductionProxyConfigTracker(
              base::Bind(&DataReductionProxyConfigService::UpdateProxyConfig,
                         base::Unretained(
                             data_reduction_proxy_config_service.get())),
            BrowserThread::GetMessageLoopProxyForThread(BrowserThread::IO)));
    data_reduction_proxy_settings_->SetProxyConfigurator(
        data_reduction_proxy_configurator_.get());
  }

  url_request_context_getter_ =
      new AwURLRequestIncognitoContextGetter(
                                    cookie_store_.get(),
                                    data_reduction_proxy_config_service.Pass());
}

net::URLRequestContextGetter* AwIncognitoBrowserContext::CreateRequestContext(
    content::ProtocolHandlerMap* protocol_handlers,
    content::URLRequestInterceptorScopedVector request_interceptors) {
  // This function cannot actually create the request context because
  // there is a reentrant dependency on GetResourceContext() via
  // content::StoragePartitionImplMap::Create(). This is not fixable
  // until http://crbug.com/159193. Until then, assert that the context
  // has already been allocated and just handle setting the protocol_handlers.
  DCHECK(url_request_context_getter_);
  url_request_context_getter_->SetHandlersAndInterceptors(
      protocol_handlers, request_interceptors.Pass());
  return url_request_context_getter_;
}

net::URLRequestContextGetter*
AwIncognitoBrowserContext::CreateRequestContextForStoragePartition(
    const base::FilePath& partition_path,
    bool in_memory,
    content::ProtocolHandlerMap* protocol_handlers,
    content::URLRequestInterceptorScopedVector request_interceptors) {
  NOTREACHED();
  return NULL;
}

AwQuotaManagerBridge* AwIncognitoBrowserContext::GetQuotaManagerBridge() {
  NOTREACHED();
  return NULL;
}

AwFormDatabaseService* AwIncognitoBrowserContext::GetFormDatabaseService() {
  AwBrowserContext* awBrowserContext = android_webview::AwBrowserContext::GetDefault();
  return awBrowserContext->GetFormDatabaseService();
}

DataReductionProxySettings* AwIncognitoBrowserContext::GetDataReductionProxySettings() {
  return data_reduction_proxy_settings_.get();
}

// Create user pref service for autofill functionality.
void AwIncognitoBrowserContext::CreateUserPrefServiceIfNecessary() {
  if (user_pref_service_)
    return;

  PrefRegistrySimple* pref_registry = new PrefRegistrySimple();
  // We only use the autocomplete feature of the Autofill, which is
  // controlled via the manager_delegate. We don't use the rest
  // of autofill, which is why it is hardcoded as disabled here.
  pref_registry->RegisterBooleanPref(
      autofill::prefs::kAutofillEnabled, false);
  pref_registry->RegisterDoublePref(
      autofill::prefs::kAutofillPositiveUploadRate, 0.0);
  pref_registry->RegisterDoublePref(
      autofill::prefs::kAutofillNegativeUploadRate, 0.0);
  data_reduction_proxy::RegisterSimpleProfilePrefs(pref_registry);
  data_reduction_proxy::RegisterPrefs(pref_registry);

  base::PrefServiceFactory pref_service_factory;
  pref_service_factory.set_user_prefs(make_scoped_refptr(new AwPrefStore()));
  pref_service_factory.set_read_error_callback(base::Bind(&HandleReadError));
  user_pref_service_ = pref_service_factory.Create(pref_registry).Pass();

  user_prefs::UserPrefs::Set(this, user_pref_service_.get());

  if (data_reduction_proxy_settings_.get()) {
    data_reduction_proxy_settings_->InitDataReductionProxySettings(
        user_pref_service_.get(),
        user_pref_service_.get(),
        GetRequestContext());

    data_reduction_proxy_settings_->SetDataReductionProxyEnabled(
        data_reduction_proxy_enabled_);
  }
}

base::FilePath AwIncognitoBrowserContext::GetPath() const {
  return context_storage_path_;
}

bool AwIncognitoBrowserContext::IsOffTheRecord() const {
  return true;
}

net::URLRequestContextGetter* AwIncognitoBrowserContext::GetRequestContext() {
  return GetDefaultStoragePartition(this)->GetURLRequestContext();
}

net::URLRequestContextGetter*
AwIncognitoBrowserContext::GetRequestContextForRenderProcess(
    int renderer_child_id) {
  return GetRequestContext();
}

net::URLRequestContextGetter* AwIncognitoBrowserContext::GetMediaRequestContext() {
  return GetRequestContext();
}

net::URLRequestContextGetter*
AwIncognitoBrowserContext::GetMediaRequestContextForRenderProcess(
    int renderer_child_id) {
  return GetRequestContext();
}

net::URLRequestContextGetter*
AwIncognitoBrowserContext::GetMediaRequestContextForStoragePartition(
    const base::FilePath& partition_path,
    bool in_memory) {
  NOTREACHED();
  return NULL;
}

content::ResourceContext* AwIncognitoBrowserContext::GetResourceContext() {
  if (!resource_context_) {
    resource_context_.reset(
        new AwResourceContext(url_request_context_getter_.get()));
  }
  return resource_context_.get();
}

content::DownloadManagerDelegate*
AwIncognitoBrowserContext::GetDownloadManagerDelegate() {
  return &download_manager_delegate_;
}

content::BrowserPluginGuestManager* AwIncognitoBrowserContext::GetGuestManager() {
  return NULL;
}

quota::SpecialStoragePolicy* AwIncognitoBrowserContext::GetSpecialStoragePolicy() {
  // Intentionally returning NULL as 'Extensions' and 'Apps' not supported.
  return NULL;
}

content::PushMessagingService* AwIncognitoBrowserContext::GetPushMessagingService() {
  // TODO(johnme): Support push messaging in WebView.
  return NULL;
}

content::SSLHostStateDelegate* AwIncognitoBrowserContext::GetSSLHostStateDelegate() {
  return NULL;
}

//SWE-feature-incognito: AddRef and ReleaseRef are used to ensure that to know
//when to invoke the appropriate cleanup functions when all incognito tabs are
//closed.
void AwIncognitoBrowserContext::AddRef() {
  ++number_of_refs;
}

void AwIncognitoBrowserContext::ReleaseRef() {
  DCHECK(BrowserThread::CurrentlyOn(BrowserThread::UI));
  --number_of_refs;
  if (number_of_refs == 0 && url_request_context_getter_.get()) {

    // Clear all cookies
    BrowserThread::PostTask(BrowserThread::IO, FROM_HERE,
      base::Bind(&AwURLRequestIncognitoContextGetter::CleanUp,
                 url_request_context_getter_.get()));
  }
}

}  // namespace android_webview
