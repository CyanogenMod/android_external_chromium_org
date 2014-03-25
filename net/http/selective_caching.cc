/*
 * Copyright (C) 2014, The Linux Foundation. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *   * Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *   * Redistributions in binary form must reproduce the above
 *     copyright notice, this list of conditions and the following
 *     disclaimer in the documentation and/or other materials provided
 *     with the distribution.
 *   * Neither the name of The Linux Foundation nor the names of its
 *     contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED "AS IS" AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
 * BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
 * OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
 * IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */

#include "base/memory/ref_counted.h"
#include "net/http/http_response_headers.h"
#include "net/libnetxt/libnetxt_base.h"
#include "net/stat_hub/stat_hub_cmd_api.h"
#include "net/stat_hub/stat_hub_def.h"

#define SELECTIVE_CACHING_VER           "34.0.1"
#define SELECTIVE_CACHING_ENABLED        1

namespace net{
namespace selective_caching {
static const char* __attribute__((__used__)) kPropNameEnabled = "net.select.caching.on";
static bool feature_enabled_ = false;

static const char* __attribute__((__used__))  kPropNameVerbose = "net.select.caching.verbose";
static int verbose_level_ = STAT_HUB_VERBOSE_LEVEL_DISABLED;

static void InitOnce() {
  static bool initialized = false;
  if (!initialized) {
    initialized = true;

    char value[PROPERTY_VALUE_MAX] = {'\0'};
    std::string version_str;

    LIBNETXT_PROPERTY_GET(kPropNameEnabled, value, PROP_VAL_TO_STR(SELECTIVE_CACHING_ENABLED));
    feature_enabled_ = (bool)atoi(value);

    LIBNETXT_PROPERTY_GET(kPropNameVerbose, value, PROP_VAL_TO_STR(STAT_HUB_VERBOSE_LEVEL_DISABLED));
    verbose_level_ = atoi(value);

    LIBNETXT_LOGI("SELECT_CACHE - Selective Caching is %s, Version: %s",
      (feature_enabled_)?"ON":"OFF", SELECTIVE_CACHING_VER);
  }
}

bool ShouldSelectivlyCache(scoped_refptr<HttpResponseHeaders> response_headers, std::string cache_key) {
  InitOnce();

  if (!feature_enabled_)
    return false;

  std::string cache_control_value;

  int64 body_size = response_headers->GetContentLength();
  bool is_size = body_size >=0 && body_size < 5120;
  bool is_type = !response_headers->HasHeaderValue("content-type", "text/html");
  bool is_cache_control = response_headers->GetNormalizedHeader("cache-control", &cache_control_value) &&
    cache_control_value=="max-age=0";

  bool enable_selective_caching = is_cache_control && is_size && is_type;

  if (verbose_level_ >= STAT_HUB_VERBOSE_LEVEL_INFO) {
    LIBNETXT_LOGI("SELECT_CACHE - %s Cache-Control:%d Content-Length:%d Content-Type %d url:%s",
      enable_selective_caching?"non-storable":"storable", is_cache_control, is_size, is_type, cache_key.c_str());
  }

  return enable_selective_caching;
}
} //namespace net
} //namespace selective_caching
