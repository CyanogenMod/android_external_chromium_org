// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_CHROMEOS_LOGIN_MOCK_USER_MANAGER_H_
#define CHROME_BROWSER_CHROMEOS_LOGIN_MOCK_USER_MANAGER_H_
#pragma once

#include <string>

#include "base/file_path.h"
#include "chrome/browser/chromeos/login/user_manager.h"
#include "testing/gmock/include/gmock/gmock.h"

namespace chromeos {

class MockUserManager : public UserManager {
 public:
  MockUserManager();
  virtual ~MockUserManager();

  MOCK_CONST_METHOD0(GetUsers, const UserList&(void));
  MOCK_METHOD1(UserLoggedIn, void(const std::string&));
  MOCK_METHOD0(DemoUserLoggedIn, void(void));
  MOCK_METHOD0(GuestUserLoggedIn, void(void));
  MOCK_METHOD2(RemoveUser, void(const std::string&, RemoveUserDelegate*));
  MOCK_METHOD1(RemoveUserFromList, void(const std::string&));
  MOCK_CONST_METHOD1(IsKnownUser, bool(const std::string&));
  MOCK_CONST_METHOD1(FindUser, const User*(const std::string&));
  MOCK_CONST_METHOD0(logged_in_user, const User&(void));
  MOCK_METHOD0(logged_in_user, User&(void));
  MOCK_CONST_METHOD1(IsDisplayNameUnique, bool(const std::string&));
  MOCK_METHOD2(SaveUserOAuthStatus, void(const std::string&,
                                         User::OAuthTokenStatus));
  MOCK_METHOD2(SaveUserDisplayEmail, void(const std::string&,
                                          const std::string&));
  MOCK_CONST_METHOD1(GetUserDisplayEmail, std::string(const std::string&));
  MOCK_METHOD2(SaveUserDefaultImageIndex, void(const std::string&, int));
  MOCK_METHOD2(SaveUserImage, void(const std::string&, const SkBitmap&));
  MOCK_METHOD2(SaveUserImageFromFile, void(const std::string&,
                                           const FilePath&));
  MOCK_METHOD1(SaveUserImageFromProfileImage, void(const std::string&));
  MOCK_METHOD1(DownloadProfileImage, void(const std::string&));
  MOCK_CONST_METHOD0(current_user_is_owner, bool(void));
  MOCK_METHOD1(set_current_user_is_owner, void(bool));
  MOCK_CONST_METHOD0(current_user_is_new, bool(void));
  MOCK_CONST_METHOD0(user_is_logged_in, bool(void));
  MOCK_CONST_METHOD0(IsLoggedInAsDemoUser, bool(void));
  MOCK_CONST_METHOD0(IsLoggedInAsGuest, bool(void));
  MOCK_METHOD1(AddObserver, void(UserManager::Observer*));
  MOCK_METHOD1(RemoveObserver, void(UserManager::Observer*));
  MOCK_METHOD0(NotifyLocalStateChanged, void(void));
  MOCK_CONST_METHOD0(downloaded_profile_image, const SkBitmap& (void));
};

}  // namespace chromeos

#endif  // CHROME_BROWSER_CHROMEOS_LOGIN_MOCK_USER_MANAGER_H_
