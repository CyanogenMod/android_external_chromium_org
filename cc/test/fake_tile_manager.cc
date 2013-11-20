// Copyright 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "cc/test/fake_tile_manager.h"

#include "cc/resources/raster_worker_pool.h"

namespace cc {

namespace {

class FakeRasterWorkerPool : public RasterWorkerPool {
 public:
  FakeRasterWorkerPool() : RasterWorkerPool(NULL, 1) {}

  virtual void ScheduleTasks(RasterTask::Queue* queue) OVERRIDE {}
  virtual GLenum GetResourceFormat() const OVERRIDE { return GL_RGBA; }
  virtual void OnRasterTasksFinished() OVERRIDE {}
  virtual void OnRasterTasksRequiredForActivationFinished() OVERRIDE {}
};

}  // namespace

FakeTileManager::FakeTileManager(TileManagerClient* client)
    : TileManager(client,
                  NULL,
                  make_scoped_ptr<RasterWorkerPool>(new FakeRasterWorkerPool),
                  1,
                  NULL) {}

FakeTileManager::FakeTileManager(TileManagerClient* client,
                                 ResourceProvider* resource_provider)
    : TileManager(client,
                  resource_provider,
                  make_scoped_ptr<RasterWorkerPool>(new FakeRasterWorkerPool),
                  1,
                  NULL) {}

FakeTileManager::~FakeTileManager() {}

void FakeTileManager::AssignMemoryToTiles() {
  tiles_for_raster.clear();
  all_tiles.Clear();

  GetPrioritizedTileSet(&all_tiles);
  AssignGpuMemoryToTiles(&all_tiles, &tiles_for_raster);
}

bool FakeTileManager::HasBeenAssignedMemory(Tile* tile) {
  return std::find(tiles_for_raster.begin(),
                   tiles_for_raster.end(),
                   tile) != tiles_for_raster.end();
}

}  // namespace cc
