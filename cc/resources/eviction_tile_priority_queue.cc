// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "cc/resources/eviction_tile_priority_queue.h"

namespace cc {

namespace {

class EvictionOrderComparator {
 public:
  explicit EvictionOrderComparator(TreePriority tree_priority)
      : tree_priority_(tree_priority) {}

  bool operator()(EvictionTilePriorityQueue::PairedPictureLayerQueue& a,
                  EvictionTilePriorityQueue::PairedPictureLayerQueue& b) const {
    if (a.IsEmpty())
      return true;

    if (b.IsEmpty())
      return false;

    PictureLayerImpl::LayerEvictionTileIterator* a_iterator =
        a.NextTileIterator(tree_priority_);
    PictureLayerImpl::LayerEvictionTileIterator* b_iterator =
        b.NextTileIterator(tree_priority_);

    Tile* a_tile = **a_iterator;
    Tile* b_tile = **b_iterator;

    const TilePriority& a_priority =
        a_tile->priority_for_tree_priority(tree_priority_);
    const TilePriority& b_priority =
        b_tile->priority_for_tree_priority(tree_priority_);
    bool prioritize_low_res = tree_priority_ == SMOOTHNESS_TAKES_PRIORITY;

    // Now we have to return true iff b is lower priority than a.

    // If the priority bin differs, b is lower priority if it has the higher
    // priority bin.
    if (a_priority.priority_bin != b_priority.priority_bin)
      return b_priority.priority_bin > a_priority.priority_bin;

    // Otherwise if the resolution differs, then the order will be determined by
    // whether we prioritize low res or not.
    // TODO(vmpstr): Remove this when TilePriority is no longer a member of Tile
    // class but instead produced by the iterators.
    if (b_priority.resolution != a_priority.resolution) {
      // Non ideal resolution should be sorted higher than other resolutions.
      if (a_priority.resolution == NON_IDEAL_RESOLUTION)
        return false;

      if (b_priority.resolution == NON_IDEAL_RESOLUTION)
        return true;

      if (prioritize_low_res)
        return a_priority.resolution == LOW_RESOLUTION;

      return a_priority.resolution == HIGH_RESOLUTION;
    }

    // Otherwise if the occlusion differs, b is lower priority if it is
    // occluded.
    bool a_is_occluded = a_tile->is_occluded_for_tree_priority(tree_priority_);
    bool b_is_occluded = b_tile->is_occluded_for_tree_priority(tree_priority_);
    if (a_is_occluded != b_is_occluded)
      return b_is_occluded;

    // b is lower priorty if it is farther from visible.
    return b_priority.distance_to_visible > a_priority.distance_to_visible;
  }

 private:
  TreePriority tree_priority_;
};

}  // namespace

EvictionTilePriorityQueue::EvictionTilePriorityQueue() {
}

EvictionTilePriorityQueue::~EvictionTilePriorityQueue() {
}

void EvictionTilePriorityQueue::Build(
    const std::vector<PictureLayerImpl::Pair>& paired_layers,
    TreePriority tree_priority) {
  tree_priority_ = tree_priority;

  for (std::vector<PictureLayerImpl::Pair>::const_iterator it =
           paired_layers.begin();
       it != paired_layers.end();
       ++it) {
    paired_queues_.push_back(PairedPictureLayerQueue(*it, tree_priority_));
  }

  std::make_heap(paired_queues_.begin(),
                 paired_queues_.end(),
                 EvictionOrderComparator(tree_priority_));
}

void EvictionTilePriorityQueue::Reset() {
  paired_queues_.clear();
}

bool EvictionTilePriorityQueue::IsEmpty() const {
  return paired_queues_.empty() || paired_queues_.front().IsEmpty();
}

Tile* EvictionTilePriorityQueue::Top() {
  DCHECK(!IsEmpty());
  return paired_queues_.front().Top(tree_priority_);
}

void EvictionTilePriorityQueue::Pop() {
  DCHECK(!IsEmpty());

  std::pop_heap(paired_queues_.begin(),
                paired_queues_.end(),
                EvictionOrderComparator(tree_priority_));
  PairedPictureLayerQueue& paired_queue = paired_queues_.back();
  paired_queue.Pop(tree_priority_);
  std::push_heap(paired_queues_.begin(),
                 paired_queues_.end(),
                 EvictionOrderComparator(tree_priority_));
}

EvictionTilePriorityQueue::PairedPictureLayerQueue::PairedPictureLayerQueue() {
}

EvictionTilePriorityQueue::PairedPictureLayerQueue::PairedPictureLayerQueue(
    const PictureLayerImpl::Pair& layer_pair,
    TreePriority tree_priority)
    : active_iterator(
          layer_pair.active
              ? PictureLayerImpl::LayerEvictionTileIterator(layer_pair.active,
                                                            tree_priority)
              : PictureLayerImpl::LayerEvictionTileIterator()),
      pending_iterator(
          layer_pair.pending
              ? PictureLayerImpl::LayerEvictionTileIterator(layer_pair.pending,
                                                            tree_priority)
              : PictureLayerImpl::LayerEvictionTileIterator()) {
}

EvictionTilePriorityQueue::PairedPictureLayerQueue::~PairedPictureLayerQueue() {
}

bool EvictionTilePriorityQueue::PairedPictureLayerQueue::IsEmpty() const {
  return !active_iterator && !pending_iterator;
}

Tile* EvictionTilePriorityQueue::PairedPictureLayerQueue::Top(
    TreePriority tree_priority) {
  DCHECK(!IsEmpty());

  PictureLayerImpl::LayerEvictionTileIterator* next_iterator =
      NextTileIterator(tree_priority);
  DCHECK(*next_iterator);

  Tile* tile = **next_iterator;
  DCHECK(std::find(returned_shared_tiles.begin(),
                   returned_shared_tiles.end(),
                   tile) == returned_shared_tiles.end());
  return tile;
}

void EvictionTilePriorityQueue::PairedPictureLayerQueue::Pop(
    TreePriority tree_priority) {
  DCHECK(!IsEmpty());

  PictureLayerImpl::LayerEvictionTileIterator* next_iterator =
      NextTileIterator(tree_priority);
  DCHECK(*next_iterator);
  returned_shared_tiles.push_back(**next_iterator);
  ++(*next_iterator);

  if (IsEmpty())
    return;

  next_iterator = NextTileIterator(tree_priority);
  while (std::find(returned_shared_tiles.begin(),
                   returned_shared_tiles.end(),
                   **next_iterator) != returned_shared_tiles.end()) {
    ++(*next_iterator);
    if (IsEmpty())
      break;
    next_iterator = NextTileIterator(tree_priority);
  }
}

PictureLayerImpl::LayerEvictionTileIterator*
EvictionTilePriorityQueue::PairedPictureLayerQueue::NextTileIterator(
    TreePriority tree_priority) {
  DCHECK(!IsEmpty());

  // If we only have one iterator with tiles, return it.
  if (!active_iterator)
    return &pending_iterator;
  if (!pending_iterator)
    return &active_iterator;

  Tile* active_tile = *active_iterator;
  Tile* pending_tile = *pending_iterator;
  if (active_tile == pending_tile)
    return &active_iterator;

  const TilePriority& active_priority =
      active_tile->priority_for_tree_priority(tree_priority);
  const TilePriority& pending_priority =
      pending_tile->priority_for_tree_priority(tree_priority);

  if (pending_priority.IsHigherPriorityThan(active_priority))
    return &active_iterator;
  return &pending_iterator;
}

}  // namespace cc
