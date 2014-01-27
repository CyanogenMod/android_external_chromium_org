// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Custom binding for the Media Gallery API.

var binding = require('binding').Binding.create('mediaGalleries');

var mediaGalleriesNatives = requireNative('mediaGalleries');
var blobNatives = requireNative('blob_natives');

var mediaGalleriesMetadata = {};

function createFileSystemObjectsAndUpdateMetadata(response) {
  var result = null;
  mediaGalleriesMetadata = {};  // Clear any previous metadata.
  if (response) {
    result = [];
    for (var i = 0; i < response.length; i++) {
      var filesystem = mediaGalleriesNatives.GetMediaFileSystemObject(
          response[i].fsid);
      $Array.push(result, filesystem);
      var metadata = response[i];
      delete metadata.fsid;
      mediaGalleriesMetadata[filesystem.name] = metadata;
    }
  }
  return result;
}

binding.registerCustomHook(function(bindingsAPI, extensionId) {
  var apiFunctions = bindingsAPI.apiFunctions;

  // getMediaFileSystems, addUserSelectedFolder, and addScanResults use a
  // custom callback so that they can instantiate and return an array of file
  // system objects.
  apiFunctions.setCustomCallback('getMediaFileSystems',
                                 function(name, request, response) {
    var result = createFileSystemObjectsAndUpdateMetadata(response);
    if (request.callback)
      request.callback(result);
    request.callback = null;
  });

  apiFunctions.setCustomCallback('addScanResults',
                                 function(name, request, response) {
    var result = createFileSystemObjectsAndUpdateMetadata(response);
    if (request.callback)
      request.callback(result);
    request.callback = null;
  });

  apiFunctions.setCustomCallback('addUserSelectedFolder',
                                 function(name, request, response) {
    var fileSystems = [];
    var selectedFileSystemName = "";
    if (response && 'mediaFileSystems' in response &&
        'selectedFileSystemIndex' in response) {
      fileSystems = createFileSystemObjectsAndUpdateMetadata(
          response['mediaFileSystems']);
      var selectedFileSystemIndex = response['selectedFileSystemIndex'];
      if (selectedFileSystemIndex >= 0) {
        selectedFileSystemName = fileSystems[selectedFileSystemIndex].name;
      }
    }
    if (request.callback)
      request.callback(fileSystems, selectedFileSystemName);
    request.callback = null;
  });

  apiFunctions.setHandleRequest('getMediaFileSystemMetadata',
                                function(filesystem) {
    if (filesystem && filesystem.name &&
        mediaGalleriesMetadata[filesystem.name]) {
      return mediaGalleriesMetadata[filesystem.name];
    }
    return {};
  });

  apiFunctions.setUpdateArgumentsPostValidate('getMetadata',
      function(mediaFile, options, callback) {
    var blobUuid = blobNatives.GetBlobUuid(mediaFile)
    return [blobUuid, options, callback];
  });
});

exports.binding = binding.generate();
