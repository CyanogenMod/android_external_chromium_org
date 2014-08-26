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

#include "base/time/time.h"

#include "net/stat_hub/stat_hub_api.h"
#include "net/stat_hub/stat_hub_cmd_api.h"

// ================================ StatHub Interface ====================================
LIBNETXT_API_PROXY_IMP_1(StatHub, Hash, unsigned int, const char*)
LIBNETXT_API_PROXY_IMP_0(StatHub, GetVerboseLevel, StatHubVerboseLevel)
LIBNETXT_API_PROXY_IMP_0(StatHub, IsVerboseEnabled, bool)
LIBNETXT_API_PROXY_IMP_0(StatHub, IsPreloaderEnabled, bool)
LIBNETXT_API_PROXY_IMP_3(StatHub, Fetch, bool, unsigned int, net::HttpRequestInfo*, net::URLRequestContext*)
LIBNETXT_API_PROXY_IMP_2(StatHub, Preconnect, bool, const char*, unsigned int)
LIBNETXT_API_PROXY_IMP_0(StatHub, GetHttpCache, net::HttpCache*)
LIBNETXT_API_PROXY_IMP_0(StatHub, GetIoMessageLoop, base::MessageLoop*)
LIBNETXT_API_PROXY_IMP_0(StatHub, IsPerfEnabled, bool)

// ================================ StatHub CMD Interface ====================================
LIBNETXT_API_PROXY_IMP_1(StatHub, CmdTimeStamp, bool, StatHubCmd*)
LIBNETXT_API_PROXY_IMP_1(StatHub, CmdRelease, bool, StatHubCmd*)
LIBNETXT_API_PROXY_IMP_1(StatHub, CmdResetParams, bool, StatHubCmd*)
LIBNETXT_API_PROXY_IMP_3(StatHub, CmdCreate, StatHubCmd*, StatHubCmdType, StatHubActionType, unsigned int)
LIBNETXT_API_PROXY_IMP_1(StatHub, CmdCommit, bool, StatHubCmd*)
LIBNETXT_API_PROXY_IMP_2(StatHub, CmdCommitDelayed, bool, StatHubCmd*, unsigned int)
LIBNETXT_API_PROXY_IMP_1(StatHub, CmdCommitSync, bool, StatHubCmd*)

// ================================ StatHub SQL Interface ====================================
LIBNETXT_API_CPP_PROXY_IMP_0(StatHub, sql, Connection, BeginTransaction, bool)
LIBNETXT_API_CPP_PROXY_IMP_0(StatHub, sql, Connection, CommitTransaction, bool)
LIBNETXT_API_CPP_PROXY_IMP_1(StatHub, sql, Connection, DoesTableExist, bool, const char*)
LIBNETXT_API_CPP_PROXY_IMP_1(StatHub, sql, Connection, Execute, bool, const char*)

LIBNETXT_API_CPP_PROXY_IMP_0(StatHub, sql, Statement, Step, bool)
LIBNETXT_API_CPP_PROXY_IMP_0(StatHub, sql, Statement, Run, bool)
LIBNETXT_API_CPP_PROXY_IMP_1V(StatHub, sql, Statement, Reset, void, bool)

LIBNETXT_API_CPP_PROXY_IMP_1(StatHub, sql, Statement, ColumnInt, int, int)
LIBNETXT_API_CPP_PROXY_IMP_1(StatHub, sql, Statement, ColumnInt64, int64, int)
LIBNETXT_API_CPP_PROXY_IMP_1(StatHub, sql, Statement, ColumnBool, bool, int)
LIBNETXT_API_CPP_PROXY_IMP_1(StatHub, sql, Statement, ColumnString, std::string, int)
LIBNETXT_API_CPP_PROXY_IMP_2(StatHub, sql, Statement, BindInt, bool, int, int)
LIBNETXT_API_CPP_PROXY_IMP_2(StatHub, sql, Statement, BindInt64, bool, int, int64)
LIBNETXT_API_CPP_PROXY_IMP_2(StatHub, sql, Statement, BindBool, bool, int, bool)
LIBNETXT_API_CPP_PROXY_IMP_2(StatHub, sql, Statement, BindCString, bool, int, const char*)

LIBNETXT_API_PROXY_IMP_3(StatHub, GetStatement, sql::Statement*, sql::Connection*, const sql::StatementID&, const char*)
LIBNETXT_API_PROXY_IMP_1(StatHub, ReleaseStatement, bool, sql::Statement*)

LIBNETXT_API_PROXY_IMP_2(StatHub, GetDBmetaData, bool, const char*, std::string&)
LIBNETXT_API_PROXY_IMP_2(StatHub, SetDBmetaData, bool, const char*, const char*)
