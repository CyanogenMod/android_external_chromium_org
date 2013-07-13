// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/ui/views/location_bar/autofill_credit_card_view.h"

#include "chrome/browser/ui/autofill/autofill_credit_card_bubble_controller.h"
#include "chrome/browser/ui/toolbar/toolbar_model.h"
#include "ui/gfx/image/image.h"

AutofillCreditCardView::AutofillCreditCardView(
    ToolbarModel* toolbar_model,
    LocationBarView::Delegate* delegate)
    : toolbar_model_(toolbar_model),
      delegate_(delegate) {
  Update();
}

AutofillCreditCardView::~AutofillCreditCardView() {}

void AutofillCreditCardView::Update() {
  autofill::AutofillCreditCardBubbleController* controller = GetController();
  if (controller && !controller->AnchorIcon().IsEmpty()) {
    SetVisible(true);
    SetImage(controller->AnchorIcon().AsImageSkia());
  } else {
    SetVisible(false);
    SetImage(NULL);
  }
}

// TODO(dbeam): figure out what to do for a tooltip and accessibility.

bool AutofillCreditCardView::CanHandleClick() const {
  autofill::AutofillCreditCardBubbleController* controller = GetController();
  return controller && !controller->IsHiding();
}

void AutofillCreditCardView::OnClick() {
  autofill::AutofillCreditCardBubbleController* controller = GetController();
  if (controller)
    controller->OnAnchorClicked();
}

autofill::AutofillCreditCardBubbleController* AutofillCreditCardView::
    GetController() const {
  content::WebContents* wc = delegate_->GetWebContents();
  if (!wc || toolbar_model_->GetInputInProgress())
    return NULL;

  return autofill::AutofillCreditCardBubbleController::FromWebContents(wc);
}
