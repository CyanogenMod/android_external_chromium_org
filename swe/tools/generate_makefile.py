#!/usr/bin/env python

#
# Copyright (c) 2014, The Linux Foundation. All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are
# met:
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
# THIS SOFTWARE IS PROVIDED "AS IS" AND ANY EXPRESS OR IMPLIED
# WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
# MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT
# ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS
# BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
# CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
# SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
# BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
# WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
# OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
# IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#

import json
import os
from os.path import basename
from pprint import pprint
import sys

LICENSE=""
makefile_generated_msg = "#This file is generated\n" + LICENSE

local_path = "LOCAL_PATH := $(my-dir)"

system_lib_makefile_template = """
include $(CLEAR_VARS)
LOCAL_MODULE := LIBNAME
ifeq ($(TARGET_ARCH),arm)
  LOCAL_SRC_FILES := $(LOCAL_MODULE).so
endif

LOCAL_MODULE_STEM := $(LOCAL_MODULE)
LOCAL_MODULE_SUFFIX := $(suffix $(LOCAL_SRC_FILES))
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_MODULE_TAGS := optional
include $(BUILD_PREBUILT)
"""

apk_makefile_template = """
include $(CLEAR_VARS)

LOCAL_MODULE := Browser
LOCAL_MODULE_TAGS := optional

LOCAL_SRC_FILES := Browser.apk

LOCAL_MODULE_CLASS := APPS
LOCAL_BUILT_MODULE_STEM := package.apk
LOCAL_MODULE_SUFFIX := $(COMMON_ANDROID_PACKAGE_SUFFIX)
LOCAL_CERTIFICATE := PRESIGNED

LOCAL_REQUIRED_MODULES := \\
                        LIBSLIST

include $(BUILD_PREBUILT)"""

readme_file = """
Follow the steps below to replace Stock Android Browser with SWE Engine based browser

1) Remove Browser source
  <Apps Build Root>$ rm -rf packages/apps/Browser/*

2) Unzip 'swe_system_package.zip' contents into <Apps Build Root>/packages/apps/Browser
  <Apps Build Root>$ unzip swe_system_package.zip -d packages/apps/Browser/

3) Follow normal system build process.

"""

def get_file_base_name(file_name):
  return  os.path.splitext(file_name)[0]

def get_libname_list(json_file):
  json_data=open(json_file)
  data = json.load(json_data)
  json_data.close()
  return data;


def write_to_file(file_dest, string):
  f = open(file_dest, 'w+')
  f.write(string)
  f.close()

def create_root_makefile(dest_file):
  write_data = makefile_generated_msg
  write_data += "include $(call all-subdir-makefiles)"
  write_to_file(dest_file, write_data)

def create_apk_makefile(dest_file, libs_list_data):
   write_data = makefile_generated_msg
   write_data += local_path + "\n"
   lib_list = ""
   for libname in libs_list_data:
     lib_list += get_file_base_name(libname) + " \\\n"
   write_data += apk_makefile_template.replace('LIBSLIST', lib_list)
   write_to_file(dest_file ,write_data)

def create_libs_makefile(dest_file, libs_list_data):
  write_data = makefile_generated_msg
  write_data += local_path + "\n"

  for libname in libs_list_data:
    write_data += system_lib_makefile_template.replace('LIBNAME',
        get_file_base_name(libname))
    write_data += "\n"
  write_to_file(dest_file, write_data)

def create_readme(dest_file):
  write_data = readme_file
  write_to_file(dest_file, write_data)

def create_android_makefile(json_file, dest_folder):
  libs_data = get_libname_list(json_file)
  create_root_makefile(dest_folder+"Android.mk")
  create_libs_makefile(dest_folder+"libs/Android.mk", libs_data)
  create_apk_makefile(dest_folder+"apps/Android.mk", libs_data)
  create_readme(dest_folder+"README")

def main():
  src = sys.argv[1]
  dest = sys.argv[2]
  create_android_makefile(src, dest)

if __name__ == '__main__':
    main()
