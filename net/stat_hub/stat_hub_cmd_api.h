/*
* Copyright (c) 2011-2013, The Linux Foundation. All rights reserved.
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

#ifndef STAT_HUB_CMD_API_H_
#define STAT_HUB_CMD_API_H_

#include "net/stat_hub/stat_hub_all_api_def.h"
#include "net/stat_hub/stat_hub_cmd.h"

// ================================ StatHub CMD Interface ====================================
LIBNETXT_API_DEF_1(StatHub, CmdTimeStamp, bool, StatHubCmd*)
LIBNETXT_API_DEF_1(StatHub, CmdRelease, bool, StatHubCmd*)
LIBNETXT_API_DEF_1(StatHub, CmdResetParams, bool, StatHubCmd*)

LIBNETXT_API_DEF_0(StatHub, GetCmdMask, unsigned int )
LIBNETXT_API_DEF_3(StatHub, CmdCreate, StatHubCmd*, StatHubCmdType, StatHubActionType, unsigned int)
LIBNETXT_API_DEF_1(StatHub, CmdCommit, bool, StatHubCmd*)
LIBNETXT_API_DEF_2(StatHub, CmdCommitDelayed, bool, StatHubCmd*, unsigned int)
LIBNETXT_API_DEF_1(StatHub, CmdCommitSync, bool, StatHubCmd*)
LIBNETXT_API_DEF_1(StatHub, CmdPush, bool, StatHubCmd*)
LIBNETXT_API_DEF_3(StatHub, CmdPop, StatHubCmd*, unsigned int, StatHubCmdType, StatHubActionType)

#endif /* STAT_HUB_CMD_API_H_ */
