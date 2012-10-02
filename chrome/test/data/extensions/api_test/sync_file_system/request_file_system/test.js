// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

var syncableNameSuffix = ':Syncable';

chrome.test.runTests([
  function requestFileSystem() {
    chrome.syncFileSystem.requestFileSystem(
        'drive',
        chrome.test.callbackPass(function(fs) {
            chrome.test.assertEq('DOMFileSystem', fs.constructor.name);
            chrome.test.assertTrue(fs.name != undefined);
            chrome.test.assertEq(fs.name.length - syncableNameSuffix.length,
                                 fs.name.lastIndexOf(syncableNameSuffix));
        }));
  },
  function requestFileSystemForInvalidService() {
    chrome.syncFileSystem.requestFileSystem(
        'foo',
        chrome.test.callbackFail(
            'Cloud service foo not supported.',
            function(fs) { chrome.test.assertEq(null, fs); }));
  }
]);
