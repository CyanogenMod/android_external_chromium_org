// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "base/utf_string_conversions.h"
#include "chrome/browser/ui/browser.h"
#include "chrome/browser/ui/tabs/tab_strip_model.h"
#include "chrome/common/url_constants.h"
#include "chrome/test/base/in_process_browser_test.h"
#include "chrome/test/base/ui_test_utils.h"
#include "content/public/test/browser_test_utils.h"
#include "net/http/http_response_headers.h"
#include "net/url_request/test_url_fetcher_factory.h"
#include "testing/gmock/include/gmock/gmock.h"

using ::testing::Exactly;
using ::testing::Invoke;
using ::testing::_;

namespace {

const char kEULAURL[] = "https://www.google.com/intl/en-US/chrome/eula_text.html";
const char kFakeOnlineEULA[] = "No obligations at all";
const char kOfflineEULAWarning[] = "A copy of the Google Terms of Service";

class TermsOfServiceProcessBrowserTest : public InProcessBrowserTest {
};

class TestURLFetcherCallback {
 public:
  scoped_ptr<net::FakeURLFetcher> CreateURLFetcher(
      const GURL& url,
      net::URLFetcherDelegate* d,
      const std::string& response_data,
      bool success) {
    scoped_ptr<net::FakeURLFetcher> fetcher(
        new net::FakeURLFetcher(url, d, response_data, success));
    OnRequestCreate(url, fetcher.get());
    return fetcher.Pass();
  }
  MOCK_METHOD2(OnRequestCreate,
               void(const GURL&, net::FakeURLFetcher*));
};

void AddMimeHeader(const GURL& url, net::FakeURLFetcher* fetcher) {
  scoped_refptr<net::HttpResponseHeaders> download_headers =
      new net::HttpResponseHeaders("");
  download_headers->AddHeader("Content-Type: text/html");
  fetcher->set_response_headers(download_headers);
}

// Load chrome://terms. Make sure online version is shown.
IN_PROC_BROWSER_TEST_F(TermsOfServiceProcessBrowserTest, LoadOnline) {
  TestURLFetcherCallback url_callback;
  net::FakeURLFetcherFactory factory(
      NULL,
      base::Bind(&TestURLFetcherCallback::CreateURLFetcher,
                 base::Unretained(&url_callback)));
  factory.SetFakeResponse(kEULAURL, kFakeOnlineEULA, true);
  EXPECT_CALL(url_callback, OnRequestCreate(GURL(kEULAURL), _))
      .Times(Exactly(1))
      .WillRepeatedly(Invoke(AddMimeHeader));

  ui_test_utils::NavigateToURL(browser(), GURL(chrome::kChromeUITermsURL));
  content::WebContents* web_contents =
      browser()->tab_strip_model()->GetActiveWebContents();
  EXPECT_EQ(1, ui_test_utils::FindInPage(web_contents,
                                         ASCIIToUTF16(kFakeOnlineEULA),
                                         true,
                                         true,
                                         NULL,
                                         NULL));
}

// Load chrome://terms with no internet connectivity.
// Make sure offline version is shown.
IN_PROC_BROWSER_TEST_F(TermsOfServiceProcessBrowserTest, LoadOffline) {
  net::FakeURLFetcherFactory factory(NULL);
  factory.SetFakeResponse(kEULAURL, "", false);

  ui_test_utils::NavigateToURL(browser(), GURL(chrome::kChromeUITermsURL));
  content::WebContents* web_contents =
      browser()->tab_strip_model()->GetActiveWebContents();

#if defined(GOOGLE_CHROME_BUILD)
  EXPECT_NE(0, ui_test_utils::FindInPage(web_contents,
                                         ASCIIToUTF16(kOfflineEULAWarning),
                                         true,
                                         true,
                                         NULL,
                                         NULL));
#else
  std::string body;
  ASSERT_TRUE(content::ExecuteScriptAndExtractString(
      web_contents,
      "window.domAutomationController.send(document.body.textContent)",
      &body));
  EXPECT_NE(std::string(), body);
#endif
}

}  // namespace
