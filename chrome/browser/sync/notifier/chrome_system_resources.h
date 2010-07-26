// Copyright (c) 2010 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// Simple system resources class that uses the current message loop
// for scheduling.  Assumes the current message loop is already
// running.

#ifndef CHROME_BROWSER_SYNC_NOTIFIER_CHROME_SYSTEM_RESOURCES_H_
#define CHROME_BROWSER_SYNC_NOTIFIER_CHROME_SYSTEM_RESOURCES_H_
#pragma once

#include <set>

#include "base/non_thread_safe.h"
#include "base/scoped_ptr.h"
#include "base/task.h"
#include "google/cacheinvalidation/invalidation-client.h"

namespace sync_notifier {

class ChromeSystemResources : public invalidation::SystemResources {
 public:
  ChromeSystemResources();

  ~ChromeSystemResources();

  // invalidation::SystemResources implementation.

  virtual invalidation::Time current_time();

  virtual void StartScheduler();

  virtual void StopScheduler();

  virtual void ScheduleWithDelay(invalidation::TimeDelta delay,
                                 invalidation::Closure* task);

  virtual void ScheduleImmediately(invalidation::Closure* task);

  virtual void Log(LogLevel level, const char* file, int line,
                   const char* format, ...);

 private:
  NonThreadSafe non_thread_safe_;
  scoped_ptr<ScopedRunnableMethodFactory<ChromeSystemResources> >
      scoped_runnable_method_factory_;
  // Holds all posted tasks that have not yet been run.
  std::set<invalidation::Closure*> posted_tasks_;

  // If the scheduler has been started, inserts |task| into
  // |posted_tasks_| and returns a Task* to post.  Otherwise,
  // immediately deletes |task| and returns NULL.
  Task* MakeTaskToPost(invalidation::Closure* task);

  // Runs the task, deletes it, and removes it from |posted_tasks_|.
  void RunPostedTask(invalidation::Closure* task);
};

}  // namespace sync_notifier

#endif  // CHROME_BROWSER_SYNC_NOTIFIER_CHROME_SYSTEM_RESOURCES_H_
