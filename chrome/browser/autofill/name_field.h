// Copyright (c) 2009 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_AUTOFILL_NAME_FIELD_H_
#define CHROME_BROWSER_AUTOFILL_NAME_FIELD_H_
#pragma once

#include <vector>

#include "base/logging.h"
#include "chrome/browser/autofill/autofill_field.h"
#include "chrome/browser/autofill/form_field.h"

// A form field that can parse either a FullNameField or a FirstLastNameField.
class NameField : public FormField {
 public:
  static NameField* Parse(std::vector<AutoFillField*>::const_iterator* iter,
                          bool is_ecml);

 protected:
  NameField() {}

 private:
  DISALLOW_COPY_AND_ASSIGN(NameField);
};

// A form field that can parse a full name field.
class FullNameField : public NameField {
 public:
  virtual bool GetFieldInfo(FieldTypeMap* field_type_map) const {
    bool ok = Add(field_type_map, field_, AutoFillType(NAME_FULL));
    DCHECK(ok);
    return true;
  }

  static FullNameField* Parse(
      std::vector<AutoFillField*>::const_iterator* iter);

 private:
  explicit FullNameField(AutoFillField* field) : field_(field) {}

  AutoFillField* field_;
  DISALLOW_COPY_AND_ASSIGN(FullNameField);
};

// A form field that can parse a first and last name field.
class FirstLastNameField : public NameField {
 public:
  static FirstLastNameField* Parse1(
      std::vector<AutoFillField*>::const_iterator* iter);
  static FirstLastNameField* Parse2(
      std::vector<AutoFillField*>::const_iterator* iter);
  static FirstLastNameField* ParseEcmlName(
      std::vector<AutoFillField*>::const_iterator* iter);
  static FirstLastNameField* Parse(
      std::vector<AutoFillField*>::const_iterator* iter, bool is_ecml);

  virtual bool GetFieldInfo(FieldTypeMap* field_type_map) const;

 private:
  FirstLastNameField();

  AutoFillField* first_name_;
  AutoFillField* middle_name_;  // Optional.
  AutoFillField* last_name_;
  bool middle_initial_;  // True if middle_name_ is a middle initial.

  DISALLOW_COPY_AND_ASSIGN(FirstLastNameField);
};

#endif  // CHROME_BROWSER_AUTOFILL_NAME_FIELD_H_
