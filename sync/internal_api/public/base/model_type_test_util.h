// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SYNC_INTERNAL_PUBLIC_API_BASE_MODEL_TYPE_TEST_UTIL_H_
#define SYNC_INTERNAL_PUBLIC_API_BASE_MODEL_TYPE_TEST_UTIL_H_
#pragma once

#include <ostream>

#include "sync/internal_api/public/base/model_type.h"
#include "testing/gmock/include/gmock/gmock.h"

namespace syncer {

// Defined for googletest.  Forwards to ModelTypeSetToString().
void PrintTo(ModelTypeSet model_types, ::std::ostream* os);

// A gmock matcher for ModelTypeSet.  Use like:
//
//   EXPECT_CALL(mock, ProcessModelTypes(HasModelTypes(expected_types)));
::testing::Matcher<ModelTypeSet> HasModelTypes(ModelTypeSet expected_types);

}  // namespace syncer

#endif  // SYNC_INTERNAL_PUBLIC_API_BASE_MODEL_TYPE_TEST_UTIL_H_
