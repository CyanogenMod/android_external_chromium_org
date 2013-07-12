// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_UI_WEBUI_MEMORY_INTERNALS_MEMORY_INTERNALS_PROXY_H_
#define CHROME_BROWSER_UI_WEBUI_MEMORY_INTERNALS_MEMORY_INTERNALS_PROXY_H_

#include <string>

#include "base/memory/ref_counted.h"
#include "base/strings/string16.h"
#include "base/values.h"
#include "chrome/browser/memory_details.h"
#include "content/public/browser/browser_thread.h"

class MemoryInternalsHandler;
class RendererDetails;

namespace base {
class ListValue;
class Value;
}

class MemoryInternalsProxy
    : public base::RefCountedThreadSafe<
          MemoryInternalsProxy, content::BrowserThread::DeleteOnUIThread> {
 public:
  MemoryInternalsProxy();

  // Registers a handler.
  void Attach(MemoryInternalsHandler* handler);

  // Unregisters a handler.
  void Detach();

  // Sends a message to an internal client to send all process information it
  // knows.
  void StartFetch(const base::ListValue* list);

 private:
  friend struct
      content::BrowserThread::DeleteOnThread<content::BrowserThread::UI>;
  friend class base::DeleteHelper<MemoryInternalsProxy>;

  typedef ProcessMemoryInformationList::const_iterator PMIIterator;

  virtual ~MemoryInternalsProxy();

  // Enumerates all processes information, appending a few common information.
  void OnProcessAvailable(const ProcessData& browser);

  // Measures memory usage of V8.
  void OnRendererAvailable(const base::ProcessId pid,
                           const size_t v8_allocated,
                           const size_t v8_used);

  // Requests all renderer processes to get detailed memory information.
  void RequestRendererDetails();

  // Be called on finish of collection.
  void FinishCollection();

  // Calls a JavaScript function on a UI page.
  void CallJavaScriptFunctionOnUIThread(const std::string& function,
                                        const base::Value& args);

  MemoryInternalsHandler* handler_;
  base::DictionaryValue* information_;
  RendererDetails* renderer_details_;

  DISALLOW_COPY_AND_ASSIGN(MemoryInternalsProxy);
};

#endif  // CHROME_BROWSER_UI_WEBUI_MEMORY_INTERNALS_MEMORY_INTERNALS_PROXY_H_
