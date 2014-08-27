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
CHROMIUM_SRC=$PWD
EXT_DIR="swe/ext"
SWE_ENGINE="swe/engine/java"
SWE_ENGINE_PACKAGE="org.codeaurora.swe"
AWC_ENGINE="android_webview/java"
AWC_ENGINE_PACKAGE="org.chromium.android_webview"
AWC_TEST_APP="android_webview/javatests"
SWE_ANDROID_BROWSER="swe/browser"
PROJECT_PROPERTIES="target=android-17\nandroid.library=true"
ENGINES=$SWE_ENGINE

function relpath {
       python -c "import os.path; print os.path.relpath('$1','${2:-$PWD}')" ;
}

function manifest {
    echo "<manifest xmlns:android=\"http://schemas.android.com/apk/res/android\"\n
      package=\""$1"\">\n</manifest>";
}

# Make sure we're in the src directory.
if [ ! `basename $PWD` = "src" ]
then
  printf "\n${PROGNAME}: Please run from src directory\n"
  exit 1
fi

# First pass through arguments to set BUILD_TYPE and AWC_ENGINE
BUILD_TYPE="Release"
for var in "$@"
do
  case ${var,,} in
    release ) continue;;
    debug   ) BUILD_TYPE="Debug"; continue;;
    awc     ) ENGINES="$ENGINES $AWC_ENGINE"; continue;;
    --help  ) cat <<EOF
Usage: ./swe/tools/$PROGNAME [build] [awc] [dirs]
where,
  build: debug|release. If not present, build is set to 'release'.
  awc:   Include Android WebView (android_webview/java) in the setup. If not present,
         the script will only setup the SweEngine (swe/engine/java) project.
  dirs:  Space separated list of relative to CHROMIUM_SRC directories that
         contain AndroidManifest.xml files of applications that consume the
         SweEngine or Android WebView.
Example:
  The following invocation adds the necessary symlinks and files for the SWE
  engine and the SWE android browser in release mode.
./swe/tools/$PROGNAME swe/browser
EOF
              exit 0;;
  esac
done
OUT="$CHROMIUM_SRC/out/$BUILD_TYPE"

printf "\nSetting dependencies from $OUT\n"

####################
# App Dependencies #
####################

# Second scan through arguments to extract path names and setup apps
for var in "$@"
do
  # remove trailing "/"
  var=${var/%\/}
  case ${var,,} in
    release ) continue;;
    debug   ) continue;;
    awc     ) continue;;
  esac
  if [ -d $CHROMIUM_SRC/$var ]
  then
    echo " ==> $var"
    mkdir -p $CHROMIUM_SRC/$var/libs
    pushd $CHROMIUM_SRC/$var/libs > /dev/null

    if [ "$var" !=  "$AWC_TEST_APP" ]
    # Link pak file and native libraries for apps other than $AWC_TEST_APP
    then
      ln -sf `relpath $OUT/swe_android_browser_apk/libs/armeabi-v7a/ .`
      ln -sf `relpath $CHROMIUM_SRC/swe/fast-webview/target/fast_webview_java.jar .`
      popd > /dev/null
      mkdir -p $CHROMIUM_SRC/$var/assets
      pushd $CHROMIUM_SRC/$var/assets > /dev/null
      ln -sf `relpath $OUT/swe_android_browser_apk/assets/webviewchromium.pak .`
    else
      # Link support libraries for $AWC_TEST_APP
      ln -sf `relpath $OUT/lib.java/base_java_test_support.jar .`
      ln -sf `relpath $OUT/lib.java/content_java_test_support.jar .`
      ln -sf `relpath $OUT/lib.java/net_java_test_support.jar .`
    fi
    popd > /dev/null
  else
    printf "\n[WARNING] Skipping $CHROMIUM_SRC/$var (does not exist) \n\n"
  fi
done

#######################
# Engine Dependencies #
#######################

mkdir -p $EXT_DIR

echo " ==> $EXT_DIR/ui_res"
mkdir -p $EXT_DIR/ui_res/res/values
MANIFEST=`manifest "org.chromium.ui"`
[ ! -e $EXT_DIR/ui_res/AndroidManifest.xml ] &&
  echo -e $MANIFEST > $EXT_DIR/ui_res/AndroidManifest.xml
[ ! -e $EXT_DIR/ui_res/project.properties ] &&
  echo -e $PROJECT_PROPERTIES > $EXT_DIR/ui_res/project.properties
pushd $EXT_DIR/ui_res/res > /dev/null
ln -sf `relpath $OUT/gen/ui_java/res_grit .`/values-* .
cd values
ln -sf `relpath $OUT/gen/ui_java/res_grit/values/android_ui_strings.xml .`

#ln -sf `relpath $OUT/gen/ui_java/res_v14_compatibility/values/styles.xml .`
popd > /dev/null

echo " ==> $EXT_DIR/content_res"
mkdir -p $EXT_DIR/content_res/res/values
MANIFEST=`manifest "org.chromium.content"`
[ ! -e $EXT_DIR/content_res/AndroidManifest.xml ] &&
  echo -e $MANIFEST > $EXT_DIR/content_res/AndroidManifest.xml
[ ! -e $EXT_DIR/content_res/project.properties ] &&
  echo -e $PROJECT_PROPERTIES > $EXT_DIR/content_res/project.properties
pushd $EXT_DIR/content_res/res > /dev/null
# Link resources from content source
ln -sf `relpath $CHROMIUM_SRC/content/public/android/java/res .`/drawable* .
ln -sf `relpath $CHROMIUM_SRC/content/public/android/java/res .`/layout* .
ln -sf `relpath $CHROMIUM_SRC/content/public/android/java/res .`/values-* .
cd values
ln -sf `relpath $CHROMIUM_SRC/content/public/android/java/res/values/attrs.xml .`
ln -sf `relpath $CHROMIUM_SRC/content/public/android/java/res/values/dimens.xml .`
ln -sf `relpath $CHROMIUM_SRC/content/public/android/java/res/values/strings.xml .`

# Link generated localized value resources from out directory
ln -sf `relpath $OUT/gen/content_java/res_grit/values/android_content_strings.xml .`
cd ..
ln -sf `relpath $OUT/gen/content_java/res_grit .`/values-* .
popd > /dev/null

echo " ==> $EXT_DIR/other_src"
mkdir -p $EXT_DIR/other_src/org/chromium/base/library_loader
pushd $EXT_DIR/other_src/org/chromium/ > /dev/null
ln -sf `relpath $CHROMIUM_SRC/android_webview/java/src/org/chromium/android_webview .`
cd base/library_loader
ln -sf `relpath $OUT/swe_android_browser_apk/native_libraries_java/NativeLibraries.java .`
popd > /dev/null
pushd $EXT_DIR/other_src/org > /dev/null
ln -sf `relpath $CHROMIUM_SRC/android_webview/java/src/org/codeaurora .`
popd > /dev/null

for engine in $ENGINES
do
  echo " ==> $engine"
  mkdir -p $CHROMIUM_SRC/$engine/libs
  pushd $CHROMIUM_SRC/$engine/libs > /dev/null
  # The following list may change as new features are added.
  ln -sf `relpath $OUT/lib.java/autofill_java.jar .`
  ln -sf `relpath $OUT/lib.java/base_java.jar .`
  ln -sf `relpath $OUT/lib.java/content_java.jar .`
  ln -sf `relpath $OUT/lib.java/eyesfree_java.jar .`
  ln -sf `relpath $OUT/lib.java/guava_javalib.jar .`
  ln -sf `relpath $OUT/lib.java/jsr_305_javalib.jar .`
  ln -sf `relpath $OUT/lib.java/media_java.jar .`
  ln -sf `relpath $OUT/lib.java/navigation_interception_java.jar .`
  ln -sf `relpath $OUT/lib.java/net_java.jar .`
  ln -sf `relpath $OUT/lib.java/ui_java.jar .`
  ln -sf `relpath $OUT/lib.java/web_contents_delegate_android_java.jar .`
  popd > /dev/null

  case $engine in
    "$SWE_ENGINE" ) package="$SWE_ENGINE_PACKAGE";;
    "$AWC_ENGINE" ) package="$AWC_ENGINE_PACKAGE";;
  esac

  MANIFEST=`manifest $package`
  [ ! -e $CHROMIUM_SRC/$engine/AndroidManifest.xml ] &&
    echo -e $MANIFEST > $CHROMIUM_SRC/$engine/AndroidManifest.xml
  [ ! -e $CHROMIUM_SRC/$engine/project.properties ] &&
    echo -e $PROJECT_PROPERTIES > $CHROMIUM_SRC/$engine/project.properties
done

printf "\nDone!\n\n"
