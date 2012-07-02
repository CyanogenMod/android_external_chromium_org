// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_DOWNLOAD_DOWNLOAD_PATH_RESERVATION_TRACKER_H_
#define CHROME_BROWSER_DOWNLOAD_DOWNLOAD_PATH_RESERVATION_TRACKER_H_
#pragma once

#include <map>

#include "base/callback_forward.h"
#include "base/file_path.h"
#include "base/lazy_instance.h"
#include "content/public/browser/download_id.h"

// DownloadPathReservationTracker: Track download paths that are in use by
// active downloads.
//
// Chrome attempts to uniquify filenames that are assigned to downloads in order
// to avoid overwriting files that already exist on the file system. Downloads
// that are considered potentially dangerous use random intermediate filenames.
// Therefore only considering files that exist on the filesystem is
// insufficient. This class tracks files that are assigned to active downloads
// so that uniquification can take those into account as well.
//
// When a path needs to be assigned to a download, the |GetReservedPath()|
// static method is called on the UI thread along with a reference to the
// download item that will eventually receive the reserved path:
//
//   DownloadItem& download_item = <...>
//   DownloadPathReservationTracker::GetReservedPath(
//       download_item,
//       requested_target_path,
//       default_download_path,
//       should_uniquify_path,
//       completion_callback);
//
// This call creates a path reservation that will live until |download_item| is
// interrupted, cancelled, completes or is removed.
//
// The process of issuing a reservation happens on the FILE thread, and
// involves:
//
// - Creating |default_download_path| if it doesn't already exist.
//
// - Verifying that |requested_target_path| is writeable. If not, the user's
//   documents folder is used instead.
//
// - Uniquifying |requested_target_path| by suffixing the filename with a
//   uniquifier (e.g. "foo.txt" -> "foo (1).txt") in order to avoid conflicts
//   with files that already exist on the file system or other download path
//   reservations. Uniquifying is only done if |should_uniquify_path| is true.
//
// - Posting a task back to the UI thread to invoke |completion_callback| with
//   the reserved path and a bool indicating whether the returned path was
//   verified as being writeable and unique.
//
// In addition, if the target path of |download_item| is changed to a path other
// than the reserved path, then the reservation will be updated to match. Such
// changes can happen if a "Save As" dialog was displayed and the user chose a
// different path. The new target path is not checked against active paths to
// enforce uniqueness. It is only used for uniquifying new reservations.
//
// Once |completion_callback| is invoked, it is the caller's responsibility to
// handle cases where the target path could not be verified and set the target
// path of the |download_item| appropriately.
//
// Note: The current implementation doesn't look at symlinks/mount points. E.g.:
// It considers 'foo/bar/x.pdf' and 'foo/baz/x.pdf' to be two different paths,
// even though 'bar' might be a symlink to 'baz'.

namespace content {
class DownloadItem;
}

// Issues and tracks download paths that are in use by the download system. When
// a target path is set for a download, this object tracks the path and the
// associated download item so that subsequent downloads can avoid using the
// same path. All non-static methods must be invoked on the FILE thread.
class DownloadPathReservationTracker {
 public:
  // Callback used with |GetReservedPath|. |target_path| specifies the target
  // path for the download. |target_path_verified| is true if all of the
  // following is true:
  // - |requested_target_path| (passed into GetReservedPath()) was writeable.
  // - |target_path| was verified as being unique if uniqueness was
  //   required.
  //
  // If |requested_target_path| was not writeable, then the parent directory of
  // |target_path| may be different from that of |requested_target_path|.
  typedef base::Callback<void(const FilePath& target_path,
                              bool target_path_verified)> ReservedPathCallback;

  // The largest index for the uniquification suffix that we will try while
  // attempting to come up with a unique path.
  static const int kMaxUniqueFiles = 100;

  // Called on the UI thread to request a download path reservation. Begins
  // observing |download_item| and invokes ReserveInternal() on the FILE thread
  // to create the path reservation. Will not modify any state of
  // |download_item|.
  //
  // |default_download_path| is the user's default download path. If this
  // directory does not exist and is the parent directory of
  // |requested_target_path|, the directory will be created.
  static void GetReservedPath(content::DownloadItem& download_item,
                              const FilePath& requested_target_path,
                              const FilePath& default_download_path,
                              bool should_uniquify_path,
                              const ReservedPathCallback& callback);

 private:
  friend class DownloadPathReservationTrackerTest;
  friend struct base::DefaultLazyInstanceTraits<DownloadPathReservationTracker>;

  typedef std::map<content::DownloadId, FilePath> ReservationMap;

  DownloadPathReservationTracker();
  ~DownloadPathReservationTracker();

  // Called on the FILE thread to reserve a download path. This method:
  // - Creates directory |default_download_path| if it doesn't exist.
  // - Verifies that the parent directory of |suggested_path| exists and is
  //   writeable.
  // - Uniquifies |suggested_path| if |should_uniquify_path| is true.
  // - Schedules |callback| on the UI thread with the reserved path and a flag
  //   indicating whether the returned path has been successfully verified.
  void ReserveInternal(content::DownloadId download_id,
                       const FilePath& suggested_path,
                       const FilePath& default_download_path,
                       bool should_uniquify_path,
                       const ReservedPathCallback& callback);

  // Returns true if the given path is in use by any path reservation or the
  // file system. Called on the FILE thread.
  bool IsPathInUse(const FilePath& path) const;

  // Called on the FILE thread to update the path of the reservation associated
  // with |download_id| to |new_path|.
  void Update(content::DownloadId download_id, const FilePath& new_path);

  // Called on the FILE thread to remove the path reservation associated with
  // |download_id|.
  void Revoke(content::DownloadId download_id);

  // Get a pointer to the browser global instace of this object. Called on the
  // UI thread.
  static DownloadPathReservationTracker* GetInstance();

  ReservationMap reservations_;

  DISALLOW_COPY_AND_ASSIGN(DownloadPathReservationTracker);
};

#endif  // CHROME_BROWSER_DOWNLOAD_DOWNLOAD_PATH_RESERVATION_TRACKER_H_
