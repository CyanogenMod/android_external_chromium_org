// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ash/system/network/tray_sms.h"

#include "ash/ash_switches.h"
#include "ash/shell.h"
#include "ash/system/tray/system_tray.h"
#include "ash/system/tray/tray_constants.h"
#include "ash/system/tray/tray_details_view.h"
#include "ash/system/tray/tray_item_more.h"
#include "ash/system/tray/tray_item_view.h"
#include "ash/system/tray/tray_views.h"
#include "base/command_line.h"
#include "base/string_number_conversions.h"
#include "base/utf_string_conversions.h"
#include "base/values.h"
#include "grit/ash_strings.h"
#include "grit/ui_resources.h"
#include "grit/ui_resources_standard.h"
#include "ui/base/l10n/l10n_util.h"
#include "ui/base/resource/resource_bundle.h"
#include "ui/views/controls/image_view.h"
#include "ui/views/controls/label.h"
#include "ui/views/layout/box_layout.h"
#include "ui/views/layout/fill_layout.h"
#include "ui/views/layout/grid_layout.h"
#include "ui/views/view.h"

#if defined(OS_CHROMEOS)
#include "chromeos/network/network_sms_handler.h"
#endif

namespace {

// Min height of the list of messages in the popup.
const int kMessageListMinHeight = 200;
// Top/bottom padding of the text items.
const int kPaddingVertical = 10;

}  // namespace

namespace ash {
namespace internal {

class SmsObserverBase {
 public:
  explicit SmsObserverBase(TraySms* tray) : tray_(tray) {}
  virtual ~SmsObserverBase() {}

  virtual bool GetMessageFromDictionary(const base::DictionaryValue* message,
                                        std::string* number,
                                        std::string* text) {
    return false;
  }

  virtual void RequestUpdate() {
  }

  void RemoveMessage(size_t index) {
    if (index < messages_.GetSize())
      messages_.Remove(index, NULL);
  }

  const base::ListValue& messages() const { return messages_; }

 protected:
  base::ListValue messages_;
  TraySms* tray_;

 private:
  DISALLOW_COPY_AND_ASSIGN(SmsObserverBase);
};

#if defined(OS_CHROMEOS)

class TraySms::SmsObserver : public SmsObserverBase,
                             public chromeos::NetworkSmsHandler::Observer {
 public:
  explicit SmsObserver(TraySms* tray) : SmsObserverBase(tray) {
    if (!CommandLine::ForCurrentProcess()->HasSwitch(switches::kAshNotify))
      return;
    sms_handler_.reset(new chromeos::NetworkSmsHandler());
    sms_handler_->AddObserver(this);
    sms_handler_->Init();
  }

  virtual ~SmsObserver() {
    if (sms_handler_.get())
      sms_handler_->RemoveObserver(this);
  }

  // Overridden from SmsObserverBase
  virtual bool GetMessageFromDictionary(const base::DictionaryValue* message,
                                        std::string* number,
                                        std::string* text) OVERRIDE {
    if (!message->GetStringWithoutPathExpansion(
            chromeos::NetworkSmsHandler::kNumberKey, number))
      return false;
    if (!message->GetStringWithoutPathExpansion(
            chromeos::NetworkSmsHandler::kTextKey, text))
      return false;
    return true;
  }

  // Requests an immediate check for new messages.
  virtual void RequestUpdate() OVERRIDE {
    if (sms_handler_.get())
      sms_handler_->RequestUpdate();
  }

  // Overridden from chromeos::NetworkSmsHandler::Observer
  virtual void MessageReceived(const base::DictionaryValue& message) {
    messages_.Append(message.DeepCopy());
    tray_->Update(true);
  }

 private:
  scoped_ptr<chromeos::NetworkSmsHandler> sms_handler_;

  DISALLOW_COPY_AND_ASSIGN(SmsObserver);
};

#else  // OS_CHROMEOS

class TraySms::SmsObserver : public SmsObserverBase {
 public:
  explicit SmsObserver(TraySms* tray) : SmsObserverBase(tray) {
  }
  virtual ~SmsObserver() {}

 private:
  DISALLOW_COPY_AND_ASSIGN(SmsObserver);
};

#endif  // OS_CHROMEOS

class TraySms::SmsDefaultView : public TrayItemMore {
 public:
  explicit SmsDefaultView(TraySms* tray)
      : TrayItemMore(tray),
        tray_(tray) {
    SetImage(ResourceBundle::GetSharedInstance().GetImageSkiaNamed(
        IDR_AURA_UBER_TRAY_SMS));
    Update();
  }

  virtual ~SmsDefaultView() {}

  void Update() {
    int message_count =
        static_cast<int>(tray_->sms_observer()->messages().GetSize());
    string16 label = l10n_util::GetStringFUTF16(
        IDS_ASH_STATUS_TRAY_SMS_MESSAGES, base::IntToString16(message_count));
    SetLabel(label);
    SetAccessibleName(label);
  }

 private:
  TraySms* tray_;

  DISALLOW_COPY_AND_ASSIGN(SmsDefaultView);
};

// An entry (row) in SmsDetailedView or NotificationView.
class TraySms::SmsMessageView : public views::View,
                                public views::ButtonListener {
 public:
  enum ViewType {
    VIEW_DETAILED,
    VIEW_NOTIFICATION
  };

  SmsMessageView(TraySms* tray,
                 ViewType view_type,
                 size_t index,
                 const std::string& number,
                 const std::string& message)
      : tray_(tray),
        index_(index) {
    number_label_ = new views::Label(
        l10n_util::GetStringFUTF16(IDS_ASH_STATUS_TRAY_SMS_NUMBER,
                                   UTF8ToUTF16(number)));
    number_label_->SetHorizontalAlignment(views::Label::ALIGN_LEFT);
    number_label_->SetFont(
        number_label_->font().DeriveFont(0, gfx::Font::BOLD));

    message_label_ = new views::Label(UTF8ToUTF16(message));
    message_label_->SetHorizontalAlignment(views::Label::ALIGN_LEFT);
    message_label_->SetMultiLine(true);

    if (view_type == VIEW_DETAILED)
      LayoutDetailedView();
    else
      LayoutNotificationView();
  }

  virtual ~SmsMessageView() {
  }

  // Overridden from ButtonListener.
  virtual void ButtonPressed(views::Button* sender,
                             const views::Event& event) OVERRIDE {
    tray_->sms_observer()->RemoveMessage(index_);
    tray_->Update(false);
  }

 private:
  void LayoutDetailedView() {
    views::ImageButton* close_button = new views::ImageButton(this);
    close_button->SetImage(views::CustomButton::BS_NORMAL,
        ResourceBundle::GetSharedInstance().GetImageSkiaNamed(
            IDR_AURA_WINDOW_CLOSE));

    int msg_width = kTrayPopupWidth - kNotificationIconWidth -
        kTrayPopupPaddingHorizontal * 2;
    message_label_->SizeToFit(msg_width);

    views::GridLayout* layout = new views::GridLayout(this);
    SetLayoutManager(layout);

    views::ColumnSet* columns = layout->AddColumnSet(0);

    // Message
    columns->AddPaddingColumn(0, kTrayPopupPaddingHorizontal);
    columns->AddColumn(views::GridLayout::FILL, views::GridLayout::FILL,
                       0 /* resize percent */,
                       views::GridLayout::FIXED, msg_width, msg_width);

    // Close button
    columns->AddColumn(views::GridLayout::TRAILING, views::GridLayout::CENTER,
                       0, /* resize percent */
                       views::GridLayout::FIXED,
                       kNotificationIconWidth, kNotificationIconWidth);


    layout->AddPaddingRow(0, kPaddingVertical);
    layout->StartRow(0, 0);
    layout->AddView(number_label_);
    layout->AddView(close_button, 1, 2);  // 2 rows for icon
    layout->StartRow(0, 0);
    layout->AddView(message_label_);

    layout->AddPaddingRow(0, kPaddingVertical);
  }

  void LayoutNotificationView() {
    SetLayoutManager(
        new views::BoxLayout(views::BoxLayout::kVertical, 0, 0, 1));
    AddChildView(number_label_);
    message_label_->SizeToFit(kTrayNotificationContentsWidth);
    AddChildView(message_label_);
  }

  TraySms* tray_;
  size_t index_;
  views::Label* number_label_;
  views::Label* message_label_;

  DISALLOW_COPY_AND_ASSIGN(SmsMessageView);
};

class TraySms::SmsDetailedView : public TrayDetailsView,
                                 public ViewClickListener {
 public:
  explicit SmsDetailedView(TraySms* tray)
      : tray_(tray) {
    Init();
    Update();
  }

  virtual ~SmsDetailedView() {
  }

  void Init() {
    CreateScrollableList();
    CreateSpecialRow(IDS_ASH_STATUS_TRAY_SMS, this);
  }

  void Update() {
    UpdateMessageList();
    Layout();
    SchedulePaint();
  }

  // Overridden from views::View.
  gfx::Size GetPreferredSize() {
    gfx::Size preferred_size = TrayDetailsView::GetPreferredSize();
    if (preferred_size.height() < kMessageListMinHeight)
      preferred_size.set_height(kMessageListMinHeight);
    return preferred_size;
  }

 private:
  void UpdateMessageList() {
    const base::ListValue& messages = tray_->sms_observer()->messages();
    scroll_content()->RemoveAllChildViews(true);
    for (size_t index = 0; index < messages.GetSize(); ++index) {
      base::DictionaryValue* message = NULL;
      if (!messages.GetDictionary(index, &message)) {
        LOG(ERROR) << "SMS message not a dictionary at: " << index;
        continue;
      }
      std::string number, text;
      if (!tray_->sms_observer()->GetMessageFromDictionary(
              message, &number, &text)) {
        LOG(ERROR) << "Error parsing SMS message";
        continue;
      }
      SmsMessageView* msgview = new SmsMessageView(
          tray_, SmsMessageView::VIEW_DETAILED, index, number, text);
      scroll_content()->AddChildView(msgview);
    }
    scroller()->Layout();
  }

  // Overridden from ViewClickListener.
  virtual void ClickedOn(views::View* sender) OVERRIDE {
    if (sender == footer()->content())
      Shell::GetInstance()->tray()->ShowDefaultView(BUBBLE_USE_EXISTING);
  }

  TraySms* tray_;

  DISALLOW_COPY_AND_ASSIGN(SmsDetailedView);
};

class TraySms::SmsNotificationView : public TrayNotificationView {
 public:
  SmsNotificationView(TraySms* tray,
                      size_t message_index,
                      const std::string& number,
                      const std::string& text)
      : TrayNotificationView(IDR_AURA_UBER_TRAY_SMS),
        tray_(tray),
        message_index_(message_index) {
    SmsMessageView* message_view = new SmsMessageView(
        tray_, SmsMessageView::VIEW_NOTIFICATION, message_index_, number, text);
    InitView(message_view);
  }

  void Update(size_t message_index,
              const std::string& number,
              const std::string& text) {
    SmsMessageView* message_view = new SmsMessageView(
        tray_, SmsMessageView::VIEW_NOTIFICATION, message_index_, number, text);
    UpdateView(message_view);
  }

  // Overridden from views::View.
  virtual bool OnMousePressed(const views::MouseEvent& event) OVERRIDE {
    tray_->PopupDetailedView(0, true);
    return true;
  }

  // Overridden from TrayNotificationView:
  virtual void OnClose() OVERRIDE {
    tray_->sms_observer()->RemoveMessage(message_index_);
    tray_->HideNotificationView();
  }

 private:
  TraySms* tray_;
  size_t message_index_;

  DISALLOW_COPY_AND_ASSIGN(SmsNotificationView);
};

TraySms::TraySms()
    : default_(NULL),
      detailed_(NULL),
      notification_(NULL) {
  sms_observer_.reset(new SmsObserver(this));
}

TraySms::~TraySms() {
}

views::View* TraySms::CreateDefaultView(user::LoginStatus status) {
  CHECK(default_ == NULL);
  sms_observer()->RequestUpdate();
  default_ = new SmsDefaultView(this);
  default_->SetVisible(!sms_observer()->messages().empty());
  return default_;
}

views::View* TraySms::CreateDetailedView(user::LoginStatus status) {
  CHECK(detailed_ == NULL);
  sms_observer()->RequestUpdate();
  HideNotificationView();
  if (sms_observer()->messages().empty())
    return NULL;
  detailed_ = new SmsDetailedView(this);
  return detailed_;
}

views::View* TraySms::CreateNotificationView(user::LoginStatus status) {
  CHECK(notification_ == NULL);
  size_t index;
  std::string number, text;
  if (GetLatestMessage(&index, &number, &text))
    notification_ = new SmsNotificationView(this, index, number, text);
  return notification_;
}

void TraySms::DestroyDefaultView() {
  default_ = NULL;
}

void TraySms::DestroyDetailedView() {
  detailed_ = NULL;
}

void TraySms::DestroyNotificationView() {
  notification_ = NULL;
}

bool TraySms::GetLatestMessage(size_t* index,
                               std::string* number,
                               std::string* text) {
  const base::ListValue& messages = sms_observer()->messages();
  if (messages.empty())
    return false;
  DictionaryValue* message;
  size_t message_index = messages.GetSize() - 1;
  if (!messages.GetDictionary(message_index, &message))
    return false;
  if (!sms_observer()->GetMessageFromDictionary(message, number, text))
    return false;
  *index = message_index;
  return true;
}

void TraySms::Update(bool notify) {
  if (sms_observer()->messages().empty()) {
    if (default_)
      default_->SetVisible(false);
    if (detailed_)
      HideDetailedView();
    HideNotificationView();
  } else {
    if (default_) {
      default_->SetVisible(true);
      default_->Update();
    }
    if (detailed_)
      detailed_->Update();
    if (notification_) {
      size_t index;
      std::string number, text;
      if (GetLatestMessage(&index, &number, &text))
        notification_->Update(index, number, text);
    } else if (notify) {
      ShowNotificationView();
    }
  }
}

}  // namespace internal
}  // namespace ash
