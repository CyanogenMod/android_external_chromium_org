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

#include "build/build_config.h"
#include "base/compiler_specific.h"

#include <set>

#include "base/threading/thread.h"
#include "net/base/completion_callback.h"

#include "stat_hub.h"
#include "stat_hub_def.h"
#include "stat_hub_api.h"
#include "stat_hub_cmd.h"
#include "stat_hub_cmd_api.h"

typedef std::multimap<unsigned int, StatHubCmd*> StatHubCmdMapType;

// ============================ Local ===============================
StatHubCmdMapType& StatHubCmdMapGetInstance() {
    CR_DEFINE_STATIC_LOCAL(StatHubCmdMapType, stat_hub_cmd_map_, ());
    return stat_hub_cmd_map_;
}

// ============================ StatHub Functional Interface Proxies ===============================
void CmdCommitProxy(StatHubCmd* cmd) {
    stat_hub::StatHub::GetInstance()->Cmd(cmd);
    StatHubCmd::Release(cmd);
}

// ================================ StatHub CMD Interface ====================================
extern unsigned int STAT_HUB_API(GetCmdMask)() {
    return stat_hub::StatHub::GetInstance()->GetCmdMask();
}

StatHubCmd* STAT_HUB_API(CmdCreate)(StatHubCmdType cmd_id, StatHubActionType action, unsigned int cookie) {
    unsigned int cmd_mask = STAT_HUB_API(GetCmdMask)();

    if ((cmd_id>SH_CMD_USER_DEFINED || (cmd_mask&(1<<cmd_id))) && StatHubIsReady()) {
        return new StatHubCmd(cmd_id, action, cookie);
    }
    return NULL;
}

bool STAT_HUB_API(CmdAddParamAsUint32)(StatHubCmd* cmd, unsigned int param) {
    if (cmd) {
        cmd->AddParamAsUint32(param);
        return true;
    }
    return false;
}

bool STAT_HUB_API(CmdAddParamAsString)(StatHubCmd* cmd, const char* param) {
    if (cmd) {
        cmd->AddParamAsString(param);
        return true;
    }
    return false;
}

bool STAT_HUB_API(CmdAddParamAsBuf)(StatHubCmd* cmd, const void* param, unsigned int size) {
    if (cmd) {
        cmd->AddParamAsBuf(param, size);
        return true;
    }
    return false;
}

bool STAT_HUB_API(CmdTimeStamp)(StatHubCmd* cmd) {
    if (cmd) {
        cmd->SetStartTimeStamp(StatHubTimeStamp::NowFromSystemTime());
        return true;
    }
    return false;
}

bool STAT_HUB_API(CmdAddParamAsBool)(StatHubCmd* cmd, bool param) {
    if (cmd) {
        cmd->AddParamAsBool(param);
        return true;
    }
    return false;
}

unsigned int STAT_HUB_API(CmdGetParamAsUint32)(StatHubCmd* cmd, unsigned int param_index) {
    if (cmd) {
        return cmd->GetParamAsUint32(param_index);
    }
    return 0;
}

const char* STAT_HUB_API(CmdGetParamAsString)(StatHubCmd* cmd, unsigned int param_index) {
    if (cmd) {
        return cmd->GetParamAsString(param_index);
    }
    return NULL;
}

void* STAT_HUB_API(CmdGetParamAsBuf)(StatHubCmd* cmd,unsigned int param_index, unsigned int& size) {
    if(cmd) {
        return cmd->GetParamAsBuf(param_index, size);
    }
    size = 0;
    return NULL;
}

bool STAT_HUB_API(CmdGetParamAsBool)(StatHubCmd* cmd,unsigned int param_index) {
    if(cmd) {
        return cmd->GetParamAsBool(param_index);
    }
    return false;
}

bool STAT_HUB_API(CmdCommit)(StatHubCmd* cmd) {
    if(NULL!=cmd) {
        if (STAT_HUB_IS_VERBOSE_LEVEL_DEBUG && STAT_HUB_DEV_LOG_ENABLED) {
            LIBNETXT_LOGD("STAT_HUB - CmdCommit CMD:%d Action:%d", cmd->GetCmd(), cmd->GetAction());
        }
        cmd->SetCommitTimeStamp(StatHubTimeStamp::NowFromSystemTime());
        stat_hub::StatHub::GetInstance()->GetThread()->message_loop()->PostTask( FROM_HERE, base::Bind(
            &CmdCommitProxy, cmd));
        return true;
    }
    return false;
}

bool STAT_HUB_API(CmdCommitDelayed)(StatHubCmd* cmd, unsigned int delay_ms) {
    if(NULL!=cmd) {
        if (STAT_HUB_IS_VERBOSE_LEVEL_DEBUG && STAT_HUB_DEV_LOG_ENABLED) {
            LIBNETXT_LOGD("STAT_HUB - CmdCommitDelayed CMD:%d Action:%d", cmd->GetCmd(), cmd->GetAction());
        }
        cmd->SetCommitTimeStamp(StatHubTimeStamp::NowFromSystemTime());
        stat_hub::StatHub::GetInstance()->GetThread()->message_loop()->PostDelayedTask( FROM_HERE, base::Bind(
            &CmdCommitProxy, cmd), base::TimeDelta::FromMilliseconds(delay_ms));
        return true;
    }
    return false;
}

bool STAT_HUB_API(CmdCommitSync)(StatHubCmd* cmd) {
    if(NULL!=cmd) {
        if (STAT_HUB_IS_VERBOSE_LEVEL_DEBUG && STAT_HUB_DEV_LOG_ENABLED) {
            LIBNETXT_LOGD("STAT_HUB - CmdCommitSync CMD:%d Action:%d", cmd->GetCmd(), cmd->GetAction());
        }
        cmd->SetCommitTimeStamp(StatHubTimeStamp::NowFromSystemTime());
        return stat_hub::StatHub::GetInstance()->Cmd(cmd, true);
    }
    return false;
}

bool STAT_HUB_API(CmdPush)(StatHubCmd* cmd) {
    if(NULL!=cmd) {
        StatHubCmdMapGetInstance().insert(std::pair<unsigned int, StatHubCmd*>(cmd->GetCookie(), cmd));
        return true;
    }
    return false;
}

StatHubCmd* STAT_HUB_API(CmdPop)(unsigned int cookie, StatHubCmdType cmd_id, StatHubActionType action) {
    unsigned int cmd_mask = STAT_HUB_API(GetCmdMask)();

    if ((cmd_id>SH_CMD_USER_DEFINED || (cmd_mask&(1<<cmd_id))) && StatHubIsReady()) {
        StatHubCmdMapType::iterator iter;
        std::pair<StatHubCmdMapType::iterator,StatHubCmdMapType::iterator> ret = StatHubCmdMapGetInstance().equal_range(cookie);

        for (iter=ret.first; iter!=ret.second; ++iter) {
            StatHubCmd* cmd = (*iter).second;
            if (NULL!=cmd && cmd->GetCmd()==cmd_id && cmd->GetAction()==action) {
                StatHubCmdMapGetInstance().erase(iter);
                return cmd;
            }
        }
    }
    return NULL;
}

bool STAT_HUB_API(CmdRelease)(StatHubCmd* cmd) {
    StatHubCmd::Release(cmd);
    return true;
}

bool STAT_HUB_API(CmdResetParams)(StatHubCmd* cmd) {
    if (cmd) {
        cmd->ResetParams();
        return true;
    }
    return true;
}
