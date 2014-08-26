/*
* Copyright (c) 2012, 2013, The Linux Foundation. All rights reserved.
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

#ifndef STAT_HUB_MESSAGES_H_
#define STAT_HUB_MESSAGES_H_

#define IPC_MESSAGE_IMPL

#include "ipc/ipc_message_macros.h"

#define IPC_MESSAGE_START StatHubMsgStart

IPC_MESSAGE_CONTROL0(StatHubMsg_Commit)

IPC_SYNC_MESSAGE_CONTROL0_1(StatHubMsg_GetCmdMask,
        unsigned int
    )

IPC_SYNC_MESSAGE_CONTROL0_1(StatHubMsg_IsReady,
        bool
    )

IPC_SYNC_MESSAGE_CONTROL0_1(StatHubMsg_IsPerfEnabled,
        bool
    )

IPC_SYNC_MESSAGE_CONTROL0_1(StatHubMsg_IsCacheEnabled,
        bool
    )

IPC_SYNC_MESSAGE_CONTROL0_1(StatHubMsg_IsPreloaderEnabled,
        bool
    )

IPC_SYNC_MESSAGE_CONTROL1_1(StatHubMsg_IsPreloaded,
        std::string,
        unsigned int
    )

IPC_SYNC_MESSAGE_CONTROL2_4(StatHubMsg_GetPreloaded,
        std::string,
        unsigned int,
        std::string,
        std::string,
        unsigned int,
        bool
    )

IPC_SYNC_MESSAGE_CONTROL1_1(StatHubMsg_ReleasePreloaded,
        std::string,
        bool
    );

#endif  // STAT_HUB_MESSAGES_H_
