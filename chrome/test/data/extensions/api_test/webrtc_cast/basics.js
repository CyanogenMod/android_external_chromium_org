// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

var sendTransport = chrome.webrtc.castSendTransport;
var tabCapture = chrome.tabCapture;
var udpTransport = chrome.webrtc.castUdpTransport;
var createSession = chrome.cast.streaming.session.create;

chrome.test.runTests([
  function sendTransportStart() {
    tabCapture.capture({audio: true, video: true}, function(stream) {
      console.log("Got MediaStream.");
      chrome.test.assertTrue(!!stream);
      createSession(stream.getAudioTracks()[0],
                    stream.getVideoTracks()[0],
            function(stream, audioId, videoId, udpId) {
        var audioParams = sendTransport.getCaps(audioId);
        var videoParams = sendTransport.getCaps(videoId);
        sendTransport.start(audioId, audioParams);
        sendTransport.start(videoId, videoParams);
        sendTransport.stop(audioId);
        sendTransport.stop(videoId);
        sendTransport.destroy(audioId);
        sendTransport.destroy(videoId);
        udpTransport.destroy(udpId);
        stream.stop();
        chrome.test.assertEq(audioParams.payloads[0].codecName, "OPUS");
        chrome.test.assertEq(videoParams.payloads[0].codecName, "VP8");
        chrome.test.succeed();
      }.bind(null, stream));
    });
  },
]);
