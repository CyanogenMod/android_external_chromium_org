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

#ifndef STAT_HUB_CMD_DEF_H_
#define STAT_HUB_CMD_DEF_H_

typedef enum {
    SH_CMD_GP_EVENT,                // 0
    SH_CMD_WK_MAIN_URL,             // 1
    SH_CMD_WK_MEMORY_CACHE,         // 2
    SH_CMD_WK_RESOURCE,             // 3
    SH_CMD_WK_PAGE,                 // 4
    SH_CMD_WK_INSPECTOR_RECORD,     // 5
    SH_CMD_WK_JS_SEQ,               // 6
    SH_CMD_JAVA_GP_EVENT,           // 7
    SH_CMD_CH_URL_REQUEST,          // 8
    SH_CMD_CH_TRANS_NET,            // 9
    SH_CMD_CH_TRANS_CACHE,          // 10
    SH_CMD_TCPIP_SOCKET,            // 11
    SH_CMD_PRELOADER,               // 12
    SH_CMD_SELF,                    // 13
    SH_CMD_TBD_14,                  // 14
    SH_CMD_TBD_15,                  // 15
    SH_CMD_TBD_16,                  // 16
    SH_CMD_TBD_17,                  // 17
    SH_CMD_TBD_18,                  // 18
    SH_CMD_TBD_19,                  // 19
    SH_CMD_TBD_20,                  // 20
    SH_CMD_TBD_21,                  // 21
    SH_CMD_TBD_22,                  // 22
    SH_CMD_TBD_23,                  // 23
    SH_CMD_TBD_24,                  // 24
    SH_CMD_TBD_25,                  // 25
    SH_CMD_TBD_26,                  // 26
    SH_CMD_TBD_27,                  // 27
    SH_CMD_TBD_28,                  // 28
    SH_CMD_TBD_29,                  // 29
    SH_CMD_TBD_30,                  // 30
    SH_CMD_TBD_31,                  // 31

    SH_CMD_USER_DEFINED = 32        // 32
} StatHubCmdType;

typedef enum {
    SH_ACTION_NONE,
    SH_ACTION_WILL_START,
    SH_ACTION_DID_START,
    SH_ACTION_WILL_FINISH,
    SH_ACTION_DID_FINISH,
    SH_ACTION_WILL_START_LOAD,
    SH_ACTION_DID_START_LOAD,
    SH_ACTION_WILL_FINISH_LOAD,
    SH_ACTION_DID_FINISH_LOAD,
    SH_ACTION_WILL_SEND_REQUEST,
    SH_ACTION_DID_SEND_REQUEST,
    SH_ACTION_WILL_RECEIVE_RESPONSE,
    SH_ACTION_DID_RECEIVE_RESPONSE,
    SH_ACTION_STATUS,
    SH_ACTION_CLEAR,
    SH_ACTION_CREATE,
    SH_ACTION_CONNECT,
    SH_ACTION_CLOSE,
    SH_ACTION_WILL_START_READ,
    SH_ACTION_DID_FINISH_READ,
    SH_ACTION_WRITE,
    SH_ACTION_FETCH_DELAYED,
    SH_ACTION_PREFETCH,
    SH_ACTION_JS_SEQ,
    SH_ACTION_IS_PRELOADER_ENABLED,
    SH_ACTION_IS_PRELOADED,
    SH_ACTION_GET_PRELOADED,
    SH_ACTION_RELEASE_PRELOADED,
    SH_ACTION_ON_LOAD,
    SH_ACTION_FIRST_PIXEL,
    SH_ACTION_PROGRESS_UPDATE,
    SH_ACTION_PAGELOAD_WILL_START,
    SH_ACTION_PAGELOAD_DID_START,
    SH_ACTION_PAGELOAD_DID_FINISH,
    SH_ACTION_IS_CACHE_ENABLED,

    SH_ACTION_LAST,
} StatHubActionType;

#endif /* STAT_HUB_CMD_DEF_H_ */
