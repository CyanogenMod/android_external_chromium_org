// Copyright 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "cc/animation/animation.h"

#include "cc/test/animation_test_common.h"
#include "testing/gmock/include/gmock/gmock.h"
#include "testing/gtest/include/gtest/gtest.h"

namespace cc {
namespace {

scoped_ptr<Animation> CreateAnimation(int iterations, double duration) {
  scoped_ptr<Animation> to_return(Animation::Create(
      make_scoped_ptr(
          new FakeFloatAnimationCurve(duration)).PassAs<AnimationCurve>(),
      0,
      1,
      Animation::Opacity));
  to_return->set_iterations(iterations);
  return to_return.Pass();
}

scoped_ptr<Animation> CreateAnimation(int iterations) {
  return CreateAnimation(iterations, 1);
}

TEST(AnimationTest, TrimTimeZeroIterations) {
  scoped_ptr<Animation> anim(CreateAnimation(0));
  EXPECT_EQ(0, anim->TrimTimeToCurrentIteration(-1.0));
  EXPECT_EQ(0, anim->TrimTimeToCurrentIteration(0.0));
  EXPECT_EQ(0, anim->TrimTimeToCurrentIteration(1.0));
}

TEST(AnimationTest, TrimTimeOneIteration) {
  scoped_ptr<Animation> anim(CreateAnimation(1));
  EXPECT_EQ(0, anim->TrimTimeToCurrentIteration(-1.0));
  EXPECT_EQ(0, anim->TrimTimeToCurrentIteration(0.0));
  EXPECT_EQ(1, anim->TrimTimeToCurrentIteration(1.0));
  EXPECT_EQ(1, anim->TrimTimeToCurrentIteration(2.0));
}

TEST(AnimationTest, TrimTimeInfiniteIterations) {
  scoped_ptr<Animation> anim(CreateAnimation(-1));
  EXPECT_EQ(0, anim->TrimTimeToCurrentIteration(0.0));
  EXPECT_EQ(0.5, anim->TrimTimeToCurrentIteration(0.5));
  EXPECT_EQ(0, anim->TrimTimeToCurrentIteration(1.0));
  EXPECT_EQ(0.5, anim->TrimTimeToCurrentIteration(1.5));
}

TEST(AnimationTest, TrimTimeReverse) {
  scoped_ptr<Animation> anim(CreateAnimation(-1));
  anim->set_direction(Animation::Reverse);
  EXPECT_EQ(1.0, anim->TrimTimeToCurrentIteration(0));
  EXPECT_EQ(0.75, anim->TrimTimeToCurrentIteration(0.25));
  EXPECT_EQ(0.5, anim->TrimTimeToCurrentIteration(0.5));
  EXPECT_EQ(0.25, anim->TrimTimeToCurrentIteration(0.75));
  EXPECT_EQ(1.0, anim->TrimTimeToCurrentIteration(1.0));
  EXPECT_EQ(0.75, anim->TrimTimeToCurrentIteration(1.25));
}

TEST(AnimationTest, TrimTimeAlternateInfiniteIterations) {
  scoped_ptr<Animation> anim(CreateAnimation(-1));
  anim->set_direction(Animation::Alternate);
  EXPECT_EQ(0.0, anim->TrimTimeToCurrentIteration(0.0));
  EXPECT_EQ(0.25, anim->TrimTimeToCurrentIteration(0.25));
  EXPECT_EQ(0.5, anim->TrimTimeToCurrentIteration(0.5));
  EXPECT_EQ(0.75, anim->TrimTimeToCurrentIteration(0.75));
  EXPECT_EQ(1.0, anim->TrimTimeToCurrentIteration(1.0));
  EXPECT_EQ(0.75, anim->TrimTimeToCurrentIteration(1.25));
}

TEST(AnimationTest, TrimTimeAlternateOneIteration) {
  scoped_ptr<Animation> anim(CreateAnimation(1));
  anim->set_direction(Animation::Alternate);
  EXPECT_EQ(0.0, anim->TrimTimeToCurrentIteration(0.0));
  EXPECT_EQ(0.25, anim->TrimTimeToCurrentIteration(0.25));
  EXPECT_EQ(0.5, anim->TrimTimeToCurrentIteration(0.5));
  EXPECT_EQ(0.75, anim->TrimTimeToCurrentIteration(0.75));
  EXPECT_EQ(1.0, anim->TrimTimeToCurrentIteration(1.0));
  EXPECT_EQ(1.0, anim->TrimTimeToCurrentIteration(1.25));
}

TEST(AnimationTest, TrimTimeAlternateTwoIterations) {
  scoped_ptr<Animation> anim(CreateAnimation(2));
  anim->set_direction(Animation::Alternate);
  EXPECT_EQ(0.0, anim->TrimTimeToCurrentIteration(0.0));
  EXPECT_EQ(0.25, anim->TrimTimeToCurrentIteration(0.25));
  EXPECT_EQ(0.5, anim->TrimTimeToCurrentIteration(0.5));
  EXPECT_EQ(0.75, anim->TrimTimeToCurrentIteration(0.75));
  EXPECT_EQ(1.0, anim->TrimTimeToCurrentIteration(1.0));
  EXPECT_EQ(0.75, anim->TrimTimeToCurrentIteration(1.25));
  EXPECT_EQ(0.25, anim->TrimTimeToCurrentIteration(1.75));
  EXPECT_EQ(0.0, anim->TrimTimeToCurrentIteration(2.0));
  EXPECT_EQ(0.0, anim->TrimTimeToCurrentIteration(2.25));
}

TEST(AnimationTest, TrimTimeAlternateReverseInfiniteIterations) {
  scoped_ptr<Animation> anim(CreateAnimation(-1));
  anim->set_direction(Animation::AlternateReverse);
  EXPECT_EQ(1.0, anim->TrimTimeToCurrentIteration(0.0));
  EXPECT_EQ(0.75, anim->TrimTimeToCurrentIteration(0.25));
  EXPECT_EQ(0.5, anim->TrimTimeToCurrentIteration(0.5));
  EXPECT_EQ(0.25, anim->TrimTimeToCurrentIteration(0.75));
  EXPECT_EQ(0.0, anim->TrimTimeToCurrentIteration(1.0));
  EXPECT_EQ(0.25, anim->TrimTimeToCurrentIteration(1.25));
}

TEST(AnimationTest, TrimTimeAlternateReverseOneIteration) {
  scoped_ptr<Animation> anim(CreateAnimation(1));
  anim->set_direction(Animation::AlternateReverse);
  EXPECT_EQ(1.0, anim->TrimTimeToCurrentIteration(0.0));
  EXPECT_EQ(0.75, anim->TrimTimeToCurrentIteration(0.25));
  EXPECT_EQ(0.5, anim->TrimTimeToCurrentIteration(0.5));
  EXPECT_EQ(0.25, anim->TrimTimeToCurrentIteration(0.75));
  EXPECT_EQ(0.0, anim->TrimTimeToCurrentIteration(1.0));
  EXPECT_EQ(0.0, anim->TrimTimeToCurrentIteration(1.25));
}

TEST(AnimationTest, TrimTimeAlternateReverseTwoIterations) {
  scoped_ptr<Animation> anim(CreateAnimation(2));
  anim->set_direction(Animation::AlternateReverse);
  EXPECT_EQ(1.0, anim->TrimTimeToCurrentIteration(0.0));
  EXPECT_EQ(0.75, anim->TrimTimeToCurrentIteration(0.25));
  EXPECT_EQ(0.5, anim->TrimTimeToCurrentIteration(0.5));
  EXPECT_EQ(0.25, anim->TrimTimeToCurrentIteration(0.75));
  EXPECT_EQ(0.0, anim->TrimTimeToCurrentIteration(1.0));
  EXPECT_EQ(0.25, anim->TrimTimeToCurrentIteration(1.25));
  EXPECT_EQ(0.75, anim->TrimTimeToCurrentIteration(1.75));
  EXPECT_EQ(1.0, anim->TrimTimeToCurrentIteration(2.0));
  EXPECT_EQ(1.0, anim->TrimTimeToCurrentIteration(2.25));
}

TEST(AnimationTest, TrimTimeStartTime) {
  scoped_ptr<Animation> anim(CreateAnimation(1));
  anim->set_start_time(4);
  EXPECT_EQ(0, anim->TrimTimeToCurrentIteration(0.0));
  EXPECT_EQ(0, anim->TrimTimeToCurrentIteration(4.0));
  EXPECT_EQ(0.5, anim->TrimTimeToCurrentIteration(4.5));
  EXPECT_EQ(1, anim->TrimTimeToCurrentIteration(5.0));
  EXPECT_EQ(1, anim->TrimTimeToCurrentIteration(6.0));
}

TEST(AnimationTest, TrimTimeStartTimeReverse) {
  scoped_ptr<Animation> anim(CreateAnimation(1));
  anim->set_start_time(4);
  anim->set_direction(Animation::Reverse);
  EXPECT_EQ(0, anim->TrimTimeToCurrentIteration(0.0));
  EXPECT_EQ(1.0, anim->TrimTimeToCurrentIteration(4.0));
  EXPECT_EQ(0.5, anim->TrimTimeToCurrentIteration(4.5));
  EXPECT_EQ(0, anim->TrimTimeToCurrentIteration(5.0));
  EXPECT_EQ(0, anim->TrimTimeToCurrentIteration(6.0));
}

TEST(AnimationTest, TrimTimeTimeOffset) {
  scoped_ptr<Animation> anim(CreateAnimation(1));
  anim->set_time_offset(4);
  anim->set_start_time(4);
  EXPECT_EQ(0, anim->TrimTimeToCurrentIteration(0.0));
  EXPECT_EQ(0.5, anim->TrimTimeToCurrentIteration(0.5));
  EXPECT_EQ(1, anim->TrimTimeToCurrentIteration(1.0));
  EXPECT_EQ(1, anim->TrimTimeToCurrentIteration(1.0));
}

TEST(AnimationTest, TrimTimeTimeOffsetReverse) {
  scoped_ptr<Animation> anim(CreateAnimation(1));
  anim->set_time_offset(4);
  anim->set_start_time(4);
  anim->set_direction(Animation::Reverse);
  EXPECT_EQ(1.0, anim->TrimTimeToCurrentIteration(0.0));
  EXPECT_EQ(0.5, anim->TrimTimeToCurrentIteration(0.5));
  EXPECT_EQ(0, anim->TrimTimeToCurrentIteration(1.0));
  EXPECT_EQ(0, anim->TrimTimeToCurrentIteration(1.0));
}

TEST(AnimationTest, TrimTimeNegativeTimeOffset) {
  scoped_ptr<Animation> anim(CreateAnimation(1));
  anim->set_time_offset(-4);

  EXPECT_EQ(0, anim->TrimTimeToCurrentIteration(0.0));
  EXPECT_EQ(0, anim->TrimTimeToCurrentIteration(4.0));
  EXPECT_EQ(0.5, anim->TrimTimeToCurrentIteration(4.5));
  EXPECT_EQ(1, anim->TrimTimeToCurrentIteration(5.0));
}

TEST(AnimationTest, TrimTimeNegativeTimeOffsetReverse) {
  scoped_ptr<Animation> anim(CreateAnimation(1));
  anim->set_time_offset(-4);
  anim->set_direction(Animation::Reverse);

  EXPECT_EQ(0, anim->TrimTimeToCurrentIteration(0.0));
  EXPECT_EQ(1.0, anim->TrimTimeToCurrentIteration(4.0));
  EXPECT_EQ(0.5, anim->TrimTimeToCurrentIteration(4.5));
  EXPECT_EQ(0, anim->TrimTimeToCurrentIteration(5.0));
}

TEST(AnimationTest, TrimTimePauseResume) {
  scoped_ptr<Animation> anim(CreateAnimation(1));
  anim->SetRunState(Animation::Running, 0.0);
  EXPECT_EQ(0, anim->TrimTimeToCurrentIteration(0.0));
  EXPECT_EQ(0.5, anim->TrimTimeToCurrentIteration(0.5));
  anim->SetRunState(Animation::Paused, 0.5);
  EXPECT_EQ(0.5, anim->TrimTimeToCurrentIteration(1024.0));
  anim->SetRunState(Animation::Running, 1024.0);
  EXPECT_EQ(0.5, anim->TrimTimeToCurrentIteration(1024.0));
  EXPECT_EQ(1, anim->TrimTimeToCurrentIteration(1024.5));
}

TEST(AnimationTest, TrimTimePauseResumeReverse) {
  scoped_ptr<Animation> anim(CreateAnimation(1));
  anim->set_direction(Animation::Reverse);
  anim->SetRunState(Animation::Running, 0.0);
  EXPECT_EQ(1.0, anim->TrimTimeToCurrentIteration(0.0));
  EXPECT_EQ(0.5, anim->TrimTimeToCurrentIteration(0.5));
  anim->SetRunState(Animation::Paused, 0.25);
  EXPECT_EQ(0.75, anim->TrimTimeToCurrentIteration(1024.0));
  anim->SetRunState(Animation::Running, 1024.0);
  EXPECT_EQ(0.75, anim->TrimTimeToCurrentIteration(1024.0));
  EXPECT_EQ(0, anim->TrimTimeToCurrentIteration(1024.75));
}

TEST(AnimationTest, TrimTimeSuspendResume) {
  scoped_ptr<Animation> anim(CreateAnimation(1));
  anim->SetRunState(Animation::Running, 0.0);
  EXPECT_EQ(0, anim->TrimTimeToCurrentIteration(0.0));
  EXPECT_EQ(0.5, anim->TrimTimeToCurrentIteration(0.5));
  anim->Suspend(0.5);
  EXPECT_EQ(0.5, anim->TrimTimeToCurrentIteration(1024.0));
  anim->Resume(1024);
  EXPECT_EQ(0.5, anim->TrimTimeToCurrentIteration(1024.0));
  EXPECT_EQ(1, anim->TrimTimeToCurrentIteration(1024.5));
}

TEST(AnimationTest, TrimTimeSuspendResumeReverse) {
  scoped_ptr<Animation> anim(CreateAnimation(1));
  anim->set_direction(Animation::Reverse);
  anim->SetRunState(Animation::Running, 0.0);
  EXPECT_EQ(1.0, anim->TrimTimeToCurrentIteration(0.0));
  EXPECT_EQ(0.75, anim->TrimTimeToCurrentIteration(0.25));
  anim->Suspend(0.75);
  EXPECT_EQ(0.25, anim->TrimTimeToCurrentIteration(1024.0));
  anim->Resume(1024);
  EXPECT_EQ(0.25, anim->TrimTimeToCurrentIteration(1024.0));
  EXPECT_EQ(0, anim->TrimTimeToCurrentIteration(1024.25));
}

TEST(AnimationTest, TrimTimeZeroDuration) {
  scoped_ptr<Animation> anim(CreateAnimation(0, 0));
  anim->SetRunState(Animation::Running, 0.0);
  EXPECT_EQ(0, anim->TrimTimeToCurrentIteration(-1.0));
  EXPECT_EQ(0, anim->TrimTimeToCurrentIteration(0.0));
  EXPECT_EQ(0, anim->TrimTimeToCurrentIteration(1.0));
}

TEST(AnimationTest, TrimTimeStarting) {
  scoped_ptr<Animation> anim(CreateAnimation(1, 5.0));
  anim->SetRunState(Animation::Starting, 0.0);
  EXPECT_EQ(0.0, anim->TrimTimeToCurrentIteration(-1.0));
  EXPECT_EQ(0.0, anim->TrimTimeToCurrentIteration(0.0));
  EXPECT_EQ(0.0, anim->TrimTimeToCurrentIteration(1.0));
  anim->set_time_offset(2.0);
  EXPECT_EQ(2.0, anim->TrimTimeToCurrentIteration(-1.0));
  EXPECT_EQ(2.0, anim->TrimTimeToCurrentIteration(0.0));
  EXPECT_EQ(2.0, anim->TrimTimeToCurrentIteration(1.0));
  anim->set_start_time(1.0);
  EXPECT_EQ(0.0, anim->TrimTimeToCurrentIteration(-1.0));
  EXPECT_EQ(1.0, anim->TrimTimeToCurrentIteration(0.0));
  EXPECT_EQ(2.0, anim->TrimTimeToCurrentIteration(1.0));
  EXPECT_EQ(3.0, anim->TrimTimeToCurrentIteration(2.0));
}

TEST(AnimationTest, TrimTimeNeedsSynchronizedStartTime) {
  scoped_ptr<Animation> anim(CreateAnimation(1, 5.0));
  anim->SetRunState(Animation::Running, 0.0);
  anim->set_needs_synchronized_start_time(true);
  EXPECT_EQ(0.0, anim->TrimTimeToCurrentIteration(-1.0));
  EXPECT_EQ(0.0, anim->TrimTimeToCurrentIteration(0.0));
  EXPECT_EQ(0.0, anim->TrimTimeToCurrentIteration(1.0));
  anim->set_time_offset(2.0);
  EXPECT_EQ(2.0, anim->TrimTimeToCurrentIteration(-1.0));
  EXPECT_EQ(2.0, anim->TrimTimeToCurrentIteration(0.0));
  EXPECT_EQ(2.0, anim->TrimTimeToCurrentIteration(1.0));
  anim->set_start_time(1.0);
  anim->set_needs_synchronized_start_time(false);
  EXPECT_EQ(1.0, anim->TrimTimeToCurrentIteration(0.0));
  EXPECT_EQ(2.0, anim->TrimTimeToCurrentIteration(1.0));
  EXPECT_EQ(3.0, anim->TrimTimeToCurrentIteration(2.0));
}

TEST(AnimationTest, IsFinishedAtZeroIterations) {
  scoped_ptr<Animation> anim(CreateAnimation(0));
  anim->SetRunState(Animation::Running, 0.0);
  EXPECT_FALSE(anim->IsFinishedAt(-1.0));
  EXPECT_TRUE(anim->IsFinishedAt(0.0));
  EXPECT_TRUE(anim->IsFinishedAt(1.0));
}

TEST(AnimationTest, IsFinishedAtOneIteration) {
  scoped_ptr<Animation> anim(CreateAnimation(1));
  anim->SetRunState(Animation::Running, 0.0);
  EXPECT_FALSE(anim->IsFinishedAt(-1.0));
  EXPECT_FALSE(anim->IsFinishedAt(0.0));
  EXPECT_TRUE(anim->IsFinishedAt(1.0));
  EXPECT_TRUE(anim->IsFinishedAt(2.0));
}

TEST(AnimationTest, IsFinishedAtInfiniteIterations) {
  scoped_ptr<Animation> anim(CreateAnimation(-1));
  anim->SetRunState(Animation::Running, 0.0);
  EXPECT_FALSE(anim->IsFinishedAt(0.0));
  EXPECT_FALSE(anim->IsFinishedAt(0.5));
  EXPECT_FALSE(anim->IsFinishedAt(1.0));
  EXPECT_FALSE(anim->IsFinishedAt(1.5));
}

TEST(AnimationTest, IsFinishedNegativeTimeOffset) {
  scoped_ptr<Animation> anim(CreateAnimation(1));
  anim->set_time_offset(-0.5);
  anim->SetRunState(Animation::Running, 0.0);

  EXPECT_FALSE(anim->IsFinishedAt(-1.0));
  EXPECT_FALSE(anim->IsFinishedAt(0.0));
  EXPECT_FALSE(anim->IsFinishedAt(0.5));
  EXPECT_FALSE(anim->IsFinishedAt(1.0));
  EXPECT_TRUE(anim->IsFinishedAt(1.5));
  EXPECT_TRUE(anim->IsFinishedAt(2.0));
  EXPECT_TRUE(anim->IsFinishedAt(2.5));
}

TEST(AnimationTest, IsFinishedPositiveTimeOffset) {
  scoped_ptr<Animation> anim(CreateAnimation(1));
  anim->set_time_offset(0.5);
  anim->SetRunState(Animation::Running, 0.0);

  EXPECT_FALSE(anim->IsFinishedAt(-1.0));
  EXPECT_FALSE(anim->IsFinishedAt(0.0));
  EXPECT_TRUE(anim->IsFinishedAt(0.5));
  EXPECT_TRUE(anim->IsFinishedAt(1.0));
}

TEST(AnimationTest, IsFinishedAtNotRunning) {
  scoped_ptr<Animation> anim(CreateAnimation(0));
  anim->SetRunState(Animation::Running, 0.0);
  EXPECT_TRUE(anim->IsFinishedAt(0.0));
  anim->SetRunState(Animation::Paused, 0.0);
  EXPECT_FALSE(anim->IsFinishedAt(0.0));
  anim->SetRunState(Animation::WaitingForTargetAvailability, 0.0);
  EXPECT_FALSE(anim->IsFinishedAt(0.0));
  anim->SetRunState(Animation::Finished, 0.0);
  EXPECT_TRUE(anim->IsFinishedAt(0.0));
  anim->SetRunState(Animation::Aborted, 0.0);
  EXPECT_TRUE(anim->IsFinishedAt(0.0));
}

TEST(AnimationTest, IsFinished) {
  scoped_ptr<Animation> anim(CreateAnimation(1));
  anim->SetRunState(Animation::Running, 0.0);
  EXPECT_FALSE(anim->is_finished());
  anim->SetRunState(Animation::Paused, 0.0);
  EXPECT_FALSE(anim->is_finished());
  anim->SetRunState(Animation::WaitingForTargetAvailability, 0.0);
  EXPECT_FALSE(anim->is_finished());
  anim->SetRunState(Animation::Finished, 0.0);
  EXPECT_TRUE(anim->is_finished());
  anim->SetRunState(Animation::Aborted, 0.0);
  EXPECT_TRUE(anim->is_finished());
}

TEST(AnimationTest, IsFinishedNeedsSynchronizedStartTime) {
  scoped_ptr<Animation> anim(CreateAnimation(1));
  anim->SetRunState(Animation::Running, 2.0);
  EXPECT_FALSE(anim->is_finished());
  anim->SetRunState(Animation::Paused, 2.0);
  EXPECT_FALSE(anim->is_finished());
  anim->SetRunState(Animation::WaitingForTargetAvailability, 2.0);
  EXPECT_FALSE(anim->is_finished());
  anim->SetRunState(Animation::Finished, 0.0);
  EXPECT_TRUE(anim->is_finished());
  anim->SetRunState(Animation::Aborted, 0.0);
  EXPECT_TRUE(anim->is_finished());
}

TEST(AnimationTest, RunStateChangesIgnoredWhileSuspended) {
  scoped_ptr<Animation> anim(CreateAnimation(1));
  anim->Suspend(0);
  EXPECT_EQ(Animation::Paused, anim->run_state());
  anim->SetRunState(Animation::Running, 0.0);
  EXPECT_EQ(Animation::Paused, anim->run_state());
  anim->Resume(0);
  anim->SetRunState(Animation::Running, 0.0);
  EXPECT_EQ(Animation::Running, anim->run_state());
}

}  // namespace
}  // namespace cc
