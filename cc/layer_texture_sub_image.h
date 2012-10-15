// Copyright 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef LayerTextureSubImage_h
#define LayerTextureSubImage_h

#include "base/memory/scoped_ptr.h"
#include "GraphicsTypes3D.h"
#include "IntRect.h"
#include "IntSize.h"

namespace WebKit {
class WebGraphicsContext3D;
}

namespace cc {

class LayerTextureSubImage {
public:
    explicit LayerTextureSubImage(bool useMapSubForUpload);
    ~LayerTextureSubImage();

    void upload(const uint8_t* image, const IntRect& imageRect,
                const IntRect& sourceRect, const IntSize& destOffset,
                GC3Denum format, WebKit::WebGraphicsContext3D*);

private:
    void uploadWithTexSubImage(const uint8_t* image, const IntRect& imageRect,
                               const IntRect& sourceRect, const IntSize& destOffset,
                               GC3Denum format, WebKit::WebGraphicsContext3D*);
    void uploadWithMapTexSubImage(const uint8_t* image, const IntRect& imageRect,
                                  const IntRect& sourceRect, const IntSize& destOffset,
                                  GC3Denum format, WebKit::WebGraphicsContext3D*);

    bool m_useMapTexSubImage;
    size_t m_subImageSize;
    scoped_array<uint8_t> m_subImage;
};

} // namespace cc

#endif // LayerTextureSubImage_h
