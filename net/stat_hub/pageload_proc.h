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
#ifndef NET_PL_PROC_H_
#define NET_PL_PROC_H_
#pragma once

#include "base/compiler_specific.h"
#include "build/build_config.h"
#include "net/stat_hub/stat_hub.h"

namespace pl_proc {

typedef enum {
    REPORT_ON_BOTH,
    REPORT_ON_PROGRESS_DONE,
    REPORT_ON_ON_LOAD
} ReportOnEvent;

class PageLoadProcessor : public stat_hub::StatProcessor {
public:
    PageLoadProcessor();
    virtual ~PageLoadProcessor() {};

    bool OnInit(sql::Connection* db);
    bool OnGetProcInfo(std::string& name, std::string& version);
    bool OnGetCmdMask(unsigned int& cmd_mask);
    bool OnFlushDb(sql::Connection* db);

    bool PlProc_CmdHandlerWkPageLoad(StatHubCmd* cmd);

private:

    bool CheckUrl(const char* url);
    bool CheckDone(StatHubCmd* cmd, bool failed);

    bool log_all_frames_;
    ReportOnEvent report_on_;
    bool progress_report_enabled_;
};

} //namespace pl_proc

#endif  // NET_PL_PROC_H_
