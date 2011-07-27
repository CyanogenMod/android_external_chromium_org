// Copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CONTENT_BROWSER_BROWSER_CONTEXT_H_
#define CONTENT_BROWSER_BROWSER_CONTEXT_H_
#pragma once

#include "base/hash_tables.h"

namespace net {
class URLRequestContextGetter;
}

namespace quota {
class QuotaManager;
}

namespace webkit_database {
class DatabaseTracker;
}

class ChromeBlobStorageContext;
class DownloadManager;
class FilePath;
class GeolocationContentSettingsMap;
class GeolocationPermissionContext;
class HostZoomMap;
class SSLHostState;
class WebKitContext;

namespace content {

class ResourceContext;

// This class holds the context needed for a browsing session.

class BrowserContext {
 public:
  // Returns the path of the directory where this context's data is stored.
  virtual FilePath GetPath() = 0;

  // Return whether this context is incognito. Default is false.
  // This doesn't belong here; http://crbug.com/89628
  virtual bool IsOffTheRecord() = 0;

  // Returns a pointer to the DatabaseTracker instance for this context.
  virtual webkit_database::DatabaseTracker* GetDatabaseTracker() = 0;

  // Retrieves a pointer to the SSLHostState associated with this context.
  // The SSLHostState is lazily created the first time that this method is
  // called.
  virtual SSLHostState* GetSSLHostState() = 0;

  // Returns the DownloadManager associated with this context.
  virtual DownloadManager* GetDownloadManager() = 0;
  virtual bool HasCreatedDownloadManager() const = 0;

  virtual quota::QuotaManager* GetQuotaManager() = 0;

  // Returns the request context information associated with this context.  Call
  // this only on the UI thread, since it can send notifications that should
  // happen on the UI thread.
  virtual net::URLRequestContextGetter* GetRequestContext() = 0;

  // Returns the request context appropriate for the given renderer. If the
  // renderer process doesn't have an associated installed app, or if the
  // installed app's is_storage_isolated() returns false, this is equivalent to
  // calling GetRequestContext().
  // TODO(creis): After isolated app storage is no longer an experimental
  // feature, consider making this the default contract for GetRequestContext.
  virtual net::URLRequestContextGetter* GetRequestContextForRenderProcess(
      int renderer_child_id) = 0;

  // Returns the request context for media resources asociated with this
  // context.
  virtual net::URLRequestContextGetter* GetRequestContextForMedia() = 0;

  // Returns the resource context.
  virtual const ResourceContext& GetResourceContext() = 0;

  // Returns the Hostname <-> Zoom Level map for this context.
  virtual HostZoomMap* GetHostZoomMap() = 0;

  // Returns the geolocation settings map for this context.
  virtual GeolocationContentSettingsMap* GetGeolocationContentSettingsMap() = 0;

  // Returns the geolocation permission context for this context.
  virtual GeolocationPermissionContext* GetGeolocationPermissionContext() = 0;

  // Returns true if the last time this context was open it was exited cleanly.
  // This doesn't belong here; http://crbug.com/76788
  virtual bool DidLastSessionExitCleanly() = 0;

  // Returns the WebKitContext assigned to this context.
  virtual WebKitContext* GetWebKitContext() = 0;

  // Returns a pointer to the ChromeBlobStorageContext instance for this
  // context.
  virtual ChromeBlobStorageContext* GetBlobStorageContext() = 0;
};

}  // namespace content

#if defined(COMPILER_GCC)
namespace __gnu_cxx {

template<>
struct hash<content::BrowserContext*> {
  std::size_t operator()(content::BrowserContext* const& p) const {
    return reinterpret_cast<std::size_t>(p);
  }
};

}  // namespace __gnu_cxx
#endif

#endif  // CONTENT_BROWSER_BROWSER_CONTEXT_H_
