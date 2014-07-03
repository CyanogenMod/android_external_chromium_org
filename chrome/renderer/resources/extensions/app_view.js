// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

var DocumentNatives = requireNative('document_natives');
var GuestViewInternal =
    require('binding').Binding.create('guestViewInternal').generate();
var IdGenerator = requireNative('id_generator');

function AppViewInternal(appviewNode) {
  privates(appviewNode).internal = this;
  this.appviewNode = appviewNode;

  this.browserPluginNode = this.createBrowserPluginNode();
  var shadowRoot = this.appviewNode.createShadowRoot();
  shadowRoot.appendChild(this.browserPluginNode);
  this.viewInstanceId = IdGenerator.GetNextId();
}

AppViewInternal.prototype.createBrowserPluginNode = function() {
  // We create BrowserPlugin as a custom element in order to observe changes
  // to attributes synchronously.
  var browserPluginNode = new AppViewInternal.BrowserPlugin();
  privates(browserPluginNode).internal = this;
  return browserPluginNode;
};

AppViewInternal.prototype.connect = function(src, callback) {
  var params = {
  };
  var self = this;
  GuestViewInternal.createGuest(
    'appview',
    params,
    function(instanceId) {
      self.attachWindow(instanceId, src);
      if (callback) {
        callback();
      }
    }
  );
};

AppViewInternal.prototype.attachWindow = function(instanceId, src) {
  this.instanceId = instanceId;
  var params = {
    'instanceId': this.viewInstanceId,
    'src': src
  };
  return this.browserPluginNode['-internal-attach'](instanceId, params);
};

function registerBrowserPluginElement() {
  var proto = Object.create(HTMLObjectElement.prototype);

  proto.createdCallback = function() {
    this.setAttribute('type', 'application/browser-plugin');
    this.style.width = '100%';
    this.style.height = '100%';
  };

  proto.attachedCallback = function() {
    // Load the plugin immediately.
    var unused = this.nonExistentAttribute;
  };

  AppViewInternal.BrowserPlugin =
      DocumentNatives.RegisterElement('appplugin', {extends: 'object',
                                                    prototype: proto});

  delete proto.createdCallback;
  delete proto.attachedCallback;
  delete proto.detachedCallback;
  delete proto.attributeChangedCallback;
}

function registerAppViewElement() {
  var proto = Object.create(HTMLElement.prototype);

  proto.createdCallback = function() {
    new AppViewInternal(this);
  };

  proto.connect = function() {
    var internal = privates(this).internal;
    $Function.apply(internal.connect, internal, arguments);
  }
  window.AppView =
      DocumentNatives.RegisterElement('appview', {prototype: proto});

  // Delete the callbacks so developers cannot call them and produce unexpected
  // behavior.
  delete proto.createdCallback;
  delete proto.attachedCallback;
  delete proto.detachedCallback;
  delete proto.attributeChangedCallback;
}

var useCapture = true;
window.addEventListener('readystatechange', function listener(event) {
  if (document.readyState == 'loading')
    return;

  registerBrowserPluginElement();
  registerAppViewElement();
  window.removeEventListener(event.type, listener, useCapture);
}, useCapture);
