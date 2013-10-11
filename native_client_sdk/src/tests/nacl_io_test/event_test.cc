/* Copyright (c) 2013 The Chromium Authors. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <errno.h>
#include <fcntl.h>
#include <pthread.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/time.h>

#include "gtest/gtest.h"

#include "nacl_io/event_emitter.h"
#include "nacl_io/event_listener.h"
#include "nacl_io/event_listener.h"
#include "nacl_io/event_listener.h"
#include "nacl_io/kernel_intercept.h"
#include "nacl_io/kernel_proxy.h"
#include "nacl_io/kernel_wrap.h"
#include "nacl_io/mount_node_pipe.h"
#include "nacl_io/mount_stream.h"

#include "ppapi_simple/ps.h"


using namespace nacl_io;
using namespace sdk_util;


class EventListenerTester : public EventListener {
 public:
  EventListenerTester() : EventListener(), events_(0) {};

  virtual void ReceiveEvents(EventEmitter* emitter, uint32_t events) {
    events_ |= events;
  }

  uint32_t Events() {
    return events_;
  }

  void Clear() {
    events_ = 0;
  }

  uint32_t events_;
};


TEST(EmitterBasic, SingleThread) {
  EventListenerTester listener_a;
  EventListenerTester listener_b;
  EventEmitter emitter;

  emitter.RegisterListener(&listener_a, POLLIN | POLLOUT | POLLERR);
  emitter.RegisterListener(&listener_b, POLLIN | POLLOUT | POLLERR);

  EXPECT_EQ(0, emitter.GetEventStatus());
  EXPECT_EQ(0, listener_a.Events());

  {
    AUTO_LOCK(emitter.GetLock())
    emitter.RaiseEvents_Locked(POLLIN);
  }
  EXPECT_EQ(POLLIN, listener_a.Events());

  listener_a.Clear();

  {
    AUTO_LOCK(emitter.GetLock())
    emitter.RaiseEvents_Locked(POLLOUT);
  }
  EXPECT_EQ(POLLOUT, listener_a.Events());
  EXPECT_EQ(POLLIN | POLLOUT, listener_b.Events());
}

class EmitterTest : public ::testing::Test {
 public:
  void SetUp() {
    pthread_cond_init(&multi_cond_, NULL);
    waiting_ = 0;
    signaled_ = 0;
  }

  void TearDown() {
    pthread_cond_destroy(&multi_cond_);
  }

  void CreateThread() {
    pthread_t id;
    EXPECT_EQ(0, pthread_create(&id, NULL, ThreadThunk, this));
  }

  static void* ThreadThunk(void *ptr) {
    return static_cast<EmitterTest*>(ptr)->ThreadEntry();
  }

  void* ThreadEntry() {
    EventListenerLock listener(&emitter_);

    pthread_cond_signal(&multi_cond_);
    waiting_++;
    EXPECT_EQ(0, listener.WaitOnEvent(POLLIN, -1));
    emitter_.ClearEvents_Locked(POLLIN);
    signaled_++;
    return NULL;
  }

 protected:
  pthread_cond_t multi_cond_;
  EventEmitter emitter_;

  uint32_t waiting_;
  uint32_t signaled_;
};


const int NUM_THREADS = 10;
TEST_F(EmitterTest, MultiThread) {
  for (int a=0; a <NUM_THREADS; a++)
    CreateThread();

  {
    AUTO_LOCK(emitter_.GetLock());

    // Wait for all threads to wait
    while(waiting_ < NUM_THREADS)
      pthread_cond_wait(&multi_cond_, emitter_.GetLock().mutex());

    ASSERT_EQ(0, signaled_);

    emitter_.RaiseEvents_Locked(POLLIN);
  }

  // sleep for 50 milliseconds
  struct timespec sleeptime = { 0,  50 * 1000 * 1000 };
  nanosleep(&sleeptime, NULL);

  EXPECT_EQ(1, signaled_);

  {
    AUTO_LOCK(emitter_.GetLock());
    emitter_.RaiseEvents_Locked(POLLIN);
  }

  nanosleep(&sleeptime, NULL);
  EXPECT_EQ(2, signaled_);

  // Clean up remaining threads.
  while (signaled_ < waiting_) {
    AUTO_LOCK(emitter_.GetLock());
    emitter_.RaiseEvents_Locked(POLLIN);
  }
}

TEST(PipeTest, Listener) {
  const char hello[] = "Hello World.";
  char tmp[64] = "Goodbye";

  EventEmitterPipe pipe(32);

  // Expect to time out on input.
  {
    EventListenerLock locker(&pipe);
    EXPECT_EQ(ETIMEDOUT, locker.WaitOnEvent(POLLIN, 0));
  }

  // Output should be ready to go.
  {
    EventListenerLock locker(&pipe);
    EXPECT_EQ(0, locker.WaitOnEvent(POLLOUT, 0));
    EXPECT_EQ(sizeof(hello), pipe.Write_Locked(hello, sizeof(hello)));
  }

  // We should now be able to poll
  {
    EventListenerLock locker(&pipe);
    EXPECT_EQ(0, locker.WaitOnEvent(POLLIN, 0));
    EXPECT_EQ(sizeof(hello), pipe.Read_Locked(tmp, sizeof(tmp)));
  }

  // Verify we can read it correctly.
  EXPECT_EQ(0, strcmp(hello, tmp));
}


class TestMountStream : public MountStream {
 public:
  TestMountStream() {}
};

TEST(PipeNodeTest, Basic) {
  ScopedMount mnt(new TestMountStream());

  MountNodePipe* pipe_node = new MountNodePipe(mnt.get());
  ScopedRef<MountNodePipe> pipe(pipe_node);

  EXPECT_EQ(POLLOUT, pipe_node->GetEventStatus());
}

const int MAX_FDS = 32;
class SelectPollTest : public ::testing::Test {
 public:
  void SetUp() {
    kp = new KernelProxy();
    kp->Init(NULL);
    EXPECT_EQ(0, kp->umount("/"));
    EXPECT_EQ(0, kp->mount("", "/", "memfs", 0, NULL));

    memset(&tv, 0, sizeof(tv));
  }

  void TearDown() {
    delete kp;
  }

  void SetFDs(int* fds, int cnt) {
    FD_ZERO(&rd_set);
    FD_ZERO(&wr_set);
    FD_ZERO(&ex_set);

    for (int index = 0; index < cnt; index++) {
      EXPECT_NE(-1, fds[index]);
      FD_SET(fds[index], &rd_set);
      FD_SET(fds[index], &wr_set);
      FD_SET(fds[index], &ex_set);

      pollfds[index].fd = fds[index];
      pollfds[index].events = POLLIN | POLLOUT;
      pollfds[index].revents = -1;
    }
  }

  void CloseFDs(int* fds, int cnt) {
    for (int index = 0; index < cnt; index++)
      kp->close(fds[index]);
  }

 protected:
  KernelProxy* kp;

  timeval tv;
  fd_set rd_set;
  fd_set wr_set;
  fd_set ex_set;
  struct pollfd pollfds[MAX_FDS];
};

TEST_F(SelectPollTest, PollMemPipe) {
  int fds[2];

  // Both FDs for regular files should be read/write but not exception.
  fds[0] = kp->open("/test.txt", O_CREAT | O_WRONLY);
  fds[1] = kp->open("/test.txt", O_RDONLY);

  SetFDs(fds, 2);

  EXPECT_EQ(2, kp->poll(pollfds, 2, 0));
  EXPECT_EQ(POLLIN | POLLOUT, pollfds[0].revents);
  EXPECT_EQ(POLLIN | POLLOUT, pollfds[1].revents);
  CloseFDs(fds, 2);

  // The write FD should select for write-only, read FD should not select
  EXPECT_EQ(0, kp->pipe(fds));
  SetFDs(fds, 2);

  EXPECT_EQ(2, kp->poll(pollfds, 2, 0));
  // TODO(noelallen) fix poll based on open mode
  // EXPECT_EQ(0, pollfds[0].revents);
  // Bug 291018
  EXPECT_EQ(POLLOUT, pollfds[1].revents);

  CloseFDs(fds, 2);
}

TEST_F(SelectPollTest, SelectMemPipe) {
  int fds[2];

  // Both FDs for regular files should be read/write but not exception.
  fds[0] = kp->open("/test.txt", O_CREAT | O_WRONLY);
  fds[1] = kp->open("/test.txt", O_RDONLY);
  SetFDs(fds, 2);

  EXPECT_EQ(4, kp->select(fds[1] + 1, &rd_set, &wr_set, &ex_set, &tv));
  EXPECT_NE(0, FD_ISSET(fds[0], &rd_set));
  EXPECT_NE(0, FD_ISSET(fds[1], &rd_set));
  EXPECT_NE(0, FD_ISSET(fds[0], &wr_set));
  EXPECT_NE(0, FD_ISSET(fds[1], &wr_set));
  EXPECT_EQ(0, FD_ISSET(fds[0], &ex_set));
  EXPECT_EQ(0, FD_ISSET(fds[1], &ex_set));

  CloseFDs(fds, 2);

  // The write FD should select for write-only, read FD should not select
  EXPECT_EQ(0, kp->pipe(fds));
  SetFDs(fds, 2);

  EXPECT_EQ(2, kp->select(fds[1] + 1, &rd_set, &wr_set, &ex_set, &tv));
  EXPECT_EQ(0, FD_ISSET(fds[0], &rd_set));
  EXPECT_EQ(0, FD_ISSET(fds[1], &rd_set));
  // TODO(noelallen) fix poll based on open mode
  // EXPECT_EQ(0, FD_ISSET(fds[0], &wr_set));
  // Bug 291018
  EXPECT_NE(0, FD_ISSET(fds[1], &wr_set));
  EXPECT_EQ(0, FD_ISSET(fds[0], &ex_set));
  EXPECT_EQ(0, FD_ISSET(fds[1], &ex_set));
}


