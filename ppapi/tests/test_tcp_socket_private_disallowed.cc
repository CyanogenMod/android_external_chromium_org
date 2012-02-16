// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ppapi/tests/test_tcp_socket_private_disallowed.h"

#include "ppapi/cpp/module.h"
#include "ppapi/tests/test_utils.h"
#include "ppapi/tests/testing_instance.h"

namespace {

const char kServerName[] = "www.google.com";
const int kPort = 80;

}

REGISTER_TEST_CASE(TCPSocketPrivateDisallowed);

TestTCPSocketPrivateDisallowed::TestTCPSocketPrivateDisallowed(
    TestingInstance* instance)
    : TestCase(instance), tcp_socket_private_interface_(NULL) {
}

bool TestTCPSocketPrivateDisallowed::Init() {
  tcp_socket_private_interface_ = static_cast<const PPB_TCPSocket_Private*>(
      pp::Module::Get()->GetBrowserInterface(PPB_TCPSOCKET_PRIVATE_INTERFACE));
  if (!tcp_socket_private_interface_)
    instance_->AppendError("TCPSocketPrivate interface not available");
  return tcp_socket_private_interface_ && CheckTestingInterface();
}

void TestTCPSocketPrivateDisallowed::RunTests(const std::string& filter) {
  RUN_TEST(Connect, filter);
}

std::string TestTCPSocketPrivateDisallowed::TestConnect() {
  PP_Resource socket =
      tcp_socket_private_interface_->Create(instance_->pp_instance());
  if (0 != socket) {
    TestCompletionCallback callback(instance_->pp_instance());
    int32_t rv = tcp_socket_private_interface_->Connect(
        socket, kServerName, kPort,
        static_cast<pp::CompletionCallback>(callback).pp_completion_callback());

    if (PP_OK_COMPLETIONPENDING == rv)
      rv = callback.WaitForResult();

    if (PP_ERROR_FAILED != rv)
      return "PPB_TCPSocket_Private can connect without allowing switch";
  }
  PASS();
}
