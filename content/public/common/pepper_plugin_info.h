// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CONTENT_PUBLIC_COMMON_PEPPER_PLUGIN_INFO_H_
#define CONTENT_PUBLIC_COMMON_PEPPER_PLUGIN_INFO_H_

#include <string>
#include <vector>

#include "base/files/file_path.h"
#include "content/common/content_export.h"
#include "content/public/common/webplugininfo.h"
#include "ppapi/c/pp_module.h"
#include "ppapi/c/ppb.h"

namespace content {

struct CONTENT_EXPORT PepperPluginInfo {
  typedef const void* (*GetInterfaceFunc)(const char*);
  typedef int (*PPP_InitializeModuleFunc)(PP_Module, PPB_GetInterface);
  typedef void (*PPP_ShutdownModuleFunc)();

  struct EntryPoints {
    // This structure is POD, with the constructor initializing to NULL.
    CONTENT_EXPORT EntryPoints();

    GetInterfaceFunc get_interface;
    PPP_InitializeModuleFunc initialize_module;
    PPP_ShutdownModuleFunc shutdown_module;  // Optional, may be NULL.
  };

  PepperPluginInfo();
  ~PepperPluginInfo();

  WebPluginInfo ToWebPluginInfo() const;

  // Indicates internal plugins for which there's not actually a library.
  // These plugins are implemented in the Chrome binary using a separate set
  // of entry points (see internal_entry_points below).
  // Defaults to false.
  bool is_internal;

  // True when this plugin should be run out of process. Defaults to false.
  bool is_out_of_process;

  // True when an out-of-process plugin should also be run within sandbox.
  // Defaults to true.
  bool is_sandboxed;

  base::FilePath path;  // Internal plugins have "internal-[name]" as path.
  std::string name;
  std::string description;
  std::string version;
  std::vector<WebPluginMimeType> mime_types;

  // When is_internal is set, this contains the function pointers to the
  // entry points for the internal plugins.
  EntryPoints internal_entry_points;

  // Permission bits from ppapi::Permission.
  uint32 permissions;
};

}  // namespace content

#endif  // CONTENT_PUBLIC_COMMON_PEPPER_PLUGIN_INFO_H_
