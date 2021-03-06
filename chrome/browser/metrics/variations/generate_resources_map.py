#!/usr/bin/python
# Copyright 2014 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import collections
import hashlib
import operator
import os
import re
import sys


RESOURCE_EXTRACT_REGEX = re.compile('^#define (\S*) (\d*)$', re.MULTILINE)

class Error(Exception):
  """Base error class for all exceptions in generated_resources_map."""


class HashCollisionError(Error):
  """Multiple resource names hash to the same value."""


Resource = collections.namedtuple("Resource", ['hash', 'name', 'index'])


def _HashName(name):
  """Returns the hash id for a name.

  Args:
    name: The name to hash.

  Returns:
    An int that is at most 32 bits.
  """
  md5hash = hashlib.md5()
  md5hash.update(name)
  return int(md5hash.hexdigest()[:8], 16)


def _GetNameIndexPairsIter(string_to_scan):
  """Gets an iterator of the resource name and index pairs of the given string.

  Scans the input string for lines of the form "#define NAME INDEX" and returns
  an iterator over all matching (NAME, INDEX) pairs.

  Args:
    string_to_scan: The input string to scan.

  Yields:
    A tuple of name and index.
  """
  for match in RESOURCE_EXTRACT_REGEX.finditer(string_to_scan):
    yield match.group(1, 2)


def _GetResourceListFromString(resources_content):
  """Produces a list of |Resource| objects from a string.

  The input string conaints lines of the form "#define NAME INDEX". The returned
  list is sorted primarily by hash, then name, and then index.

  Args:
    resources_content: The input string to process, contains lines of the form
        "#define NAME INDEX".

  Returns:
    A sorted list of |Resource| objects.
  """
  resources = [Resource(_HashName(name), name, index) for name, index in
               _GetNameIndexPairsIter(resources_content)]

  # The default |Resource| order makes |resources| sorted by the hash, then
  # name, then index.
  resources.sort()

  return resources


def _CheckForHashCollisions(sorted_resource_list):
  """Checks a sorted list of |Resource| objects for hash collisions.

  Args:
    sorted_resource_list: A sorted list of |Resource| objects.

  Returns:
    A set of all |Resource| objects with collisions.
  """
  collisions = set()
  for i in xrange(len(sorted_resource_list) - 1):
    resource = sorted_resource_list[i]
    next_resource = sorted_resource_list[i+1]
    if resource.hash == next_resource.hash:
      collisions.add(resource)
      collisions.add(next_resource)

  return collisions


def _GenDataArray(
    resources, entry_pattern, array_name, array_type, data_getter):
  """Generates a C++ statement defining a literal array containing the hashes.

  Args:
    resources: A sorted list of |Resource| objects.
    entry_pattern: A pattern to be used to generate each entry in the array. The
        pattern is expected to have a place for data and one for a comment, in
        that order.
    array_name: The name of the array being generated.
    array_type: The type of the array being generated.
    data_getter: A function that gets the array data from a |Resource| object.

  Returns:
    A string containing a C++ statement defining the an array.
  """
  lines = [entry_pattern % (data_getter(r), r.name) for r in resources]
  pattern = """const %(type)s %(name)s[] = {
%(content)s
};
"""
  return pattern % {'type': array_type,
                    'name': array_name,
                    'content': '\n'.join(lines)}


def _GenerateFileContent(resources_content):
  """Generates the .cc content from the given generated_resources.h content.

  Args:
    resources_content: The input string to process, contains lines of the form
        "#define NAME INDEX".

  Returns:
    .cc file content defining the kResourceHashes and kResourceIndices arrays.
  """
  hashed_tuples = _GetResourceListFromString(resources_content)

  collisions = _CheckForHashCollisions(hashed_tuples)
  if collisions:
    error_message = "\n".join(
        ["hash: %i, name: %s" % (i[0], i[1]) for i in sorted(collisions)])
    error_message = ("\nThe following names had hash collisions "
                     "(sorted by the hash value):\n%s\n" %(error_message))
    raise HashCollisionError(error_message)

  hashes_array = _GenDataArray(
      hashed_tuples, "    %iU,  // %s", 'kResourceHashes', 'uint32_t',
      operator.attrgetter('hash'))
  indices_array = _GenDataArray(
      hashed_tuples, "    %s,  // %s", 'kResourceIndices', 'int',
      operator.attrgetter('index'))

  return (
      "// This file was generated by generate_resources_map.py. Do not edit.\n"
      "\n\n"
      "#include "
      "\"chrome/browser/metrics/variations/generated_resources_map.h\"\n\n"
      "namespace chrome_variations {\n\n"
      "%s"
      "\n"
      "%s"
      "\n"
      "}  // namespace chrome_variations\n") % (hashes_array, indices_array)


def main(resources_file, map_file):
  generated_resources_h = ""
  with open(resources_file, "r") as resources:
    generated_resources_h = resources.read()

  if len(generated_resources_h) == 0:
    raise Error("No content loaded for %s." % (resources_file))

  file_content = _GenerateFileContent(generated_resources_h)

  with open(map_file, "w") as generated_file:
    generated_file.write(file_content)


if __name__ == '__main__':
  sys.exit(main(sys.argv[1], sys.argv[2]))
