#Copyright (c) 2013-2014, The Linux Foundation. All rights reserved.
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

#!/usr/bin/env python

import subprocess
import sys
import os

def abs_path():
    return os.path.dirname( __file__ )

# Parse args:
#   arg1 : base path to prebilt libraries
#   arg2 : '0' if gcc build, '1' if llvm (clang)
rel_prebuilt_path = os.path.relpath(sys.argv[1])
is_llvm_build = sys.argv[2]
target_arch = sys.argv[3]  # arm or arm64

# Setup default version based on whether the current build is clang or not
if is_llvm_build != '0':
    default_skia_compiler = 'llvm'
else:
    default_skia_compiler = 'gcc'

if target_arch == 'arm':
    debugDirName = "Debug"
    releaseDirName = "Release"
elif target_arch == 'arm64':
    debugDirName = "Debug_x64"
    releaseDirName = "Release_x64"
else:
    print "Error: bad target arch [" + target_arch + "]"

# Potentially override default version based on ENV var
skia_compiler = os.getenv('SWE_SKIA_PREBUILT_COMPILER', default_skia_compiler)

# Check to see if the prebuilts are in the legacy config
if os.path.isdir(rel_prebuilt_path + "/Debug") and os.path.isdir(rel_prebuilt_path + "/Release"):
    # Legacy will never have 64 bit
    debug_lib   = rel_prebuilt_path + "/Debug/libsweskia.so"
    release_lib = rel_prebuilt_path + "/Release/libsweskia.so"
else:
    # Path to the libs
    debug_lib   = rel_prebuilt_path + "/" + skia_compiler + "/" + debugDirName   + "/libsweskia.so"
    release_lib = rel_prebuilt_path + "/" + skia_compiler + "/" + releaseDirName + "/libsweskia.so"

    # If skia_compiler is llvm, there is a chance that the llvm libraries are not
    # yet available.  Default back to gcc in this case.
    if not os.path.isfile(debug_lib) or not os.path.isfile(release_lib) :
        skia_compiler = 'gcc'
        debug_lib   = rel_prebuilt_path + "/" + skia_compiler + "/" + debugDirName   + "/libsweskia.so"
        release_lib = rel_prebuilt_path + "/" + skia_compiler + "/" + releaseDirName + "/libsweskia.so"

# Set dest path
path = abs_path()

# Finally, perform the copy.
if "out/Debug" in path:
    print "Debug " + skia_compiler + " " + target_arch
    subprocess.call(["cp", debug_lib, path+"/lib"]);
elif "out/Release" in path:
    print "Release " + skia_compiler + " " + target_arch
    subprocess.call(["cp", release_lib, path+"/lib"]);
else:
    print "Error unknown path"

