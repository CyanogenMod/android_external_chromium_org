// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CONTENT_BROWSER_SERVICE_WORKER_SERVICE_WORKER_VERSION_H_
#define CONTENT_BROWSER_SERVICE_WORKER_SERVICE_WORKER_VERSION_H_

#include "base/basictypes.h"
#include "base/callback.h"
#include "base/gtest_prod_util.h"
#include "base/memory/ref_counted.h"
#include "base/memory/scoped_ptr.h"
#include "content/browser/service_worker/embedded_worker_instance.h"
#include "content/common/content_export.h"
#include "content/common/service_worker/service_worker_status_code.h"

class GURL;

namespace content {

class EmbeddedWorkerRegistry;
class ServiceWorkerProviderHost;
class ServiceWorkerRegistration;
struct ServiceWorkerFetchRequest;

// This class corresponds to a specific version of a ServiceWorker
// script for a given pattern. When a script is upgraded, there may be
// more than one ServiceWorkerVersion "running" at a time, but only
// one of them is active. This class connects the actual script with a
// running worker.
// Instances of this class are in one of two install states:
// - Pending: The script is in the process of being installed. There
//            may be another active script running.
// - Active: The script is the only worker handling requests for the
//           registration's pattern.
//
// In addition, a version has a running state (this is a rough
// sketch). Since a service worker can be stopped and started at any
// time, it will transition among these states multiple times during
// its lifetime.
// - Stopped: The script is not running
// - Starting: A request to fire an event against the version has been
//             queued, but the worker is not yet
//             loaded/initialized/etc.
// - Started: The worker is ready to receive events
// - Stopping: The worker is returning to the stopped state.
//
// The worker can "run" in both the Pending and the Active
// install states above. During the Pending state, the worker is only
// started in order to fire the 'install' and 'activate'
// events. During the Active state, it can receive other events such
// as 'fetch'.
//
// And finally, is_shutdown_ is detects the live-ness of the object
// itself. If the object is shut down, then it is in the process of
// being deleted from memory. This happens when a version is replaced
// as well as at browser shutdown.
class CONTENT_EXPORT ServiceWorkerVersion
    : NON_EXPORTED_BASE(public base::RefCounted<ServiceWorkerVersion>) {
 public:
  typedef base::Callback<void(ServiceWorkerStatusCode)> StatusCallback;

  enum Status {
    STOPPED = EmbeddedWorkerInstance::STOPPED,
    STARTING = EmbeddedWorkerInstance::STARTING,
    RUNNING = EmbeddedWorkerInstance::RUNNING,
    STOPPING = EmbeddedWorkerInstance::STOPPING,
  };

  ServiceWorkerVersion(
      ServiceWorkerRegistration* registration,
      EmbeddedWorkerRegistry* worker_registry,
      int64 version_id);

  int64 version_id() const { return version_id_; }

  void Shutdown();
  bool is_shutdown() const { return is_shutdown_; }

  Status status() const {
    return static_cast<Status>(embedded_worker_->status());
  }

  // Starts an embedded worker for this version.
  // It is not valid to call this while there's other inflight start or
  // stop process running.
  // This returns OK (success) if the worker is already running.
  void StartWorker(const StatusCallback& callback);

  // Starts an embedded worker for this version.
  // It is not valid to call this while there's other inflight start or
  // stop process running.
  // This returns OK (success) if the worker is already stopped.
  void StopWorker(const StatusCallback& callback);

  // Sends fetch event to the associated embedded worker.
  // This immediately returns false if the worker is not running
  // or sending a message to the child process fails.
  // TODO(kinuko): Make this take callback as well.
  bool DispatchFetchEvent(const ServiceWorkerFetchRequest& request);

  // These are expected to be called when a renderer process host for the
  // same-origin as for this ServiceWorkerVersion is created.  The added
  // processes are used to run an in-renderer embedded worker.
  void AddProcessToWorker(int process_id);
  void RemoveProcessToWorker(int process_id);

  EmbeddedWorkerInstance* embedded_worker() { return embedded_worker_.get(); }

 private:
  friend class base::RefCounted<ServiceWorkerVersion>;

  // Embedded worker observer classes.
  class WorkerObserverBase;
  class StartObserver;
  class StopObserver;

  ~ServiceWorkerVersion();

  const int64 version_id_;

  bool is_shutdown_;
  scoped_refptr<ServiceWorkerRegistration> registration_;

  scoped_ptr<EmbeddedWorkerInstance> embedded_worker_;
  scoped_ptr<EmbeddedWorkerInstance::Observer> observer_;

  DISALLOW_COPY_AND_ASSIGN(ServiceWorkerVersion);
};

}  // namespace content

#endif  // CONTENT_BROWSER_SERVICE_WORKER_SERVICE_WORKER_VERSION_H_
