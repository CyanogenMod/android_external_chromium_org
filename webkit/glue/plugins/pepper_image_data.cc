// Copyright (c) 2010 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "webkit/glue/plugins/pepper_image_data.h"

#include <algorithm>
#include <limits>

#include "base/logging.h"
#include "base/scoped_ptr.h"
#include "skia/ext/platform_canvas.h"
#include "ppapi/c/pp_instance.h"
#include "ppapi/c/pp_module.h"
#include "ppapi/c/pp_resource.h"
#include "ppapi/c/ppb_image_data.h"
#include "ppapi/c/trusted/ppb_image_data_trusted.h"
#include "third_party/skia/include/core/SkColorPriv.h"
#include "webkit/glue/plugins/pepper_plugin_instance.h"
#include "webkit/glue/plugins/pepper_plugin_module.h"

namespace pepper {

namespace {

PP_ImageDataFormat GetNativeImageDataFormat() {
  return ImageData::GetNativeImageDataFormat();
}

bool IsImageDataFormatSupported(PP_ImageDataFormat format) {
  return ImageData::IsImageDataFormatSupported(format);
}

PP_Resource Create(PP_Module module_id,
                   PP_ImageDataFormat format,
                   const PP_Size* size,
                   bool init_to_zero) {
  PluginModule* module = ResourceTracker::Get()->GetModule(module_id);
  if (!module)
    return 0;

  scoped_refptr<ImageData> data(new ImageData(module));
  if (!data->Init(format, size->width, size->height, init_to_zero))
    return 0;

  return data->GetReference();
}

bool IsImageData(PP_Resource resource) {
  return !!Resource::GetAs<ImageData>(resource);
}

bool Describe(PP_Resource resource, PP_ImageDataDesc* desc) {
  // Give predictable values on failure.
  memset(desc, 0, sizeof(PP_ImageDataDesc));

  scoped_refptr<ImageData> image_data(Resource::GetAs<ImageData>(resource));
  if (!image_data)
    return false;
  image_data->Describe(desc);
  return true;
}

void* Map(PP_Resource resource) {
  scoped_refptr<ImageData> image_data(Resource::GetAs<ImageData>(resource));
  if (!image_data)
    return NULL;
  return image_data->Map();
}

void Unmap(PP_Resource resource) {
  scoped_refptr<ImageData> image_data(Resource::GetAs<ImageData>(resource));
  if (image_data)
    image_data->Unmap();
}

uint64_t GetNativeMemoryHandle2(PP_Resource resource) {
  scoped_refptr<ImageData> image_data(Resource::GetAs<ImageData>(resource));
  if (image_data)
    return image_data->GetNativeMemoryHandle();
  return 0;
}

const PPB_ImageData ppb_imagedata = {
  &GetNativeImageDataFormat,
  &IsImageDataFormatSupported,
  &Create,
  &IsImageData,
  &Describe,
  &Map,
  &Unmap,
};

const PPB_ImageDataTrusted ppb_imagedata_trusted = {
  &GetNativeMemoryHandle2,
};

}  // namespace

ImageData::ImageData(PluginModule* module)
    : Resource(module),
      format_(PP_IMAGEDATAFORMAT_BGRA_PREMUL),
      width_(0),
      height_(0) {
}

ImageData::~ImageData() {
}

// static
const PPB_ImageData* ImageData::GetInterface() {
  return &ppb_imagedata;
}

// static
const PPB_ImageDataTrusted* ImageData::GetTrustedInterface() {
  return &ppb_imagedata_trusted;
}

// static
PP_ImageDataFormat ImageData::GetNativeImageDataFormat() {
  if (SK_B32_SHIFT == 0)
    return PP_IMAGEDATAFORMAT_BGRA_PREMUL;
  else if (SK_R32_SHIFT == 0)
    return PP_IMAGEDATAFORMAT_RGBA_PREMUL;
  else
    return PP_IMAGEDATAFORMAT_BGRA_PREMUL;  // Default to something on failure.
}

// static
bool ImageData::IsImageDataFormatSupported(PP_ImageDataFormat format) {
  return format == PP_IMAGEDATAFORMAT_BGRA_PREMUL ||
      format == PP_IMAGEDATAFORMAT_RGBA_PREMUL;
}

bool ImageData::Init(PP_ImageDataFormat format,
                     int width, int height,
                     bool init_to_zero) {
  // TODO(brettw) this should be called only on the main thread!
  // TODO(brettw) use init_to_zero when we implement caching.
  if (!IsImageDataFormatSupported(format))
    return false;  // Only support this one format for now.
  if (width <= 0 || height <= 0)
    return false;
  if (static_cast<int64>(width) * static_cast<int64>(height) >=
      std::numeric_limits<int32>::max())
    return false;  // Prevent overflow of signed 32-bit ints.

  platform_image_.reset(
      module()->GetSomeInstance()->delegate()->CreateImage2D(width, height));
  format_ = format;
  width_ = width;
  height_ = height;
  return !!platform_image_.get();
}

void ImageData::Describe(PP_ImageDataDesc* desc) const {
  desc->format = format_;
  desc->size.width = width_;
  desc->size.height = height_;
  desc->stride = width_ * 4;
}

void* ImageData::Map() {
  if (!mapped_canvas_.get()) {
    mapped_canvas_.reset(platform_image_->Map());
    if (!mapped_canvas_.get())
      return NULL;
  }
  const SkBitmap& bitmap =
      mapped_canvas_->getTopPlatformDevice().accessBitmap(true);

  // Our platform bitmaps are set to opaque by default, which we don't want.
  const_cast<SkBitmap&>(bitmap).setIsOpaque(false);

  bitmap.lockPixels();
  return bitmap.getAddr32(0, 0);
}

void ImageData::Unmap() {
  // This is currently unimplemented, which is OK. The data will just always
  // be around once it's mapped. Chrome's TransportDIB isn't currently
  // unmappable without freeing it, but this may be something we want to support
  // in the future to save some memory.
}

uint64 ImageData::GetNativeMemoryHandle() const {
  return platform_image_->GetSharedMemoryHandle();
}

const SkBitmap* ImageData::GetMappedBitmap() const {
  if (!mapped_canvas_.get())
    return NULL;
  return &mapped_canvas_->getTopPlatformDevice().accessBitmap(false);
}

void ImageData::Swap(ImageData* other) {
  swap(other->platform_image_, platform_image_);
  swap(other->mapped_canvas_, mapped_canvas_);
  std::swap(other->format_, format_);
  std::swap(other->width_, width_);
  std::swap(other->height_, height_);
}

}  // namespace pepper
