// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/website_settings.h"

#include "base/at_exit.h"
#include "base/message_loop.h"
#include "base/string16.h"
#include "base/utf_string_conversions.h"
#include "chrome/browser/content_settings/host_content_settings_map.h"
#include "chrome/browser/content_settings/tab_specific_content_settings.h"
#include "chrome/browser/ui/website_settings_ui.h"
#include "chrome/common/content_settings.h"
#include "chrome/common/content_settings_types.h"
#include "chrome/test/base/chrome_render_view_host_test_harness.h"
#include "chrome/test/base/testing_profile.h"
#include "content/public/browser/cert_store.h"
#include "content/public/common/ssl_status.h"
#include "content/test/test_browser_thread.h"
#include "net/base/cert_status_flags.h"
#include "net/base/ssl_connection_status_flags.h"
#include "net/base/test_certificate_data.h"
#include "net/base/x509_certificate.h"
#include "testing/gmock/include/gmock/gmock.h"
#include "testing/gtest/include/gtest/gtest.h"

using content::SSLStatus;
using namespace testing;

namespace {

// SSL cipher suite like specified in RFC5246 Appendix A.5. "The Cipher Suite".
static int TLS_RSA_WITH_AES_256_CBC_SHA256 = 0x3D;

int SetSSLVersion(int connection_status, int version) {
  // Clear SSL version bits (Bits 20, 21 and 22).
  connection_status &=
      ~(net::SSL_CONNECTION_VERSION_MASK << net::SSL_CONNECTION_VERSION_SHIFT);
  int bitmask = version << net::SSL_CONNECTION_VERSION_SHIFT;
  return bitmask | connection_status;
}

int SetSSLCipherSuite(int connection_status, int cipher_suite) {
  // Clear cipher suite bits (the 16 lowest bits).
  connection_status &= ~net::SSL_CONNECTION_CIPHERSUITE_MASK;
  return cipher_suite | connection_status;
}

class MockCertStore : public content::CertStore {
 public:
  virtual ~MockCertStore() {}
  MOCK_METHOD2(StoreCert, int(net::X509Certificate*, int));
  MOCK_METHOD2(RetrieveCert, bool(int, scoped_refptr<net::X509Certificate>*));
};

class MockWebsiteSettingsUI : public WebsiteSettingsUI {
 public:
  virtual ~MockWebsiteSettingsUI() {}
  MOCK_METHOD1(SetPresenter, void(WebsiteSettings* presenter));
  MOCK_METHOD1(SetCookieInfo, void(const CookieInfoList& cookie_info_list));
  MOCK_METHOD1(SetPermissionInfo,
               void(const PermissionInfoList& permission_info_list));
  MOCK_METHOD1(SetIdentityInfo, void(const IdentityInfo& identity_info));
  MOCK_METHOD1(SetFirstVisit, void(const string16& first_visit));
};

class WebsiteSettingsTest : public ChromeRenderViewHostTestHarness {
 public:
  WebsiteSettingsTest()
      : website_settings_(NULL),
        mock_ui_(NULL),
        cert_id_(0),
        browser_thread_(content::BrowserThread::UI, &message_loop_),
        tab_specific_content_settings_(NULL),
        url_("http://www.example.com") {
  }

  virtual ~WebsiteSettingsTest() {
  }

  virtual void SetUp() {
    ChromeRenderViewHostTestHarness::SetUp();
    // Setup stub SSLStatus.
    ssl_.security_style = content::SECURITY_STYLE_UNAUTHENTICATED;

    // Create the certificate.
    cert_id_ = 1;
    base::Time start_date = base::Time::Now();
    base::Time expiration_date = base::Time::FromInternalValue(
        start_date.ToInternalValue() + base::Time::kMicrosecondsPerWeek);
    cert_ = new net::X509Certificate("subject",
                                     "issuer",
                                     start_date,
                                     expiration_date);

    tab_specific_content_settings_.reset(
        new TabSpecificContentSettings(contents()));

    // Setup the mock cert store.
    EXPECT_CALL(cert_store_, RetrieveCert(cert_id_, _) )
        .Times(AnyNumber())
        .WillRepeatedly(DoAll(SetArgPointee<1>(cert_), Return(true)));

    // Setup mock ui.
    mock_ui_ = new MockWebsiteSettingsUI();
  }

  virtual void TearDown() {
    ASSERT_TRUE(website_settings_) << "No WebsiteSettings instance created.";
    // Call OnUIClosing to destroy the |website_settings| object.
    EXPECT_CALL(*mock_ui_, SetPresenter(NULL));
    website_settings_->OnUIClosing();
    RenderViewHostTestHarness::TearDown();
  }

  void SetDefaultUIExpectations(MockWebsiteSettingsUI* mock_ui) {
    // During creation |WebsiteSettings| makes the following calls to the ui.
    EXPECT_CALL(*mock_ui, SetPermissionInfo(_));
    EXPECT_CALL(*mock_ui, SetIdentityInfo(_));
    EXPECT_CALL(*mock_ui, SetPresenter(_));
    EXPECT_CALL(*mock_ui, SetCookieInfo(_));
    EXPECT_CALL(*mock_ui, SetFirstVisit(string16()));
  }

  const GURL& url() const { return url_; }
  MockCertStore* cert_store() { return &cert_store_; }
  int cert_id() { return cert_id_; }
  MockWebsiteSettingsUI* mock_ui() { return mock_ui_; }
  const SSLStatus& ssl() { return ssl_; }
  TabSpecificContentSettings* tab_specific_content_settings() {
    return tab_specific_content_settings_.get();
  }

  WebsiteSettings* website_settings() {
    if (!website_settings_) {
      website_settings_ = new WebsiteSettings(
          mock_ui(), profile(), tab_specific_content_settings_.get(), url(),
          ssl(), cert_store());
    }
    return website_settings_;
  }

  SSLStatus ssl_;

 private:
  // The |Website_settings| object deletes itself after the OnUIClosing()
  // method is called.
  WebsiteSettings* website_settings_;  // Weak pointer.
  // The UI is owned by |web_site_settings_|.
  MockWebsiteSettingsUI* mock_ui_;  // Weak pointer.
  int cert_id_;
  scoped_refptr<net::X509Certificate> cert_;
  content::TestBrowserThread browser_thread_;
  scoped_ptr<TabSpecificContentSettings> tab_specific_content_settings_;
  MockCertStore cert_store_;
  GURL url_;
};

}  // namespace

TEST_F(WebsiteSettingsTest, OnPermissionsChanged) {
  // Setup site permissions.
  HostContentSettingsMap* content_settings =
      profile()->GetHostContentSettingsMap();
  ContentSetting setting = content_settings->GetContentSetting(
      url(), url(), CONTENT_SETTINGS_TYPE_POPUPS, "");
  EXPECT_EQ(setting, CONTENT_SETTING_BLOCK);
  setting = content_settings->GetContentSetting(
      url(), url(), CONTENT_SETTINGS_TYPE_PLUGINS, "");
  EXPECT_EQ(setting, CONTENT_SETTING_ALLOW);
  setting = content_settings->GetContentSetting(
      url(), url(), CONTENT_SETTINGS_TYPE_GEOLOCATION, "");
  EXPECT_EQ(setting, CONTENT_SETTING_ASK);
  setting = content_settings->GetContentSetting(
      url(), url(), CONTENT_SETTINGS_TYPE_NOTIFICATIONS, "");
  EXPECT_EQ(setting, CONTENT_SETTING_ASK);

  SetDefaultUIExpectations(mock_ui());

  // Execute code under tests.
  website_settings()->OnSitePermissionChanged(CONTENT_SETTINGS_TYPE_POPUPS,
                                              CONTENT_SETTING_ALLOW);
  website_settings()->OnSitePermissionChanged(CONTENT_SETTINGS_TYPE_PLUGINS,
                                              CONTENT_SETTING_BLOCK);
  website_settings()->OnSitePermissionChanged(CONTENT_SETTINGS_TYPE_GEOLOCATION,
                                              CONTENT_SETTING_ALLOW);
  website_settings()->OnSitePermissionChanged(
      CONTENT_SETTINGS_TYPE_NOTIFICATIONS, CONTENT_SETTING_ALLOW);

  // Verify that the site permissions were changed correctly.
  setting = content_settings->GetContentSetting(
      url(), url(), CONTENT_SETTINGS_TYPE_POPUPS, "");
  EXPECT_EQ(setting, CONTENT_SETTING_ALLOW);
  setting = content_settings->GetContentSetting(
      url(), url(), CONTENT_SETTINGS_TYPE_PLUGINS, "");
  EXPECT_EQ(setting, CONTENT_SETTING_BLOCK);
  setting = content_settings->GetContentSetting(
      url(), url(), CONTENT_SETTINGS_TYPE_GEOLOCATION, "");
  EXPECT_EQ(setting, CONTENT_SETTING_ALLOW);
  setting = content_settings->GetContentSetting(
      url(), url(), CONTENT_SETTINGS_TYPE_NOTIFICATIONS, "");
  EXPECT_EQ(setting, CONTENT_SETTING_ALLOW);
}

TEST_F(WebsiteSettingsTest, OnSiteDataAccessed) {
  EXPECT_CALL(*mock_ui(), SetPermissionInfo(_));
  EXPECT_CALL(*mock_ui(), SetIdentityInfo(_));
  EXPECT_CALL(*mock_ui(), SetPresenter(_));
  EXPECT_CALL(*mock_ui(), SetFirstVisit(string16()));
  EXPECT_CALL(*mock_ui(), SetCookieInfo(_)).Times(2);

  website_settings()->OnSiteDataAccessed();
}

TEST_F(WebsiteSettingsTest, HTTPConnection) {
  SetDefaultUIExpectations(mock_ui());
  EXPECT_EQ(WebsiteSettings::SITE_CONNECTION_STATUS_UNENCRYPTED,
            website_settings()->site_connection_status());
  EXPECT_EQ(WebsiteSettings::SITE_IDENTITY_STATUS_NO_CERT,
            website_settings()->site_identity_status());
  EXPECT_EQ(string16(), website_settings()->organization_name());
}

TEST_F(WebsiteSettingsTest, HTTPSConnection) {
  ssl_.security_style = content::SECURITY_STYLE_AUTHENTICATED;
  ssl_.cert_id = cert_id();
  ssl_.cert_status = 0;
  ssl_.security_bits = 81;  // No error if > 80.
  int status = 0;
  status = SetSSLVersion(status, net::SSL_CONNECTION_VERSION_TLS1);
  status = SetSSLCipherSuite(status, TLS_RSA_WITH_AES_256_CBC_SHA256);
  ssl_.connection_status = status;

  SetDefaultUIExpectations(mock_ui());

  EXPECT_EQ(WebsiteSettings::SITE_CONNECTION_STATUS_ENCRYPTED,
            website_settings()->site_connection_status());
  EXPECT_EQ(WebsiteSettings::SITE_IDENTITY_STATUS_CERT,
            website_settings()->site_identity_status());
  EXPECT_EQ(string16(), website_settings()->organization_name());
}

TEST_F(WebsiteSettingsTest, HTTPSMixedContent) {
  ssl_.security_style = content::SECURITY_STYLE_AUTHENTICATED;
  ssl_.cert_id = cert_id();
  ssl_.cert_status = 0;
  ssl_.security_bits = 81;  // No error if > 80.
  ssl_.content_status = SSLStatus::DISPLAYED_INSECURE_CONTENT;
  int status = 0;
  status = SetSSLVersion(status, net::SSL_CONNECTION_VERSION_TLS1);
  status = SetSSLCipherSuite(status, TLS_RSA_WITH_AES_256_CBC_SHA256);
  ssl_.connection_status = status;

  SetDefaultUIExpectations(mock_ui());

  EXPECT_EQ(WebsiteSettings::SITE_CONNECTION_STATUS_MIXED_CONTENT,
            website_settings()->site_connection_status());
  EXPECT_EQ(WebsiteSettings::SITE_IDENTITY_STATUS_CERT,
            website_settings()->site_identity_status());
  EXPECT_EQ(string16(), website_settings()->organization_name());
}

TEST_F(WebsiteSettingsTest, HTTPSEVCert) {
  scoped_refptr<net::X509Certificate> ev_cert =
      net::X509Certificate::CreateFromBytes(
          reinterpret_cast<const char*>(google_der),
          sizeof(google_der));
  int ev_cert_id = 1;
  EXPECT_CALL(*cert_store(), RetrieveCert(ev_cert_id, _)).WillRepeatedly(
      DoAll(SetArgPointee<1>(ev_cert), Return(true)));

  ssl_.security_style = content::SECURITY_STYLE_AUTHENTICATED;
  ssl_.cert_id = ev_cert_id;
  ssl_.cert_status = net::CERT_STATUS_IS_EV;
  ssl_.security_bits = 81;  // No error if > 80.
  ssl_.content_status = SSLStatus::DISPLAYED_INSECURE_CONTENT;
  int status = 0;
  status = SetSSLVersion(status, net::SSL_CONNECTION_VERSION_TLS1);
  status = SetSSLCipherSuite(status, TLS_RSA_WITH_AES_256_CBC_SHA256);
  ssl_.connection_status = status;

  SetDefaultUIExpectations(mock_ui());

  EXPECT_EQ(WebsiteSettings::SITE_CONNECTION_STATUS_MIXED_CONTENT,
            website_settings()->site_connection_status());
  EXPECT_EQ(WebsiteSettings::SITE_IDENTITY_STATUS_EV_CERT,
            website_settings()->site_identity_status());
  EXPECT_EQ(UTF8ToUTF16("Google Inc"), website_settings()->organization_name());
}

TEST_F(WebsiteSettingsTest, HTTPSRevocationError) {
  ssl_.security_style = content::SECURITY_STYLE_AUTHENTICATED;
  ssl_.cert_id = cert_id();
  ssl_.cert_status = net::CERT_STATUS_UNABLE_TO_CHECK_REVOCATION;
  ssl_.security_bits = 81;  // No error if > 80.
  int status = 0;
  status = SetSSLVersion(status, net::SSL_CONNECTION_VERSION_TLS1);
  status = SetSSLCipherSuite(status, TLS_RSA_WITH_AES_256_CBC_SHA256);
  ssl_.connection_status = status;

  SetDefaultUIExpectations(mock_ui());

  EXPECT_EQ(WebsiteSettings::SITE_CONNECTION_STATUS_ENCRYPTED,
            website_settings()->site_connection_status());
  EXPECT_EQ(WebsiteSettings::SITE_IDENTITY_STATUS_CERT_REVOCATION_UNKNOWN,
            website_settings()->site_identity_status());
  EXPECT_EQ(string16(), website_settings()->organization_name());
}

TEST_F(WebsiteSettingsTest, HTTPSConnectionError) {
  ssl_.security_style = content::SECURITY_STYLE_AUTHENTICATED;
  ssl_.cert_id = cert_id();
  ssl_.cert_status = 0;
  ssl_.security_bits = 1;
  int status = 0;
  status = SetSSLVersion(status, net::SSL_CONNECTION_VERSION_TLS1);
  status = SetSSLCipherSuite(status, TLS_RSA_WITH_AES_256_CBC_SHA256);
  ssl_.connection_status = status;

  SetDefaultUIExpectations(mock_ui());

  EXPECT_EQ(WebsiteSettings::SITE_CONNECTION_STATUS_ENCRYPTED_ERROR,
            website_settings()->site_connection_status());
  EXPECT_EQ(WebsiteSettings::SITE_IDENTITY_STATUS_CERT,
            website_settings()->site_identity_status());
  EXPECT_EQ(string16(), website_settings()->organization_name());
}
