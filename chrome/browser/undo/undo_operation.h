// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_UNDO_UNDO_OPERATION_H_
#define CHROME_BROWSER_UNDO_UNDO_OPERATION_H_

// Base class for all undo operations.
class UndoOperation {
 public:
  virtual ~UndoOperation() {}

  virtual void Undo() = 0;
};

#endif  // CHROME_BROWSER_UNDO_UNDO_OPERATION_H_
