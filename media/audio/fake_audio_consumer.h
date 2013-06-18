// Copyright (c) 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef MEDIA_AUDIO_FAKE_AUDIO_CONSUMER_H_
#define MEDIA_AUDIO_FAKE_AUDIO_CONSUMER_H_

#include "base/callback_forward.h"
#include "base/memory/ref_counted.h"
#include "media/base/media_export.h"

namespace base {
class MessageLoopProxy;
}

namespace media {
class AudioBus;
class AudioParameters;

// A fake audio consumer.  Using a provided message loop, FakeAudioConsumer will
// simulate a real time consumer of audio data.
class MEDIA_EXPORT FakeAudioConsumer {
 public:
  // |worker_loop| is the loop on which the ReadCB provided to Start() will be
  // executed on.  This may or may not be the be for the same thread that
  // invokes the Start/Stop methods.
  // |params| is used to determine the frequency of callbacks.
  FakeAudioConsumer(const scoped_refptr<base::MessageLoopProxy>& worker_loop,
                    const AudioParameters& params);
  ~FakeAudioConsumer();

  // Start executing |read_cb| at a regular intervals.  Stop() must be called by
  // the same thread before destroying FakeAudioConsumer.
  typedef base::Callback<void(AudioBus* audio_bus)> ReadCB;
  void Start(const ReadCB& read_cb);

  // Stop executing the ReadCB provided to Start().  Blocks until the worker
  // loop is not inside a ReadCB invocation.  Safe to call multiple times.  Must
  // be called on the same thread that called Start().
  void Stop();

 private:
  // All state and implementation is kept within this class because it must hang
  // around until tasks posted on |worker_loop| have all run (or been canceled).
  class Worker;
  Worker* const worker_;

  DISALLOW_COPY_AND_ASSIGN(FakeAudioConsumer);
};

}  // namespace media

#endif  // MEDIA_AUDIO_FAKE_AUDIO_CONSUMER_H_
