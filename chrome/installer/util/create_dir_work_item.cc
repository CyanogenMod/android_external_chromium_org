// Copyright (c) 2010 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "base/file_util.h"
#include "base/logging.h"
#include "chrome/installer/util/create_dir_work_item.h"
#include "chrome/installer/util/logging_installer.h"

CreateDirWorkItem::~CreateDirWorkItem() {
}

CreateDirWorkItem::CreateDirWorkItem(const base::FilePath& path)
    : path_(path),
      rollback_needed_(false) {
}

void CreateDirWorkItem::GetTopDirToCreate() {
  if (base::PathExists(path_)) {
    top_path_ = base::FilePath();
    return;
  }

  base::FilePath parent_dir(path_);
  do {
    top_path_ = parent_dir;
    parent_dir = parent_dir.DirName();
  } while ((parent_dir != top_path_) && !base::PathExists(parent_dir));
  return;
}

bool CreateDirWorkItem::Do() {
  VLOG(1) << "creating directory " << path_.value();
  GetTopDirToCreate();
  if (top_path_.empty())
    return true;

  VLOG(1) << "top directory that needs to be created: " << top_path_.value();
  bool result = file_util::CreateDirectory(path_);
  VLOG(1) << "directory creation result: " << result;

  rollback_needed_ = true;

  return result;
}

void CreateDirWorkItem::Rollback() {
  if (!rollback_needed_)
    return;

  // Delete all the directories we created to rollback.
  // Note we can not recusively delete top_path_ since we don't want to
  // delete non-empty directory. (We may have created a shared directory).
  // Instead we walk through path_ to top_path_ and delete directories
  // along the way.
  base::FilePath path_to_delete(path_);

  while (1) {
    if (base::PathExists(path_to_delete)) {
      if (!RemoveDirectory(path_to_delete.value().c_str()))
        break;
    }
    if (path_to_delete == top_path_)
      break;
    path_to_delete = path_to_delete.DirName();
  }

  return;
}
