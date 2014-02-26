#Copyright (c) 2013, The Linux Foundation. All rights reserved.
#
#Redistribution and use in source and binary forms, with or without
#modification, are permitted provided that the following conditions are
#met:
#    * Redistributions of source code must retain the above copyright
#      notice, this list of conditions and the following disclaimer.
#    * Redistributions in binary form must reproduce the above
#      copyright notice, this list of conditions and the following
#      disclaimer in the documentation and/or other materials provided
#      with the distribution.
#    * Neither the name of The Linux Foundation nor the names of its
#      contributors may be used to endorse or promote products derived
#      from this software without specific prior written permission.
#
#THIS SOFTWARE IS PROVIDED "AS IS" AND ANY EXPRESS OR IMPLIED
#WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
#MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT
#ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS
#BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
#CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
#SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
#BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
#WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
#OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
#IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

import os
import urllib2
import tarfile
import urlparse
import shutil
import sys

script_dir = os.path.dirname(os.path.realpath(__file__))
chrome_src = os.path.abspath(os.path.join(script_dir, os.pardir))

def createGypi(arg, srcPath, gypiFileName):
  # directory to be created
  dir_path = os.path.join(chrome_src, srcPath)
  dest_filename = os.path.join(dir_path, gypiFileName)
  dest_file_exists = False
  try:
    with open(dest_filename):
      dest_file_exists = True
  except IOError:
      dest_file_exists = False

  # check if file does not exist
  if (dest_file_exists == False):
    print "SWE "+ arg +" location: " + str(dir_path) + " - Create default"
    # Check if  path exists
    if (os.path.isdir(dir_path) == False):
      os.mkdir(dir_path)
    print "Create default gypi: " + str(dest_filename)
    with open(dest_filename, "w+") as outfile:
      outfile.write("{}")
  else:
    print "SWE "+arg+" location: " + str(dir_path) + " - Ok"


def main():
  if sys.argv[1] == 'javatests':
    createGypi(sys.argv[1], 'swe/engine/javatests', 'swe_webview_tests.gypi')
  elif sys.argv[1] == 'swe-android-browser':
    createGypi(sys.argv[1], 'swe/browser', 'swe_android_browser.gypi')
  elif sys.argv[1] == 'swe-android-support':
    createGypi(sys.argv[1], 'swe', 'android_support.gypi')
  elif sys.argv[1] == 'swe-jarjar':
    createGypi(sys.argv[1], 'third_party/jarjar', 'jarjar_multiple.gypi')
  elif sys.argv[1] == 'swe-browser-test':
    createGypi(sys.argv[1], 'swe/browser/test', 'swe_browser_test_apk.gypi')
if __name__ == '__main__':
  main()
