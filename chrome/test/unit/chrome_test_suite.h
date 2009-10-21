// Copyright (c) 2006-2008 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_TEST_UNIT_CHROME_TEST_SUITE_H_
#define CHROME_TEST_UNIT_CHROME_TEST_SUITE_H_

#include <string>

#include "build/build_config.h"

#include "app/app_paths.h"
#include "app/resource_bundle.h"
#include "base/stats_table.h"
#include "base/file_util.h"
#if defined(OS_MACOSX)
#include "base/mac_util.h"
#endif
#include "base/path_service.h"
#include "base/ref_counted.h"
#include "base/scoped_nsautorelease_pool.h"
#include "base/test/test_suite.h"
#include "chrome/app/scoped_ole_initializer.h"
#include "chrome/browser/browser_process.h"
#include "chrome/common/chrome_constants.h"
#include "chrome/common/chrome_paths.h"
#include "chrome/common/chrome_switches.h"
#include "chrome/test/testing_browser_process.h"
#include "net/base/mock_host_resolver.h"
#include "net/base/net_util.h"

// In many cases it may be not obvious that a test makes a real DNS lookup.
// We generally don't want to rely on external DNS servers for our tests,
// so this host resolver procedure catches external queries.
class WarningHostResolverProc : public net::HostResolverProc {
 public:
  WarningHostResolverProc() : HostResolverProc(NULL) {}

  virtual int Resolve(const std::string& host,
                      net::AddressFamily address_family,
                      net::AddressList* addrlist) {
    const char* kLocalHostNames[] = {"localhost", "127.0.0.1"};
    bool local = false;

    if (host == net::GetHostName()) {
      local = true;
    } else {
      for (size_t i = 0; i < arraysize(kLocalHostNames); i++)
        if (host == kLocalHostNames[i]) {
          local = true;
          break;
        }
    }

    // Make the test fail so it's harder to ignore.
    // If you really need to make real DNS query, use
    // net::RuleBasedHostResolverProc and its AllowDirectLookup method.
    EXPECT_TRUE(local) << "Making external DNS lookup of " << host;

    return ResolveUsingPrevious(host, address_family, addrlist);
  }
};

class ChromeTestSuite : public TestSuite {
 public:
  ChromeTestSuite(int argc, char** argv)
      : TestSuite(argc, argv),
        stats_table_(NULL) {
  }

 protected:

  virtual void Initialize() {
    base::ScopedNSAutoreleasePool autorelease_pool;

    TestSuite::Initialize();

    host_resolver_proc_ = new WarningHostResolverProc();
    scoped_host_resolver_proc_.Init(host_resolver_proc_.get());

    chrome::RegisterPathProvider();
    app::RegisterPathProvider();
    g_browser_process = new TestingBrowserProcess;

    // Notice a user data override, and otherwise default to using a custom
    // user data directory that lives alongside the current app.
    // NOTE: The user data directory will be erased before each UI test that
    //       uses it, in order to ensure consistency.
    FilePath user_data_dir = FilePath::FromWStringHack(
        CommandLine::ForCurrentProcess()->GetSwitchValue(
            switches::kUserDataDir));
    if (user_data_dir.empty() &&
        file_util::CreateNewTempDirectory(FILE_PATH_LITERAL("chrome_test_"),
                                          &user_data_dir)) {
      user_data_dir = user_data_dir.AppendASCII("test_user_data");
    }
    if (!user_data_dir.empty())
      PathService::Override(chrome::DIR_USER_DATA, user_data_dir);

#if defined(OS_MACOSX)
    // Look in the framework bundle for resources.
    FilePath path;
    PathService::Get(base::DIR_EXE, &path);
    path = path.Append(chrome::kFrameworkName);
    mac_util::SetOverrideAppBundlePath(path);
#endif

    // Force unittests to run using en-US so if we test against string
    // output, it'll pass regardless of the system language.
    ResourceBundle::InitSharedInstance(L"en-US");
    ResourceBundle::GetSharedInstance().LoadThemeResources();

    // initialize the global StatsTable for unit_tests
    std::string statsfile = "unit_tests";
    std::string pid_string = StringPrintf("-%d", base::GetCurrentProcId());
    statsfile += pid_string;
    stats_table_ = new StatsTable(statsfile, 20, 200);
    StatsTable::set_current(stats_table_);
  }

  virtual void Shutdown() {
    ResourceBundle::CleanupSharedInstance();

#if defined(OS_MACOSX)
    mac_util::SetOverrideAppBundle(NULL);
#endif

    delete g_browser_process;
    g_browser_process = NULL;

    // Tear down shared StatsTable; prevents unit_tests from leaking it.
    StatsTable::set_current(NULL);
    delete stats_table_;

    // Delete the test_user_data dir recursively
    FilePath user_data_dir;
    if (PathService::Get(chrome::DIR_USER_DATA, &user_data_dir) &&
        !user_data_dir.empty()) {
      file_util::Delete(user_data_dir, true);
      file_util::Delete(user_data_dir.DirName(), false);
    }
    TestSuite::Shutdown();
  }

  StatsTable* stats_table_;
  ScopedOleInitializer ole_initializer_;
  scoped_refptr<WarningHostResolverProc> host_resolver_proc_;
  net::ScopedDefaultHostResolverProc scoped_host_resolver_proc_;
};

#endif  // CHROME_TEST_UNIT_CHROME_TEST_SUITE_H_
