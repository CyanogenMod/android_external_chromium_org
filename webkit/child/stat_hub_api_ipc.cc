/*
* Copyright (c) 2012-2014, The Linux Foundation. All rights reserved.
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

#include <map>
#include <set>
#include <stdio.h>
#include <string>
#include <unistd.h>

#include "base/basictypes.h"
#include "base/memory/ref_counted.h"
#include "base/threading/thread.h"
#include "base/time/time.h"
#include "content/renderer/render_thread_impl.h"
#include "url/gurl.h"
#include "net/base/net_errors.h"
#include "net/stat_hub/stat_hub_cmd.h"
#include "net/stat_hub/stat_hub_messages.h"
#include "net/stat_hub/stat_hub_api.h"
#include "net/stat_hub/stat_hub_cmd_api.h"

using content::RenderThread;

typedef std::multimap<unsigned int, StatHubCmd*> StatHubCmdMapType;
CR_DEFINE_STATIC_LOCAL(StatHubCmdMapType, stat_hub_cmd_map_ipc_,);

// ========================================================================
bool IpcStatHubCmdPush(StatHubCmd* cmd) {
    if(NULL!=cmd && IpcStatHubIsReady()) {
        stat_hub_cmd_map_ipc_.insert(std::pair<unsigned int, StatHubCmd*>(cmd->GetCookie(), cmd));
        return true;
    }
    return false;
}

// ========================================================================
StatHubCmd* IpcStatHubCmdPop(unsigned int cookie, StatHubCmdType cmd_id, StatHubActionType action) {
    unsigned int cmd_mask = IpcStatHubGetCmdMask();

    if ((cmd_id>SH_CMD_USER_DEFINED || (cmd_mask&(1<<cmd_id))) && IpcStatHubIsReady()) {
        StatHubCmdMapType::iterator iter;
        std::pair<StatHubCmdMapType::iterator,StatHubCmdMapType::iterator> ret = stat_hub_cmd_map_ipc_.equal_range(cookie);

        for (iter=ret.first; iter!=ret.second; ++iter) {
            StatHubCmd* cmd = (*iter).second;
            if (NULL!=cmd && cmd->GetCmd()==cmd_id && cmd->GetAction()==action) {
                stat_hub_cmd_map_ipc_.erase(iter);
                return cmd;
            }
        }
    }
    return NULL;
}

// ======================== StatHub Sync IPC Interface Forwarders ================================
LIBNETXT_API_IPC_FORWARDER_1(StatHub, Hash, unsigned int, const char*)

// ================================ StatHub Sync IPC Interface ====================================
static int is_perf_enabled_ = -1;
static int is_cache_enabled_ = -1;
static int is_preloader_enabled_ = -1;
static int is_ready_ = -1;

static bool is_cmd_mask_ready_ = false;
static unsigned int cmd_mask_ = 0;

// ========================================================================
unsigned int IpcStatHubGetCmdMask() {
    if (is_cmd_mask_ready_) {
        return cmd_mask_;
    }
    if (RenderThread::Get() && RenderThread::Get()->Send(new StatHubMsg_GetCmdMask(&cmd_mask_))) {
        is_cmd_mask_ready_ = true;
    }
    return cmd_mask_;
}

// ========================================================================
bool IpcStatHubIsReady() {
    bool res = false;

    if (-1!=is_ready_) {
        return (is_ready_==1);
    }
    if (RenderThread::Get() && RenderThread::Get()->Send(new StatHubMsg_IsReady(&res))) {
        is_ready_ = res?1:0;
    }
    return res;
}

// ========================================================================
bool IpcStatHubIsPerfEnabled() {
    bool res = false;

    if (-1!=is_perf_enabled_) {
        return (is_perf_enabled_==1);
    }
    if (RenderThread::Get() && RenderThread::Get()->Send(new StatHubMsg_IsPerfEnabled(&res))) {
        is_perf_enabled_ = res?1:0;
    }
    return res;
}

// ========================================================================
bool IpcStatHubIsCacheEnabled() {
    bool res = false;

    if (-1!=is_cache_enabled_) {
        return (is_cache_enabled_==1);
    }
    if (RenderThread::Get() && RenderThread::Get()->Send(new StatHubMsg_IsCacheEnabled(&res))) {
        is_cache_enabled_ = res?1:0;
    }
    return res;
}

// ========================================================================
bool IpcStatHubIsPreloaderEnabled() {
    bool res = false;

    if (-1!=is_preloader_enabled_) {
        return (is_preloader_enabled_==1);
    }
    if (RenderThread::Get() && RenderThread::Get()->Send(new StatHubMsg_IsPreloaderEnabled(&res))) {
        is_preloader_enabled_ = res?1:0;
    }
    return res;
}

// ========================================================================
unsigned int IpcStatHubIsPreloaded(const char* url) {
    unsigned int res = 0;

    if (IpcStatHubIsReady() && IpcStatHubIsCacheEnabled()) {
        RenderThread::Get()->Send(new StatHubMsg_IsPreloaded(std::string(url), &res));
    }
    return res;
}

// ========================================================================
bool IpcStatHubGetPreloaded(const char* url, unsigned int get_from,
    std::string& headers, std::string& data, unsigned int& size) {
    bool res = false;

    if (IpcStatHubIsReady() && IpcStatHubIsCacheEnabled()) {
        RenderThread::Get()->Send(new StatHubMsg_GetPreloaded(std::string(url), get_from,
            &headers, &data, &size, &res));
    }
    return res;
}

// ========================================================================
bool IpcStatHubReleasePreloaded(const char* url) {
    bool res = false;

    if (IpcStatHubIsReady() && IpcStatHubIsCacheEnabled()) {
        RenderThread::Get()->Send(new StatHubMsg_ReleasePreloaded(std::string(url), &res));
    }
    return res;
}

// ================================ StatHub Async CMD IPC Interface ====================================
StatHubCmd* IpcStatHubCmdCreate(StatHubCmdType cmd_id, StatHubActionType action, unsigned int cookie) {
    unsigned int cmd_mask = IpcStatHubGetCmdMask();

    if ((cmd_id>SH_CMD_USER_DEFINED || (cmd_mask&(1<<cmd_id))) && IpcStatHubIsReady()) {
        return new StatHubCmd(cmd_id, action, cookie);
    }
    return NULL;
}

// ========================================================================
bool IpcStatHubCmdCommitInternal(StatHubCmd* cmd, unsigned int delay_ms) {
    if (!IpcStatHubIsReady() || !cmd) {
        return false;
    }
    //in case we need - start the client IPC thread (done once)
    cmd->SetCommitTimeStamp(StatHubTimeStamp::NowFromSystemTime());
    IPC::Message* msg = new StatHubMsg_Commit();
    //delay
    msg->WriteUInt32(delay_ms);
    //StatHubCmdType          cmd_;
    msg->WriteInt((int)cmd->cmd_);
    //StatHubActionType       action_;
    msg->WriteInt((int)cmd->action_);
    //unsigned int            cookie_;
    msg->WriteUInt32((int)cmd->cookie_);
    //unsigned int            referenced_;
    msg->WriteUInt32(cmd->referenced_);
    //performance
    //std::string             stat_;
    msg->WriteString(cmd->stat_);
    //StatHubTimeStamp        start_timestamp_;
    msg->WriteBytes(&cmd->start_timestamp_,sizeof(cmd->start_timestamp_));
    //StatHubTimeStamp        commit_timestamp_;
    msg->WriteBytes(&cmd->commit_timestamp_,sizeof(cmd->commit_timestamp_));
    //StatHubCmdParamsType    params_;
    msg->WriteInt(cmd->params_.size());

    for(unsigned int i=0;i<cmd->params_.size();i++) {
        //class member
        msg->WriteInt(cmd->params_[i]->param_size_);
        if (cmd->params_[i]->param_size_<=0) {
            msg->WriteInt((int)cmd->params_[i]->param_uint32_);
            msg->WriteBytes(&cmd->params_[i]->param_, sizeof(cmd->params_[i]->param_));
        } else {
            msg->WriteBytes(cmd->params_[i]->param_,cmd->params_[i]->param_size_);
        }
    }
    RenderThread::Get()->Send(msg);
    delete cmd;
    return true;
}

// ========================================================================
bool IpcStatHubCmdCommit(StatHubCmd* cmd) {
    return IpcStatHubCmdCommitInternal(cmd, 0);
}

// ========================================================================
bool IpcStatHubCmdCommitDelayed(StatHubCmd* cmd, unsigned int delay_ms) {
    return IpcStatHubCmdCommitInternal(cmd, delay_ms);
}
