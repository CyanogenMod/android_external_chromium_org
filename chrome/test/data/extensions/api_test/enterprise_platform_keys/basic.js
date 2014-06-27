// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Must be packed to ../enterprise_platform_keys.crx using the private key
// ../enterprise_platform_keys.pem .

'use strict';

var assertEq = chrome.test.assertEq;
var assertTrue = chrome.test.assertTrue;
var assertThrows = chrome.test.assertThrows;
var fail = chrome.test.fail;
var succeed = chrome.test.succeed;
var callbackPass = chrome.test.callbackPass;
var callbackFail= chrome.test.callbackFail;

// openssl req -new -x509 -key privkey.pem \
//   -outform der -out cert.der -days 36500
// xxd -i cert.der
// based on privateKeyPkcs8
var cert1a = new Uint8Array([
  0x30, 0x82, 0x01, 0xd5, 0x30, 0x82, 0x01, 0x7f, 0xa0, 0x03, 0x02, 0x01,
  0x02, 0x02, 0x09, 0x00, 0xd2, 0xcc, 0x76, 0xeb, 0x19, 0xb9, 0x3a, 0x33,
  0x30, 0x0d, 0x06, 0x09, 0x2a, 0x86, 0x48, 0x86, 0xf7, 0x0d, 0x01, 0x01,
  0x05, 0x05, 0x00, 0x30, 0x45, 0x31, 0x0b, 0x30, 0x09, 0x06, 0x03, 0x55,
  0x04, 0x06, 0x13, 0x02, 0x41, 0x55, 0x31, 0x13, 0x30, 0x11, 0x06, 0x03,
  0x55, 0x04, 0x08, 0x0c, 0x0a, 0x53, 0x6f, 0x6d, 0x65, 0x2d, 0x53, 0x74,
  0x61, 0x74, 0x65, 0x31, 0x21, 0x30, 0x1f, 0x06, 0x03, 0x55, 0x04, 0x0a,
  0x0c, 0x18, 0x49, 0x6e, 0x74, 0x65, 0x72, 0x6e, 0x65, 0x74, 0x20, 0x57,
  0x69, 0x64, 0x67, 0x69, 0x74, 0x73, 0x20, 0x50, 0x74, 0x79, 0x20, 0x4c,
  0x74, 0x64, 0x30, 0x20, 0x17, 0x0d, 0x31, 0x34, 0x30, 0x34, 0x31, 0x35,
  0x31, 0x34, 0x35, 0x32, 0x30, 0x33, 0x5a, 0x18, 0x0f, 0x32, 0x31, 0x31,
  0x34, 0x30, 0x33, 0x32, 0x32, 0x31, 0x34, 0x35, 0x32, 0x30, 0x33, 0x5a,
  0x30, 0x45, 0x31, 0x0b, 0x30, 0x09, 0x06, 0x03, 0x55, 0x04, 0x06, 0x13,
  0x02, 0x41, 0x55, 0x31, 0x13, 0x30, 0x11, 0x06, 0x03, 0x55, 0x04, 0x08,
  0x0c, 0x0a, 0x53, 0x6f, 0x6d, 0x65, 0x2d, 0x53, 0x74, 0x61, 0x74, 0x65,
  0x31, 0x21, 0x30, 0x1f, 0x06, 0x03, 0x55, 0x04, 0x0a, 0x0c, 0x18, 0x49,
  0x6e, 0x74, 0x65, 0x72, 0x6e, 0x65, 0x74, 0x20, 0x57, 0x69, 0x64, 0x67,
  0x69, 0x74, 0x73, 0x20, 0x50, 0x74, 0x79, 0x20, 0x4c, 0x74, 0x64, 0x30,
  0x5c, 0x30, 0x0d, 0x06, 0x09, 0x2a, 0x86, 0x48, 0x86, 0xf7, 0x0d, 0x01,
  0x01, 0x01, 0x05, 0x00, 0x03, 0x4b, 0x00, 0x30, 0x48, 0x02, 0x41, 0x00,
  0xc7, 0xc1, 0x4d, 0xd5, 0xdc, 0x3a, 0x2e, 0x1f, 0x42, 0x30, 0x3d, 0x21,
  0x1e, 0xa2, 0x1f, 0x60, 0xcb, 0x71, 0x11, 0x53, 0xb0, 0x75, 0xa0, 0x62,
  0xfe, 0x5e, 0x0a, 0xde, 0xb0, 0x0f, 0x48, 0x97, 0x5e, 0x42, 0xa7, 0x3a,
  0xd1, 0xca, 0x4c, 0xe3, 0xdb, 0x5f, 0x31, 0xc2, 0x99, 0x08, 0x89, 0xcd,
  0x6d, 0x20, 0xaa, 0x75, 0xe6, 0x2b, 0x98, 0xd2, 0xf3, 0x7b, 0x4b, 0xe5,
  0x9b, 0xfe, 0xe2, 0x6d, 0x02, 0x03, 0x01, 0x00, 0x01, 0xa3, 0x50, 0x30,
  0x4e, 0x30, 0x1d, 0x06, 0x03, 0x55, 0x1d, 0x0e, 0x04, 0x16, 0x04, 0x14,
  0xbd, 0x85, 0x6b, 0xdd, 0x84, 0xd1, 0x54, 0x2e, 0xad, 0xb4, 0x5e, 0xdd,
  0x24, 0x7e, 0x16, 0x9c, 0x84, 0x1e, 0x19, 0xf0, 0x30, 0x1f, 0x06, 0x03,
  0x55, 0x1d, 0x23, 0x04, 0x18, 0x30, 0x16, 0x80, 0x14, 0xbd, 0x85, 0x6b,
  0xdd, 0x84, 0xd1, 0x54, 0x2e, 0xad, 0xb4, 0x5e, 0xdd, 0x24, 0x7e, 0x16,
  0x9c, 0x84, 0x1e, 0x19, 0xf0, 0x30, 0x0c, 0x06, 0x03, 0x55, 0x1d, 0x13,
  0x04, 0x05, 0x30, 0x03, 0x01, 0x01, 0xff, 0x30, 0x0d, 0x06, 0x09, 0x2a,
  0x86, 0x48, 0x86, 0xf7, 0x0d, 0x01, 0x01, 0x05, 0x05, 0x00, 0x03, 0x41,
  0x00, 0x37, 0x23, 0x2f, 0x81, 0x24, 0xfc, 0xec, 0x2d, 0x0b, 0xd1, 0xa0,
  0x74, 0xdf, 0x2e, 0x34, 0x9a, 0x92, 0x33, 0xae, 0x75, 0xd6, 0x60, 0xfc,
  0x44, 0x1d, 0x65, 0x8c, 0xb7, 0xd9, 0x60, 0x3b, 0xc7, 0x20, 0x30, 0xdf,
  0x17, 0x07, 0xd1, 0x87, 0xda, 0x2b, 0x7f, 0x84, 0xf3, 0xfc, 0xb0, 0x31,
  0x42, 0x08, 0x17, 0x96, 0xd2, 0x1b, 0xdc, 0x28, 0xae, 0xf8, 0xbd, 0xf9,
  0x4e, 0x78, 0xc3, 0xe8, 0x80
]);

// based on privateKeyPkcs8, different from cert1a
var cert1b = new Uint8Array([
  0x30, 0x82, 0x01, 0xd5, 0x30, 0x82, 0x01, 0x7f, 0xa0, 0x03, 0x02, 0x01,
  0x02, 0x02, 0x09, 0x00, 0xe7, 0x1e, 0x6e, 0xb0, 0x12, 0x87, 0xf5, 0x09,
  0x30, 0x0d, 0x06, 0x09, 0x2a, 0x86, 0x48, 0x86, 0xf7, 0x0d, 0x01, 0x01,
  0x05, 0x05, 0x00, 0x30, 0x45, 0x31, 0x0b, 0x30, 0x09, 0x06, 0x03, 0x55,
  0x04, 0x06, 0x13, 0x02, 0x41, 0x55, 0x31, 0x13, 0x30, 0x11, 0x06, 0x03,
  0x55, 0x04, 0x08, 0x0c, 0x0a, 0x53, 0x6f, 0x6d, 0x65, 0x2d, 0x53, 0x74,
  0x61, 0x74, 0x65, 0x31, 0x21, 0x30, 0x1f, 0x06, 0x03, 0x55, 0x04, 0x0a,
  0x0c, 0x18, 0x49, 0x6e, 0x74, 0x65, 0x72, 0x6e, 0x65, 0x74, 0x20, 0x57,
  0x69, 0x64, 0x67, 0x69, 0x74, 0x73, 0x20, 0x50, 0x74, 0x79, 0x20, 0x4c,
  0x74, 0x64, 0x30, 0x20, 0x17, 0x0d, 0x31, 0x34, 0x30, 0x34, 0x31, 0x35,
  0x31, 0x35, 0x31, 0x39, 0x30, 0x30, 0x5a, 0x18, 0x0f, 0x32, 0x31, 0x31,
  0x34, 0x30, 0x33, 0x32, 0x32, 0x31, 0x35, 0x31, 0x39, 0x30, 0x30, 0x5a,
  0x30, 0x45, 0x31, 0x0b, 0x30, 0x09, 0x06, 0x03, 0x55, 0x04, 0x06, 0x13,
  0x02, 0x41, 0x55, 0x31, 0x13, 0x30, 0x11, 0x06, 0x03, 0x55, 0x04, 0x08,
  0x0c, 0x0a, 0x53, 0x6f, 0x6d, 0x65, 0x2d, 0x53, 0x74, 0x61, 0x74, 0x65,
  0x31, 0x21, 0x30, 0x1f, 0x06, 0x03, 0x55, 0x04, 0x0a, 0x0c, 0x18, 0x49,
  0x6e, 0x74, 0x65, 0x72, 0x6e, 0x65, 0x74, 0x20, 0x57, 0x69, 0x64, 0x67,
  0x69, 0x74, 0x73, 0x20, 0x50, 0x74, 0x79, 0x20, 0x4c, 0x74, 0x64, 0x30,
  0x5c, 0x30, 0x0d, 0x06, 0x09, 0x2a, 0x86, 0x48, 0x86, 0xf7, 0x0d, 0x01,
  0x01, 0x01, 0x05, 0x00, 0x03, 0x4b, 0x00, 0x30, 0x48, 0x02, 0x41, 0x00,
  0xc7, 0xc1, 0x4d, 0xd5, 0xdc, 0x3a, 0x2e, 0x1f, 0x42, 0x30, 0x3d, 0x21,
  0x1e, 0xa2, 0x1f, 0x60, 0xcb, 0x71, 0x11, 0x53, 0xb0, 0x75, 0xa0, 0x62,
  0xfe, 0x5e, 0x0a, 0xde, 0xb0, 0x0f, 0x48, 0x97, 0x5e, 0x42, 0xa7, 0x3a,
  0xd1, 0xca, 0x4c, 0xe3, 0xdb, 0x5f, 0x31, 0xc2, 0x99, 0x08, 0x89, 0xcd,
  0x6d, 0x20, 0xaa, 0x75, 0xe6, 0x2b, 0x98, 0xd2, 0xf3, 0x7b, 0x4b, 0xe5,
  0x9b, 0xfe, 0xe2, 0x6d, 0x02, 0x03, 0x01, 0x00, 0x01, 0xa3, 0x50, 0x30,
  0x4e, 0x30, 0x1d, 0x06, 0x03, 0x55, 0x1d, 0x0e, 0x04, 0x16, 0x04, 0x14,
  0xbd, 0x85, 0x6b, 0xdd, 0x84, 0xd1, 0x54, 0x2e, 0xad, 0xb4, 0x5e, 0xdd,
  0x24, 0x7e, 0x16, 0x9c, 0x84, 0x1e, 0x19, 0xf0, 0x30, 0x1f, 0x06, 0x03,
  0x55, 0x1d, 0x23, 0x04, 0x18, 0x30, 0x16, 0x80, 0x14, 0xbd, 0x85, 0x6b,
  0xdd, 0x84, 0xd1, 0x54, 0x2e, 0xad, 0xb4, 0x5e, 0xdd, 0x24, 0x7e, 0x16,
  0x9c, 0x84, 0x1e, 0x19, 0xf0, 0x30, 0x0c, 0x06, 0x03, 0x55, 0x1d, 0x13,
  0x04, 0x05, 0x30, 0x03, 0x01, 0x01, 0xff, 0x30, 0x0d, 0x06, 0x09, 0x2a,
  0x86, 0x48, 0x86, 0xf7, 0x0d, 0x01, 0x01, 0x05, 0x05, 0x00, 0x03, 0x41,
  0x00, 0x82, 0x95, 0xa7, 0x08, 0x6c, 0xbd, 0x49, 0xe6, 0x1e, 0xc1, 0xd9,
  0x58, 0x54, 0x11, 0x11, 0x84, 0x77, 0x1e, 0xad, 0xe9, 0x73, 0x69, 0x1c,
  0x5c, 0xaa, 0x26, 0x3e, 0x5f, 0x1d, 0x89, 0x20, 0xc3, 0x90, 0xa4, 0x67,
  0xfa, 0x26, 0x20, 0xd7, 0x1f, 0xae, 0x42, 0x89, 0x30, 0x61, 0x43, 0x8a,
  0x8c, 0xbe, 0xd4, 0x32, 0xf7, 0x96, 0x71, 0x2a, 0xcd, 0xeb, 0x26, 0xf6,
  0xdb, 0x54, 0x95, 0xca, 0x5a
]);

// based on a private key different than privateKeyPkcs8
var cert2 = new Uint8Array([
  0x30, 0x82, 0x01, 0xd5, 0x30, 0x82, 0x01, 0x7f, 0xa0, 0x03, 0x02, 0x01,
  0x02, 0x02, 0x09, 0x00, 0x9e, 0x11, 0x7e, 0xff, 0x43, 0x84, 0xd4, 0xe6,
  0x30, 0x0d, 0x06, 0x09, 0x2a, 0x86, 0x48, 0x86, 0xf7, 0x0d, 0x01, 0x01,
  0x05, 0x05, 0x00, 0x30, 0x45, 0x31, 0x0b, 0x30, 0x09, 0x06, 0x03, 0x55,
  0x04, 0x06, 0x13, 0x02, 0x41, 0x55, 0x31, 0x13, 0x30, 0x11, 0x06, 0x03,
  0x55, 0x04, 0x08, 0x0c, 0x0a, 0x53, 0x6f, 0x6d, 0x65, 0x2d, 0x53, 0x74,
  0x61, 0x74, 0x65, 0x31, 0x21, 0x30, 0x1f, 0x06, 0x03, 0x55, 0x04, 0x0a,
  0x0c, 0x18, 0x49, 0x6e, 0x74, 0x65, 0x72, 0x6e, 0x65, 0x74, 0x20, 0x57,
  0x69, 0x64, 0x67, 0x69, 0x74, 0x73, 0x20, 0x50, 0x74, 0x79, 0x20, 0x4c,
  0x74, 0x64, 0x30, 0x20, 0x17, 0x0d, 0x31, 0x34, 0x30, 0x34, 0x30, 0x37,
  0x31, 0x35, 0x35, 0x30, 0x30, 0x38, 0x5a, 0x18, 0x0f, 0x32, 0x31, 0x31,
  0x34, 0x30, 0x33, 0x31, 0x34, 0x31, 0x35, 0x35, 0x30, 0x30, 0x38, 0x5a,
  0x30, 0x45, 0x31, 0x0b, 0x30, 0x09, 0x06, 0x03, 0x55, 0x04, 0x06, 0x13,
  0x02, 0x41, 0x55, 0x31, 0x13, 0x30, 0x11, 0x06, 0x03, 0x55, 0x04, 0x08,
  0x0c, 0x0a, 0x53, 0x6f, 0x6d, 0x65, 0x2d, 0x53, 0x74, 0x61, 0x74, 0x65,
  0x31, 0x21, 0x30, 0x1f, 0x06, 0x03, 0x55, 0x04, 0x0a, 0x0c, 0x18, 0x49,
  0x6e, 0x74, 0x65, 0x72, 0x6e, 0x65, 0x74, 0x20, 0x57, 0x69, 0x64, 0x67,
  0x69, 0x74, 0x73, 0x20, 0x50, 0x74, 0x79, 0x20, 0x4c, 0x74, 0x64, 0x30,
  0x5c, 0x30, 0x0d, 0x06, 0x09, 0x2a, 0x86, 0x48, 0x86, 0xf7, 0x0d, 0x01,
  0x01, 0x01, 0x05, 0x00, 0x03, 0x4b, 0x00, 0x30, 0x48, 0x02, 0x41, 0x00,
  0xac, 0x6c, 0x72, 0x46, 0xa2, 0xde, 0x88, 0x30, 0x54, 0x06, 0xad, 0xc7,
  0x2d, 0x64, 0x6e, 0xf6, 0x0f, 0x72, 0x3e, 0x92, 0x31, 0xcc, 0x0b, 0xa0,
  0x18, 0x20, 0xb0, 0xdb, 0x86, 0xab, 0x11, 0xc6, 0xa5, 0x78, 0xea, 0x64,
  0xe8, 0xeb, 0xa5, 0xb3, 0x78, 0x5d, 0xbb, 0x10, 0x57, 0xe6, 0x12, 0x23,
  0x89, 0x92, 0x1d, 0xa0, 0xe5, 0x1e, 0xd1, 0xc9, 0x0e, 0x62, 0xcb, 0xc9,
  0xaf, 0xde, 0x4e, 0x83, 0x02, 0x03, 0x01, 0x00, 0x01, 0xa3, 0x50, 0x30,
  0x4e, 0x30, 0x1d, 0x06, 0x03, 0x55, 0x1d, 0x0e, 0x04, 0x16, 0x04, 0x14,
  0x75, 0x6c, 0x61, 0xfb, 0xb0, 0x6e, 0x37, 0x32, 0x41, 0x62, 0x3b, 0x55,
  0xbd, 0x5f, 0x6b, 0xe0, 0xdb, 0xb9, 0xc7, 0xec, 0x30, 0x1f, 0x06, 0x03,
  0x55, 0x1d, 0x23, 0x04, 0x18, 0x30, 0x16, 0x80, 0x14, 0x75, 0x6c, 0x61,
  0xfb, 0xb0, 0x6e, 0x37, 0x32, 0x41, 0x62, 0x3b, 0x55, 0xbd, 0x5f, 0x6b,
  0xe0, 0xdb, 0xb9, 0xc7, 0xec, 0x30, 0x0c, 0x06, 0x03, 0x55, 0x1d, 0x13,
  0x04, 0x05, 0x30, 0x03, 0x01, 0x01, 0xff, 0x30, 0x0d, 0x06, 0x09, 0x2a,
  0x86, 0x48, 0x86, 0xf7, 0x0d, 0x01, 0x01, 0x05, 0x05, 0x00, 0x03, 0x41,
  0x00, 0xa5, 0xe8, 0x9d, 0x3d, 0xc4, 0x1a, 0x6e, 0xd2, 0x92, 0x42, 0x37,
  0xb9, 0x3a, 0xb3, 0x8e, 0x2f, 0x55, 0xb5, 0xf2, 0xe4, 0x6e, 0x39, 0x0d,
  0xa8, 0xba, 0x10, 0x43, 0x57, 0xdd, 0x4e, 0x4e, 0x52, 0xc6, 0xbe, 0x07,
  0xdb, 0x83, 0x05, 0x97, 0x97, 0xc1, 0x7b, 0xd5, 0x5c, 0x50, 0x64, 0x0f,
  0x96, 0xff, 0x3d, 0x83, 0x37, 0x8f, 0x3a, 0x85, 0x08, 0x62, 0x5c, 0xb1,
  0x2f, 0x68, 0xb2, 0x4a, 0x4a
]);

/**
 * Runs an array of asynchronous functions [f1, f2, ...] of the form
 *   function(callback) {}
 * by chaining, i.e. f1(f2(...)). Additionally, each callback is wrapped with
 * callbackPass.
 */
function runAsyncSequence(funcs) {
  if (funcs.length == 0)
    return;
  function go(i) {
    var current = funcs[i];
    console.log('#' + (i + 1) + ' of ' + funcs.length);
    if (i == funcs.length - 1) {
      current(callbackPass());
    } else {
      current(callbackPass(go.bind(undefined, i + 1)));
    }
  };
  go(0);
}

// Some array comparison. Note: not lexicographical!
function compareArrays(array1, array2) {
  if (array1.length < array2.length)
    return -1;
  if (array1.length > array2.length)
    return 1;
  for (var i = 0; i < array1.length; i++) {
    if (array1[i] < array2[i])
      return -1;
    if (array1[i] > array2[i])
      return 1;
  }
  return 0;
}

/**
 * @param {ArrayBufferView[]} certs
 * @return {ArrayBufferView[]} |certs| sorted in some order.
 */
function sortCerts(certs) {
  return certs.sort(compareArrays);
}

/**
 * Checks whether the certificates currently stored in |token| match
 * |expectedCerts| by comparing to the result of platformKeys.getCertificates.
 * The order of |expectedCerts| is ignored. Afterwards calls |callback|.
 */
function assertCertsStored(token, expectedCerts, callback) {
  chrome.enterprise.platformKeys.getCertificates(
      token.id,
      callbackPass(function(actualCerts) {
        assertEq(expectedCerts.length,
                 actualCerts.length,
                 'Number of stored certs not as expected');
        if (expectedCerts.length == actualCerts.length) {
          actualCerts = actualCerts.map(
              function(buffer) { return new Uint8Array(buffer); });
          actualCerts = sortCerts(actualCerts);
          expectedCerts = sortCerts(expectedCerts);
          for (var i = 0; i < expectedCerts.length; i++) {
            assertTrue(compareArrays(expectedCerts[i], actualCerts[i]) == 0,
                       'Certs at index ' + i + ' differ');
          }
        }
        if (callback)
          callback();
      }));
}

/**
 * Fetches all available tokens using platformKeys.getTokens and calls
 * |callback| with the user token if available or with undefined otherwise.
 */
function getUserToken(callback) {
  chrome.enterprise.platformKeys.getTokens(function(tokens) {
    for (var i = 0; i < tokens.length; i++) {
      if (tokens[i].id == 'user') {
        callback(tokens[i]);
        return;
      }
    }
    callback(undefined);
  });
}

/**
 * Runs preparations before the actual tests. Calls |callback| with |userToken|.
 */
function beforeTests(callback) {
  assertTrue(!!chrome.enterprise, "No enterprise namespace.");
  assertTrue(!!chrome.enterprise.platformKeys, "No platformKeys namespace.");
  assertTrue(!!chrome.enterprise.platformKeys.getTokens,
             "No getTokens function.");
  assertTrue(!!chrome.enterprise.platformKeys.importCertificate,
             "No importCertificate function.");
  assertTrue(!!chrome.enterprise.platformKeys.removeCertificate,
             "No removeCertificate function.");

  getUserToken(function(userToken) {
    if (!userToken)
      fail('no user token');
    if (userToken.id != 'user')
      fail('token is not named "user".');

    callback(userToken);
  });
}

function checkAlgorithmIsCopiedOnRead(key) {
  var algorithm = key.algorithm;
  var originalAlgorithm = {
    name: algorithm.name,
    modulusLength: algorithm.modulusLength,
    publicExponent: algorithm.publicExponent,
    hash: {name: algorithm.hash.name}
  };
  var originalModulusLength = algorithm.modulusLength;
  algorithm.hash.name = null;
  algorithm.hash = null;
  algorithm.name = null;
  algorithm.modulusLength = null;
  algorithm.publicExponent = null;
  assertEq(originalAlgorithm, key.algorithm);
}

function checkPropertyIsReadOnly(object, key) {
  var original = object[key];
  try {
    object[key] = {};
    fail('Expected the property to be read-only and an exception to be thrown');
  } catch (error) {
    assertEq(original, object[key]);
  }
}

function checkKeyPairCommonFormat(keyPair) {
  checkPropertyIsReadOnly(keyPair, 'privateKey');
  var privateKey = keyPair.privateKey;
  assertEq('private', privateKey.type);
  assertEq(false, privateKey.extractable);
  checkPropertyIsReadOnly(privateKey, 'algorithm');
  checkAlgorithmIsCopiedOnRead(privateKey);

  checkPropertyIsReadOnly(keyPair, 'publicKey');
  var publicKey = keyPair.publicKey;
  assertEq('public', publicKey.type);
  assertEq(true, publicKey.extractable);
  checkPropertyIsReadOnly(publicKey, 'algorithm');
  checkAlgorithmIsCopiedOnRead(publicKey);
}

// Generates a key with the |algorithm| parameters. Signs |data| using the new
// key and verifies the signature using WebCrypto. Returns the generated key to
// |callback| for further operations.
// Also freezes |algorithm|.
function generateKeyAndVerify(token, algorithm, data, callback) {
  // Ensure that this algorithm object is not modified, so that later
  // comparisons really do the right thing.
  Object.freeze(algorithm.hash);
  Object.freeze(algorithm);

  var cachedSignature;
  var cachedKeyPair;
  var cachedSpki;
  token.subtleCrypto.generateKey(algorithm, false, ["sign"])
      .then(callbackPass(
                function(keyPair) {
                  assertTrue(!!keyPair, "No key pair.");
                  cachedKeyPair = keyPair;
                  return token.subtleCrypto.exportKey('spki',
                                                      keyPair.publicKey);
                }),
            function(error) { fail("GenerateKey failed: " + error); })
      .then(callbackPass(
                function(publicKeySpki) {
                  // Ensure that the returned key pair has the expected format.
                  // Some parameter independent checks:
                  checkKeyPairCommonFormat(cachedKeyPair);

                  // Checks depending on the generateKey arguments:
                  var privateKey = cachedKeyPair.privateKey;
                  assertEq(['sign'], privateKey.usages);
                  assertEq(algorithm, privateKey.algorithm);

                  var publicKey = cachedKeyPair.publicKey;
                  assertEq([], publicKey.usages);
                  assertEq(algorithm, publicKey.algorithm);

                  cachedSpki = publicKeySpki;
                  var signParams = {name: 'RSASSA-PKCS1-v1_5'};
                  return token.subtleCrypto.sign(signParams, privateKey, data);
                }),
            function(error) { fail("Export failed: " + error); })
      .then(callbackPass(
                function(signature) {
                  var importParams = {
                    name: algorithm.name,
                    // RsaHashedImportParams
                    hash: {
                      name: algorithm.hash.name,
                    }
                  };
                  assertTrue(!!signature, "No signature.");
                  assertTrue(signature.length != 0, "Signature is empty.");
                  cachedSignature = signature;
                  return window.crypto.subtle.importKey(
                      "spki", cachedSpki, importParams, false, ["verify"]);
                }),
            function(error) { fail("Sign failed: " + error); })
      .then(callbackPass(
                function(webCryptoPublicKey) {
                  assertTrue(!!webCryptoPublicKey);
                  assertEq(algorithm.modulusLength,
                           webCryptoPublicKey.algorithm.modulusLength);
                  assertEq(algorithm.publicExponent,
                           webCryptoPublicKey.algorithm.publicExponent);
                  return window.crypto.subtle.verify(
                      algorithm, webCryptoPublicKey, cachedSignature, data);
                }),
            function(error) { fail("Import failed: " + error); })
      .then(callbackPass(function(success) {
    assertEq(true, success, "Signature invalid.");
    callback(cachedKeyPair);
  }), function(error) { fail("Verification failed: " + error); });
}

function runTests(userToken) {
  chrome.test.runTests([
    function hasSubtleCryptoMethods() {
      assertTrue(!!userToken.subtleCrypto.generateKey,
                 "user token has no generateKey method");
      assertTrue(!!userToken.subtleCrypto.sign,
                 "user token has no sign method");
      assertTrue(!!userToken.subtleCrypto.exportKey,
                 "user token has no exportKey method");
      succeed();
    },
    function initiallyNoCerts() { assertCertsStored(userToken, []); },

    // Generates a key and signs some data with it. Verifies the signature using
    // WebCrypto. Verifies also that a second sign operation fails.
    function generateKeyAndSign() {
      var algorithm = {
        name: "RSASSA-PKCS1-v1_5",
        // RsaHashedKeyGenParams
        modulusLength: 512,
        // Equivalent to 65537
        publicExponent: new Uint8Array([0x01, 0x00, 0x01]),
        hash: {
          name: "SHA-1",
        }
      };

      // Some random data to sign.
      var data = new Uint8Array([0, 1, 2, 3, 4, 5, 1, 2, 3, 4, 5, 6]);
      generateKeyAndVerify(userToken,
                           algorithm,
                           data,
                           callbackPass(function(keyPair) {
        // Try to sign data with the same key a second time, which
        // must fail.
        var signParams = {name: 'RSASSA-PKCS1-v1_5'};
        userToken.subtleCrypto.sign(signParams, keyPair.privateKey, data).then(
            function(signature) {
              fail("Second sign call was expected to fail.");
            },
            callbackPass(function(error) {
          assertTrue(error instanceof Error);
          assertEq('The operation failed for an operation-specific reason',
                   error.message);
        }));
      }));
    },

    // Generates a key and signs some data with other parameters. Verifies the
    // signature using WebCrypto.
    function generateKeyAndSignOtherParameters() {
      var algorithm = {
        name: "RSASSA-PKCS1-v1_5",
        // RsaHashedKeyGenParams
        modulusLength: 1024,
        // Equivalent to 65537
        publicExponent: new Uint8Array([0x01, 0x00, 0x01]),
        hash: {
          name: "SHA-512",
        }
      };

      // Some random data to sign.
      var data = new Uint8Array([5, 1, 2, 3, 4, 5, 1, 2, 3, 4, 5, 0, 0, 254]);
      generateKeyAndVerify(userToken, algorithm, data, callbackPass());
    },

    // Imports and removes certificates for privateKeyPkcs8, which was imported
    // by on C++'s side.
    // Note: After this test, privateKeyPkcs8 is not stored anymore!
    function importAndRemoveCerts() {
      runAsyncSequence([
        chrome.enterprise.platformKeys.importCertificate.bind(
            null, userToken.id, cert1a.buffer),
        assertCertsStored.bind(null, userToken, [cert1a]),
        // Importing the same cert again shouldn't change anything.
        chrome.enterprise.platformKeys.importCertificate.bind(
            null, userToken.id, cert1a.buffer),
        assertCertsStored.bind(null, userToken, [cert1a]),
        // Importing another certificate should succeed.
        chrome.enterprise.platformKeys.importCertificate.bind(
            null, userToken.id, cert1b.buffer),
        assertCertsStored.bind(null, userToken, [cert1a, cert1b]),
        chrome.enterprise.platformKeys.removeCertificate.bind(
            null, userToken.id, cert1a.buffer),
        assertCertsStored.bind(null, userToken, [cert1b]),
        chrome.enterprise.platformKeys.removeCertificate.bind(
            null, userToken.id, cert1b.buffer),
        assertCertsStored.bind(null, userToken, [])
      ]);
    },

    // Call generate key with invalid algorithm parameter, missing
    // modulusLength.
    function algorithmParameterMissingModulusLength() {
      var algorithm = {
        name: "RSASSA-PKCS1-v1_5",
        // Equivalent to 65537
        publicExponent: new Uint8Array([0x01, 0x00, 0x01]),
        hash: {
          name: "SHA-1",
        }
      };
      userToken.subtleCrypto.generateKey(algorithm, false, ['sign']).then(
          function(keyPair) { fail('generateKey was expected to fail'); },
          callbackPass(function(error) {
      assertTrue(error instanceof Error);
      assertEq('A required parameter was missing or out-of-range',
               error.message);
      }));
    },

    // Call generate key with invalid algorithm parameter, missing hash.
    function algorithmParameterMissingHash() {
      var algorithm = {
        name: 'RSASSA-PKCS1-v1_5',
        modulusLength: 512,
        // Equivalent to 65537
        publicExponent: new Uint8Array([0x01, 0x00, 0x01]),
      };
      userToken.subtleCrypto.generateKey(algorithm, false, ['sign']).then(
          function(keyPair) { fail('generateKey was expected to fail'); },
          callbackPass(function(error) {
      assertEq(
          new Error('Error: A required parameter was missing our out-of-range'),
          error);
      }));
    },

    // Call generate key with invalid algorithm parameter, unsupported public
    // exponent.
    function algorithmParameterUnsupportedPublicExponent() {
      var algorithm = {
        name: 'RSASSA-PKCS1-v1_5',
        modulusLength: 512,
        // Different from 65537.
        publicExponent: new Uint8Array([0x01, 0x01]),
      };
      userToken.subtleCrypto.generateKey(algorithm, false, ['sign']).then(
          function(keyPair) { fail('generateKey was expected to fail'); },
          callbackPass(function(error) {
      assertTrue(error instanceof Error);
      assertEq('A required parameter was missing or out-of-range',
               error.message);
      }));
    },

    // Imports a certificate for which now private key was imported/generated
    // before.
    function missingPrivateKey() {
      chrome.enterprise.platformKeys.importCertificate(
          userToken.id, cert2.buffer, callbackFail('Key not found.'));
    },
    function importInvalidCert() {
      var invalidCert = new ArrayBuffer(16);
      chrome.enterprise.platformKeys.importCertificate(
          userToken.id,
          invalidCert,
          callbackFail('Certificate is not a valid X.509 certificate.'));
    },
    function removeUnknownCert() {
      chrome.enterprise.platformKeys.removeCertificate(
          userToken.id,
          cert2.buffer,
          callbackFail('Certificate could not be found.'));
    },
    function removeInvalidCert() {
      var invalidCert = new ArrayBuffer(16);
      chrome.enterprise.platformKeys.removeCertificate(
          userToken.id,
          invalidCert,
          callbackFail('Certificate is not a valid X.509 certificate.'));
    },
    function getCertsInvalidToken() {
      chrome.enterprise.platformKeys.getCertificates(
          'invalid token id', callbackFail('The token is not valid.'));
    }
  ]);
}

beforeTests(runTests);
