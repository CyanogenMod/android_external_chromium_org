// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/ui/autofill/country_combobox_model.h"

#include "base/logging.h"
#include "base/strings/utf_string_conversions.h"
#include "chrome/browser/browser_process.h"
#include "components/autofill/core/browser/autofill_country.h"
#include "components/autofill/core/browser/personal_data_manager.h"
#include "ui/base/l10n/l10n_util_collator.h"
#include "ui/base/models/combobox_model_observer.h"

// TODO(rouslan): Remove this check. http://crbug.com/337587
#if defined(ENABLE_AUTOFILL_DIALOG)
#include "chrome/browser/ui/autofill/autofill_dialog_i18n_input.h"
#include "third_party/libaddressinput/chromium/cpp/include/libaddressinput/address_ui.h"
#endif

namespace autofill {

namespace {

bool ShouldShowCountry(const std::string& country_code,
                       bool show_partially_supported_countries,
                       const std::set<base::string16>& candidate_countries) {
#if defined(ENABLE_AUTOFILL_DIALOG)
  if (!show_partially_supported_countries &&
      !i18ninput::CountryIsFullySupported(country_code)) {
    return false;
  }
#endif

  if (!candidate_countries.empty() &&
      !candidate_countries.count(base::ASCIIToUTF16(country_code))) {
    return false;
  }

  return true;
}

}  // namespace

CountryComboboxModel::CountryComboboxModel(
    const PersonalDataManager& manager,
    const std::set<base::string16>& country_filter,
    bool show_partially_supported_countries) {
  // Insert the default country at the top as well as in the ordered list.
  std::string default_country_code =
      manager.GetDefaultCountryCodeForNewAddress();
  DCHECK(!default_country_code.empty());

  const std::string& app_locale = g_browser_process->GetApplicationLocale();
  if (ShouldShowCountry(default_country_code,
                        show_partially_supported_countries,
                        country_filter)) {
    countries_.push_back(new AutofillCountry(default_country_code, app_locale));
    // The separator item.
    countries_.push_back(NULL);
  }

  // The sorted list of countries.
#if defined(ENABLE_AUTOFILL_DIALOG)
  const std::vector<std::string>& available_countries =
      ::i18n::addressinput::GetRegionCodes();
#else
  std::vector<std::string> available_countries;
  AutofillCountry::GetAvailableCountries(&available_countries);
#endif

  std::vector<AutofillCountry*> sorted_countries;
  for (std::vector<std::string>::const_iterator it =
           available_countries.begin(); it != available_countries.end(); ++it) {
    if (ShouldShowCountry(*it,
                          show_partially_supported_countries,
                          country_filter)) {
      sorted_countries.push_back(new AutofillCountry(*it, app_locale));
    }
  }

  l10n_util::SortStringsUsingMethod(app_locale,
                                    &sorted_countries,
                                    &AutofillCountry::name);
  countries_.insert(countries_.end(),
                    sorted_countries.begin(),
                    sorted_countries.end());
}

CountryComboboxModel::~CountryComboboxModel() {}

int CountryComboboxModel::GetItemCount() const {
  return countries_.size();
}

base::string16 CountryComboboxModel::GetItemAt(int index) {
  AutofillCountry* country = countries_[index];
  if (country)
    return countries_[index]->name();

  // The separator item. Implemented for platforms that don't yet support
  // IsItemSeparatorAt().
  return base::ASCIIToUTF16("---");
}

bool CountryComboboxModel::IsItemSeparatorAt(int index) {
  return !countries_[index];
}

std::string CountryComboboxModel::GetDefaultCountryCode() const {
  return countries_[GetDefaultIndex()]->country_code();
}

}  // namespace autofill
