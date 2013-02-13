#!/usr/bin/env python
#
# Copyright (c) 2013 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

"""Command line tool for forwarding ports from a device to the host.

Allows an Android device to connect to services running on the host machine,
i.e., "adb forward" in reverse. Requires |host_forwarder| and |device_forwarder|
to be built.
"""

import optparse
import sys
import time

from pylib import android_commands, forwarder
from pylib.utils import run_tests_helper
from pylib.valgrind_tools import CreateTool


def main(argv):
  parser = optparse.OptionParser(usage='Usage: %prog [options] device_port '
                                 'host_port [device_port_2 host_port_2] ...',
                                 description=__doc__)
  parser.add_option('-v',
                    '--verbose',
                    dest='verbose_count',
                    default=0,
                    action='count',
                    help='Verbose level (multiple times for more)')
  parser.add_option('--device',
                    help='Serial number of device we should use.')
  parser.add_option('--host',
                    help='Host address to forward to from the host machine. '
                    '127.0.0.1 by default', default='127.0.0.1')
  parser.add_option('--debug', action='store_const', const='Debug',
                    dest='build_type', default='Release',
                    help='Use Debug build of host tools instead of Release.')

  options, args = parser.parse_args(argv)
  run_tests_helper.SetLogLevel(options.verbose_count)

  if len(args) < 2 or not len(args) % 2:
    parser.error('Need even number of port pairs')
    sys.exit(1)

  try:
    port_pairs = map(int, args[1:])
    port_pairs = zip(port_pairs[::2], port_pairs[1::2])
  except ValueError:
    parser.error('Bad port number')
    sys.exit(1)

  adb = android_commands.AndroidCommands(options.device)
  tool = CreateTool(None, adb)
  forwarder_instance = forwarder.Forwarder(adb, options.build_type)
  try:
    forwarder_instance.Run(port_pairs, tool, options.host)
    while True:
      time.sleep(60)
  except KeyboardInterrupt:
    sys.exit(0)
  finally:
    forwarder_instance.Close()


if __name__ == '__main__':
  main(sys.argv)
