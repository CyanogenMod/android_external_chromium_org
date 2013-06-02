// Copyright (c) 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "content/browser/renderer_host/media/web_contents_audio_input_stream.h"

#include <string>

#include "base/bind.h"
#include "base/bind_helpers.h"
#include "base/logging.h"
#include "base/memory/scoped_ptr.h"
#include "base/message_loop_proxy.h"
#include "content/browser/browser_main_loop.h"
#include "content/browser/renderer_host/media/audio_mirroring_manager.h"
#include "content/browser/renderer_host/media/web_contents_capture_util.h"
#include "content/browser/renderer_host/media/web_contents_tracker.h"
#include "content/public/browser/browser_thread.h"
#include "media/audio/virtual_audio_input_stream.h"
#include "media/audio/virtual_audio_output_stream.h"

namespace content {

class WebContentsAudioInputStream::Impl
    : public base::RefCountedThreadSafe<WebContentsAudioInputStream::Impl>,
      public AudioMirroringManager::MirroringDestination {
 public:
  // Takes ownership of |mixer_stream|.  The rest outlive this instance.
  Impl(int render_process_id, int render_view_id,
       const scoped_refptr<base::MessageLoopProxy>& message_loop,
       AudioMirroringManager* mirroring_manager,
       const scoped_refptr<WebContentsTracker>& tracker,
       media::VirtualAudioInputStream* mixer_stream);

  // Open underlying VirtualAudioInputStream and start tracker.
  bool Open();

  // Start the underlying VirtualAudioInputStream and instruct
  // AudioMirroringManager to begin a mirroring session.
  void Start(AudioInputCallback* callback);

  // Stop the underlying VirtualAudioInputStream and instruct
  // AudioMirroringManager to shutdown a mirroring session.
  void Stop();

  // Close the underlying VirtualAudioInputStream and stop the tracker.
  void Close();

  // Accessor to underlying VirtualAudioInputStream.
  media::VirtualAudioInputStream* mixer_stream() const {
    return mixer_stream_.get();
  }

 private:
  friend class base::RefCountedThreadSafe<WebContentsAudioInputStream::Impl>;

  enum State {
    CONSTRUCTED,
    OPENED,
    MIRRORING,
    CLOSED
  };

  virtual ~Impl();

  // Returns true if the mirroring target has been permanently lost.
  bool IsTargetLost() const;

  // Notifies the consumer callback that the stream is now dead.
  void ReportError();

  // Start/Stop mirroring by posting a call to AudioMirroringManager on the IO
  // BrowserThread.
  void StartMirroring();
  void StopMirroring();

  // AudioMirroringManager::MirroringDestination implementation
  virtual media::AudioOutputStream* AddInput(
      const media::AudioParameters& params) OVERRIDE;

  // Callback which is run when |stream| is closed.  Deletes |stream|.
  void ReleaseInput(media::VirtualAudioOutputStream* stream);

  // Called by WebContentsTracker when the target of the audio mirroring has
  // changed.
  void OnTargetChanged(int render_process_id, int render_view_id);

  // Injected dependencies.
  const scoped_refptr<base::MessageLoopProxy> message_loop_;
  AudioMirroringManager* const mirroring_manager_;
  const scoped_refptr<WebContentsTracker> tracker_;
  // The AudioInputStream implementation that handles the audio conversion and
  // mixing details.
  const scoped_ptr<media::VirtualAudioInputStream> mixer_stream_;

  State state_;

  // Current audio mirroring target.
  int target_render_process_id_;
  int target_render_view_id_;

  // Current callback used to consume the resulting mixed audio data.
  AudioInputCallback* callback_;

  DISALLOW_COPY_AND_ASSIGN(Impl);
};

WebContentsAudioInputStream::Impl::Impl(
    int render_process_id, int render_view_id,
    const scoped_refptr<base::MessageLoopProxy>& message_loop,
    AudioMirroringManager* mirroring_manager,
    const scoped_refptr<WebContentsTracker>& tracker,
    media::VirtualAudioInputStream* mixer_stream)
    : message_loop_(message_loop), mirroring_manager_(mirroring_manager),
      tracker_(tracker), mixer_stream_(mixer_stream), state_(CONSTRUCTED),
      target_render_process_id_(render_process_id),
      target_render_view_id_(render_view_id),
      callback_(NULL) {
  DCHECK(message_loop_.get());
  DCHECK(mirroring_manager_);
  DCHECK(tracker_.get());
  DCHECK(mixer_stream_.get());
}

WebContentsAudioInputStream::Impl::~Impl() {
  DCHECK(state_ == CONSTRUCTED || state_ == CLOSED);
}

bool WebContentsAudioInputStream::Impl::Open() {
  DCHECK(message_loop_->BelongsToCurrentThread());

  DCHECK_EQ(CONSTRUCTED, state_) << "Illegal to Open more than once.";

  if (!mixer_stream_->Open())
    return false;

  state_ = OPENED;

  tracker_->Start(
      target_render_process_id_, target_render_view_id_,
      base::Bind(&Impl::OnTargetChanged, this));

  return true;
}

void WebContentsAudioInputStream::Impl::Start(AudioInputCallback* callback) {
  DCHECK(message_loop_->BelongsToCurrentThread());
  DCHECK(callback);

  if (state_ != OPENED)
    return;

  callback_ = callback;
  if (IsTargetLost()) {
    ReportError();
    callback_ = NULL;
    return;
  }

  state_ = MIRRORING;
  mixer_stream_->Start(callback);

  StartMirroring();
}

void WebContentsAudioInputStream::Impl::Stop() {
  DCHECK(message_loop_->BelongsToCurrentThread());

  if (state_ != MIRRORING)
    return;

  state_ = OPENED;

  mixer_stream_->Stop();
  callback_ = NULL;

  if (!IsTargetLost())
    StopMirroring();
}

void WebContentsAudioInputStream::Impl::Close() {
  DCHECK(message_loop_->BelongsToCurrentThread());

  Stop();

  if (state_ == OPENED) {
    state_ = CONSTRUCTED;
    tracker_->Stop();
    mixer_stream_->Close();
  }

  DCHECK_EQ(CONSTRUCTED, state_);
  state_ = CLOSED;
}

bool WebContentsAudioInputStream::Impl::IsTargetLost() const {
  DCHECK(message_loop_->BelongsToCurrentThread());

  return target_render_process_id_ <= 0 || target_render_view_id_ <= 0;
}

void WebContentsAudioInputStream::Impl::ReportError() {
  DCHECK(message_loop_->BelongsToCurrentThread());

  // TODO(miu): Need clean-up of AudioInputCallback interface in a future
  // change, since its only implementation ignores the first argument entirely
  callback_->OnError(NULL);
}

void WebContentsAudioInputStream::Impl::StartMirroring() {
  DCHECK(message_loop_->BelongsToCurrentThread());

  BrowserThread::PostTask(
      BrowserThread::IO,
      FROM_HERE,
      base::Bind(&AudioMirroringManager::StartMirroring,
                 base::Unretained(mirroring_manager_),
                 target_render_process_id_, target_render_view_id_,
                 make_scoped_refptr(this)));
}

void WebContentsAudioInputStream::Impl::StopMirroring() {
  DCHECK(message_loop_->BelongsToCurrentThread());

  BrowserThread::PostTask(
      BrowserThread::IO,
      FROM_HERE,
      base::Bind(&AudioMirroringManager::StopMirroring,
                 base::Unretained(mirroring_manager_),
                 target_render_process_id_, target_render_view_id_,
                 make_scoped_refptr(this)));
}

media::AudioOutputStream* WebContentsAudioInputStream::Impl::AddInput(
    const media::AudioParameters& params) {
  // Note: The closure created here holds a reference to "this," which will
  // guarantee the VirtualAudioInputStream (mixer_stream_) outlives the
  // VirtualAudioOutputStream.
  return new media::VirtualAudioOutputStream(
      params,
      message_loop_.get(),
      mixer_stream_.get(),
      base::Bind(&Impl::ReleaseInput, this));
}

void WebContentsAudioInputStream::Impl::ReleaseInput(
    media::VirtualAudioOutputStream* stream) {
  delete stream;
}

void WebContentsAudioInputStream::Impl::OnTargetChanged(int render_process_id,
                                                        int render_view_id) {
  DCHECK(message_loop_->BelongsToCurrentThread());

  if (target_render_process_id_ == render_process_id &&
      target_render_view_id_ == render_view_id) {
    return;
  }

  DVLOG(1) << "Target RenderView has changed from "
           << target_render_process_id_ << ':' << target_render_view_id_
           << " to " << render_process_id << ':' << render_view_id;

  if (state_ == MIRRORING)
    StopMirroring();

  target_render_process_id_ = render_process_id;
  target_render_view_id_ = render_view_id;

  if (state_ == MIRRORING) {
    if (IsTargetLost()) {
      ReportError();
      Stop();
    } else {
      StartMirroring();
    }
  }
}

// static
WebContentsAudioInputStream* WebContentsAudioInputStream::Create(
    const std::string& device_id,
    const media::AudioParameters& params,
    const scoped_refptr<base::MessageLoopProxy>& message_loop) {
  int render_process_id;
  int render_view_id;
  if (!WebContentsCaptureUtil::ExtractTabCaptureTarget(
          device_id, &render_process_id, &render_view_id)) {
    return NULL;
  }

  return new WebContentsAudioInputStream(
      render_process_id, render_view_id, message_loop,
      BrowserMainLoop::GetAudioMirroringManager(),
      new WebContentsTracker(),
      new media::VirtualAudioInputStream(
          params, message_loop,
          media::VirtualAudioInputStream::AfterCloseCallback()));
}

WebContentsAudioInputStream::WebContentsAudioInputStream(
    int render_process_id, int render_view_id,
    const scoped_refptr<base::MessageLoopProxy>& message_loop,
    AudioMirroringManager* mirroring_manager,
    const scoped_refptr<WebContentsTracker>& tracker,
    media::VirtualAudioInputStream* mixer_stream)
    : impl_(new Impl(render_process_id, render_view_id, message_loop,
                     mirroring_manager, tracker, mixer_stream)) {}

WebContentsAudioInputStream::~WebContentsAudioInputStream() {}

bool WebContentsAudioInputStream::Open() {
  return impl_->Open();
}

void WebContentsAudioInputStream::Start(AudioInputCallback* callback) {
  impl_->Start(callback);
}

void WebContentsAudioInputStream::Stop() {
  impl_->Stop();
}

void WebContentsAudioInputStream::Close() {
  impl_->Close();
  delete this;
}

double WebContentsAudioInputStream::GetMaxVolume() {
  return impl_->mixer_stream()->GetMaxVolume();
}

void WebContentsAudioInputStream::SetVolume(double volume) {
  impl_->mixer_stream()->SetVolume(volume);
}

double WebContentsAudioInputStream::GetVolume() {
  return impl_->mixer_stream()->GetVolume();
}

void WebContentsAudioInputStream::SetAutomaticGainControl(bool enabled) {
  impl_->mixer_stream()->SetAutomaticGainControl(enabled);
}

bool WebContentsAudioInputStream::GetAutomaticGainControl() {
  return impl_->mixer_stream()->GetAutomaticGainControl();
}

}  // namespace content
