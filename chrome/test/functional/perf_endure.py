#!/usr/bin/env python
# Copyright (c) 2012 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

"""Performance tests for Chrome Endure (long-running perf tests on Chrome).

This module accepts the following environment variable inputs:
  TEST_LENGTH: The number of seconds in which to run each test.
  PERF_STATS_INTERVAL: The number of seconds to wait in-between each sampling
      of performance/memory statistics.

The following variables are related to the Deep Memory Profiler.
  DEEP_MEMORY_PROFILE: Enable the Deep Memory Profiler if it's set to 'True'.
  DEEP_MEMORY_PROFILE_SAVE: Don't clean up dump files if it's set to 'True'.
  DEEP_MEMORY_PROFILE_UPLOAD: Upload dumped files if the variable has a Google
      Storage bucket like gs://chromium-endure/.  The 'gsutil' script in $PATH
      is used by default, or set a variable 'GSUTIL' to specify a path to the
      'gsutil' script.  A variable 'REVISION' (or 'BUILDBOT_GOT_REVISION') is
      used as a subdirectory in the destination if it is set.
  GSUTIL: A path to the 'gsutil' script.  Not mandatory.
  REVISION: A string that represents the revision or some build configuration.
      Not mandatory.
  BUILDBOT_GOT_REVISION: Similar to 'REVISION', but checked only if 'REVISION'
      is not specified.  Not mandatory.
"""

from datetime import datetime
import json
import logging
import os
import re
import subprocess
import tempfile
import time

import perf
import pyauto_functional  # Must be imported before pyauto.
import pyauto
import pyauto_errors
import pyauto_utils
import remote_inspector_client
import selenium.common.exceptions
from selenium.webdriver.support.ui import WebDriverWait


class NotSupportedEnvironmentError(RuntimeError):
  """Represent an error raised since the environment (OS) is not supported."""
  pass


class DeepMemoryProfiler(object):
  """Controls Deep Memory Profiler (dmprof) for endurance tests."""
  DEEP_MEMORY_PROFILE = False
  DEEP_MEMORY_PROFILE_SAVE = False
  DEEP_MEMORY_PROFILE_UPLOAD = ''

  _WORKDIR_PATTERN = re.compile('endure\.[0-9]+\.[0-9]+\.[A-Za-z0-9]+')
  _SAVED_WORKDIRS = 8

  _DMPROF_DIR_PATH = os.path.abspath(os.path.join(
      os.path.dirname(__file__), os.pardir, os.pardir, os.pardir,
      'tools', 'deep_memory_profiler'))
  _DMPROF_SCRIPT_PATH = os.path.join(_DMPROF_DIR_PATH, 'dmprof')
  _POLICIES = ['l0', 'l1', 'l2', 't0']

  def __init__(self):
    self._enabled = self.GetEnvironmentVariable(
        'DEEP_MEMORY_PROFILE', bool, self.DEEP_MEMORY_PROFILE)
    self._save = self.GetEnvironmentVariable(
        'DEEP_MEMORY_PROFILE_SAVE', bool, self.DEEP_MEMORY_PROFILE_SAVE)
    self._upload = self.GetEnvironmentVariable(
        'DEEP_MEMORY_PROFILE_UPLOAD', str, self.DEEP_MEMORY_PROFILE_UPLOAD)
    if self._upload and not self._upload.endswith('/'):
      self._upload += '/'

    self._revision = ''
    self._gsutil = ''
    self._json_file = None
    self._last_json_filename = ''
    self._proc = None
    self._last_time = {}
    for policy in self._POLICIES:
      self._last_time[policy] = -1.0

  def __nonzero__(self):
    return self._enabled

  @staticmethod
  def GetEnvironmentVariable(env_name, converter, default):
    """Returns a converted environment variable for Deep Memory Profiler.

    Args:
      env_name: A string name of an environment variable.
      converter: A function taking a string to convert an environment variable.
      default: A value used if the environment variable is not specified.

    Returns:
      A value converted from the environment variable with 'converter'.
    """
    return converter(os.environ.get(env_name, default))

  def SetUp(self, is_linux, revision, gsutil):
    """Sets up Deep Memory Profiler settings for a Chrome process.

    It sets environment variables and makes a working directory.
    """
    if not self._enabled:
      return

    if not is_linux:
      raise NotSupportedEnvironmentError(
          'Deep Memory Profiler is not supported in this environment (OS).')

    self._revision = revision
    self._gsutil = gsutil

    # Remove old dumped files with keeping latest _SAVED_WORKDIRS workdirs.
    # It keeps the latest workdirs not to miss data by failure in uploading
    # and other operations.  Dumped files are no longer available if they are
    # removed.  Re-execution doesn't generate the same files.
    tempdir = tempfile.gettempdir()
    saved_workdirs = 0
    for filename in sorted(os.listdir(tempdir), reverse=True):
      if self._WORKDIR_PATTERN.match(filename):
        saved_workdirs += 1
        if saved_workdirs > self._SAVED_WORKDIRS:
          fullpath = os.path.abspath(os.path.join(tempdir, filename))
          logging.info('Removing an old workdir: %s' % fullpath)
          pyauto_utils.RemovePath(fullpath)

    dir_prefix = 'endure.%s.' % datetime.today().strftime('%Y%m%d.%H%M%S')
    self._workdir = tempfile.mkdtemp(prefix=dir_prefix, dir=tempdir)
    os.environ['HEAPPROFILE'] = os.path.join(self._workdir, 'endure')
    os.environ['HEAP_PROFILE_MMAP'] = '1'
    os.environ['DEEP_HEAP_PROFILE'] = '1'

  def TearDown(self):
    """Tear down Deep Memory Profiler settings for the Chrome process.

    It removes the environment variables and the temporary directory.
    Call it after Chrome finishes.  Chrome may dump last files at the end.
    """
    if not self._enabled:
      return

    del os.environ['DEEP_HEAP_PROFILE']
    del os.environ['HEAP_PROFILE_MMAP']
    del os.environ['HEAPPROFILE']
    if not self._save and self._workdir:
      pyauto_utils.RemovePath(self._workdir)

  def LogFirstMessage(self):
    """Logs first messages."""
    if not self._enabled:
      return

    logging.info('Running with the Deep Memory Profiler.')
    if self._save:
      logging.info('  Dumped files won\'t be cleaned.')
    else:
      logging.info('  Dumped files will be cleaned up after every test.')

  def StartProfiler(self, proc_info, is_last, webapp_name, test_description):
    """Starts Deep Memory Profiler in background."""
    if not self._enabled:
      return

    logging.info('  Profiling with the Deep Memory Profiler...')

    # Wait for a running dmprof process for last _GetPerformanceStat call to
    # cover last dump files.
    if is_last:
      logging.info('    Waiting for the last dmprof.')
      self._WaitForDeepMemoryProfiler()

    if self._proc and self._proc.poll() is None:
      logging.info('    Last dmprof is still running.')
    else:
      if self._json_file:
        self._last_json_filename = self._json_file.name
        self._json_file.close()
        self._json_file = None
      first_dump = ''
      last_dump = ''
      for filename in sorted(os.listdir(self._workdir)):
        if re.match('^endure.%05d.\d+.heap$' % proc_info['tab_pid'],
                    filename):
          logging.info('    Profiled dump file: %s' % filename)
          last_dump = filename
          if not first_dump:
            first_dump = filename
      if first_dump:
        logging.info('    First dump file: %s' % first_dump)
        matched = re.match('^endure.\d+.(\d+).heap$', last_dump)
        last_sequence_id = matched.group(1)
        self._json_file = open(
            os.path.join(self._workdir,
                         'endure.%05d.%s.json' % (proc_info['tab_pid'],
                                                  last_sequence_id)), 'w+')
        self._proc = subprocess.Popen(
            '%s json %s' % (self._DMPROF_SCRIPT_PATH,
                            os.path.join(self._workdir, first_dump)),
            shell=True, stdout=self._json_file)
        if is_last:
          # Wait only when it is the last profiling.  dmprof may take long time.
          self._WaitForDeepMemoryProfiler()

          # Upload the dumped files.
          if first_dump and self._upload and self._gsutil:
            if self._revision:
              destination_path = '%s%s/' % (self._upload, self._revision)
            else:
              destination_path = self._upload
            destination_path += '%s-%s-%s.zip' % (
                webapp_name,
                test_description,
                os.path.basename(self._workdir))
            gsutil_command = '%s upload --gsutil %s %s %s' % (
                self._DMPROF_SCRIPT_PATH,
                self._gsutil,
                os.path.join(self._workdir, first_dump),
                destination_path)
            logging.info('Uploading: %s' % gsutil_command)
            try:
              returncode = subprocess.call(gsutil_command, shell=True)
              logging.info('  Return code: %d' % returncode)
            except OSError, e:
              logging.error('  Error while uploading: %s', e)
          else:
            logging.info('Note that the dumped files are not uploaded.')
      else:
        logging.info('    No dump files.')

  def ParseResultAndOutputPerfGraphValues(
      self, webapp_name, test_description, output_perf_graph_value):
    """Parses Deep Memory Profiler result, and outputs perf graph values."""
    if not self._enabled:
      return

    results = {}
    for policy in self._POLICIES:
      if self._last_json_filename:
        json_data = {}
        with open(self._last_json_filename) as json_f:
          json_data = json.load(json_f)
        if json_data['version'] == 'JSON_DEEP_1':
          results[policy] = json_data['snapshots']
        elif json_data['version'] == 'JSON_DEEP_2':
          results[policy] = json_data['policies'][policy]['snapshots']
    for policy, result in results.iteritems():
      if result and result[-1]['second'] > self._last_time[policy]:
        started = False
        for legend in json_data['policies'][policy]['legends']:
          if legend == 'FROM_HERE_FOR_TOTAL':
            started = True
          elif legend == 'UNTIL_HERE_FOR_TOTAL':
            break
          elif started:
            output_perf_graph_value(
                legend.encode('utf-8'), [
                    (int(round(snapshot['second'])), snapshot[legend] / 1024)
                    for snapshot in result
                    if snapshot['second'] > self._last_time[policy]],
                'KB',
                graph_name='%s%s-%s-DMP' % (
                    webapp_name, test_description, policy),
                units_x='seconds', is_stacked=True)
        self._last_time[policy] = result[-1]['second']

  def _WaitForDeepMemoryProfiler(self):
    """Waits for the Deep Memory Profiler to finish if running."""
    if not self._enabled or not self._proc:
      return

    self._proc.wait()
    self._proc = None
    if self._json_file:
      self._last_json_filename = self._json_file.name
      self._json_file.close()
      self._json_file = None


class ChromeEndureBaseTest(perf.BasePerfTest):
  """Implements common functionality for all Chrome Endure tests.

  All Chrome Endure test classes should inherit from this class.
  """

  _DEFAULT_TEST_LENGTH_SEC = 60 * 60 * 6  # Tests run for 6 hours.
  _GET_PERF_STATS_INTERVAL = 60 * 5  # Measure perf stats every 5 minutes.
  # TODO(dennisjeffrey): Do we still need to tolerate errors?
  _ERROR_COUNT_THRESHOLD = 50  # Number of errors to tolerate.
  _REVISION = ''
  _GSUTIL = 'gsutil'

  def setUp(self):
    # The environment variables for the Deep Memory Profiler must be set
    # before perf.BasePerfTest.setUp() to inherit them to Chrome.
    self._dmprof = DeepMemoryProfiler()
    self._revision = str(os.environ.get('REVISION', self._REVISION))
    if not self._revision:
      self._revision = str(os.environ.get('BUILDBOT_GOT_REVISION',
                                          self._REVISION))
    self._gsutil = str(os.environ.get('GSUTIL', self._GSUTIL))
    if self._dmprof:
      self._dmprof.SetUp(self.IsLinux(), self._revision, self._gsutil)

    perf.BasePerfTest.setUp(self)

    self._test_length_sec = int(
        os.environ.get('TEST_LENGTH', self._DEFAULT_TEST_LENGTH_SEC))
    self._get_perf_stats_interval = int(
        os.environ.get('PERF_STATS_INTERVAL', self._GET_PERF_STATS_INTERVAL))

    logging.info('Running test for %d seconds.', self._test_length_sec)
    logging.info('Gathering perf stats every %d seconds.',
                 self._get_perf_stats_interval)

    if self._dmprof:
      self._dmprof.LogFirstMessage()

    # Set up a remote inspector client associated with tab 0.
    logging.info('Setting up connection to remote inspector...')
    self._remote_inspector_client = (
        remote_inspector_client.RemoteInspectorClient())
    logging.info('Connection to remote inspector set up successfully.')

    self._test_start_time = 0
    self._num_errors = 0
    self._events_to_output = []

  def tearDown(self):
    logging.info('Terminating connection to remote inspector...')
    self._remote_inspector_client.Stop()
    logging.info('Connection to remote inspector terminated.')

    # Must be done at end of this function except for post-cleaning after
    # Chrome finishes.
    perf.BasePerfTest.tearDown(self)

    # Must be done after perf.BasePerfTest.tearDown()
    if self._dmprof:
      self._dmprof.TearDown()

  def ExtraChromeFlags(self):
    """Ensures Chrome is launched with custom flags.

    Returns:
      A list of extra flags to pass to Chrome when it is launched.
    """
    # The same with setUp, but need to fetch the environment variable since
    # ExtraChromeFlags is called before setUp.
    deep_memory_profile = DeepMemoryProfiler.GetEnvironmentVariable(
        'DEEP_MEMORY_PROFILE', bool, DeepMemoryProfiler.DEEP_MEMORY_PROFILE)

    # Ensure Chrome enables remote debugging on port 9222.  This is required to
    # interact with Chrome's remote inspector.
    # Also, enable the memory benchmarking V8 extension for heap dumps.
    extra_flags = ['--remote-debugging-port=9222',
                   '--enable-memory-benchmarking']
    if deep_memory_profile:
      extra_flags.append('--no-sandbox')
    return perf.BasePerfTest.ExtraChromeFlags(self) + extra_flags

  def _OnTimelineEvent(self, event_info):
    """Invoked by the Remote Inspector Client when a timeline event occurs.

    Args:
      event_info: A dictionary containing raw information associated with a
         timeline event received from Chrome's remote inspector.  Refer to
         chrome/src/third_party/WebKit/Source/WebCore/inspector/Inspector.json
         for the format of this dictionary.
    """
    elapsed_time = int(round(time.time() - self._test_start_time))

    if event_info['type'] == 'GCEvent':
      self._events_to_output.append({
        'type': 'GarbageCollection',
        'time': elapsed_time,
        'data':
            {'collected_bytes': event_info['data']['usedHeapSizeDelta']},
      })

  def _RunEndureTest(self, webapp_name, tab_title_substring, test_description,
                     do_scenario, frame_xpath=''):
    """The main test harness function to run a general Chrome Endure test.

    After a test has performed any setup work and has navigated to the proper
    starting webpage, this function should be invoked to run the endurance test.

    Args:
      webapp_name: A string name for the webapp being tested.  Should not
          include spaces.  For example, 'Gmail', 'Docs', or 'Plus'.
      tab_title_substring: A unique substring contained within the title of
          the tab to use, for identifying the appropriate tab.
      test_description: A string description of what the test does, used for
          outputting results to be graphed.  Should not contain spaces.  For
          example, 'ComposeDiscard' for Gmail.
      do_scenario: A callable to be invoked that implements the scenario to be
          performed by this test.  The callable is invoked iteratively for the
          duration of the test.
      frame_xpath: The string xpath of the frame in which to inject javascript
          to clear chromedriver's cache (a temporary workaround until the
          WebDriver team changes how they handle their DOM node cache).
    """
    self._num_errors = 0
    self._test_start_time = time.time()
    last_perf_stats_time = time.time()
    if self._dmprof:
      self.HeapProfilerDump('renderer', 'Chrome Endure (first)')
    self._GetPerformanceStats(
        webapp_name, test_description, tab_title_substring)
    self._iteration_num = 0  # Available to |do_scenario| if needed.

    self._remote_inspector_client.StartTimelineEventMonitoring(
        self._OnTimelineEvent)

    while time.time() - self._test_start_time < self._test_length_sec:
      self._iteration_num += 1

      if self._num_errors >= self._ERROR_COUNT_THRESHOLD:
        logging.error('Error count threshold (%d) reached. Terminating test '
                      'early.' % self._ERROR_COUNT_THRESHOLD)
        break

      if time.time() - last_perf_stats_time >= self._get_perf_stats_interval:
        last_perf_stats_time = time.time()
        if self._dmprof:
          self.HeapProfilerDump('renderer', 'Chrome Endure')
        self._GetPerformanceStats(
            webapp_name, test_description, tab_title_substring)

      if self._iteration_num % 10 == 0:
        remaining_time = self._test_length_sec - (time.time() -
                                                  self._test_start_time)
        logging.info('Chrome interaction #%d. Time remaining in test: %d sec.' %
                     (self._iteration_num, remaining_time))

      do_scenario()
      # Clear ChromeDriver's DOM node cache so its growth doesn't affect the
      # results of Chrome Endure.
      # TODO(dennisjeffrey): Once the WebDriver team implements changes to
      # handle their DOM node cache differently, we need to revisit this.  It
      # may no longer be necessary at that point to forcefully delete the cache.
      # Additionally, the Javascript below relies on an internal property of
      # WebDriver that may change at any time.  This is only a temporary
      # workaround to stabilize the Chrome Endure test results.
      js = """
        (function() {
          delete document.$wdc_;
          window.domAutomationController.send('done');
        })();
      """
      try:
        self.ExecuteJavascript(js, frame_xpath=frame_xpath)
      except pyauto_errors.AutomationCommandTimeout:
        self._num_errors += 1
        logging.warning('Logging an automation timeout: delete chromedriver '
                        'cache.')

    self._remote_inspector_client.StopTimelineEventMonitoring()

    if self._dmprof:
      self.HeapProfilerDump('renderer', 'Chrome Endure (last)')
    self._GetPerformanceStats(
        webapp_name, test_description, tab_title_substring, is_last=True)

  def _GetProcessInfo(self, tab_title_substring):
    """Gets process info associated with an open browser/tab.

    Args:
      tab_title_substring: A unique substring contained within the title of
          the tab to use; needed for locating the tab info.

    Returns:
      A dictionary containing information about the browser and specified tab
      process:
      {
        'browser_private_mem': integer,  # Private memory associated with the
                                         # browser process, in KB.
        'tab_private_mem': integer,  # Private memory associated with the tab
                                     # process, in KB.
        'tab_pid': integer,  # Process ID of the tab process.
      }
    """
    browser_process_name = (
        self.GetBrowserInfo()['properties']['BrowserProcessExecutableName'])
    info = self.GetProcessInfo()

    # Get the information associated with the browser process.
    browser_proc_info = []
    for browser_info in info['browsers']:
      if browser_info['process_name'] == browser_process_name:
        for proc_info in browser_info['processes']:
          if proc_info['child_process_type'] == 'Browser':
            browser_proc_info.append(proc_info)
    self.assertEqual(len(browser_proc_info), 1,
                     msg='Expected to find 1 Chrome browser process, but found '
                         '%d instead.\nCurrent process info:\n%s.' % (
                         len(browser_proc_info), self.pformat(info)))

    # Get the process information associated with the specified tab.
    tab_proc_info = []
    for browser_info in info['browsers']:
      for proc_info in browser_info['processes']:
        if (proc_info['child_process_type'] == 'Tab' and
            [x for x in proc_info['titles'] if tab_title_substring in x]):
          tab_proc_info.append(proc_info)
    self.assertEqual(len(tab_proc_info), 1,
                     msg='Expected to find 1 %s tab process, but found %d '
                         'instead.\nCurrent process info:\n%s.' % (
                         tab_title_substring, len(tab_proc_info),
                         self.pformat(info)))

    browser_proc_info = browser_proc_info[0]
    tab_proc_info = tab_proc_info[0]
    return {
      'browser_private_mem': browser_proc_info['working_set_mem']['priv'],
      'tab_private_mem': tab_proc_info['working_set_mem']['priv'],
      'tab_pid': tab_proc_info['pid'],
    }

  def _GetPerformanceStats(self, webapp_name, test_description,
                           tab_title_substring, is_last=False):
    """Gets performance statistics and outputs the results.

    Args:
      webapp_name: A string name for the webapp being tested.  Should not
          include spaces.  For example, 'Gmail', 'Docs', or 'Plus'.
      test_description: A string description of what the test does, used for
          outputting results to be graphed.  Should not contain spaces.  For
          example, 'ComposeDiscard' for Gmail.
      tab_title_substring: A unique substring contained within the title of
          the tab to use, for identifying the appropriate tab.
      is_last: A boolean value which should be True if it's the last call of
          _GetPerformanceStats.  The default is False.
    """
    logging.info('Gathering performance stats...')
    elapsed_time = int(round(time.time() - self._test_start_time))

    memory_counts = self._remote_inspector_client.GetMemoryObjectCounts()
    proc_info = self._GetProcessInfo(tab_title_substring)

    if self._dmprof:
      self._dmprof.StartProfiler(
          proc_info, is_last, webapp_name, test_description)

    # DOM node count.
    dom_node_count = memory_counts['DOMNodeCount']
    self._OutputPerfGraphValue(
        'TotalDOMNodeCount', [(elapsed_time, dom_node_count)], 'nodes',
        graph_name='%s%s-Nodes-DOM' % (webapp_name, test_description),
        units_x='seconds')

    # Event listener count.
    event_listener_count = memory_counts['EventListenerCount']
    self._OutputPerfGraphValue(
        'EventListenerCount', [(elapsed_time, event_listener_count)],
        'listeners',
        graph_name='%s%s-EventListeners' % (webapp_name, test_description),
        units_x='seconds')

    # Browser process private memory.
    self._OutputPerfGraphValue(
        'BrowserPrivateMemory',
        [(elapsed_time, proc_info['browser_private_mem'])], 'KB',
        graph_name='%s%s-BrowserMem-Private' % (webapp_name, test_description),
        units_x='seconds')

    # Tab process private memory.
    self._OutputPerfGraphValue(
        'TabPrivateMemory',
        [(elapsed_time, proc_info['tab_private_mem'])], 'KB',
        graph_name='%s%s-TabMem-Private' % (webapp_name, test_description),
        units_x='seconds')

    # V8 memory used.
    v8_info = self.GetV8HeapStats()  # First window, first tab.
    v8_mem_used = v8_info['v8_memory_used'] / 1024.0  # Convert to KB.
    self._OutputPerfGraphValue(
        'V8MemoryUsed', [(elapsed_time, v8_mem_used)], 'KB',
        graph_name='%s%s-V8MemUsed' % (webapp_name, test_description),
        units_x='seconds')

    # V8 memory allocated.
    v8_mem_allocated = v8_info['v8_memory_allocated'] / 1024.0  # Convert to KB.
    self._OutputPerfGraphValue(
        'V8MemoryAllocated', [(elapsed_time, v8_mem_allocated)], 'KB',
        graph_name='%s%s-V8MemAllocated' % (webapp_name, test_description),
        units_x='seconds')

    if self._dmprof:
      self._dmprof.ParseResultAndOutputPerfGraphValues(
          webapp_name, test_description, self._OutputPerfGraphValue)

    logging.info('  Total DOM node count: %d nodes' % dom_node_count)
    logging.info('  Event listener count: %d listeners' % event_listener_count)
    logging.info('  Browser process private memory: %d KB' %
                 proc_info['browser_private_mem'])
    logging.info('  Tab process private memory: %d KB' %
                 proc_info['tab_private_mem'])
    logging.info('  V8 memory used: %f KB' % v8_mem_used)
    logging.info('  V8 memory allocated: %f KB' % v8_mem_allocated)

    # Output any new timeline events that have occurred.
    if self._events_to_output:
      logging.info('Logging timeline events...')
      event_type_to_value_list = {}
      for event_info in self._events_to_output:
        if not event_info['type'] in event_type_to_value_list:
          event_type_to_value_list[event_info['type']] = []
        event_type_to_value_list[event_info['type']].append(
            (event_info['time'], event_info['data']))
      for event_type, value_list in event_type_to_value_list.iteritems():
        self._OutputEventGraphValue(event_type, value_list)
      self._events_to_output = []
    else:
      logging.info('No new timeline events to log.')

  def _GetElement(self, find_by, value):
    """Gets a WebDriver element object from the webpage DOM.

    Args:
      find_by: A callable that queries WebDriver for an element from the DOM.
      value: A string value that can be passed to the |find_by| callable.

    Returns:
      The identified WebDriver element object, if found in the DOM, or
      None, otherwise.
    """
    try:
      return find_by(value)
    except selenium.common.exceptions.NoSuchElementException:
      return None

  def _ClickElementByXpath(self, driver, xpath):
    """Given the xpath for a DOM element, clicks on it using WebDriver.

    Args:
      driver: A WebDriver object, as returned by self.NewWebDriver().
      xpath: The string xpath associated with the DOM element to click.

    Returns:
      True, if the DOM element was found and clicked successfully, or
      False, otherwise.
    """
    try:
      self.WaitForDomNode(xpath)
    except (pyauto_errors.JSONInterfaceError,
            pyauto_errors.JavascriptRuntimeError) as e:
      logging.exception('PyAuto exception: %s' % e)
      return False

    try:
      element = self._GetElement(driver.find_element_by_xpath, xpath)
      element.click()
    except (selenium.common.exceptions.StaleElementReferenceException,
            selenium.common.exceptions.TimeoutException) as e:
      logging.exception('WebDriver exception: %s' % e)
      return False

    return True


class ChromeEndureControlTest(ChromeEndureBaseTest):
  """Control tests for Chrome Endure."""

  _WEBAPP_NAME = 'Control'
  _TAB_TITLE_SUBSTRING = 'Chrome Endure Control Test'

  def testControlAttachDetachDOMTree(self):
    """Continually attach and detach a DOM tree from a basic document."""
    test_description = 'AttachDetachDOMTree'
    url = self.GetHttpURLForDataPath('chrome_endure', 'endurance_control.html')
    self.NavigateToURL(url)
    loaded_tab_title = self.GetActiveTabTitle()
    self.assertTrue(self._TAB_TITLE_SUBSTRING in loaded_tab_title,
                    msg='Loaded tab title does not contain "%s": "%s"' %
                        (self._TAB_TITLE_SUBSTRING, loaded_tab_title))

    def scenario():
      # Just sleep.  Javascript in the webpage itself does the work.
      time.sleep(5)

    self._RunEndureTest(self._WEBAPP_NAME, self._TAB_TITLE_SUBSTRING,
                        test_description, scenario)

  def testControlAttachDetachDOMTreeWebDriver(self):
    """Use WebDriver to attach and detach a DOM tree from a basic document."""
    test_description = 'AttachDetachDOMTreeWebDriver'
    url = self.GetHttpURLForDataPath('chrome_endure',
                                     'endurance_control_webdriver.html')
    self.NavigateToURL(url)
    loaded_tab_title = self.GetActiveTabTitle()
    self.assertTrue(self._TAB_TITLE_SUBSTRING in loaded_tab_title,
                    msg='Loaded tab title does not contain "%s": "%s"' %
                        (self._TAB_TITLE_SUBSTRING, loaded_tab_title))

    driver = self.NewWebDriver()

    def scenario(driver):
      # Click the "attach" button to attach a large DOM tree (with event
      # listeners) to the document, wait half a second, click "detach" to detach
      # the DOM tree from the document, wait half a second.
      self._ClickElementByXpath(driver, 'id("attach")')
      time.sleep(0.5)
      self._ClickElementByXpath(driver, 'id("detach")')
      time.sleep(0.5)

    self._RunEndureTest(self._WEBAPP_NAME, self._TAB_TITLE_SUBSTRING,
                        test_description, lambda: scenario(driver))


class IndexedDBOfflineTest(ChromeEndureBaseTest):
  """Long-running performance tests for IndexedDB, modeling offline usage."""

  _WEBAPP_NAME = 'IndexedDBOffline'
  _TAB_TITLE_SUBSTRING = 'IndexedDB Offline'

  def setUp(self):
    ChromeEndureBaseTest.setUp(self)

    url = self.GetHttpURLForDataPath('indexeddb', 'endure', 'app.html')
    self.NavigateToURL(url)
    loaded_tab_title = self.GetActiveTabTitle()
    self.assertTrue(self._TAB_TITLE_SUBSTRING in loaded_tab_title,
                    msg='Loaded tab title does not contain "%s": "%s"' %
                        (self._TAB_TITLE_SUBSTRING, loaded_tab_title))

    self._driver = self.NewWebDriver()

  def testOfflineOnline(self):
    """Simulates user input while offline and sync while online.

    This test alternates between a simulated "Offline" state (where user
    input events are queued) and an "Online" state (where user input events
    are dequeued, sync data is staged, and sync data is unstaged).
    """
    test_description = 'OnlineOfflineSync'

    def scenario():
      # Click the "Online" button and let simulated sync run for 1 second.
      if not self._ClickElementByXpath(self._driver, 'id("online")'):
        self._num_errors += 1
        logging.warning('Logging an automation error: click "online" button.')

      try:
        self.WaitForDomNode('id("state")[text()="online"]')
      except (pyauto_errors.JSONInterfaceError,
              pyauto_errors.JavascriptRuntimeError):
        self._num_errors += 1
        logging.warning('Logging an automation error: wait for "online".')

      time.sleep(1)

      # Click the "Offline" button and let user input occur for 1 second.
      if not self._ClickElementByXpath(self._driver, 'id("offline")'):
        self._num_errors += 1
        logging.warning('Logging an automation error: click "offline" button.')

      try:
        self.WaitForDomNode('id("state")[text()="offline"]')
      except (pyauto_errors.JSONInterfaceError,
              pyauto_errors.JavascriptRuntimeError):
        self._num_errors += 1
        logging.warning('Logging an automation error: wait for "offline".')

      time.sleep(1)

    self._RunEndureTest(self._WEBAPP_NAME, self._TAB_TITLE_SUBSTRING,
                        test_description, scenario)


if __name__ == '__main__':
  pyauto_functional.Main()
