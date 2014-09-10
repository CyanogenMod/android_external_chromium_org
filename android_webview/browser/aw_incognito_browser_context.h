// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Copyright (c) 2013-2014 The Linux Foundation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ANDROID_WEBVIEW_BROWSER_AW_INCOGNITO_BROWSER_CONTEXT_H_
#define ANDROID_WEBVIEW_BROWSER_AW_INCOGNITO_BROWSER_CONTEXT_H_

#include <vector>

#include "android_webview/browser/aw_browser_context.h"
#include "android_webview/browser/aw_download_manager_delegate.h"
#include "base/basictypes.h"
#include "base/compiler_specific.h"
#include "base/files/file_path.h"
#include "base/memory/ref_counted.h"
#include "base/memory/scoped_ptr.h"
#include "content/public/browser/browser_context.h"
#include "content/public/browser/content_browser_client.h"
//SWE-FIXME remove comment after bringing geolocation changes.
//include "content/public/browser/geolocation_permission_context.h"
#include "net/url_request/url_request_job_factory.h"

class GURL;
class PrefService;

namespace content {
class ResourceContext;
class SSLHostStateDelegate;
class WebContents;
}

namespace data_reduction_proxy {
class DataReductionProxyConfigurator;
class DataReductionProxySettings;
}

namespace net {
class CookieStore;
}

namespace visitedlink {
class VisitedLinkMaster;
}

namespace android_webview {

class AwFormDatabaseService;
class AwQuotaManagerBridge;
class AwURLRequestIncognitoContextGetter;
class JniDependencyFactory;

class AwIncognitoBrowserContext : public AwBrowserContext {
 public:
  AwIncognitoBrowserContext(const base::FilePath path,
                   JniDependencyFactory* native_factory);
  virtual ~AwIncognitoBrowserContext();

  // Currently only one instance per process is supported.
  static AwIncognitoBrowserContext* GetDefault();

  // Convenience method to returns the AwIncognitoBrowserContext corresponding to the
  // given WebContents.
  static AwIncognitoBrowserContext* FromWebContents(
      content::WebContents* web_contents);

  //SWE-feature-incognito: For incognito, data reduction proxy is disabled.
  static void SetDataReductionProxyEnabled(bool enabled){};

  AwURLRequestIncognitoContextGetter* getURLRequestContextGetter();

  // Maps to BrowserMainParts::PreMainMessageLoopRun.
  void PreMainMessageLoopRun();

  void AddVisitedURLs(const std::vector<GURL>& urls){};


  net::URLRequestContextGetter* CreateRequestContext(
      content::ProtocolHandlerMap* protocol_handlers,
      content::URLRequestInterceptorScopedVector request_interceptors);

  virtual net::URLRequestContextGetter* CreateRequestContextForStoragePartition(
      const base::FilePath& partition_path,
      bool in_memory,
      content::ProtocolHandlerMap* protocol_handlers,
      content::URLRequestInterceptorScopedVector request_interceptors);

  AwQuotaManagerBridge* GetQuotaManagerBridge();

  data_reduction_proxy::DataReductionProxySettings*
      GetDataReductionProxySettings();

  virtual AwFormDatabaseService* GetFormDatabaseService();
  virtual void CreateUserPrefServiceIfNecessary();

  // SWE-feature-incognito: Methods to update number_of_refs
  void AddRef();
  void ReleaseRef();

  // content::BrowserContext implementation.
  virtual base::FilePath GetPath() const OVERRIDE;
  virtual bool IsOffTheRecord() const OVERRIDE;
  virtual net::URLRequestContextGetter* GetRequestContext() OVERRIDE;
  virtual net::URLRequestContextGetter* GetRequestContextForRenderProcess(
      int renderer_child_id) OVERRIDE;
  virtual net::URLRequestContextGetter* GetMediaRequestContext() OVERRIDE;
  virtual net::URLRequestContextGetter* GetMediaRequestContextForRenderProcess(
      int renderer_child_id) OVERRIDE;
  virtual net::URLRequestContextGetter*
      GetMediaRequestContextForStoragePartition(
          const base::FilePath& partition_path, bool in_memory) OVERRIDE;
  virtual content::ResourceContext* GetResourceContext() OVERRIDE;
  virtual content::DownloadManagerDelegate*
      GetDownloadManagerDelegate() OVERRIDE;
  virtual content::BrowserPluginGuestManager* GetGuestManager() OVERRIDE;
  virtual quota::SpecialStoragePolicy* GetSpecialStoragePolicy() OVERRIDE;
  virtual content::PushMessagingService* GetPushMessagingService() OVERRIDE;
  virtual content::SSLHostStateDelegate* GetSSLHostStateDelegate() OVERRIDE;

 private:
  static bool data_reduction_proxy_enabled_;

  // The file path where data for this context is persisted.
  base::FilePath context_storage_path_;

  JniDependencyFactory* native_factory_;
  scoped_refptr<net::CookieStore> cookie_store_;
  scoped_refptr<AwURLRequestIncognitoContextGetter> url_request_context_getter_;
  scoped_refptr<AwQuotaManagerBridge> quota_manager_bridge_;

  AwDownloadManagerDelegate download_manager_delegate_;

  scoped_ptr<content::ResourceContext> resource_context_;

  scoped_ptr<PrefService> user_pref_service_;

  scoped_ptr<data_reduction_proxy::DataReductionProxyConfigurator>
      data_reduction_proxy_configurator_;
  scoped_ptr<data_reduction_proxy::DataReductionProxySettings>
      data_reduction_proxy_settings_;

  // SWE-feature-incognito: The following keeps track of the number of incognito
  // tabs (AwContent instances) that have a reference to this object. This is
  // necessary to know when to invoke the appropriate cleanup functions when all
  // incognito tabs are closed.
  int number_of_refs;

  DISALLOW_COPY_AND_ASSIGN(AwIncognitoBrowserContext);
};

}  // namespace android_webview

#endif  // ANDROID_WEBVIEW_BROWSER_AW_INCOGNITO_BROWSER_CONTEXT_H_
