#!/usr/bin/env python
#
# Copyright 2013 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

"""Writes dependency ordered list of native libraries.

The list excludes any Android system libraries, as those are not bundled with
the APK.

This list of libraries is used for several steps of building an APK.
In the component build, the --input-libraries only needs to be the top-level
library (i.e. libcontent_shell_content_view). This will then use readelf to
inspect the shared libraries and determine the full list of (non-system)
libraries that should be included in the APK.
"""

# TODO(cjhopman): See if we can expose the list of library dependencies from
# gyp, rather than calculating it ourselves.
# http://crbug.com/225558
import fnmatch
import optparse
import os
import re
import sys

from util import build_utils

_options = None
_library_re = re.compile(
    '.*NEEDED.*Shared library: \[(?P<library_name>[\w/.]+)\]')

#return filenames only in '_options.input_libraries'
def InputLibNames():
  input_libs = build_utils.ParseGypList(_options.input_libraries)
  return input_libs

#search recursively in all the folders under 'path' and check
#if file exist and return the path of file
def FindFullPath(name, path):
  matches = []
  for root, dirnames, filenames in os.walk(path):
    for filename in fnmatch.filter(filenames, name):
      abs_path = os.path.join(root)+"/"+name
      #Update match if abs_path exist in _options.input_libraries
      for file_name in fnmatch.filter(InputLibNames(), abs_path):
        matches.append(os.path.join(root))
  if(len(matches) > 0):
    return matches.pop(0)
  return None

def FullLibraryPath(library_name):
  for directory in _options.libraries_dir.split(','):
    path = '%s/%s' % (directory, library_name)
    if os.path.exists(path):
      return path
  return library_name


def IsSystemLibrary(library_name):
  # If the library doesn't exist in the libraries directory, assume that it is
  # an Android system library.
  return not os.path.exists(FullLibraryPath(library_name))


def CallReadElf(library_or_executable):
  readelf_cmd = [_options.readelf,
                 '-d',
                 library_or_executable]
  return build_utils.CheckOutput(readelf_cmd)


def GetDependencies(library_or_executable):
  elf = CallReadElf(library_or_executable)
  return set(_library_re.findall(elf))


def GetNonSystemDependencies(library_name):
  all_deps = GetDependencies(FullLibraryPath(library_name))
  return set((lib for lib in all_deps if not IsSystemLibrary(lib)))


def GetSortedTransitiveDependencies(libraries):
  """Returns all transitive library dependencies in dependency order."""
  return build_utils.GetSortedTransitiveDependencies(
      libraries, GetNonSystemDependencies)


def GetSortedTransitiveDependenciesForBinaries(binaries):
  if binaries[0].endswith('.so'):
    libraries = [os.path.basename(lib) for lib in binaries]
  else:
    assert len(binaries) == 1
    all_deps = GetDependencies(binaries[0])
    libraries = [lib for lib in all_deps if not IsSystemLibrary(lib)]

  return GetSortedTransitiveDependencies(libraries)


def main():
  parser = optparse.OptionParser()

  parser.add_option('--input-libraries',
      help='A list of top-level input libraries.')
  parser.add_option('--libraries-dir',
      help='The directory which contains shared libraries.')
  parser.add_option('--readelf', help='Path to the readelf binary.')
  parser.add_option('--output', help='Path to the generated .json file.')
  parser.add_option('--output_abs_path', help='Path to the generated .json file with absolute path')
  parser.add_option('--stamp', help='Path to touch on success.')

  global _options
  _options, _ = parser.parse_args()

  libraries = build_utils.ParseGypList(_options.input_libraries)
  if len(libraries):
    libraries = GetSortedTransitiveDependenciesForBinaries(libraries)

  full_path_libraries = [FullLibraryPath(lib) for lib in libraries]
  build_utils.WriteJson(full_path_libraries, _options.output_abs_path, only_if_changed=True)
  build_utils.WriteJson(libraries, _options.output, only_if_changed=True)

  if _options.stamp:
    build_utils.Touch(_options.stamp)


if __name__ == '__main__':
  sys.exit(main())


