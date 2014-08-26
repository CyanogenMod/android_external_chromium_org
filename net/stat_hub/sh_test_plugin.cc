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

//Enable Ptr API
#define STAT_HUB_API_BY_PTR

#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/queue.h>
#include <unistd.h>
#include <sys/time.h>

#include "base/time/time.h"

#include "sh_test_plugin.h"

#include "net/stat_hub/stat_hub_cmd.h"
#include "net/stat_hub/stat_hub_api.h"
#include "net/stat_hub/stat_hub_cmd_api.h"
#include "net/libnetxt/plugin_api_ptr.h"

// Version numbers.
#define CURRENT_VERSION  "1.0.0"

#define SHTEST_PROC_REPORT_ON       REPORT_ON_BOTH
#define SHTEST_PROC_PROGRESS_REPORT 0
#define SHTEST_PROC_LOG_ALL_FRAMES  0

#ifdef LOG_TAG
    #undef LOG_TAG
    #define LOG_TAG "ShTest"
#endif

namespace sh_test_proc {

class FrameEntry {
public:
    FrameEntry(StatHubTimeStamp time_stamp) :
        time_stamp_(time_stamp),
        fp_time_stamp_(time_stamp),
        ol_time_stamp_(time_stamp),
        dn_time_stamp_(time_stamp),
        finish_received(false), on_load_received(false),
        bytes_received(0) {
    }

    StatHubTimeStamp    time_stamp_;
    StatHubTimeStamp    fp_time_stamp_;
    StatHubTimeStamp    ol_time_stamp_;
    StatHubTimeStamp    dn_time_stamp_;
    bool                finish_received;
    bool                on_load_received;
    unsigned int        bytes_received;
};

typedef std::map<void*, FrameEntry> FrameMapType;
static FrameMapType frame_map_;

static const char* kProcName = "stathub_test_plugin";

static const char* kPropNameLogAllFrames = "net.sh.test.logallframes";
static const char* kPropNameReportOn = "net.sh.test.reporton";
static const char* kPropNameProgressReportEnabled = "net.sh.test.progrrepen";

//=========================================================================
//                 Dynamically Loadable PlugIn Interface
//=========================================================================
#if defined(STAT_HUB_DL_PLUGIN_IF_EN)
    extern "C" void* OnCreate()
        __attribute__ ((visibility ("default")));

    void* OnCreate() {
        return (void*)new StatHubTestPlugin;
    }
#endif //defined(STAT_HUB_DL_PLUGIN_IF_EN)

//=========================================================================
//                  Processor Implementation
//=========================================================================

StatHubTestPlugin::StatHubTestPlugin():
        log_all_frames_(false),
        report_on_(SHTEST_PROC_REPORT_ON),
        progress_report_enabled_(false) {
}

// ========================================================================
bool StatHubTestPlugin::OnGetProcInfo(std::string& name, std::string& version) {
    name = kProcName;
    version = CURRENT_VERSION;
    return true;
}

// ========================================================================
bool StatHubTestPlugin::OnInit(sql::Connection* db) {
    char value[PROPERTY_VALUE_MAX] = {'\0'};

    LIBNETXT_API(SysPropertyGet)(kPropNameLogAllFrames, value, PROP_VAL_TO_STR(SHTEST_PROC_LOG_ALL_FRAMES));
    log_all_frames_ = (bool)atoi(value);
    LIBNETXT_API(SysPropertyGet)(kPropNameReportOn, value, PROP_VAL_TO_STR(SHTEST_PROC_REPORT_ON));
    report_on_ = (ReportOnEvent)atoi(value);
    LIBNETXT_API(SysPropertyGet)(kPropNameProgressReportEnabled, value, PROP_VAL_TO_STR(SHTEST_PROC_PROGRESS_REPORT));
    progress_report_enabled_ = (bool)atoi(value);

    if (STAT_HUB_API(IsVerboseEnabled)()) {
        LIBNETXT_LOGI("SHTEST_PROC - Log all frames: %s", log_all_frames_?"Enabled":"Disabled");
        LIBNETXT_LOGI("SHTEST_PROC - Report On: %s", report_on_?"ProgressEnd":"OnLoad");
        LIBNETXT_LOGI("SHTEST_PROC - Progress report: %s", progress_report_enabled_?"Enabled":"Disabled");
    }
    return true;
}

// ========================================================================
bool StatHubTestPlugin::OnFlushDb(sql::Connection* db) {
    //use the event to detect and clear stuck pages
    if (STAT_HUB_IS_VERBOSE_LEVEL_DEBUG) {
        for (FrameMapType::iterator frame_iter = frame_map_.begin(); frame_iter!=frame_map_.end(); ++frame_iter) {
            LIBNETXT_LOGD("SHTEST_PROC - Stuck - Id: %p", frame_iter->first);
        }
    }
    frame_map_.clear();
    return true;
}

// ========================================================================
bool StatHubTestPlugin::CheckUrl(const char* url) {
    std::string url_string = url;
    if (url_string=="swappedout://" || url_string.empty()) {
        return false;
    }
    return true;
}

// ========================================================================
bool StatHubTestPlugin::CheckDone(StatHubCmd* cmd, bool failed) {
    bool done = false;
    void* page_id = cmd->GetParamAsPtr(0);

    FrameMapType::iterator frame_iter = frame_map_.find(page_id);
    if (frame_iter != frame_map_.end()) {
        StatHubTimeStamp commit_timestamp = cmd->GetCommitTimeStamp();

        if (failed) {
            done = true;
        }
        else {
            if ((frame_iter->second.finish_received && frame_iter->second.on_load_received) &&
                    REPORT_ON_BOTH==report_on_) {
                done = true;
                frame_iter->second.finish_received = false;
                frame_iter->second.on_load_received = false;
            } else {
                if (frame_iter->second.finish_received &&  REPORT_ON_PROGRESS_DONE==report_on_) {
                    done = true;
                    frame_iter->second.finish_received = false;
                }
                else {
                    if (frame_iter->second.on_load_received &&  REPORT_ON_ON_LOAD==report_on_) {
                        done = true;
                        frame_iter->second.on_load_received = false;
                    }
                }
            }
        }
        if (done) {
            const char* url = cmd->GetParamAsString(2);

            if (CheckUrl(url)) {
                //(unused) required to suppress cross-platform compilation warning/error
                __attribute__((unused)) double page_load_time = ((double)LIBNETXT_API(GetTimeDeltaInMs)(frame_iter->second.time_stamp_, commit_timestamp))/1000;

                StatHubCmd* cmd = STAT_HUB_API(CmdCreate)(SH_CMD_SELF, SH_ACTION_PAGELOAD_DID_FINISH, 0);
                if (NULL!=cmd) {
                    cmd->AddParamAsString(url);
                    cmd->AddParamAsPtr(page_id);
                    cmd->AddParamAsUint32(LIBNETXT_API(GetTimeDeltaInMs)(frame_iter->second.time_stamp_, commit_timestamp));
                    cmd->AddParamAsUint32(frame_iter->second.bytes_received);
                    cmd->AddParamAsUint32(LIBNETXT_API(GetTimeDeltaInMs)
                            (frame_iter->second.time_stamp_, frame_iter->second.fp_time_stamp_));
                    cmd->AddParamAsUint32(LIBNETXT_API(GetTimeDeltaInMs)
                            (frame_iter->second.time_stamp_, frame_iter->second.ol_time_stamp_));
                    cmd->AddParamAsUint32(LIBNETXT_API(GetTimeDeltaInMs)
                            (frame_iter->second.time_stamp_, frame_iter->second.dn_time_stamp_));
                    STAT_HUB_API(CmdCommit)(cmd);
                }
                if (STAT_HUB_API(IsVerboseEnabled)()) {
                    LIBNETXT_LOGI("SHTEST_PROC - PageLoadStat FP:%.03f, OL:%.03f, DN:%.03f, PL:%.03f, Size:%d, (%s)",
                            ((double)LIBNETXT_API(GetTimeDeltaInMs)(frame_iter->second.time_stamp_, frame_iter->second.fp_time_stamp_))/1000,
                            ((double)LIBNETXT_API(GetTimeDeltaInMs)(frame_iter->second.time_stamp_, frame_iter->second.ol_time_stamp_))/1000,
                            ((double)LIBNETXT_API(GetTimeDeltaInMs)(frame_iter->second.time_stamp_, frame_iter->second.dn_time_stamp_))/1000,
                            page_load_time,
                            frame_iter->second.bytes_received,
                            url);
                }
                LIBNETXT_LOGI("PageLoad took %.03f sec, url=%s", page_load_time, url);
            }
            frame_map_.erase(frame_iter);
            return true;
        }
    }
    return false;
}

// ========================================================================
STAT_HUB_CMD_HANDLER_CONTAINER_IMPL(StatHubTestPlugin, PlProc_CmdHandlerWkPageLoad) {
    //(unused) required to suppress cross-platform compilation warning/error
    __attribute__((unused)) double page_load_time = 0;
    void* page_id = cmd->GetParamAsPtr(0);
    bool is_main = cmd->GetParamAsBool(1);

    if (!is_main && !log_all_frames_) {
        return false;
    }
    FrameMapType::iterator frame_iter = frame_map_.find(page_id);
    StatHubTimeStamp commit_timestamp = cmd->GetCommitTimeStamp();
    if (frame_iter != frame_map_.end()) {
        page_load_time = ((double)LIBNETXT_API(GetTimeDeltaInMs)(frame_iter->second.time_stamp_, commit_timestamp))/1000;
    }
    switch (cmd->GetAction()) {
        case SH_ACTION_WILL_START_LOAD:
            {
                const char* url = cmd->GetParamAsString(0);
                if (STAT_HUB_API(IsVerboseEnabled)()) {
                    LIBNETXT_LOGI("SHTEST_PROC - Page load request (%s)", url);
                }
                StatHubCmd* cmd = STAT_HUB_API(CmdCreate)(SH_CMD_SELF, SH_ACTION_PAGELOAD_WILL_START, 0);
                if (NULL!=cmd) {
                    cmd->AddParamAsString(url);
                    STAT_HUB_API(CmdCommit)(cmd);
                }
            }
            break;
        case SH_ACTION_DID_START_LOAD:
            {
                const char* url = cmd->GetParamAsString(2);
                bool url_ok = CheckUrl(url);

                if (url_ok) {
                    if (frame_iter != frame_map_.end()) {
                        //reload in the same frame - update start timestamp
                        if (!is_main) {
                            frame_iter->second.time_stamp_ = commit_timestamp;
                            if (STAT_HUB_IS_VERBOSE_LEVEL_DEBUG || (STAT_HUB_API(IsVerboseEnabled)() && log_all_frames_)) {
                                LIBNETXT_LOGD("SHTEST_PROC - Update Timestamp - Id:%p - (%s)",
                                   page_id, url);
                            }
                        }
                    }
                    else {
                        if (STAT_HUB_API(IsVerboseEnabled)() && (url_ok || log_all_frames_)) {
                            LIBNETXT_LOGI("SHTEST_PROC - Start%s - Id:%p (%s) ", is_main?" (MAIN)":"", page_id, url);
                        }
                        StatHubCmd* cmd = STAT_HUB_API(CmdCreate)(SH_CMD_SELF, SH_ACTION_PAGELOAD_DID_START, 0);
                        if (NULL!=cmd) {
                            cmd->AddParamAsString(url);
                            cmd->AddParamAsPtr(page_id);
                            STAT_HUB_API(CmdCommit)(cmd);
                        }
                        frame_map_.insert(std::pair<void*, FrameEntry>(page_id, FrameEntry(commit_timestamp)));
                    }
                }
            }
            break;
        case SH_ACTION_DID_FINISH_LOAD:
            {
                const char* url = cmd->GetParamAsString(2);
                bool failed  = cmd->GetParamAsBool(3);
                //(unused) required to suppress cross-platform compilation warning/error
                __attribute__((unused)) unsigned int bytes_received = 0;
                bool url_ok = CheckUrl(url);

                if (url_ok) {
                    if (frame_iter != frame_map_.end()) {
                        frame_iter->second.finish_received = true;
                        bytes_received = frame_iter->second.bytes_received;
                        frame_iter->second.dn_time_stamp_ = commit_timestamp;
                    }
                }
                if (STAT_HUB_API(IsVerboseEnabled)() && (url_ok || log_all_frames_)) {
                    LIBNETXT_LOGI("SHTEST_PROC - Done%s - Id:%p, Time:%.03f sec, Received:%d (%s) - %s",
                        is_main?" (MAIN)":"", page_id, page_load_time, bytes_received, url, (failed)?"FAILED":"succeeded");
                }
                if (url_ok) {
                    CheckDone(cmd, failed);
                }
            }
            break;
        case SH_ACTION_ON_LOAD:
            {
                const char* url = cmd->GetParamAsString(2);
                unsigned int bytes_received = cmd->GetParamAsUint32(3);
                bool url_ok = CheckUrl(url);

                if (url_ok) {
                    if (frame_iter != frame_map_.end()) {
                        frame_iter->second.on_load_received = true;
                        frame_iter->second.ol_time_stamp_ = commit_timestamp;
                        if (!bytes_received) {
                            bytes_received = frame_iter->second.bytes_received;
                        }
                    }
                }
                if (STAT_HUB_API(IsVerboseEnabled)() && (url_ok || log_all_frames_)) {
                    LIBNETXT_LOGI("SHTEST_PROC - OnLoad%s - Id:%p, Time:%.03f sec, Received:%d (%s)",
                        is_main?" (MAIN)":"", page_id, page_load_time, bytes_received, url);
                }
                if (url_ok) {
                    CheckDone(cmd, false);
                }
            }
            break;
        case SH_ACTION_FIRST_PIXEL:
            {
                const char* url = cmd->GetParamAsString(2);
                //(unused) required to suppress cross-platform compilation warning/error
                __attribute__((unused)) unsigned int bytes_received = cmd->GetParamAsUint32(3);
                bool url_ok = CheckUrl(url);

                if (url_ok) {
                    if (frame_iter != frame_map_.end()) {
                        frame_iter->second.fp_time_stamp_ = commit_timestamp;
                    }
                }
                if (STAT_HUB_API(IsVerboseEnabled)() && (url_ok || log_all_frames_)) {
                    LIBNETXT_LOGI("SHTEST_PROC - FirstPixel%s - Id:%p, Time:%.03f sec, Received:%d (%s)",
                        is_main?" (MAIN)":"", page_id, page_load_time, bytes_received, url);
                }
            }
            break;
        case SH_ACTION_PROGRESS_UPDATE:
            {
                unsigned int bytes_received = cmd->GetParamAsUint32(2);
                unsigned int progress_update = cmd->GetParamAsUint32(3);

                if (!progress_update) {
                    if (frame_iter == frame_map_.end()) {
                        frame_iter = frame_map_.insert(frame_iter, std::pair<void*, FrameEntry>(page_id, FrameEntry(commit_timestamp)));
                    }
                }
                frame_iter->second.bytes_received = bytes_received;
                if (STAT_HUB_API(IsVerboseEnabled)() && progress_report_enabled_ ) {
                    LIBNETXT_LOGI("SHTEST_PROC - ProgressUpdate%s (%3d) - Id:%p, Time:%.03f sec, Received:%d",
                        is_main?" (MAIN)":"", progress_update, page_id, page_load_time, bytes_received);
                }
            }
            break;
        default:
            return false;
    }
    return true;
}

// ========================================================================
bool StatHubTestPlugin::OnGetCmdMask(uint32& cmd_mask) {
    cmd_mask = 0;
    STAT_HUB_CMD_HANDLER_CONTAINER_INIT(cmd_handlers_, cmd_mask, SH_CMD_WK_PAGE, PlProc_CmdHandlerWkPageLoad,
        SH_ACTION_WILL_START_LOAD, SH_ACTION_DID_START_LOAD, SH_ACTION_DID_FINISH_LOAD, SH_ACTION_ON_LOAD, SH_ACTION_FIRST_PIXEL, SH_ACTION_PROGRESS_UPDATE);
    return true;
}

} //namespace sh_test_proc
