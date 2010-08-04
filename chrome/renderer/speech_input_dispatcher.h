// Copyright (c) 2010 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_RENDERER_SPEECH_INPUT_DISPATCHER_H_
#define CHROME_RENDERER_SPEECH_INPUT_DISPATCHER_H_

#include "base/basictypes.h"
#include "ipc/ipc_message.h"
#include "third_party/WebKit/WebKit/chromium/public/WebSpeechInputController.h"

class GURL;
class RenderView;

namespace WebKit {
class WebSpeechInputListener;
}

// SpeechInputDispatcher is a delegate for speech input messages used by WebKit.
// It's the complement of SpeechInputDispatcherHost (owned by RenderViewHost).
class SpeechInputDispatcher : public WebKit::WebSpeechInputController {
 public:
  SpeechInputDispatcher(RenderView* render_view,
                        WebKit::WebSpeechInputListener* listener);

  // Called to possibly handle the incoming IPC message. Returns true if
  // handled. Called in render thread.
  bool OnMessageReceived(const IPC::Message& msg);

  // WebKit::WebSpeechInputController.
  bool startRecognition();
  void cancelRecognition();
  void stopRecording();

 private:
  void OnSpeechRecognitionResult(const string16& result);
  void OnSpeechRecordingComplete();
  void OnSpeechRecognitionComplete();

  RenderView* render_view_;
  WebKit::WebSpeechInputListener* listener_;

  DISALLOW_COPY_AND_ASSIGN(SpeechInputDispatcher);
};

#endif  // CHROME_RENDERER_SPEECH_INPUT_DISPATCHER_H_
