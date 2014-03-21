/* Copyright (c) 2014 The Chromium Authors. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

/* From dev/ppp_find_dev.idl modified Wed Mar 19 14:02:22 2014. */

#ifndef PPAPI_C_DEV_PPP_FIND_DEV_H_
#define PPAPI_C_DEV_PPP_FIND_DEV_H_

#include "ppapi/c/pp_bool.h"
#include "ppapi/c/pp_instance.h"
#include "ppapi/c/pp_macros.h"
#include "ppapi/c/pp_stdint.h"

#define PPP_FIND_DEV_INTERFACE_0_3 "PPP_Find(Dev);0.3"
#define PPP_FIND_DEV_INTERFACE PPP_FIND_DEV_INTERFACE_0_3

/**
 * @file
 */


/**
 * @addtogroup Interfaces
 * @{
 */
/**
 * TODO(raymes): Make PPP/PPB_Find_Dev a private interface.
 */
struct PPP_Find_Dev_0_3 {
  /**
   * Finds the given UTF-8 text starting at the current selection. The number of
   * results will be updated asynchronously via NumberOfFindResultsChanged in
   * PPB_Find. Note that multiple StartFind calls can happen before StopFind is
   * called in the case of the search term changing.
   *
   * Return PP_FALSE if the plugin doesn't support find in page. Consequently,
   * it won't call any callbacks.
   */
  PP_Bool (*StartFind)(PP_Instance instance,
                       const char* text,
                       PP_Bool case_sensitive);
  /**
   * Go to the next/previous result.
   */
  void (*SelectFindResult)(PP_Instance instance, PP_Bool forward);
  /**
   * Tells the plugin that the find operation has stopped, so it should clear
   * any highlighting.
   */
  void (*StopFind)(PP_Instance instance);
};

typedef struct PPP_Find_Dev_0_3 PPP_Find_Dev;
/**
 * @}
 */

#endif  /* PPAPI_C_DEV_PPP_FIND_DEV_H_ */

