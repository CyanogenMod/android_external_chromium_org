// Copyright (c) 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "base/command_line.h"
#include "base/strings/string_util.h"
#include "base/strings/utf_string_conversions.h"
#include "chrome/browser/extensions/extension_function_test_utils.h"
#include "chrome/browser/extensions/extension_service.h"
#include "chrome/browser/extensions/suspicious_extension_bubble.h"
#include "chrome/browser/extensions/suspicious_extension_bubble_controller.h"
#include "chrome/browser/extensions/test_extension_system.h"
#include "chrome/test/base/testing_profile.h"
#include "content/public/test/test_browser_thread_bundle.h"
#include "extensions/common/extension.h"

namespace extensions {

// A test class for the SuspiciousExtensionBubbleController.
class TestSuspiciousExtensionBubbleController
    : public SuspiciousExtensionBubbleController {
 public:
  explicit TestSuspiciousExtensionBubbleController(Profile* profile)
      : SuspiciousExtensionBubbleController(profile),
        dismiss_button_callback_count_(0),
        link_click_callback_count_(0) {
  }

  // Returns how often the dismiss button has been called.
  size_t dismiss_click_count() {
    return dismiss_button_callback_count_;
  }

  // Returns how often the link has been clicked.
  size_t link_click_count() {
    return link_click_callback_count_;
  }

  // Callbacks from bubble.
  virtual void OnBubbleDismiss() OVERRIDE {
    ++dismiss_button_callback_count_;
    SuspiciousExtensionBubbleController::OnBubbleDismiss();
  }
  virtual void OnLinkClicked() OVERRIDE {
    ++link_click_callback_count_;
    SuspiciousExtensionBubbleController::OnLinkClicked();
  }

 private:
  size_t dismiss_button_callback_count_;
  size_t link_click_callback_count_;
};

// A fake bubble used for testing the controller. Takes an action that specifies
// what should happen when the bubble is "shown" (the bubble is actually not
// shown, the corresponding action is taken immediately).
class FakeSuspiciousExtensionBubble : public SuspiciousExtensionBubble {
 public:
  enum SuspiciousExtensionBubbleAction {
    BUBBLE_ACTION_CLICK_BUTTON = 0,
    BUBBLE_ACTION_CLICK_LINK
  };

  FakeSuspiciousExtensionBubble() {}

  void set_action_on_show(SuspiciousExtensionBubbleAction action) {
   action_ = action;
  }

  virtual void Show() OVERRIDE {
    if (action_ == BUBBLE_ACTION_CLICK_BUTTON)
      button_callback_.Run();
    else if (action_ == BUBBLE_ACTION_CLICK_LINK)
      link_callback_.Run();
  }

  virtual void OnButtonClicked(const base::Closure& callback) OVERRIDE {
    button_callback_ = callback;
  }

  virtual void OnLinkClicked(const base::Closure& callback) OVERRIDE {
    link_callback_ = callback;
  }

 private:
  SuspiciousExtensionBubbleAction action_;

  base::Closure button_callback_;
  base::Closure link_callback_;
};

class SuspiciousExtensionBubbleTest : public testing::Test {
 public:
  SuspiciousExtensionBubbleTest() {
    // The two lines of magical incantation required to get the extension
    // service to work inside a unit test and access the extension prefs.
    thread_bundle_.reset(new content::TestBrowserThreadBundle);
    profile_.reset(new TestingProfile);
  }
  virtual ~SuspiciousExtensionBubbleTest() {}
  virtual void SetUp() {
    command_line_.reset(new CommandLine(CommandLine::NO_PROGRAM));
  }
  scoped_ptr<CommandLine> command_line_;

 protected:
  scoped_refptr<Extension> CreateExtension(
      Manifest::Location location,
      const std::string& data,
      const std::string& id) {
    scoped_ptr<base::DictionaryValue> parsed_manifest(
        extension_function_test_utils::ParseDictionary(data));
    return extension_function_test_utils::CreateExtension(
        location,
        parsed_manifest.get(),
        id);
  }

  scoped_ptr<content::TestBrowserThreadBundle> thread_bundle_;
  scoped_ptr<TestingProfile> profile_;

 private:
  DISALLOW_COPY_AND_ASSIGN(SuspiciousExtensionBubbleTest);
};

// The feature this is meant to test is only implemented on Windows.
#if defined(OS_WIN)
#define MAYBE_ControllerTest ControllerTest
#else
#define MAYBE_ControllerTest DISABLED_ControllerTest
#endif

TEST_F(SuspiciousExtensionBubbleTest, MAYBE_ControllerTest) {
  std::string basic_extension =
      "{\"name\": \"Extension #\","
      "\"version\": \"1.0\","
      "\"manifest_version\": 2}";

  std::string extension_data;
  base::ReplaceChars(basic_extension, "#", "1", &extension_data);
  scoped_refptr<Extension> my_test_extension1(
      CreateExtension(
          Manifest::COMMAND_LINE,
          extension_data,
          "Autogenerated 1"));

  base::ReplaceChars(basic_extension, "#", "2", &extension_data);
  scoped_refptr<Extension> my_test_extension2(
      CreateExtension(
          Manifest::COMMAND_LINE,
          extension_data,
          "Autogenerated 2"));

  base::ReplaceChars(basic_extension, "#", "3", &extension_data);
  scoped_refptr<Extension> regular_extension(
      CreateExtension(
          Manifest::COMMAND_LINE,
          extension_data,
          "UnimportantId"));

  std::string extension_id1 = my_test_extension1->id();
  std::string extension_id2 = my_test_extension2->id();

  static_cast<TestExtensionSystem*>(
      ExtensionSystem::Get(profile_.get()))->CreateExtensionService(
          CommandLine::ForCurrentProcess(),
          base::FilePath(),
          false);
  ExtensionService* service = profile_->GetExtensionService();
  service->Init();

  service->AddExtension(regular_extension);
  service->AddExtension(my_test_extension1);
  service->AddExtension(my_test_extension2);

  scoped_ptr<TestSuspiciousExtensionBubbleController> controller(
      new TestSuspiciousExtensionBubbleController(profile_.get()));
  FakeSuspiciousExtensionBubble bubble;
  bubble.set_action_on_show(
      FakeSuspiciousExtensionBubble::BUBBLE_ACTION_CLICK_BUTTON);

  // Validate that we don't have a suppress value for the extensions.
  ExtensionPrefs* prefs = service->extension_prefs();
  EXPECT_FALSE(prefs->HasWipeoutBeenAcknowledged(extension_id1));
  EXPECT_FALSE(prefs->HasWipeoutBeenAcknowledged(extension_id2));

  EXPECT_FALSE(controller->HasSuspiciousExtensions());
  std::vector<base::string16> suspicious_extensions =
      controller->GetSuspiciousExtensionNames();
  EXPECT_EQ(0U, suspicious_extensions.size());
  EXPECT_EQ(0U, controller->link_click_count());
  EXPECT_EQ(0U, controller->dismiss_click_count());

  // Now disable an extension, specifying the wipeout flag.
  service->DisableExtension(extension_id1,
                            Extension::DISABLE_NOT_VERIFIED);

  EXPECT_FALSE(prefs->HasWipeoutBeenAcknowledged(extension_id1));
  EXPECT_FALSE(prefs->HasWipeoutBeenAcknowledged(extension_id2));
  controller.reset(new TestSuspiciousExtensionBubbleController(profile_.get()));
  EXPECT_TRUE(controller->HasSuspiciousExtensions());
  suspicious_extensions = controller->GetSuspiciousExtensionNames();
  ASSERT_EQ(1U, suspicious_extensions.size());
  EXPECT_TRUE(ASCIIToUTF16("Extension 1") == suspicious_extensions[0]);
  controller->Show(&bubble);  // Simulate showing the bubble.
  EXPECT_EQ(0U, controller->link_click_count());
  EXPECT_EQ(1U, controller->dismiss_click_count());
  // Now the acknowledge flag should be set only for the first extension.
  EXPECT_TRUE(prefs->HasWipeoutBeenAcknowledged(extension_id1));
  EXPECT_FALSE(prefs->HasWipeoutBeenAcknowledged(extension_id2));
  // Clear the flag.
  prefs->SetWipeoutAcknowledged(extension_id1, false);
  EXPECT_FALSE(prefs->HasWipeoutBeenAcknowledged(extension_id1));

  // Now disable the other extension and exercise the link click code path.
  service->DisableExtension(extension_id2,
                            Extension::DISABLE_NOT_VERIFIED);

  bubble.set_action_on_show(
      FakeSuspiciousExtensionBubble::BUBBLE_ACTION_CLICK_LINK);
  controller.reset(new TestSuspiciousExtensionBubbleController(profile_.get()));
  EXPECT_TRUE(controller->HasSuspiciousExtensions());
  suspicious_extensions = controller->GetSuspiciousExtensionNames();
  ASSERT_EQ(2U, suspicious_extensions.size());
  EXPECT_TRUE(ASCIIToUTF16("Extension 1") == suspicious_extensions[1]);
  EXPECT_TRUE(ASCIIToUTF16("Extension 2") == suspicious_extensions[0]);
  controller->Show(&bubble);  // Simulate showing the bubble.
  EXPECT_EQ(1U, controller->link_click_count());
  EXPECT_EQ(0U, controller->dismiss_click_count());
}

}  // namespace extensions
