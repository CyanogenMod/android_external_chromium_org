// Copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef WEBKIT_PLUGINS_PPAPI_TIME_CONVERSION_H_
#define WEBKIT_PLUGINS_PPAPI_TIME_CONVERSION_H_

#include "base/time.h"
#include "ppapi/c/pp_time.h"

namespace webkit {
namespace ppapi {

PP_Time TimeToPPTime(base::Time t);
base::Time PPTimeToTime(PP_Time t);

PP_TimeTicks TimeTicksToPPTimeTicks(base::TimeTicks t);

// Converts between WebKit event times and time ticks. WebKit event times are
// currently expressed in terms of wall clock time. This function does the
// proper mapping to time ticks, assuming the wall clock time doesn't change
// (which isn't necessarily the case).
PP_TimeTicks EventTimeToPPTimeTicks(double event_time);
double PPTimeTicksToEventTime(PP_TimeTicks t);

}  // namespace ppapi
}  // namespace webkit

#endif  // WEBKIT_PLUGINS_PPAPI_TIME_CONVERSION_H_
