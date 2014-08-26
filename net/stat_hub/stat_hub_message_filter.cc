/*
* Copyright (c) 2012-2014 The Linux Foundation. All rights reserved.
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

#include "ipc/ipc_message_macros.h"

#include "stat_hub_message_filter.h"
#include "stat_hub_messages.h"
#include "stat_hub_api.h"
#include "stat_hub_cmd.h"
#include "stat_hub_cmd_api.h"

namespace stat_hub {

// ========================================================================
StatHubMessageFilter::StatHubMessageFilter():
    BrowserMessageFilter(StatHubMsgStart) {

}

// ========================================================================
void StatHubMessageFilter::OnChannelConnected(int32 peer_pid) {
    BrowserMessageFilter::OnChannelConnected(peer_pid);
}

// ========================================================================
StatHubMessageFilter::~StatHubMessageFilter() {
}

// ========================================================================
bool StatHubMessageFilter::OnMessageReceived(const IPC::Message& message) {
    bool handled = true;
    IPC_BEGIN_MESSAGE_MAP(StatHubMessageFilter, message)
        IPC_MESSAGE_HANDLER_GENERIC(StatHubMsg_Commit, OnCommit(message))
        IPC_MESSAGE_HANDLER(StatHubMsg_GetCmdMask, OnGetCmdMask)
        IPC_MESSAGE_HANDLER(StatHubMsg_IsReady, OnIsReady)
        IPC_MESSAGE_HANDLER(StatHubMsg_IsPerfEnabled, OnIsPerfEnabled)
        IPC_MESSAGE_HANDLER(StatHubMsg_IsCacheEnabled, OnIsCacheEnabled)
        IPC_MESSAGE_HANDLER(StatHubMsg_IsPreloaderEnabled, OnIsPreloaderEnabled)
        IPC_MESSAGE_HANDLER(StatHubMsg_IsPreloaded, OnIsPreloaded)
        IPC_MESSAGE_HANDLER(StatHubMsg_GetPreloaded, OnGetPreloaded)
        IPC_MESSAGE_HANDLER(StatHubMsg_ReleasePreloaded, OnReleasePreloaded)
        IPC_MESSAGE_UNHANDLED(handled = false)
    IPC_END_MESSAGE_MAP()
    return handled;
}

// ========================================================================
void StatHubMessageFilter::OnCommit(const IPC::Message& msg) {
    //StatHubCmd* cmd = new StatHubCmd();
    unsigned int delay_ms = 0;
    StatHubCmdType cmd_id = SH_CMD_USER_DEFINED;
    StatHubActionType action_id = SH_ACTION_LAST;
    unsigned int cookie = 0;
    bool rs = true;
    PickleIterator iter(msg);
    //delay
    rs = rs && msg.ReadUInt32(&iter, &delay_ms);
    //StatHubCmdType          cmd_;
    rs = rs && msg.ReadInt(&iter, (int*)&cmd_id);
    //StatHubActionType       action_;
    rs = rs && msg.ReadInt(&iter, (int*)&action_id);
    //unsigned int            cookie_;
    rs = rs && msg.ReadUInt32(&iter, &cookie);
    StatHubCmd* cmd = STAT_HUB_API(CmdCreate)(cmd_id, action_id, cookie);
    if (cmd) {
        //unsigned int            referenced_;
        rs = rs && msg.ReadUInt32(&iter, &cmd->referenced_);
        //performance
        //std::string             stat_;
        rs = rs && msg.ReadString(&iter, &cmd->stat_);
        //StatHubTimeStamp        start_timestamp_;
        const char* dataPtr = NULL;
        rs = rs && msg.ReadBytes(&iter, &dataPtr, sizeof(cmd->start_timestamp_));
        memcpy(&cmd->start_timestamp_, dataPtr, sizeof(cmd->start_timestamp_));
        //StatHubTimeStamp        commit_timestamp_;
        rs = rs && msg.ReadBytes(&iter,&dataPtr,sizeof(cmd->commit_timestamp_));
        memcpy(&cmd->commit_timestamp_, dataPtr, sizeof(cmd->commit_timestamp_));

        //StatHubCmdParamsType    params_;
        int size=0;
        rs = rs && msg.ReadInt(&iter,&size);
        for(int i=0;i<size;i++) {

            int param_size=0;
            rs = rs && msg.ReadInt(&iter, &param_size);
            if (param_size<=0) {
                int value = 0;
                void* ptr = NULL;
                const char* dataPtr = NULL;

                rs = rs && msg.ReadInt(&iter, &value);
                rs = rs && msg.ReadBytes(&iter, &dataPtr, sizeof(ptr));
                ptr = *((void**) dataPtr);
                cmd->params_.push_back(new StatHubCmd::StatHubCmdParam((unsigned int)value, ptr));
            } else {
                const char* dataPtr = NULL;

                rs = rs && msg.ReadBytes(&iter, &dataPtr, param_size);
                cmd->params_.push_back(new StatHubCmd::StatHubCmdParam((void*)dataPtr, param_size));
            }
        }

        if (delay_ms>0) {
            STAT_HUB_API(CmdCommitDelayed)(cmd, delay_ms);
        } else {
            STAT_HUB_API(CmdCommit)(cmd);
        }
    }
}

// ========================================================================
void StatHubMessageFilter::OnGetCmdMask(unsigned int* ret) {
    *ret = STAT_HUB_API(GetCmdMask)();
}

// ========================================================================
void StatHubMessageFilter::OnIsReady(bool* ret) {
    *ret = STAT_HUB_API(IsReady)();
}

// ========================================================================
void StatHubMessageFilter::OnIsPerfEnabled(bool* ret) {
    *ret = STAT_HUB_API(IsPerfEnabled)();
}

// ========================================================================
void StatHubMessageFilter::OnIsCacheEnabled(bool* ret) {
    *ret = STAT_HUB_API(IsCacheEnabled)();
}

// ========================================================================
void StatHubMessageFilter::OnIsPreloaderEnabled(bool* ret) {
    *ret = STAT_HUB_API(IsPreloaderEnabled)();
}

// ========================================================================
void StatHubMessageFilter::OnIsPreloaded(std::string url, unsigned int* ret) {
    *ret = STAT_HUB_API(IsPreloaded)(url.c_str());
}

// ========================================================================
void StatHubMessageFilter::OnGetPreloaded(std::string url, unsigned int get_from,
        std::string* headers, std::string* data, unsigned int* size, bool* ret) {
    unsigned int tmp_size = 0;
    *ret = STAT_HUB_API(GetPreloaded)(url.c_str(), get_from, *headers, *data, tmp_size);
    if (*ret) {
        *size = tmp_size;
    }
}

// ========================================================================
void StatHubMessageFilter::OnReleasePreloaded(std::string url, bool* ret) {
    *ret = STAT_HUB_API(ReleasePreloaded)(url.c_str());
}

}
