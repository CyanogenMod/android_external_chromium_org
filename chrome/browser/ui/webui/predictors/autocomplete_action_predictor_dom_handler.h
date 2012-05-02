// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_UI_WEBUI_PREDICTORS_AUTOCOMPLETE_ACTION_PREDICTOR_DOM_HANDLER_H_
#define CHROME_BROWSER_UI_WEBUI_PREDICTORS_AUTOCOMPLETE_ACTION_PREDICTOR_DOM_HANDLER_H_
#pragma once

#include "base/compiler_specific.h"
#include "content/public/browser/web_ui_message_handler.h"

namespace base {
class ListValue;
}

class AutocompleteActionPredictor;
class Profile;

// The handler for Javascript messages for about:predictors.
class AutocompleteActionPredictorDOMHandler
    : public content::WebUIMessageHandler {
 public:
  explicit AutocompleteActionPredictorDOMHandler(Profile* profile);
  virtual ~AutocompleteActionPredictorDOMHandler();

  // WebUIMessageHandler implementation.
  virtual void RegisterMessages() OVERRIDE;

 private:
  // Synchronously fetches the database from AutocompleteActionPredictor and
  // calls into JS with the resulting DictionaryValue.
  void RequestAutocompleteActionPredictorDb(const base::ListValue* args);

  AutocompleteActionPredictor* autocomplete_action_predictor_;

  DISALLOW_COPY_AND_ASSIGN(AutocompleteActionPredictorDOMHandler);
};

#endif  // CHROME_BROWSER_UI_WEBUI_PREDICTORS_AUTOCOMPLETE_ACTION_PREDICTOR_DOM_HANDLER_H_
