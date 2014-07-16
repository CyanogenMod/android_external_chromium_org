// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "cc/resources/scoped_gpu_raster.h"
#include "gpu/command_buffer/client/gles2_interface.h"
#include "third_party/khronos/GLES2/gl2.h"
#include "third_party/khronos/GLES2/gl2ext.h"
#include "third_party/skia/include/gpu/GrContext.h"

using gpu::gles2::GLES2Interface;

namespace cc {

ScopedGpuRaster::ScopedGpuRaster(ContextProvider* context_provider)
    : context_provider_(context_provider) {
  BeginGpuRaster();
}

ScopedGpuRaster::~ScopedGpuRaster() {
  EndGpuRaster();
}

void ScopedGpuRaster::BeginGpuRaster() {
  GLES2Interface* gl = context_provider_->ContextGL();

  // TODO(alokp): Use a trace macro to push/pop markers.
  // Using push/pop functions directly incurs cost to evaluate function
  // arguments even when tracing is disabled.
  gl->PushGroupMarkerEXT(0, "GpuRasterization");

  class GrContext* gr_context = context_provider_->GrContext();
  if (gr_context)
    gr_context->resetContext();
}

void ScopedGpuRaster::EndGpuRaster() {
  GLES2Interface* gl = context_provider_->ContextGL();

  class GrContext* gr_context = context_provider_->GrContext();
  if (gr_context)
    gr_context->flush();

  // TODO(alokp): Use a trace macro to push/pop markers.
  // Using push/pop functions directly incurs cost to evaluate function
  // arguments even when tracing is disabled.
  gl->PopGroupMarkerEXT();
}

}  // namespace cc
