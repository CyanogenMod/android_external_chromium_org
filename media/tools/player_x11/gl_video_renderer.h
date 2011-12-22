// Copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef MEDIA_TOOLS_PLAYER_X11_GL_VIDEO_RENDERER_H_
#define MEDIA_TOOLS_PLAYER_X11_GL_VIDEO_RENDERER_H_

#include "base/basictypes.h"
#include "ui/gfx/gl/gl_bindings.h"

class MessageLoop;

namespace media {
class VideoFrame;
}

class GlVideoRenderer {
 public:
  GlVideoRenderer(Display* display, Window window);
  ~GlVideoRenderer();

  void Paint(media::VideoFrame* video_frame);

 private:
  // Initializes GL rendering for the given dimensions.
  void Initialize(int width, int height);

  Display* display_;
  Window window_;

  // GL context.
  GLXContext gl_context_;

  // 3 textures, one for each plane.
  GLuint textures_[3];

  DISALLOW_COPY_AND_ASSIGN(GlVideoRenderer);
};

#endif  // MEDIA_TOOLS_PLAYER_X11_GL_VIDEO_RENDERER_H_
