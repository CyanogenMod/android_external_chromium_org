// Copyright (c) 2011 The Native Client Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// This is an internal header. Do not include in your test implementation.

#ifndef NATIVE_CLIENT_TESTS_PPAPI_TEST_PPB_TEMPLATE_INTERNAL_UTILS_H
#define NATIVE_CLIENT_TESTS_PPAPI_TEST_PPB_TEMPLATE_INTERNAL_UTILS_H

#include "native_client/src/include/nacl_string.h"

#include "ppapi/c/pp_instance.h"
#include "ppapi/c/pp_module.h"
#include "ppapi/c/pp_resource.h"
#include "ppapi/c/pp_var.h"
#include "ppapi/c/ppb.h"

void set_ppb_get_interface(PPB_GetInterface get_interface);
void set_pp_instance(PP_Instance instance);
void set_pp_module(PP_Module module);
PPB_GetInterface ppb_get_interface();
PP_Module pp_module();
PP_Instance pp_instance();

PP_Var GetScriptableObject(PP_Instance instance);

bool HasScriptableTest(nacl::string test_name);
PP_Var RunScriptableTest(nacl::string test_name);

void RunTest(nacl::string test_name);
#endif  // NATIVE_CLIENT_TESTS_PPAPI_TEST_PPB_TEMPLATE_INTERNAL_UTILS_H
