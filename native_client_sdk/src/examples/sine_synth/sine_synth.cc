// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <cassert>
#include <cmath>
#include <limits>
#include <sstream>
#include "ppapi/cpp/audio.h"
#include "ppapi/cpp/instance.h"
#include "ppapi/cpp/module.h"
#include "ppapi/cpp/var.h"

namespace {
const char* const kPlaySoundId = "playSound";
const char* const kStopSoundId = "stopSound";
const char* const kSetFrequencyId = "setFrequency";
static const char kMessageArgumentSeparator = ':';

const double kDefaultFrequency = 440.0;
const double kPi = 3.141592653589;
const double kTwoPi = 2.0 * kPi;
// The sample count we will request.
const uint32_t kSampleFrameCount = 4096u;
// Only supporting stereo audio for now.
const uint32_t kChannels = 2u;
}  // namespace

namespace sine_synth {
// The Instance class.  One of these exists for each instance of your NaCl
// module on the web page.  The browser will ask the Module object to create
// a new Instance for each occurrence of the <embed> tag that has these
// attributes:
//     type="application/x-nacl"
//     src="sine_synth.nmf"
class SineSynthInstance : public pp::Instance {
 public:
  explicit SineSynthInstance(PP_Instance instance)
      : pp::Instance(instance),
        frequency_(kDefaultFrequency),
        theta_(0),
        sample_frame_count_(kSampleFrameCount) {}
  virtual ~SineSynthInstance() {}

  // Called by the browser once the NaCl module is loaded and ready to
  // initialize.  Creates a Pepper audio context and initializes it. Returns
  // true on success.  Returning false causes the NaCl module to be deleted and
  // no other functions to be called.
  virtual bool Init(uint32_t argc, const char* argn[], const char* argv[]);

  // Called by the browser to handle the postMessage() call in Javascript.
  // |var_message| is expected to be a string that contains the name of the
  // method to call.  Note that the setFrequency method takes a single
  // parameter, the frequency.  The frequency parameter is encoded as a string
  // and appended to the 'setFrequency' method name after a ':'.  Examples
  // of possible message strings are:
  //     playSound
  //     stopSound
  //     setFrequency:880
  // If |var_message| is not a recognized method name, this method does nothing.
  virtual void HandleMessage(const pp::Var& var_message);

  // Set the frequency of the sine wave to |frequency|.  Posts a message back
  // to the browser with the new frequency value.
  void SetFrequency(double frequency);

  // The frequency property accessor.
  double frequency() const { return frequency_; }

 private:
  static void SineWaveCallback(void* samples,
                               uint32_t buffer_size,
                               void* data) {
    SineSynthInstance* sine_synth_instance =
        reinterpret_cast<SineSynthInstance*>(data);
    const double frequency = sine_synth_instance->frequency();
    const double delta = kTwoPi * frequency / PP_AUDIOSAMPLERATE_44100;
    const int16_t max_int16 = std::numeric_limits<int16_t>::max();

    int16_t* buff = reinterpret_cast<int16_t*>(samples);

    // Make sure we can't write outside the buffer.
    assert(buffer_size >= (sizeof(*buff) * kChannels *
                           sine_synth_instance->sample_frame_count_));

    for (size_t sample_i = 0;
         sample_i < sine_synth_instance->sample_frame_count_;
         ++sample_i, sine_synth_instance->theta_ += delta) {
      // Keep theta_ from going beyond 2*Pi.
      if (sine_synth_instance->theta_ > kTwoPi) {
        sine_synth_instance->theta_ -= kTwoPi;
      }
      double sin_value(std::sin(sine_synth_instance->theta_));
      int16_t scaled_value = static_cast<int16_t>(sin_value * max_int16);
      for (size_t channel = 0; channel < kChannels; ++channel) {
        *buff++ = scaled_value;
      }
    }
  }

  pp::Audio audio_;
  double frequency_;

  // The last parameter sent to the sin function.  Used to prevent sine wave
  // skips on buffer boundaries.
  double theta_;

  // The count of sample frames per channel in an audio buffer.
  uint32_t sample_frame_count_;
};

bool SineSynthInstance::Init(uint32_t argc,
                             const char* argn[],
                             const char* argv[]) {
  // Ask the device for an appropriate sample count size.
  sample_frame_count_ =
      pp::AudioConfig::RecommendSampleFrameCount(this,
                                                 PP_AUDIOSAMPLERATE_44100,
                                                 kSampleFrameCount);
  audio_ = pp::Audio(this,
                     pp::AudioConfig(this,
                                     PP_AUDIOSAMPLERATE_44100,
                                     sample_frame_count_),
                     SineWaveCallback,
                     this);
  return true;
}

void SineSynthInstance::HandleMessage(const pp::Var& var_message) {
  if (!var_message.is_string()) {
    return;
  }
  std::string message = var_message.AsString();
  if (message == kPlaySoundId) {
    audio_.StartPlayback();
  } else if (message == kStopSoundId) {
    audio_.StopPlayback();
  } else if (message.find(kSetFrequencyId) == 0) {
    // The argument to setFrequency is everything after the first ':'.
    size_t sep_pos = message.find_first_of(kMessageArgumentSeparator);
    if (sep_pos != std::string::npos) {
      std::string string_arg = message.substr(sep_pos + 1);
      // Got the argument value as a string: try to convert it to a number.
      std::istringstream stream(string_arg);
      double double_value;
      if (stream >> double_value) {
        SetFrequency(double_value);
        return;
      }
    }
  }
}

void SineSynthInstance::SetFrequency(double frequency) {
  frequency_ = frequency;
  PostMessage(pp::Var(frequency_));
}

// The Module class.  The browser calls the CreateInstance() method to create
// an instance of your NaCl module on the web page.  The browser creates a new
// instance for each <embed> tag with type="application/x-nacl".
class SineSynthModule : public pp::Module {
 public:
  SineSynthModule() : pp::Module() {}
  ~SineSynthModule() {}

  // Create and return a HelloWorldInstance object.
  virtual pp::Instance* CreateInstance(PP_Instance instance) {
    return new SineSynthInstance(instance);
  }
};

}  // namespace sine_synth

// Factory function called by the browser when the module is first loaded.
// The browser keeps a singleton of this module.  It calls the
// CreateInstance() method on the object you return to make instances.  There
// is one instance per <embed> tag on the page.  This is the main binding
// point for your NaCl module with the browser.
namespace pp {
Module* CreateModule() {
  return new sine_synth::SineSynthModule();
}
}  // namespace pp
