/*
 * Copyright 2014 The Chromium Authors. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "native_client/src/untrusted/irt/irt.h"
#include "native_client/src/untrusted/irt/irt_private.h"
#include "ppapi/nacl_irt/irt_ppapi.h"
#include "ppapi/native_client/src/shared/ppapi_proxy/ppruntime.h"

struct PP_StartFunctions g_pp_functions;

static int irt_ppapi_start(const struct PP_StartFunctions* funcs) {
  /* Disable NaCl's open_resource() interface on this thread. */
  g_is_main_thread = 1;

  g_pp_functions = *funcs;
  return PpapiPluginMain();
}

int32_t PPP_InitializeModule(PP_Module module_id,
                             PPB_GetInterface get_browser_interface) {
  return g_pp_functions.PPP_InitializeModule(module_id, get_browser_interface);
}

void PPP_ShutdownModule(void) {
  g_pp_functions.PPP_ShutdownModule();
}

const void* PPP_GetInterface(const char* interface_name) {
  return g_pp_functions.PPP_GetInterface(interface_name);
}

const struct nacl_irt_ppapihook nacl_irt_ppapihook = {
  irt_ppapi_start,
  PpapiPluginRegisterThreadCreator,
};
