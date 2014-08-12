// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "remoting/codec/video_encoder.h"
#include "remoting/host/fake_host_extension.h"
#include "remoting/host/host_extension_session_manager.h"
#include "remoting/host/host_mock_objects.h"
#include "remoting/proto/control.pb.h"
#include "remoting/protocol/protocol_mock_objects.h"
#include "testing/gtest/include/gtest/gtest.h"
#include "third_party/webrtc/modules/desktop_capture/desktop_capturer.h"

namespace remoting {

class HostExtensionSessionManagerTest : public testing::Test {
 public:
  HostExtensionSessionManagerTest()
      : extension1_("ext1", "cap1"),
        extension2_("ext2", ""),
        extension3_("ext3", "cap3") {
    extensions_.push_back(&extension1_);
    extensions_.push_back(&extension2_);
    extensions_.push_back(&extension3_);
  }
  virtual ~HostExtensionSessionManagerTest() {}

 protected:
  // Fake HostExtensions for testing.
  FakeExtension extension1_;
  FakeExtension extension2_;
  FakeExtension extension3_;
  std::vector<HostExtension*> extensions_;

  // Mocks of interfaces provided by ClientSession.
  MockClientSessionControl client_session_control_;
  protocol::MockClientStub client_stub_;
};

// Verifies that messages are passed to be handled by the correct extension.
TEST_F(HostExtensionSessionManagerTest, ExtensionMessages_MessageHandled) {
  HostExtensionSessionManager extension_manager(extensions_,
                                                &client_session_control_);
  extension_manager.OnNegotiatedCapabilities(
      &client_stub_, extension_manager.GetCapabilities());

  protocol::ExtensionMessage message;
  message.set_type("ext2");
  extension_manager.OnExtensionMessage(message);

  EXPECT_FALSE(extension1_.has_handled_message());
  EXPECT_TRUE(extension2_.has_handled_message());
  EXPECT_FALSE(extension3_.has_handled_message());
}

// Verifies that extension messages not handled by extensions don't result in a
// crash.
TEST_F(HostExtensionSessionManagerTest, ExtensionMessages_MessageNotHandled) {
  HostExtensionSessionManager extension_manager(extensions_,
                                                &client_session_control_);
  extension_manager.OnNegotiatedCapabilities(
      &client_stub_, extension_manager.GetCapabilities());

  protocol::ExtensionMessage message;
  message.set_type("ext4");
  extension_manager.OnExtensionMessage(message);

  EXPECT_FALSE(extension1_.has_handled_message());
  EXPECT_FALSE(extension2_.has_handled_message());
  EXPECT_FALSE(extension3_.has_handled_message());
}

// Verifies that the correct set of capabilities are reported to the client,
// based on the registered extensions.
TEST_F(HostExtensionSessionManagerTest, ExtensionCapabilities_AreReported) {
  HostExtensionSessionManager extension_manager(extensions_,
                                                &client_session_control_);

  EXPECT_EQ(extension_manager.GetCapabilities(), "cap1 cap3");
}

// Verifies that an extension is not instantiated if the client does not
// support its required capability, and that it does not receive messages.
TEST_F(HostExtensionSessionManagerTest, ExtensionCapabilities_AreChecked) {
  HostExtensionSessionManager extension_manager(extensions_,
                                                &client_session_control_);
  extension_manager.OnNegotiatedCapabilities(&client_stub_, "cap1");

  protocol::ExtensionMessage message;
  message.set_type("ext3");
  extension_manager.OnExtensionMessage(message);

  EXPECT_TRUE(extension1_.was_instantiated());
  EXPECT_TRUE(extension2_.was_instantiated());
  EXPECT_FALSE(extension3_.was_instantiated());
}

// Verifies that matching extensions are given the opportunity to wrap or
// replace the video capturer.
TEST_F(HostExtensionSessionManagerTest, CanWrapVideoCapturer) {
  HostExtensionSessionManager extension_manager(extensions_,
                                                &client_session_control_);

  // Set up all the extensions to request to modify the video pipeline.
  extension1_.set_steal_video_capturer(true);
  extension2_.set_steal_video_capturer(true);
  extension3_.set_steal_video_capturer(true);
  extension_manager.OnNegotiatedCapabilities(&client_stub_, "cap1");

  extension_manager.OnCreateVideoCapturer(
      scoped_ptr<webrtc::DesktopCapturer>());

  EXPECT_FALSE(extension1_.has_wrapped_video_encoder());
  EXPECT_TRUE(extension1_.has_wrapped_video_capturer());
  EXPECT_FALSE(extension2_.has_wrapped_video_encoder());
  EXPECT_TRUE(extension2_.has_wrapped_video_capturer());
  EXPECT_FALSE(extension3_.was_instantiated());
}

// Verifies that matching extensions are given the opportunity to wrap or
// replace the video encoders.
TEST_F(HostExtensionSessionManagerTest, CanWrapVideoEncoder) {
  HostExtensionSessionManager extension_manager(extensions_,
                                                &client_session_control_);

  // Set up all the extensions to request to modify the video pipeline.
  extension1_.set_steal_video_capturer(true);
  extension2_.set_steal_video_capturer(true);
  extension3_.set_steal_video_capturer(true);
  extension_manager.OnNegotiatedCapabilities(&client_stub_, "cap1");

  extension_manager.OnCreateVideoEncoder(scoped_ptr<VideoEncoder>());

  EXPECT_TRUE(extension1_.has_wrapped_video_encoder());
  EXPECT_FALSE(extension1_.has_wrapped_video_capturer());
  EXPECT_TRUE(extension2_.has_wrapped_video_encoder());
  EXPECT_FALSE(extension2_.has_wrapped_video_capturer());
  EXPECT_FALSE(extension3_.was_instantiated());
}

// Verifies that only extensions which report that they modify the video
// pipeline actually get called to modify it.
TEST_F(HostExtensionSessionManagerTest, RespectModifiesVideoPipeline) {
  HostExtensionSessionManager extension_manager(extensions_,
                                                &client_session_control_);

  // Set up the second extension to request to modify the video pipeline.
  extension2_.set_steal_video_capturer(true);
  extension_manager.OnNegotiatedCapabilities(&client_stub_, "cap1");

  extension_manager.OnCreateVideoCapturer(
      scoped_ptr<webrtc::DesktopCapturer>());
  extension_manager.OnCreateVideoEncoder(scoped_ptr<VideoEncoder>());

  EXPECT_FALSE(extension1_.has_wrapped_video_encoder());
  EXPECT_FALSE(extension1_.has_wrapped_video_capturer());
  EXPECT_TRUE(extension2_.has_wrapped_video_encoder());
  EXPECT_TRUE(extension2_.has_wrapped_video_capturer());
  EXPECT_FALSE(extension3_.was_instantiated());
}

// Verifies that if an extension reports that they modify the video pipeline
// then ResetVideoPipeline() is called on the ClientSessionControl interface.
TEST_F(HostExtensionSessionManagerTest, CallsResetVideoPipeline) {
  HostExtensionSessionManager extension_manager(extensions_,
                                                &client_session_control_);

  EXPECT_CALL(client_session_control_, ResetVideoPipeline());

  // Set up only the first extension to request to modify the video pipeline.
  extension1_.set_steal_video_capturer(true);

  extension_manager.OnNegotiatedCapabilities(&client_stub_, "cap1");
}


}  // namespace remoting
