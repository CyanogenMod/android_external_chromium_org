// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/extensions/api/cloud_print_private/cloud_print_private_api.h"

#include <string>

#include "base/threading/sequenced_worker_pool.h"
#include "chrome/browser/printing/cloud_print/cloud_print_proxy_service.h"
#include "chrome/browser/printing/cloud_print/cloud_print_proxy_service_factory.h"
#include "chrome/common/extensions/api/cloud_print_private.h"
#include "google_apis/google_api_keys.h"
#include "net/base/net_util.h"
#include "printing/backend/print_backend.h"



namespace extensions {

CloudPrintTestsDelegate* CloudPrintTestsDelegate::instance_ = NULL;

CloudPrintTestsDelegate* CloudPrintTestsDelegate::instance() {
  return instance_;
}

CloudPrintTestsDelegate::CloudPrintTestsDelegate() {
  instance_ = this;
}

CloudPrintTestsDelegate::~CloudPrintTestsDelegate() {
  instance_ = NULL;
}

CloudPrintPrivateSetupConnectorFunction::
    CloudPrintPrivateSetupConnectorFunction() {
}

CloudPrintPrivateSetupConnectorFunction::
    ~CloudPrintPrivateSetupConnectorFunction() {
}


bool CloudPrintPrivateSetupConnectorFunction::RunImpl() {
  using extensions::api::cloud_print_private::SetupConnector::Params;
  scoped_ptr<Params> params(Params::Create(*args_));
  if (CloudPrintTestsDelegate::instance()) {
    CloudPrintTestsDelegate::instance()->SetupConnector(
        params->user_email,
        params->robot_email,
        params->credentials,
        params->connect_new_printers,
        params->printer_blacklist);
  } else {
    CloudPrintProxyServiceFactory::GetForProfile(profile_)->
        EnableForUserWithRobot(params->credentials,
                               params->robot_email,
                               params->user_email,
                               params->connect_new_printers,
                               params->printer_blacklist);
  }
  SendResponse(true);
  return true;
}

CloudPrintPrivateGetHostNameFunction::CloudPrintPrivateGetHostNameFunction() {
}

CloudPrintPrivateGetHostNameFunction::~CloudPrintPrivateGetHostNameFunction() {
}

bool CloudPrintPrivateGetHostNameFunction::RunImpl() {
  SetResult(Value::CreateStringValue(
      CloudPrintTestsDelegate::instance() ?
      CloudPrintTestsDelegate::instance()->GetHostName() :
      net::GetHostName()));
  SendResponse(true);
  return true;
}

CloudPrintPrivateGetPrintersFunction::CloudPrintPrivateGetPrintersFunction() {
}

CloudPrintPrivateGetPrintersFunction::~CloudPrintPrivateGetPrintersFunction() {
}

void CloudPrintPrivateGetPrintersFunction::ReturnResult(
    const base::ListValue* printers) {
  SetResult(printers->DeepCopy());
  SendResponse(true);
}

void CloudPrintPrivateGetPrintersFunction::CollectPrinters() {
  scoped_ptr<base::ListValue> result(new base::ListValue());
  if (CloudPrintTestsDelegate::instance()) {
    std::vector<std::string> printers =
        CloudPrintTestsDelegate::instance()->GetPrinters();
    for (size_t i = 0; i < printers.size(); ++i)
      result->Append(Value::CreateStringValue(printers[i]));
  } else {
    printing::PrinterList printers;
    scoped_refptr<printing::PrintBackend> backend(
      printing::PrintBackend::CreateInstance(NULL));
    if (backend)
      backend->EnumeratePrinters(&printers);
    for (size_t i = 0; i < printers.size(); ++i)
      result->Append(Value::CreateStringValue(printers[i].printer_name));
  }
  content::BrowserThread::PostTask(content::BrowserThread::UI, FROM_HERE,
      base::Bind(&CloudPrintPrivateGetPrintersFunction::ReturnResult, this,
                 base::Owned(result.release())));
}


bool CloudPrintPrivateGetPrintersFunction::RunImpl() {
  content::BrowserThread::GetBlockingPool()->PostTask(FROM_HERE,
      base::Bind(&CloudPrintPrivateGetPrintersFunction::CollectPrinters, this));
  return true;
}


CloudPrintPrivateGetClientIdFunction::CloudPrintPrivateGetClientIdFunction() {
}

CloudPrintPrivateGetClientIdFunction::~CloudPrintPrivateGetClientIdFunction() {
}

bool CloudPrintPrivateGetClientIdFunction::RunImpl() {
  SetResult(Value::CreateStringValue(
      CloudPrintTestsDelegate::instance() ?
      CloudPrintTestsDelegate::instance()->GetClientId() :
      google_apis::GetOAuth2ClientID(google_apis::CLIENT_CLOUD_PRINT)));
  SendResponse(true);
  return true;
}

}  // namespace extensions
