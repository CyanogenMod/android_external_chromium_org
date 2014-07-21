// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CC_RESOURCES_RASTER_TILE_PRIORITY_QUEUE_H_
#define CC_RESOURCES_RASTER_TILE_PRIORITY_QUEUE_H_

#include <utility>
#include <vector>

#include "cc/base/cc_export.h"
#include "cc/layers/picture_layer_impl.h"
#include "cc/resources/tile_priority.h"

namespace cc {

class CC_EXPORT RasterTilePriorityQueue {
 public:
  struct PairedPictureLayerQueue {
    PairedPictureLayerQueue();
    PairedPictureLayerQueue(const PictureLayerImpl::Pair& layer_pair,
                            TreePriority tree_priority);
    ~PairedPictureLayerQueue();

    bool IsEmpty() const;
    Tile* Top(TreePriority tree_priority);
    void Pop(TreePriority tree_priority);

    PictureLayerImpl::LayerRasterTileIterator* NextTileIterator(
        TreePriority tree_priority);

    PictureLayerImpl::LayerRasterTileIterator active_iterator;
    PictureLayerImpl::LayerRasterTileIterator pending_iterator;

    // TODO(vmpstr): Investigate removing this.
    std::vector<Tile*> returned_shared_tiles;
  };

  RasterTilePriorityQueue();
  ~RasterTilePriorityQueue();

  void Build(const std::vector<PictureLayerImpl::Pair>& paired_layers,
             TreePriority tree_priority);
  void Reset();

  bool IsEmpty() const;
  Tile* Top();
  void Pop();

 private:
  std::vector<PairedPictureLayerQueue> paired_queues_;
  TreePriority tree_priority_;

  DISALLOW_COPY_AND_ASSIGN(RasterTilePriorityQueue);
};

}  // namespace cc

#endif  // CC_RESOURCES_RASTER_TILE_PRIORITY_QUEUE_H_
