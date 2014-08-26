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

#include "net/libnetxt/plugin_api_ptr.h"
#include "net/stat_hub/stat_hub_net_plugin_ptr.h"

//=========================================================================
void InitStatHubLibnetxtPluginApi(LibnetxtPluginApi* plugin_api) {
    // ================================ StatHub Interface ====================================
    LIBNETXT_API_PTR_IMP(plugin_api, StatHub, Hash)
    LIBNETXT_API_PTR_IMP(plugin_api, StatHub, GetVerboseLevel)
    LIBNETXT_API_PTR_IMP(plugin_api, StatHub, IsVerboseEnabled)
    LIBNETXT_API_PTR_IMP(plugin_api, StatHub, IsPreloaderEnabled)
    LIBNETXT_API_PTR_IMP(plugin_api, StatHub, Fetch)
    LIBNETXT_API_PTR_IMP(plugin_api, StatHub, Preconnect)
    LIBNETXT_API_PTR_IMP(plugin_api, StatHub, GetHttpCache)
    LIBNETXT_API_PTR_IMP(plugin_api, StatHub, GetIoMessageLoop)
    LIBNETXT_API_PTR_IMP(plugin_api, StatHub, IsPerfEnabled)

    // ================================ StatHub CMD Interface ====================================
    LIBNETXT_API_PTR_IMP(plugin_api, StatHub, CmdTimeStamp)
    LIBNETXT_API_PTR_IMP(plugin_api, StatHub, CmdRelease)
    LIBNETXT_API_PTR_IMP(plugin_api, StatHub, CmdResetParams)
    LIBNETXT_API_PTR_IMP(plugin_api, StatHub, CmdCreate)
    LIBNETXT_API_PTR_IMP(plugin_api, StatHub, CmdCommit)
    LIBNETXT_API_PTR_IMP(plugin_api, StatHub, CmdCommitDelayed)
    LIBNETXT_API_PTR_IMP(plugin_api, StatHub, CmdCommitSync)

    // ================================ StatHub SQL Interface ====================================
    LIBNETXT_API_CPP_PTR_IMP(plugin_api, StatHub, sql, Connection, BeginTransaction)
    LIBNETXT_API_CPP_PTR_IMP(plugin_api, StatHub, sql, Connection, CommitTransaction)
    LIBNETXT_API_CPP_PTR_IMP(plugin_api, StatHub, sql, Connection, DoesTableExist)
    LIBNETXT_API_CPP_PTR_IMP(plugin_api, StatHub, sql, Connection, Execute)

    LIBNETXT_API_CPP_PTR_IMP(plugin_api, StatHub, sql, Statement, Step)
    LIBNETXT_API_CPP_PTR_IMP(plugin_api, StatHub, sql, Statement, Run)
    LIBNETXT_API_CPP_PTR_IMP(plugin_api, StatHub, sql, Statement, Reset)

    LIBNETXT_API_CPP_PTR_IMP(plugin_api, StatHub, sql, Statement, ColumnInt)
    LIBNETXT_API_CPP_PTR_IMP(plugin_api, StatHub, sql, Statement, ColumnInt64)
    LIBNETXT_API_CPP_PTR_IMP(plugin_api, StatHub, sql, Statement, ColumnBool)
    LIBNETXT_API_CPP_PTR_IMP(plugin_api, StatHub, sql, Statement, ColumnString)
    LIBNETXT_API_CPP_PTR_IMP(plugin_api, StatHub, sql, Statement, BindInt)
    LIBNETXT_API_CPP_PTR_IMP(plugin_api, StatHub, sql, Statement, BindInt64)
    LIBNETXT_API_CPP_PTR_IMP(plugin_api, StatHub, sql, Statement, BindBool)
    LIBNETXT_API_CPP_PTR_IMP(plugin_api, StatHub, sql, Statement, BindCString)

    LIBNETXT_API_PTR_IMP(plugin_api, StatHub, GetStatement)
    LIBNETXT_API_PTR_IMP(plugin_api, StatHub, ReleaseStatement)

    LIBNETXT_API_PTR_IMP(plugin_api, StatHub, GetDBmetaData)
    LIBNETXT_API_PTR_IMP(plugin_api, StatHub, SetDBmetaData)
}
