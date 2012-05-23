// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CONTENT_BROWSER_LOAD_FROM_MEMORY_CACHE_DETAILS_H_
#define CONTENT_BROWSER_LOAD_FROM_MEMORY_CACHE_DETAILS_H_
#pragma once

#include <string>
#include "base/basictypes.h"
#include "googleurl/src/gurl.h"
#include "net/base/cert_status_flags.h"
#include "webkit/glue/resource_type.h"

class LoadFromMemoryCacheDetails {
 public:
   LoadFromMemoryCacheDetails(
       const GURL& url,
       int pid,
       int cert_id,
       net::CertStatus cert_status,
       std::string http_method,
       std::string mime_type,
       ResourceType::Type resource_type);
  ~LoadFromMemoryCacheDetails();

  const GURL& url() const { return url_; }
  int pid() const { return pid_; }
  int ssl_cert_id() const { return cert_id_; }
  net::CertStatus ssl_cert_status() const { return cert_status_; }
  const std::string& http_method() const { return http_method_; }
  const std::string& mime_type() const { return mime_type_; }
  ResourceType::Type resource_type() const { return resource_type_; }

 private:
  GURL url_;
  int pid_;
  int cert_id_;
  net::CertStatus cert_status_;
  std::string http_method_;
  std::string mime_type_;
  ResourceType::Type resource_type_;

  DISALLOW_COPY_AND_ASSIGN(LoadFromMemoryCacheDetails);
};

#endif  // CONTENT_BROWSER_LOAD_FROM_MEMORY_CACHE_DETAILS_H_
