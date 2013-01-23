// Copyright (c) 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "device/bluetooth/bluetooth_init_win.h"

#include "base/threading/thread_restrictions.h"

namespace {

// A frame-based exception handler filter function for a handler for exceptions
// generated by the Visual C++ delay loader helper function.
int FilterVisualCPPExceptions(DWORD exception_code) {
  return HRESULT_FACILITY(exception_code) == FACILITY_VISUALCPP ?
      EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH;
}

}  // namespace

namespace device {
namespace bluetooth_init_win {

bool HasBluetoothStack() {
  static enum {
    HBS_UNKNOWN,
    HBS_YES,
    HBS_NO,
  } has_bluetooth_stack = HBS_UNKNOWN;

  if (has_bluetooth_stack == HBS_UNKNOWN) {
    base::ThreadRestrictions::AssertIOAllowed();
    HRESULT hr = E_FAIL;
    __try {
      hr = __HrLoadAllImportsForDll("bthprops.cpl");
    } __except(FilterVisualCPPExceptions(::GetExceptionCode())) {
      hr = E_FAIL;
    }
    has_bluetooth_stack = SUCCEEDED(hr) ? HBS_YES : HBS_NO;
  }

  return has_bluetooth_stack == HBS_YES;
}

}  // namespace bluetooth_init_win
}  // namespace device