// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_CHROMEOS_INPUT_METHOD_INPUT_METHOD_WHITELIST_H_
#define CHROME_BROWSER_CHROMEOS_INPUT_METHOD_INPUT_METHOD_WHITELIST_H_

#include <set>
#include <string>
#include <vector>

#include "base/basictypes.h"
#include "base/memory/scoped_ptr.h"

namespace chromeos {
namespace input_method {

class InputMethodDescriptor;
typedef std::vector<InputMethodDescriptor> InputMethodDescriptors;

//
class InputMethodWhitelist {
 public:
  InputMethodWhitelist();
  ~InputMethodWhitelist();

  // Returns true if |input_method_id| is whitelisted.
  bool InputMethodIdIsWhitelisted(const std::string& input_method_id) const;

  // Returns all input methods that are supported, including ones not active.
  // This function never returns NULL. Note that input method extensions are not
  // included in the result.
  scoped_ptr<InputMethodDescriptors> GetSupportedInputMethods() const;

 private:
  std::set<std::string> supported_input_methods_;

  DISALLOW_COPY_AND_ASSIGN(InputMethodWhitelist);
};

}  // namespace input_method
}  // namespace chromeos

#endif  // CHROME_BROWSER_CHROMEOS_INPUT_METHOD_INPUT_METHOD_WHITELIST_H_
