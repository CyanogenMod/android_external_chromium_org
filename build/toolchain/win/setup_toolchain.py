# Copyright (c) 2013 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import os
import re
import sys

def ExtractImportantEnvironment():
  """Extracts environment variables required for the toolchain from the
  current environment."""
  envvars_to_save = (
      'goma_.*', # TODO(scottmg): This is ugly, but needed for goma.
      'Path',
      'PATHEXT',
      'SystemRoot',
      'TEMP',
      'TMP',
      )
  result = {}
  for envvar in envvars_to_save:
    if envvar in os.environ:
      if envvar == 'Path':
        # Our own rules (for running gyp-win-tool) and other actions in
        # Chromium rely on python being in the path. Add the path to this
        # python here so that if it's not in the path when ninja is run
        # later, python will still be found.
        result[envvar.upper()] = os.path.dirname(sys.executable) + \
            os.pathsep + os.environ[envvar]
      else:
        result[envvar.upper()] = os.environ[envvar]
  for required in ('SYSTEMROOT', 'TEMP', 'TMP'):
    if required not in result:
      raise Exception('Environment variable "%s" '
                      'required to be set to valid path' % required)
  return result


# VC setup will add a path like this in 32-bit mode:
#   c:\Program Files (x86)\Microsoft Visual Studio 10.0\VC\BIN
# And this in 64-bit mode:
#   c:\Program Files (x86)\Microsoft Visual Studio 10.0\VC\BIN\amd64
# Note that in 64-bit it's duplicated but the 64-bit one comes first.
#
# What we get as the path when running this will depend on which VS setup
# script you've run. The following two functions try to do this.

# For 32-bit compiles remove anything that ends in "\VC\WIN\amd64".
def FixupPath32(path):
  find_64 = re.compile("VC\\\\BIN\\\\amd64\\\\*$", flags=re.IGNORECASE)

  for i in range(len(path)):
    if find_64.search(path[i]):
      # Found 32-bit path, insert the 64-bit one immediately before it.
      dir_64 = path[i].rstrip("\\")
      dir_64 = dir_64[:len(dir_64) - 6]  # Trim off "\amd64".
      path[i] = dir_64
      break
  return path

# For 64-bit compiles, append anything ending in "\VC\BIN" with "\amd64" as
# long as that thing isn't already in the list, and append it immediately
# before the non-amd64-one.
def FixupPath64(path):
  find_32 = re.compile("VC\\\\BIN\\\\*$", flags=re.IGNORECASE)

  for i in range(len(path)):
    if find_32.search(path[i]):
      # Found 32-bit path, insert the 64-bit one immediately before it.
      dir_32 = path[i]
      if dir_32[len(dir_32) - 1] == '\\':
        dir_64 = dir_32 + "amd64"
      else:
        dir_64 = dir_32 + "\\amd64"
      path.insert(i, dir_64)
      break

  return path


def FormatAsEnvironmentBlock(envvar_dict):
  """Format as an 'environment block' directly suitable for CreateProcess.
  Briefly this is a list of key=value\0, terminated by an additional \0. See
  CreateProcess documentation for more details."""
  block = ''
  nul = '\0'
  for key, value in envvar_dict.iteritems():
    block += key + '=' + value + nul
  block += nul
  return block

def CopyTool(source_path):
  """Copies the given tool to the current directory, including a warning not
  to edit it."""
  with open(source_path) as source_file:
    tool_source = source_file.readlines()

  # Add header and write it out to the current directory (which should be the
  # root build dir).
  with open("gyp-win-tool", 'w') as tool_file:
    tool_file.write(''.join([tool_source[0],
                             '# Generated by setup_toolchain.py do not edit.\n']
                            + tool_source[1:]))


# Find the tool source, it's the first argument, and copy it.
if len(sys.argv) != 2:
  print "Need one argument (win_tool source path)."
  sys.exit(1)
CopyTool(sys.argv[1])

important_env_vars = ExtractImportantEnvironment()
path = important_env_vars["PATH"].split(";")

important_env_vars["PATH"] = ";".join(FixupPath32(path))
environ = FormatAsEnvironmentBlock(important_env_vars)
with open('environment.x86', 'wb') as env_file:
  env_file.write(environ)

important_env_vars["PATH"] = ";".join(FixupPath64(path))
environ = FormatAsEnvironmentBlock(important_env_vars)
with open('environment.x64', 'wb') as env_file:
  env_file.write(environ)
