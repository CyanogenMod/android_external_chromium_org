// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef MEDIA_FILTERS_DECRYPTING_DEMUXER_STREAM_H_
#define MEDIA_FILTERS_DECRYPTING_DEMUXER_STREAM_H_

#include "base/callback.h"
#include "base/memory/ref_counted.h"
#include "base/memory/weak_ptr.h"
#include "media/base/audio_decoder_config.h"
#include "media/base/decryptor.h"
#include "media/base/demuxer_stream.h"
#include "media/base/pipeline_status.h"
#include "media/base/video_decoder_config.h"

namespace base {
class SingleThreadTaskRunner;
}

namespace media {

class DecoderBuffer;

// Decryptor-based DemuxerStream implementation that converts a potentially
// encrypted demuxer stream to a clear demuxer stream.
// All public APIs and callbacks are trampolined to the |task_runner_| so
// that no locks are required for thread safety.
class MEDIA_EXPORT DecryptingDemuxerStream : public DemuxerStream {
 public:
  DecryptingDemuxerStream(
      const scoped_refptr<base::SingleThreadTaskRunner>& task_runner,
      const SetDecryptorReadyCB& set_decryptor_ready_cb);
  virtual ~DecryptingDemuxerStream();

  void Initialize(DemuxerStream* stream,
                  const PipelineStatusCB& status_cb);

  // Cancels all pending operations and fires all pending callbacks. If in
  // kPendingDemuxerRead or kPendingDecrypt state, waits for the pending
  // operation to finish before satisfying |closure|. Sets the state to
  // kUninitialized if |this| hasn't been initialized, or to kIdle otherwise.
  void Reset(const base::Closure& closure);

  // Cancels all pending operations immediately and fires all pending callbacks
  // and sets the state to kStopped. Does NOT wait for any pending operations.
  // Note: During the teardown process, media pipeline will be waiting on the
  // render main thread. If a Decryptor depends on the render main thread
  // (e.g. PpapiDecryptor), the pending DecryptCB would not be satisfied.
  void Stop(const base::Closure& closure);

  // DemuxerStream implementation.
  virtual void Read(const ReadCB& read_cb) OVERRIDE;
  virtual AudioDecoderConfig audio_decoder_config() OVERRIDE;
  virtual VideoDecoderConfig video_decoder_config() OVERRIDE;
  virtual Type type() OVERRIDE;
  virtual void EnableBitstreamConverter() OVERRIDE;
  virtual bool SupportsConfigChanges() OVERRIDE;
  virtual VideoRotation video_rotation() OVERRIDE;

 private:
  // For a detailed state diagram please see this link: http://goo.gl/8jAok
  // TODO(xhwang): Add a ASCII state diagram in this file after this class
  // stabilizes.
  // TODO(xhwang): Update this diagram for DecryptingDemuxerStream.
  enum State {
    kUninitialized = 0,
    kDecryptorRequested,
    kIdle,
    kPendingDemuxerRead,
    kPendingDecrypt,
    kWaitingForKey,
    kStopped
  };

  // Callback for DecryptorHost::RequestDecryptor().
  void SetDecryptor(Decryptor* decryptor);

  // Callback for DemuxerStream::Read().
  void DecryptBuffer(DemuxerStream::Status status,
                     const scoped_refptr<DecoderBuffer>& buffer);

  void DecryptPendingBuffer();

  // Callback for Decryptor::Decrypt().
  void DeliverBuffer(Decryptor::Status status,
                     const scoped_refptr<DecoderBuffer>& decrypted_buffer);

  // Callback for the |decryptor_| to notify this object that a new key has been
  // added.
  void OnKeyAdded();

  // Resets decoder and calls |reset_cb_|.
  void DoReset();

  // Returns Decryptor::StreamType converted from |stream_type_|.
  Decryptor::StreamType GetDecryptorStreamType() const;

  // Creates and initializes either |audio_config_| or |video_config_| based on
  // |demuxer_stream_|.
  void InitializeDecoderConfig();

  scoped_refptr<base::SingleThreadTaskRunner> task_runner_;

  State state_;

  PipelineStatusCB init_cb_;
  ReadCB read_cb_;
  base::Closure reset_cb_;

  // Pointer to the input demuxer stream that will feed us encrypted buffers.
  DemuxerStream* demuxer_stream_;

  AudioDecoderConfig audio_config_;
  VideoDecoderConfig video_config_;

  // Callback to request/cancel decryptor creation notification.
  SetDecryptorReadyCB set_decryptor_ready_cb_;

  Decryptor* decryptor_;

  // The buffer returned by the demuxer that needs to be decrypted.
  scoped_refptr<media::DecoderBuffer> pending_buffer_to_decrypt_;

  // Indicates the situation where new key is added during pending decryption
  // (in other words, this variable can only be set in state kPendingDecrypt).
  // If this variable is true and kNoKey is returned then we need to try
  // decrypting again in case the newly added key is the correct decryption key.
  bool key_added_while_decrypt_pending_;

  // NOTE: Weak pointers must be invalidated before all other member variables.
  base::WeakPtrFactory<DecryptingDemuxerStream> weak_factory_;
  base::WeakPtr<DecryptingDemuxerStream> weak_this_;

  DISALLOW_COPY_AND_ASSIGN(DecryptingDemuxerStream);
};

}  // namespace media

#endif  // MEDIA_FILTERS_DECRYPTING_DEMUXER_STREAM_H_
