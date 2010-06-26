// Copyright (c) 2006-2009 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "base/scoped_ptr.h"
#include "chrome/common/extensions/url_pattern.h"
#include "testing/gtest/include/gtest/gtest.h"

// See url_pattern.h for examples of valid and invalid patterns.

TEST(URLPatternTest, ParseInvalid) {
  const char* kInvalidPatterns[] = {
    "http",  // no scheme
    "http://",  // no path separator
    "http://foo",  // no path separator
    "http://*foo/bar",  // not allowed as substring of host component
    "http://foo.*.bar/baz",  // must be first component
    "http:/bar",  // scheme separator not found
    "foo://*",  // invalid scheme
    "chrome-extenstions://*/*",  // we don't support chrome extension URLs
  };

  for (size_t i = 0; i < arraysize(kInvalidPatterns); ++i) {
    URLPattern pattern;
    EXPECT_FALSE(pattern.Parse(kInvalidPatterns[i]));
  }
};

// all pages for a given scheme
TEST(URLPatternTest, Match1) {
  URLPattern pattern;
  EXPECT_TRUE(pattern.Parse("http://*/*"));
  EXPECT_EQ("http", pattern.scheme());
  EXPECT_EQ("", pattern.host());
  EXPECT_TRUE(pattern.match_subdomains());
  EXPECT_EQ("/*", pattern.path());
  EXPECT_TRUE(pattern.MatchesUrl(GURL("http://google.com")));
  EXPECT_TRUE(pattern.MatchesUrl(GURL("http://yahoo.com")));
  EXPECT_TRUE(pattern.MatchesUrl(GURL("http://google.com/foo")));
  EXPECT_FALSE(pattern.MatchesUrl(GURL("https://google.com")));
  EXPECT_TRUE(pattern.MatchesUrl(GURL("http://74.125.127.100/search")));
}

// all domains
TEST(URLPatternTest, Match2) {
  URLPattern pattern;
  EXPECT_TRUE(pattern.Parse("https://*/foo*"));
  EXPECT_EQ("https", pattern.scheme());
  EXPECT_EQ("", pattern.host());
  EXPECT_TRUE(pattern.match_subdomains());
  EXPECT_EQ("/foo*", pattern.path());
  EXPECT_TRUE(pattern.MatchesUrl(GURL("https://www.google.com/foo")));
  EXPECT_TRUE(pattern.MatchesUrl(GURL("https://www.google.com/foobar")));
  EXPECT_FALSE(pattern.MatchesUrl(GURL("http://www.google.com/foo")));
  EXPECT_FALSE(pattern.MatchesUrl(GURL("https://www.google.com/")));
}

// subdomains
TEST(URLPatternTest, Match3) {
  URLPattern pattern;
  EXPECT_TRUE(pattern.Parse("http://*.google.com/foo*bar"));
  EXPECT_EQ("http", pattern.scheme());
  EXPECT_EQ("google.com", pattern.host());
  EXPECT_TRUE(pattern.match_subdomains());
  EXPECT_EQ("/foo*bar", pattern.path());
  EXPECT_TRUE(pattern.MatchesUrl(GURL("http://google.com/foobar")));
  EXPECT_TRUE(pattern.MatchesUrl(GURL("http://www.google.com/foo?bar")));
  EXPECT_TRUE(pattern.MatchesUrl(
      GURL("http://monkey.images.google.com/foooobar")));
  EXPECT_FALSE(pattern.MatchesUrl(GURL("http://yahoo.com/foobar")));
}

// glob escaping
TEST(URLPatternTest, Match5) {
  URLPattern pattern;
  EXPECT_TRUE(pattern.Parse("file:///foo?bar\\*baz"));
  EXPECT_EQ("file", pattern.scheme());
  EXPECT_EQ("", pattern.host());
  EXPECT_FALSE(pattern.match_subdomains());
  EXPECT_EQ("/foo?bar\\*baz", pattern.path());
  EXPECT_TRUE(pattern.MatchesUrl(GURL("file:///foo?bar\\hellobaz")));
  EXPECT_FALSE(pattern.MatchesUrl(GURL("file:///fooXbar\\hellobaz")));
}

// ip addresses
TEST(URLPatternTest, Match6) {
  URLPattern pattern;
  EXPECT_TRUE(pattern.Parse("http://127.0.0.1/*"));
  EXPECT_EQ("http", pattern.scheme());
  EXPECT_EQ("127.0.0.1", pattern.host());
  EXPECT_FALSE(pattern.match_subdomains());
  EXPECT_EQ("/*", pattern.path());
  EXPECT_TRUE(pattern.MatchesUrl(GURL("http://127.0.0.1")));
}

// subdomain matching with ip addresses
TEST(URLPatternTest, Match7) {
  URLPattern pattern;
  EXPECT_TRUE(pattern.Parse("http://*.0.0.1/*")); // allowed, but useless
  EXPECT_EQ("http", pattern.scheme());
  EXPECT_EQ("0.0.1", pattern.host());
  EXPECT_TRUE(pattern.match_subdomains());
  EXPECT_EQ("/*", pattern.path());
  // Subdomain matching is never done if the argument has an IP address host.
  EXPECT_FALSE(pattern.MatchesUrl(GURL("http://127.0.0.1")));
};

// unicode
TEST(URLPatternTest, Match8) {
  URLPattern pattern;
  // The below is the ASCII encoding of the following URL:
  // http://*.\xe1\x80\xbf/a\xc2\x81\xe1*
  EXPECT_TRUE(pattern.Parse("http://*.xn--gkd/a%C2%81%E1*"));
  EXPECT_EQ("http", pattern.scheme());
  EXPECT_EQ("xn--gkd", pattern.host());
  EXPECT_TRUE(pattern.match_subdomains());
  EXPECT_EQ("/a%C2%81%E1*", pattern.path());
  EXPECT_TRUE(pattern.MatchesUrl(
      GURL("http://abc.\xe1\x80\xbf/a\xc2\x81\xe1xyz")));
  EXPECT_TRUE(pattern.MatchesUrl(
      GURL("http://\xe1\x80\xbf/a\xc2\x81\xe1\xe1")));
};

// chrome://
TEST(URLPatternTest, Match9) {
  URLPattern pattern;
  EXPECT_TRUE(pattern.Parse("chrome://favicon/*"));
  EXPECT_EQ("chrome", pattern.scheme());
  EXPECT_EQ("favicon", pattern.host());
  EXPECT_FALSE(pattern.match_subdomains());
  EXPECT_EQ("/*", pattern.path());
  EXPECT_TRUE(pattern.MatchesUrl(GURL("chrome://favicon/http://google.com")));
  EXPECT_TRUE(pattern.MatchesUrl(GURL("chrome://favicon/https://google.com")));
  EXPECT_FALSE(pattern.MatchesUrl(GURL("chrome://history")));
};

void TestPatternOverlap(const URLPattern& pattern1, const URLPattern& pattern2,
                        bool expect_overlap) {
  EXPECT_EQ(expect_overlap, pattern1.OverlapsWith(pattern2))
      << pattern1.GetAsString() << ", " << pattern2.GetAsString();
  EXPECT_EQ(expect_overlap, pattern2.OverlapsWith(pattern1))
      << pattern2.GetAsString() << ", " << pattern1.GetAsString();
}

TEST(URLPatternTest, OverlapsWith) {
  URLPattern pattern1("http://www.google.com/foo/*");
  URLPattern pattern2("https://www.google.com/foo/*");
  URLPattern pattern3("http://*.google.com/foo/*");
  URLPattern pattern4("http://*.yahooo.com/foo/*");
  URLPattern pattern5("http://www.yahooo.com/bar/*");
  URLPattern pattern6("http://www.yahooo.com/bar/baz/*");

  TestPatternOverlap(pattern1, pattern1, true);
  TestPatternOverlap(pattern1, pattern2, false);
  TestPatternOverlap(pattern1, pattern3, true);
  TestPatternOverlap(pattern1, pattern4, false);
  TestPatternOverlap(pattern3, pattern4, false);
  TestPatternOverlap(pattern4, pattern5, false);
  TestPatternOverlap(pattern5, pattern6, true);
}
