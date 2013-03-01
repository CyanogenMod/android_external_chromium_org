// Copyright (c) 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// ATL headers have to go first.
#include <atlbase.h>
#include <atlhost.h>

#include "base/basictypes.h"
#include "base/message_loop.h"
#include "base/run_loop.h"
#include "base/win/scoped_com_initializer.h"
#include "net/base/ip_endpoint.h"
#include "remoting/base/auto_thread_task_runner.h"
#include "remoting/host/win/rdp_client.h"
#include "testing/gmock/include/gmock/gmock.h"
#include "testing/gmock_mutant.h"
#include "testing/gtest/include/gtest/gtest.h"

using testing::_;
using testing::AtMost;
using testing::InvokeWithoutArgs;

namespace remoting {

namespace {

// Default width and hight of the RDP client window.
const long kDefaultWidth = 1024;
const long kDefaultHeight = 768;

class MockRdpClientEventHandler : public RdpClient::EventHandler {
 public:
  MockRdpClientEventHandler() {}
  virtual ~MockRdpClientEventHandler() {}

  MOCK_METHOD1(OnRdpConnected, void(const net::IPEndPoint&));
  MOCK_METHOD0(OnRdpClosed, void());

 private:
  DISALLOW_COPY_AND_ASSIGN(MockRdpClientEventHandler);
};

// a14498c6-7f3b-4e42-9605-6c4a20d53c87
static GUID RdpClientModuleLibid = {
  0xa14498c6,
  0x7f3b,
  0x4e42,
  { 0x96, 0x05, 0x6c, 0x4a, 0x20, 0xd5, 0x3c, 0x87 }
};

class RdpClientModule : public ATL::CAtlModuleT<RdpClientModule> {
 public:
  RdpClientModule();
  virtual ~RdpClientModule();

  DECLARE_LIBID(RdpClientModuleLibid)

 private:
  base::win::ScopedCOMInitializer com_initializer_;
};

RdpClientModule::RdpClientModule() {
  AtlAxWinInit();
}

RdpClientModule::~RdpClientModule() {
  AtlAxWinTerm();
  ATL::_pAtlModule = NULL;
}

}  // namespace

class RdpClientTest : public testing::Test {
 public:
  RdpClientTest();
  virtual ~RdpClientTest();

  virtual void SetUp() OVERRIDE;
  virtual void TearDown() OVERRIDE;

  // Tears down |rdp_client_|.
  void CloseRdpClient();

 protected:
  // The ATL module instance required by the ATL code.
  scoped_ptr<RdpClientModule> module_;

  // The UI message loop used by RdpClient. The loop is stopped once there is no
  // more references to |task_runner_|.
  MessageLoop message_loop_;
  base::RunLoop run_loop_;
  scoped_refptr<AutoThreadTaskRunner> task_runner_;

  // Mocks RdpClient::EventHandler for testing.
  MockRdpClientEventHandler event_handler_;

  // Points to the object being tested.
  scoped_ptr<RdpClient> rdp_client_;
};

RdpClientTest::RdpClientTest() : message_loop_(MessageLoop::TYPE_UI) {
}

RdpClientTest::~RdpClientTest() {
}

void RdpClientTest::SetUp() {
  // Arrange to run |message_loop_| until no components depend on it.
  task_runner_ = new AutoThreadTaskRunner(
      message_loop_.message_loop_proxy(), run_loop_.QuitClosure());

  module_.reset(new RdpClientModule());
}

void RdpClientTest::TearDown() {
  EXPECT_TRUE(!rdp_client_);

  module_.reset();
}

void RdpClientTest::CloseRdpClient() {
  EXPECT_TRUE(rdp_client_);

  rdp_client_.reset();
}

// Creates a loopback RDP connection.
TEST_F(RdpClientTest, Basic) {
  // An ability to establish a loopback RDP connection depends on many factors
  // including OS SKU and having RDP enabled. Accept both successful connection
  // and a connection error as a successful outcome.
  EXPECT_CALL(event_handler_, OnRdpConnected(_))
      .Times(AtMost(1))
      .WillOnce(InvokeWithoutArgs(this, &RdpClientTest::CloseRdpClient));
  EXPECT_CALL(event_handler_, OnRdpClosed())
      .Times(AtMost(1))
      .WillOnce(InvokeWithoutArgs(this, &RdpClientTest::CloseRdpClient));

  rdp_client_.reset(new RdpClient(task_runner_, task_runner_,
                                  SkISize::Make(kDefaultWidth, kDefaultHeight),
                                  &event_handler_));
  task_runner_ = NULL;

  run_loop_.Run();
}

}  // namespace remoting
