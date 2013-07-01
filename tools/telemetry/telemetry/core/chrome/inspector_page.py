# Copyright (c) 2012 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
import json
import logging

from telemetry.core import util

class InspectorPage(object):
  def __init__(self, inspector_backend):
    self._inspector_backend = inspector_backend
    self._inspector_backend.RegisterDomain(
        'Page',
        self._OnNotification,
        self._OnClose)

    # Turn on notifications. We need them to get the Page.frameNavigated event.
    request = {
        'method': 'Page.enable'
        }
    res = self._inspector_backend.SyncRequest(request)
    assert len(res['result'].keys()) == 0

    self._navigation_pending = False
    self._script_to_evaluate_on_commit = None

  def _OnNotification(self, msg):
    logging.debug('Notification: %s', json.dumps(msg, indent=2))
    if msg['method'] == 'Page.frameNavigated' and self._navigation_pending:
      url = msg['params']['frame']['url']
      if (not url == 'chrome://newtab/' and not url == 'about:blank'
          and not 'parentId' in msg['params']['frame']):
        # Marks the navigation as complete and unblocks the
        # PerformActionAndWaitForNavigate call.
        self._navigation_pending = False

  def _OnClose(self):
    pass

  def _SetScriptToEvaluateOnCommit(self, source):
    existing_source = (self._script_to_evaluate_on_commit and
                       self._script_to_evaluate_on_commit['source'])
    if source == existing_source:
      return
    if existing_source:
      request = {
          'method': 'Page.removeScriptToEvaluateOnLoad',
          'params': {
              'identifier': self._script_to_evaluate_on_commit['id'],
              }
          }
      self._inspector_backend.SyncRequest(request)
      self._script_to_evaluate_on_commit = None
    if source:
      request = {
          'method': 'Page.addScriptToEvaluateOnLoad',
          'params': {
              'scriptSource': source,
              }
          }
      res = self._inspector_backend.SyncRequest(request)
      self._script_to_evaluate_on_commit = {
          'id': res['result']['identifier'],
          'source': source
          }

  def PerformActionAndWaitForNavigate(self, action_function, timeout=60):
    """Executes action_function, and waits for the navigation to complete.

    action_function is expect to result in a navigation. This function returns
    when the navigation is complete or when the timeout has been exceeded.
    """
    self._navigation_pending = True
    action_function()

    def IsNavigationDone(time_left):
      self._inspector_backend.DispatchNotifications(time_left)
      return not self._navigation_pending
    util.WaitFor(IsNavigationDone, timeout, pass_time_left_to_func=True)

  def Navigate(self, url, script_to_evaluate_on_commit=None, timeout=60):
    """Navigates to |url|.

    If |script_to_evaluate_on_commit| is given, the script source string will be
    evaluated when the navigation is committed. This is after the context of
    the page exists, but before any script on the page itself has executed.
    """

    def DoNavigate():
      self._SetScriptToEvaluateOnCommit(script_to_evaluate_on_commit)
      # Navigate the page. However, there seems to be a bug in chrome devtools
      # protocol where the request id for this event gets held on the browser
      # side pretty much indefinitely.
      #
      # So, instead of waiting for the event to actually complete, wait for the
      # Page.frameNavigated event.
      request = {
          'method': 'Page.navigate',
          'params': {
              'url': url,
              }
          }
      self._inspector_backend.SendAndIgnoreResponse(request)
    self.PerformActionAndWaitForNavigate(DoNavigate, timeout)

  def GetCookieByName(self, name, timeout=60):
    """Returns the value of the cookie by the given |name|."""
    request = {
        'method': 'Page.getCookies'
        }
    res = self._inspector_backend.SyncRequest(request, timeout)
    cookies = res['result']['cookies']
    for cookie in cookies:
      if cookie['name'] == name:
        return cookie['value']
    return None

  def CollectGarbage(self, timeout=60):
    request = {
        'method': 'HeapProfiler.CollectGarbage'
        }
    self._inspector_backend.SyncRequest(request, timeout)
