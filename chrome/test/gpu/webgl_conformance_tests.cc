// Copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "base/file_util.h"
#include "base/path_service.h"
#include "base/test/test_timeouts.h"
#include "base/utf_string_conversions.h"
#include "chrome/common/chrome_paths.h"
#include "chrome/test/automation/tab_proxy.h"
#include "chrome/test/base/test_launcher_utils.h"
#include "chrome/test/ui/javascript_test_util.h"
#include "chrome/test/ui/ui_test.h"
#include "content/public/common/content_switches.h"
#include "net/base/net_util.h"
#include "ui/gfx/gl/gl_implementation.h"

namespace {

#if defined(OS_WIN)
const std::string& kGLImplementationName = gfx::kGLImplementationEGLName;
#else
const std::string& kGLImplementationName = gfx::kGLImplementationDesktopName;
#endif

class WebGLConformanceTests : public UITest {
 public:
  WebGLConformanceTests() {
    show_window_ = true;
    dom_automation_enabled_ = true;
  }

  void SetUp() {
    // Force the use of GPU hardware.
    force_use_osmesa_ = false;

    // Ensure that a GPU bot is never blacklisted.
    launch_arguments_.AppendSwitch(switches::kIgnoreGpuBlacklist);
    UITest::SetUp();
  }

  void RunTest(const std::string& url) {
    FilePath webgl_conformance_path;
    PathService::Get(base::DIR_SOURCE_ROOT, &webgl_conformance_path);
    webgl_conformance_path = webgl_conformance_path.Append(
        FILE_PATH_LITERAL("third_party"));
    webgl_conformance_path = webgl_conformance_path.Append(
        FILE_PATH_LITERAL("webgl_conformance"));
    ASSERT_TRUE(file_util::DirectoryExists(webgl_conformance_path))
        << "Missing conformance tests: " << webgl_conformance_path.value();

    FilePath test_path;
    PathService::Get(chrome::DIR_TEST_DATA, &test_path);
    test_path = test_path.Append(FILE_PATH_LITERAL("gpu"));
    test_path = test_path.Append(FILE_PATH_LITERAL("webgl_conformance.html"));

    scoped_refptr<TabProxy> tab(GetActiveTab());
    ASSERT_TRUE(tab.get());

    ASSERT_EQ(AUTOMATION_MSG_NAVIGATION_SUCCESS,
              tab->NavigateToURL(net::FilePathToFileURL(test_path)));

    ASSERT_TRUE(tab->NavigateToURLAsync(
        GURL("javascript:start('" + url + "');")));

    // Block until the test completes.
    ASSERT_TRUE(WaitUntilJavaScriptCondition(
        tab, L"", L"window.domAutomationController.send(!running);",
        TestTimeouts::large_test_timeout_ms()));

    // Read out the test result.
    std::wstring result, message;
    ASSERT_TRUE(tab->ExecuteAndExtractString(
        L"",
        L"window.domAutomationController.send(JSON.stringify(result));",
        &result));
    ASSERT_TRUE(tab->ExecuteAndExtractString(
        L"",
        L"window.domAutomationController.send(message);",
        &message));

    EXPECT_EQ(WideToUTF8(result),"true") << WideToUTF8(message);
  }
};

#define CONFORMANCE_TEST(name, url) \
TEST_F(WebGLConformanceTests, name) { \
  RunTest(url); \
}

// The test declarations are located in webgl_conformance_test_list_autogen.h,
// because the list is automatically generated by a script.
// See: generate_webgl_conformance_test_list.py
#include "webgl_conformance_test_list_autogen.h"

}  // namespace
