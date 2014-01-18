// Copyright (c) 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef TESTS_NACL_IO_TEST_FAKE_VAR_MANAGER_H_
#define TESTS_NACL_IO_TEST_FAKE_VAR_MANAGER_H_

#include <map>
#include <ppapi/c/pp_var.h>
#include <string>
#include <vector>

#include "sdk_util/macros.h"

struct FakeVarData {
  uint64_t id;
  uint64_t type;
  int32_t ref_count;
  std::string string_value;
  std::vector<PP_Var> array_value;
  struct {
    void* ptr;
    uint32_t length;
  } buffer_value;
};

class FakeVarManager {
 public:
  FakeVarManager();
  ~FakeVarManager();

  void AddRef(PP_Var var);
  void Release(PP_Var var);
  FakeVarData* CreateVarData();
  FakeVarData* GetVarData(PP_Var var);
  std::string Describe(const FakeVarData& resource);

  bool debug;
 private:
  void DestroyVarData(FakeVarData* var);

  typedef uint64_t Id;
  typedef std::map<Id, FakeVarData> VarMap;

  Id next_id_;
  VarMap var_map_;

  DISALLOW_COPY_AND_ASSIGN(FakeVarManager);
};

#endif  // TESTS_NACL_IO_TEST_FAKE_VAR_MANAGER_H_
