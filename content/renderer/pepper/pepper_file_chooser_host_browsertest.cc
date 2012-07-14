// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "base/file_path.h"
#include "base/utf_string_conversions.h"
#include "content/public/test/render_view_test.h"
#include "content/common/view_messages.h"
#include "content/public/common/file_chooser_params.h"
#include "content/renderer/pepper/pepper_file_chooser_host.h"
#include "content/renderer/pepper/pepper_instance_state_accessor.h"
#include "content/renderer/render_view_impl.h"
#include "content/test/test_content_client.h"
#include "ppapi/c/pp_errors.h"
#include "ppapi/host/host_message_context.h"
#include "ppapi/host/ppapi_host.h"
#include "ppapi/proxy/ppapi_messages.h"
#include "ppapi/proxy/resource_message_params.h"
#include "ppapi/proxy/resource_message_test_sink.h"
#include "ppapi/shared_impl/ppapi_permissions.h"
#include "ppapi/shared_impl/ppb_file_ref_shared.h"
#include "ppapi/shared_impl/resource_tracker.h"
#include "ppapi/shared_impl/test_globals.h"
#include "testing/gtest/include/gtest/gtest.h"
#include "ui/base/dialogs/selected_file_info.h"

namespace content {

namespace {

class PepperFileChooserHostTest : public RenderViewTest {
 public:
  PepperFileChooserHostTest() : pp_instance_(123456) {}

  virtual void SetUp() {
    SetContentClient(&client_);
    RenderViewTest::SetUp();

    globals_.GetResourceTracker()->DidCreateInstance(pp_instance_);
  }
  virtual void TearDown() {
    globals_.GetResourceTracker()->DidDeleteInstance(pp_instance_);

    RenderViewTest::TearDown();
    SetContentClient(NULL);
  }

  PP_Instance pp_instance() const { return pp_instance_; }

 private:
  PP_Instance pp_instance_;

  ppapi::TestGlobals globals_;
  TestContentClient client_;
};

// For testing to convert our hardcoded file paths to 8-bit.
std::string FilePathToUTF8(const FilePath::StringType& path) {
#if defined(OS_WIN)
  return UTF16ToUTF8(path);
#else
  return path;
#endif
}

class MockInstanceState : public PepperInstanceStateAccessor {
 public:
  MockInstanceState() : has_user_gesture_(true) {}
  virtual ~MockInstanceState() {}

  void set_has_user_gesture(bool has) { has_user_gesture_ = has; }

  // PepperInstanceStateAccessor.
  virtual bool IsValidInstance(PP_Instance instance) OVERRIDE {
    return true;
  }
  virtual bool HasUserGesture(PP_Instance instance) OVERRIDE {
    return has_user_gesture_;
  }

 private:
  bool has_user_gesture_;
};

}  // namespace

TEST_F(PepperFileChooserHostTest, Show) {
  PP_Resource pp_resource = 123;

  MockInstanceState state;
  ppapi::proxy::ResourceMessageTestSink sink;
  ppapi::host::PpapiHost host(&sink, NULL, ppapi::PpapiPermissions());
  RenderViewImpl* view_impl = static_cast<RenderViewImpl*>(view_);
  PepperFileChooserHost chooser(&host, pp_instance(), pp_resource, view_impl,
                                &state);

  std::vector<std::string> accept;
  accept.push_back("text/plain");
  PpapiHostMsg_FileChooser_Show show_msg(false, false, std::string(), accept);

  ppapi::proxy::ResourceMessageCallParams call_params(pp_resource, 0);
  ppapi::host::HostMessageContext context(call_params);
  int32 result = chooser.OnResourceMessageReceived(show_msg, &context);
  EXPECT_EQ(PP_OK_COMPLETIONPENDING, result);

  // The render view should have sent a chooser request to the browser
  // (caught by the render thread's test message sink).
  const IPC::Message* msg = render_thread_->sink().GetUniqueMessageMatching(
      ViewHostMsg_RunFileChooser::ID);
  ASSERT_TRUE(msg);
  ViewHostMsg_RunFileChooser::Schema::Param call_msg_param;
  ASSERT_TRUE(ViewHostMsg_RunFileChooser::Read(msg, &call_msg_param));
  const FileChooserParams& chooser_params = call_msg_param.a;

  // Basic validation of request.
  EXPECT_EQ(FileChooserParams::Open, chooser_params.mode);
  ASSERT_EQ(1u, chooser_params.accept_types.size());
  EXPECT_EQ(accept[0], UTF16ToUTF8(chooser_params.accept_types[0]));

  // Send a chooser reply to the render view. Note our reply path has to have a
  // path separator so we include both a Unix and a Windows one.
  ui::SelectedFileInfo selected_info;
  selected_info.display_name = FILE_PATH_LITERAL("Hello, world");
  selected_info.path = FilePath(FILE_PATH_LITERAL("myp\\ath/foo"));
  std::vector<ui::SelectedFileInfo> selected_info_vector;
  selected_info_vector.push_back(selected_info);
  ViewMsg_RunFileChooserResponse response(view_impl->routing_id(),
                                          selected_info_vector);
  EXPECT_TRUE(view_impl->OnMessageReceived(response));

  // This should have sent the Pepper reply to our test sink.
  ppapi::proxy::ResourceMessageReplyParams reply_params;
  IPC::Message reply_msg;
  ASSERT_TRUE(sink.GetFirstResourceReplyMatching(
      PpapiPluginMsg_FileChooser_ShowReply::ID, &reply_params, &reply_msg));

  // Basic validation of reply.
  EXPECT_EQ(call_params.sequence(), reply_params.sequence());
  EXPECT_EQ(PP_OK, reply_params.result());
  PpapiPluginMsg_FileChooser_ShowReply::Schema::Param reply_msg_param;
  ASSERT_TRUE(PpapiPluginMsg_FileChooser_ShowReply::Read(&reply_msg,
                                                         &reply_msg_param));
  const std::vector<ppapi::PPB_FileRef_CreateInfo>& chooser_results =
      reply_msg_param.a;
  ASSERT_EQ(1u, chooser_results.size());
  // Note path is empty because this is an external filesystem.
  EXPECT_EQ(std::string(), chooser_results[0].path);
  EXPECT_EQ(FilePathToUTF8(selected_info.display_name),
            chooser_results[0].name);
}

TEST_F(PepperFileChooserHostTest, NoUserGesture) {
  PP_Resource pp_resource = 123;

  MockInstanceState state;
  ppapi::proxy::ResourceMessageTestSink sink;
  ppapi::host::PpapiHost host(&sink, NULL, ppapi::PpapiPermissions());
  RenderViewImpl* view_impl = static_cast<RenderViewImpl*>(view_);
  PepperFileChooserHost chooser(&host, pp_instance(), pp_resource, view_impl,
                                &state);

  // Say there's no user gesture.
  state.set_has_user_gesture(false);

  std::vector<std::string> accept;
  accept.push_back("text/plain");
  PpapiHostMsg_FileChooser_Show show_msg(false, false, std::string(), accept);

  ppapi::proxy::ResourceMessageCallParams call_params(pp_resource, 0);
  ppapi::host::HostMessageContext context(call_params);
  int32 result = chooser.OnResourceMessageReceived(show_msg, &context);
  EXPECT_EQ(PP_ERROR_NO_USER_GESTURE, result);
}

}  // namespace content
