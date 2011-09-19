// Copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// TransportSecurityState maintains an in memory database containing the
// list of hosts that currently have transport security enabled. This
// singleton object deals with writing that data out to disk as needed and
// loading it at startup.

// At startup we need to load the transport security state from the
// disk. For the moment, we don't want to delay startup for this load, so we
// let the TransportSecurityState run for a while without being loaded.
// This means that it's possible for pages opened very quickly not to get the
// correct transport security information.
//
// To load the state, we schedule a Task on the file thread which loads,
// deserialises and configures the TransportSecurityState.
//
// The TransportSecurityState object supports running a callback function
// when it changes. This object registers the callback, pointing at itself.
//
// TransportSecurityState calls...
// TransportSecurityPersister::StateIsDirty
//   since the callback isn't allowed to block or reenter, we schedule a Task
//   on the file thread after some small amount of time
//
// ...
//
// TransportSecurityPersister::SerialiseState
//   copies the current state of the TransportSecurityState, serialises
//   and writes to disk.

#ifndef CHROME_BROWSER_TRANSPORT_SECURITY_PERSISTER_H_
#define CHROME_BROWSER_TRANSPORT_SECURITY_PERSISTER_H_
#pragma once

#include <string>

#include "base/file_path.h"
#include "base/memory/ref_counted.h"
#include "chrome/common/important_file_writer.h"
#include "content/browser/browser_thread.h"
#include "net/base/transport_security_state.h"

class TransportSecurityPersister
    : public base::RefCountedThreadSafe<TransportSecurityPersister,
                                        BrowserThread::DeleteOnUIThread>,
      public net::TransportSecurityState::Delegate,
      public ImportantFileWriter::DataSerializer {
 public:
  TransportSecurityPersister(net::TransportSecurityState* state,
                             const FilePath& profile_path,
                             bool readonly);

  // Starts transport security data load on a background thread.
  // Must be called on the UI thread right after construction.
  void Init();

  // Called by the TransportSecurityState when it changes its state.
  virtual void StateIsDirty(net::TransportSecurityState*);

  // ImportantFileWriter::DataSerializer:
  virtual bool SerializeData(std::string* data);

 private:
  friend struct BrowserThread::DeleteOnThread<BrowserThread::UI>;
  friend class DeleteTask<TransportSecurityPersister>;

  virtual ~TransportSecurityPersister();

  void Load();
  void CompleteLoad(const std::string& state);

  // IO thread only.
  scoped_refptr<net::TransportSecurityState> transport_security_state_;

  // Helper for safely writing the data.
  ImportantFileWriter writer_;

  // Whether or not we're in read-only mode.
  const bool readonly_;

  DISALLOW_COPY_AND_ASSIGN(TransportSecurityPersister);
};

#endif  // CHROME_BROWSER_TRANSPORT_SECURITY_PERSISTER_H_
