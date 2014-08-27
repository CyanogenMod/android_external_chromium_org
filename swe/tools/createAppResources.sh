#!/bin/bash
#
# Copyright (c) 2013, The Linux Foundation. All rights reserved.
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


PROGNAME=$(basename "$0")
PROGDIR=$(dirname "$0")

if  [ $# != 1 ]; then
  echo "Usage: $PROGNAME <Resources lib workspace path>"
  exit 0
fi

LIB_WORKSPACE=$(readlink -f "$1")

echo $LIB_WORKSPACE

Manifest_ContentJava_Content="<manifest xmlns:android=\"http://schemas.android.com/apk/res/android\"\n
      package=\"org.chromium.content\">\n
      </manifest>\n"
Project_Content="target=android-16\nandroid.library=true\n"

Manifest_UIJava_Content="<manifest xmlns:android=\"http://schemas.android.com/apk/res/android\"\n
      package=\"org.chromium.ui\">\n
            </manifest>\n"
            Project_Content="target=android-16\nandroid.library=true\n"
Manifest_SWEJava_Content="<manifest xmlns:android=\"http://schemas.android.com/apk/res/android\"\n
      package=\"org.codeaurora.swe\">\n
                  </manifest>\n"
                              Project_Content="target=android-16\nandroid.library=true\n"

function relpath {
       python -c "import os.path; print os.path.relpath('$1','${2:-$PWD}')" ;
}

#create Android lib project from content_java, ui_java and swe_engine_java resources

CONTENT_RES_PATH=$LIB_WORKSPACE/content_res
UI_RES_PATH=$LIB_WORKSPACE/ui_res
SWE_RES_PATH=$LIB_WORKSPACE/swe_res

#create manifest for the app
echo -e $Manifest_ContentJava_Content > $CONTENT_RES_PATH/AndroidManifest.xml
echo -e $Manifest_UIJava_Content > $UI_RES_PATH/AndroidManifest.xml
echo -e $Manifest_SWEJava_Content > $SWE_RES_PATH/AndroidManifest.xml

#create project properties
echo -e $Project_Content > $CONTENT_RES_PATH/project.properties
echo -e $Project_Content > $UI_RES_PATH/project.properties
echo -e $Project_Content > $SWE_RES_PATH/project.properties
