// Copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "content/browser/renderer_host/resource_message_filter.h"

#include "chrome/browser/net/chrome_url_request_context.h"
#include "chrome/browser/profiles/profile.h"
#include "chrome/common/render_messages.h"
#include "content/browser/browser_thread.h"
#include "content/browser/resource_context.h"
#include "content/browser/renderer_host/resource_dispatcher_host.h"

ResourceMessageFilter::ResourceMessageFilter(
    int child_id,
    ChildProcessInfo::ProcessType process_type,
    const content::ResourceContext* resource_context,
    ResourceDispatcherHost* resource_dispatcher_host)
    : child_id_(child_id),
      process_type_(process_type),
      resource_dispatcher_host_(resource_dispatcher_host),
      resource_context_(resource_context) {
  DCHECK(resource_context);
}

ResourceMessageFilter::~ResourceMessageFilter() {
}

void ResourceMessageFilter::OnChannelClosing() {
  BrowserMessageFilter::OnChannelClosing();

  // Unhook us from all pending network requests so they don't get sent to a
  // deleted object.
  resource_dispatcher_host_->CancelRequestsForProcess(child_id_);
}

bool ResourceMessageFilter::OnMessageReceived(const IPC::Message& message,
                                              bool* message_was_ok) {
  return resource_dispatcher_host_->OnMessageReceived(
      message, this, message_was_ok);
}

ChromeURLRequestContext* ResourceMessageFilter::GetURLRequestContext(
    ResourceType::Type type) {
  net::URLRequestContext* rv = NULL;
  if (url_request_context_override_.get())
    rv = url_request_context_override_->GetRequestContext(type);

  if (!rv) {
    net::URLRequestContextGetter* context_getter =
        Profile::GetDefaultRequestContext();
    if (context_getter)
      rv = context_getter->GetURLRequestContext();
  }

  return static_cast<ChromeURLRequestContext*>(rv);
}
