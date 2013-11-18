/*
* Copyright (c) 2013, The Linux Foundation. All rights reserved.
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions are
* met:
*     * Redistributions of source code must retain the above copyright
*       notice, this list of conditions and the following disclaimer.
*     * Redistributions in binary form must reproduce the above
*       copyright notice, this list of conditions and the following
*       disclaimer in the documentation and/or other materials provided
*       with the distribution.
*     * Neither the name of The Linux Foundation nor the names of its
*       contributors may be used to endorse or promote products derived
*       from this software without specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED "AS IS" AND ANY EXPRESS OR IMPLIED
* WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT
* ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS
* BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
* CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
* SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
* BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
* WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
* OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
* IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
#include "base/compiler_specific.h"
#include "build/build_config.h"

#include "net/libnetxt/libnetxt_base.h"
#include "net/libnetxt/plugin_api.h"
#include "base/command_line.h"

#include <stdlib.h>
#include <algorithm>

static const char* kPropNameCmdLineEnabled = "net.libnetxt.cmdline.on";
static bool cmd_line_enabled_ = false;

static const char* kPropNameVerboseLevel = "net.sh.verbose";  //.sh is for legacy reasons. Should be updated to libnetxt when testing is ready
static LibnetxtVerboseLevel verbose_level_ = LIBNETXT_VERBOSE_LEVEL_DISABLED;

static void InitOnce() {
    static bool initialized = false;
    if (!initialized) {
        char value[PROPERTY_VALUE_MAX] = {'\0'};

        initialized = true;
        LIBNETXT_API(SysPropertyGet)(kPropNameCmdLineEnabled, value, "1");
        cmd_line_enabled_ = (bool)atoi(value);
        LIBNETXT_LOGD("Libnetxt command line control is %s", (cmd_line_enabled_)?"ON":"OFF");

        LIBNETXT_API(SysPropertyGet)(kPropNameVerboseLevel, value, "0");
        verbose_level_= (LibnetxtVerboseLevel)atoi(value);
        LIBNETXT_LOGD("Libnetxt verbose level is %d",verbose_level_);

    }
}

int libnetxt_property_get(const char *key, char *value, const char *default_value) {
    int ret = 0;
    value[PROPERTY_VALUE_MAX-1] = '\0';

    InitOnce();
    if (cmd_line_enabled_) {
        std::string switch_string(key);

        std::replace(switch_string.begin(), switch_string.end(), '.', '-');
        std::string value_string = CommandLine::ForCurrentProcess()->GetSwitchValueASCII(switch_string);
        if (!value_string.empty()) {
            ret = strlen(strncpy(value, value_string.c_str(), PROPERTY_VALUE_MAX));
            return ret;
        }
    }
    ret = LIBNETXT_SYSPROP_GET(key, value, default_value);
    return ret;
}

bool libnetxt_isVerboseEnabled() {
   return verbose_level_ != LIBNETXT_VERBOSE_LEVEL_DISABLED;
}
