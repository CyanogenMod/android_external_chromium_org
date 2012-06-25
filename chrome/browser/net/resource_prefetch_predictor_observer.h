// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_NET_RESOURCE_PREFETCH_PREDICTOR_OBSERVER_H_
#define CHROME_BROWSER_NET_RESOURCE_PREFETCH_PREDICTOR_OBSERVER_H_
#pragma once

#include "base/basictypes.h"
#include "base/memory/weak_ptr.h"
#include "chrome/browser/predictors/resource_prefetch_predictor.h"
#include "webkit/glue/resource_type.h"

namespace net {
class URLRequest;
}

namespace chrome_browser_net {

// Observes resource requests in the ResourceDispatcherHostDelegate and notifies
// the ResourcePrefetchPredictor about the ones it is interested in.
//  - Has an instance per profile, and is owned by the corresponding
//    ProfileIOData.
//  - Needs to be constructed on UI thread. Rest of the functions can only be
//    called on the IO thread.
class ResourcePrefetchPredictorObserver  {
 public:
  explicit ResourcePrefetchPredictorObserver(
      predictors::ResourcePrefetchPredictor* predictor);
  ~ResourcePrefetchPredictorObserver();

  // Parts of the ResourceDispatcherHostDelegate that we want to observe.
  void OnRequestStarted(net::URLRequest* request,
                        ResourceType::Type resource_type,
                        int child_id,
                        int route_id);
  void OnRequestRedirected(net::URLRequest* request);
  void OnResponseStarted(net::URLRequest* request);

 private:
  // Owned by profile.
  base::WeakPtr<predictors::ResourcePrefetchPredictor> predictor_;

  DISALLOW_COPY_AND_ASSIGN(ResourcePrefetchPredictorObserver);
};

}  // namespace chrome_browser_net

#endif  // CHROME_BROWSER_NET_RESOURCE_PREFETCH_PREDICTOR_OBSERVER_H_
