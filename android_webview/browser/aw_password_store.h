/*
 *  Copyright (c) 2014, The Linux Foundation. All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions are
 *  met:
 *      * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above
 *       copyright notice, this list of conditions and the following
 *       disclaimer in the documentation and/or other materials provided
 *       with the distribution.
 *     * Neither the name of The Linux Foundation nor the names of its
 *       contributors may be used to endorse or promote products derived
 *       from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED "AS IS" AND ANY EXPRESS OR IMPLIED
 *  WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 *  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT
 *  ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS
 *  BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 *  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 *  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
 *  BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 *  WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
 *  OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
 *  IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */
// SWE-feature-username-password

#ifndef ANDROID_WEBVIEW_BROWSER_AW_PASSWORD_STORE_H_
#define ANDROID_WEBVIEW_BROWSER_AW_PASSWORD_STORE_H_

#include "base/memory/scoped_ptr.h"
#include "components/password_manager/core/browser/password_store_default.h"
#include "android_webview/browser/aw_password_manager_handler.h"
#include "components/password_manager/core/browser/login_database.h"

using password_manager::LoginDatabase;
using password_manager::PasswordStoreDefault;

namespace android_webview {

class AwPasswordStore : public PasswordStoreDefault {
 public:
  explicit AwPasswordStore(scoped_refptr<base::SingleThreadTaskRunner> main_thread_runner,
                 scoped_refptr<base::SingleThreadTaskRunner> db_thread_runner,
                 LoginDatabase* login_db);

 protected:
  friend class AwBrowserContext;

 private:
  virtual ~AwPasswordStore();

  DISALLOW_COPY_AND_ASSIGN(AwPasswordStore);
};

} // namespace android_webview

#endif  // ANDROID_WEBVIEW_BROWSER_AW_PASSWORD_STORE_H_
// SWE-feature-username-password
