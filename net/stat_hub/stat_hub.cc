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

#include "base/compiler_specific.h"
#include "build/build_config.h"

#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/queue.h>
#include <sys/time.h>
#include <sys/prctl.h>

#include "sql/statement.h"
#include "sql/transaction.h"
#include "base/command_line.h"
#include "base/files/file_path.h"
#include "base/file_util.h"
#include "base/time/time.h"
#include "base/strings/string_util.h"
#include "base/strings/utf_string_conversions.h"
#include "base/threading/thread.h"
#include "net/http/http_cache.h"

#include "stat_hub.h"
#include "stat_hub_api.h"
#include "stat_hub_cmd.h"
#include "net/libnetxt/plugin_api.h"

#include "pageload_proc.h"

#define STAT_HUB_DYNAMIC_BIND_ON

namespace stat_hub {

#define FLUSH_DB_TIMEOUT_THRESHOLD_DEF  30000
#define FLUSH_DOS_PREVENTION_THRESHOLD_DEF  (60000*5) //5 min

//Currently DB is disabled by default. Should switch it on if start using DB again (set to "1")
#define SH_DB_ENABLED "0"

typedef enum {
    INPUT_STATE_READ_MARKER,
    INPUT_STATE_READ_CMD,
    INPUT_STATE_READ_STRING_LEN,
    INPUT_STATE_READ_STRING_DATA,
    INPUT_STATE_READ_INT32,
} InputState;

const char* kPropNameEnabled = "net.sh.enabled";
const char* kPropNameDbEnabled = "net.sh.db.enabled";
const char* kPropNameDbpath = "net.sh.dbpath";
const char* kPropNameVerbose = "net.sh.verbose";
const char* kPropNameFlushDelay = "net.sh.flushdelay";
const char* kPropNameDosThreshold = "net.sh.dosth";
const char* kPropNamePlugin = "net.sh.plugin";
const char* kPropNameClearEnabled = "net.sh.clrenabled";
const char* kPropNamePerfEnabled = "net.sh.prfenabled";

const char* kDefaultDbPath = "/../databases";
const char* kDefaultDbName = "/shdb.sql";

void DoFlushDB(StatHub* database) {
    database->FlushDBrequest();
}

// Version number of the database.
static const int kCurrentVersionNumber = 1;
static const int kCompatibleVersionNumber = 1;

//=========================================================================
STAT_PROCESSOR_IMPL();

//=========================================================================
StatProcessorGenericPlugin::StatProcessorGenericPlugin(const char* name) :
    initialized_(false), fh_(NULL) {
    if (NULL!=name) {
        name_ = name;
    }
    STAT_PLUGIN_IF_DEFINE(OnInit)
    STAT_PLUGIN_IF_DEFINE(OnFlushDb)
    STAT_PLUGIN_IF_DEFINE(OnClearDb)
    STAT_PLUGIN_IF_DEFINE(OnCmd)
    STAT_PLUGIN_IF_DEFINE(OnGetProcInfo)
    STAT_PLUGIN_IF_DEFINE(OnGetCmdMask)
}

//=========================================================================
StatProcessorGenericPlugin::~StatProcessorGenericPlugin() {
    if (!name_.empty() && initialized_) {
        LibraryManager::ReleaseLibraryHandle(name_.c_str());
    }
}

//=========================================================================
STAT_PLUGIN_METHOD_IMPL_1(StatProcessorGenericPlugin, OnInit, bool, sql::Connection*, db)
STAT_PLUGIN_METHOD_IMPL_1(StatProcessorGenericPlugin, OnFlushDb, bool, sql::Connection*, db)
STAT_PLUGIN_METHOD_IMPL_1(StatProcessorGenericPlugin, OnClearDb, bool, sql::Connection*, db)
STAT_PLUGIN_METHOD_IMPL_1(StatProcessorGenericPlugin, OnCmd, bool, StatHubCmd*, cmd)
STAT_PLUGIN_METHOD_IMPL_2(StatProcessorGenericPlugin, OnGetProcInfo, bool, std::string&, name, std::string&, version)
STAT_PLUGIN_METHOD_IMPL_1(StatProcessorGenericPlugin, OnGetCmdMask, bool, unsigned int&, cmd_mask)

//=========================================================================
StatHub* StatHub::GetInstance() {
    CR_DEFINE_STATIC_LOCAL(StatHub, hub, ());
    if (!hub.IsReady() && 0==hub.under_construction_) {
        hub.under_construction_ = 1;
        hub.Init();
        hub.under_construction_ = 2;
    }
    return &hub;
}

//=========================================================================
StatHub::StatHub() :
    db_(NULL),
    ready_(false),
    flush_db_required_(false),
    flush_db_scheduled_(false),
    dos_prevention_on_(false),
    message_loop_(NULL),
    http_cache_(NULL),
    first_processor_(NULL),
    pl_processor_(NULL),
    thread_(NULL),
    flush_delay_(FLUSH_DB_TIMEOUT_THRESHOLD_DEF),
    dos_prevention_threshold_(FLUSH_DOS_PREVENTION_THRESHOLD_DEF),
    verbose_level_(STAT_HUB_VERBOSE_LEVEL_DISABLED),
    clear_enabled_(true),
    under_construction_(0),
    performance_enabled_(false)
{
    cmd_mask_ = (1<<SH_CMD_WK_MEMORY_CACHE);
    cmd_mask_ |= (1<<SH_CMD_WK_MAIN_URL);
}

StatHub::~StatHub() {
    Release();
}

//=========================================================================
bool StatHub::IsReady(bool verbose) {
    if (verbose && !ready_) {
        LIBNETXT_LOGE("STAT_HUB - StatHub is not ready!");
    }
    return ready_;
}

typedef StatProcessor* OnCreateType(const char* name);

//=========================================================================
void* StatHub::LoadPlugin(const char* name, void* fh) {
    if (!IsReady(true)) {
        return NULL;
    }
    if (!name) {
        LIBNETXT_LOGE("STAT_HUB - Undefined plugin name");
        return NULL;
    }
    if (IsVerboseEnabled()) {
        LIBNETXT_LOGI("STAT_HUB - Loading plugin: %s", name);
    }
    if (!fh) {
        fh = LibraryManager::GetLibraryHandle(name);
    }
    if (fh) {
        StatProcessor* proc = NULL;
        StatProcessorGenericPlugin* plugin = NULL;

        //try to obtain plugin as an object
        OnCreateType* OnCreate = (OnCreateType*)LibraryManager::GetLibrarySymbol(fh, "OnCreate", true);
        if (OnCreate) {
            proc = (StatProcessor*)OnCreate(name);
        }
        else {
            //use C interface
            plugin = new StatProcessorGenericPlugin(name);
            plugin->OpenPlugin(fh);
            proc = plugin;
        }
        if (RegisterProcessor(proc)) {
            LIBNETXT_LOGI("STAT_HUB - Succeeded to load plugin: %s", name);
            if (InitProcessor(proc)) {
                return fh;
            } else {
                return NULL;
            }
        }
        if (plugin) {
            delete plugin;
        }
    }
    LIBNETXT_LOGE("STAT_HUB - Failed to load plugin: %s", name);
    return NULL;
}

//=========================================================================
bool StatHub::LoadProc(StatProcessor* processor) {
    if (!IsReady(true)) {
        return false;
    }
    if (!processor) {
        LIBNETXT_LOGE("STAT_HUB - Undefined processor");
        return false;
    }
    std::string proc_name;
    std::string proc_version;

    if (!processor->OnGetProcInfo(proc_name, proc_version)) {
        LIBNETXT_LOGE("STAT_HUB - Processor name is undefined");
        delete processor;
        return false;
    }
    if (IsVerboseEnabled()) {
        LIBNETXT_LOGI("STAT_HUB - Loading processor: %s", proc_name.c_str());
    }
    if (!RegisterProcessor(processor) || !InitProcessor(processor)) {
        LIBNETXT_LOGE("STAT_HUB - Failed to load processor: %s", proc_name.c_str());
        delete processor;
        return false;
    }
    return true;
}

//=========================================================================
bool StatHub::RegisterProcessor(StatProcessor* processor) {
    if (!IsReady(true)) {
        return false;
    }
    if (processor) {
        std::string proc_name;
        std::string proc_version;

        if (!processor->OnGetProcInfo(proc_name, proc_version)) {
            LIBNETXT_LOGE("STAT_HUB - Processor name is undefined");
            return false;
        }
        if (IsProcRegistered(proc_name.c_str())) {
            LIBNETXT_LOGE("STAT_HUB - Processor %s already registered", proc_name.c_str());
            return false;
        }
        processor->next_ = first_processor_;
        first_processor_ = processor;
        return true;
    }
    LIBNETXT_LOGE("STAT_HUB - Undefined processor");
    return false;
}

//=========================================================================
bool StatHub::InitProcessor(StatProcessor* processor) {
    if (!IsReady(true)) {
        return false;
    }
    if (!processor) {
        LIBNETXT_LOGE("STAT_HUB - Undefined processor");
        return false;
    }
    std::string proc_name = "Undefined";
    std::string proc_version ="0.0.0";

    processor->OnGetProcInfo(proc_name, proc_version);
    if(!processor->OnInit(db_)) {
        LIBNETXT_LOGE("STAT_HUB - Processor %s (v%s) initialization failed",
            proc_name.c_str(), proc_version.c_str());
        processor = DeleteProcessor(processor);
        return false;
    } else {
        LIBNETXT_LOGI("STAT_HUB - Processor %s (v%s) is ready",
            proc_name.c_str(), proc_version.c_str());
        unsigned int cmd_mask;
        if (processor->OnGetCmdMask(cmd_mask)) {
            cmd_mask_ |= cmd_mask;
        }
    }
    return true;
}

//=========================================================================
StatProcessor* StatHub::DeleteProcessor(StatProcessor* processor) {
    if (NULL!=processor) {
        StatProcessor* next = processor->next_;
        if (first_processor_==processor) {
            first_processor_ = next;
        }
        else {
            for (StatProcessor* tmp_processor=first_processor_; tmp_processor!=NULL; tmp_processor=tmp_processor->next_ ) {
                if (tmp_processor->next_==processor) {
                    tmp_processor->next_=next;
                    break;
                }
            }
        }
        delete processor;
        return next;
    }
    return NULL;
}

//=========================================================================
bool StatHub::IsProcRegistered(const char* name) {
    std::string proc_name;
    std::string proc_version;

    for (StatProcessor* processor=first_processor_; processor!=NULL; processor=processor->next_) {
        if (processor->OnGetProcInfo(proc_name, proc_version)) {
            if (proc_name==name) {
                return true;
            }
        }
    }
    return false;
}

//=========================================================================
bool StatHub::IsProcReady(const char* name) {
    if (IsReady()) {
        std::string proc_name;
        std::string proc_version;

        for (StatProcessor* processor=first_processor_; processor!=NULL; processor=processor->next_) {
            if (processor->OnGetProcInfo(proc_name, proc_version)) {
                if (proc_name==name) {
                    if (STAT_HUB_IS_VERBOSE_LEVEL_DEBUG) {
                        LIBNETXT_LOGD("STAT_HUB - Processor %s is ready", name);
                    }
                    return true;
                }
            }
        }
    }
    if (IsVerboseEnabled()) {
        LIBNETXT_LOGI("STAT_HUB - Processor %s is NOT ready", name);
    }
    return false;
}

//=========================================================================
bool StatHub::Init() {
    char value[PROPERTY_VALUE_MAX] = {'\0'};

    if (ready_) {
        LIBNETXT_LOGI("STAT_HUB - Already initialized");
        return true;
    }
    base::ThreadRestrictions::SetIOAllowed(true);

    LIBNETXT_PROPERTY_GET(kPropNameEnabled, value, "1"); //!!!!!!!!! ENABLED by default !!!!!!!!!
    if (!atoi(value)) {
        LIBNETXT_LOGW("STAT_HUB - Disabled");
        return false;
    }

    LIBNETXT_PROPERTY_GET(kPropNameVerbose, value, "0"); //STAT_HUB_VERBOSE_LEVEL_DISABLED - 0
    verbose_level_ = (StatHubVerboseLevel)atoi(value);
    if (IsVerboseEnabled()) {
        LIBNETXT_LOGI("STAT_HUB - Verbose Level: %d", verbose_level_);
    }

    LIBNETXT_PROPERTY_GET(kPropNameClearEnabled, value, "1");
    if (!atoi(value)) {
        clear_enabled_ = false;
        LIBNETXT_LOGI("STAT_HUB - Cache Clear Disabled");
    }

    LIBNETXT_PROPERTY_GET(kPropNamePerfEnabled, value, "0");
    if (atoi(value)) {
        performance_enabled_ = true;
        SetPerfTimeStamp(LIBNETXT_API(GetSystemTime)());
        LIBNETXT_LOGI("STAT_HUB - Performance Piggyback Enabled");
    }

    //Application
    char path[128] = {'\0'};
    pid_t pid = getpid();
    snprintf(path, sizeof(path), "/proc/%d/cmdline", pid);
    int fd = open(path, O_RDONLY);
    int rd_len = read(fd, path , sizeof(path)-1);
    if (0 > rd_len) {
        rd_len = 0;
    }
    path[rd_len] = 0;
    close(fd);

    if(IsVerboseEnabled()) {
        LIBNETXT_LOGI("STAT_HUB - Prc Name: %s (%d)", path ,(int)pid);
    }

    LIBNETXT_PROPERTY_GET(kPropNameFlushDelay, value, PROP_VAL_TO_STR(FLUSH_DB_TIMEOUT_THRESHOLD_DEF));
    flush_delay_ = atoi(value);
    if (flush_delay_<=0) {
        flush_delay_ = FLUSH_DB_TIMEOUT_THRESHOLD_DEF;
    }
    LIBNETXT_PROPERTY_GET(kPropNameDosThreshold, value, PROP_VAL_TO_STR(FLUSH_DOS_PREVENTION_THRESHOLD_DEF));
    dos_prevention_threshold_ = atoi(value);
    if (dos_prevention_threshold_<=0) {
        dos_prevention_threshold_ = FLUSH_DOS_PREVENTION_THRESHOLD_DEF;
    }
    if(IsVerboseEnabled()) {
        LIBNETXT_LOGI("STAT_HUB - Flush delay: %d", flush_delay_);
        LIBNETXT_LOGI("STAT_HUB - DOS threshold: %d", dos_prevention_threshold_);
    }

    thread_ = new base::Thread("event_handler");
    if (!thread_->StartWithOptions(base::Thread::Options(base::MessageLoop::TYPE_IO, 0))) {
        LIBNETXT_LOGE("STAT_HUB - Event thread start error");
        Release();
        return false;
    }

    ready_ = true;
    LIBNETXT_LOGI("STAT_HUB - Initialized");

    LoadPlugins();

    return true;
}

//=========================================================================
void StatHub::LoadPlugins() {
    pl_processor_ = new pl_proc::PageLoadProcessor();
    pl_processor_->OnGetCmdMask(dos_prevention_cmd_mask_);
    LoadProc(pl_processor_);

#ifdef STAT_HUB_DYNAMIC_BIND_ON
    char value[PROPERTY_VALUE_MAX] = {'\0'};

    //load arbitrary plugins
    for(int index=1; ; index++) {
        std::ostringstream index_str;
        index_str << "." << index;
        std::string plugin_prop_name = kPropNamePlugin + index_str.str();
        LIBNETXT_PROPERTY_GET(plugin_prop_name.c_str(), value, "");
        if (!value[0]) {
            break;
        }
        LoadPlugin(value);
    }
#endif // STAT_HUB_DYNAMIC_BIND_ON

    LoadProc(new pl_proc::PageLoadProcessor());
    //TBD: LoadPlugin("libpp_proc_plugin");
    //TBD: LoadPlugin("libspl_proc_plugin");
}

//=========================================================================
void StatHub::InitDBOnce(StatHub* stat_hub) {
    static bool initialized = false;
    if (!initialized) {
        if (!stat_hub->InitDB()) {
            LIBNETXT_LOGW("STAT_HUB - Switch to DB-less mode");
        }
        initialized = true;
    }
}

//=========================================================================
bool StatHub::InitDB() {
    char value[PROPERTY_VALUE_MAX] = {'\0'};

    //check if DB can be enabled
    LIBNETXT_PROPERTY_GET(kPropNameDbEnabled,value, SH_DB_ENABLED);
    if (value[0]!='1') {
        return false;
    }

    __attribute__((unused)) base::Time start(LIBNETXT_API(GetSystemTime)());
    base::FilePath file_path;
    LIBNETXT_DATADIR_GET(&file_path);
    file_path = file_path.StripTrailingSeparators();
    std::string directory_path = file_path.value().c_str();

    std::string default_db_path = directory_path;
    default_db_path += kDefaultDbPath;
    default_db_path += kDefaultDbName;
    LIBNETXT_PROPERTY_GET(kPropNameDbpath, value, default_db_path.c_str());
    db_path_ = value;

    if (STAT_HUB_IS_VERBOSE_LEVEL_DEBUG) {
        LIBNETXT_LOGI("STAT_HUB - App Data directory: %s", file_path.value().c_str());

        LIBNETXT_CACHEDIR_GET(&file_path);
        file_path = file_path.StripTrailingSeparators();
        LIBNETXT_LOGI("STAT_HUB - App Cache directory: %s", file_path.value().c_str());

        std::string lib_directory_path;
        LibraryManager::GetLibDirectory(lib_directory_path);
        LIBNETXT_LOGI("STAT_HUB - App Lib directory: %s", lib_directory_path.c_str());
    }
    if(IsVerboseEnabled()) {
        LIBNETXT_LOGI("STAT_HUB - DB path: %s", db_path_.c_str());
    }

    if (db_path_ == default_db_path) {
        struct stat st = {0};
        std::string datbases_directory_path = directory_path + kDefaultDbPath;
        if (stat(datbases_directory_path.c_str(), &st) == -1) {
            mkdir(datbases_directory_path.c_str(), (S_IRWXU | S_IRWXG | S_IXOTH));
            if(IsVerboseEnabled()) {
                LIBNETXT_LOGI("STAT_HUB - Create DB folder: %s",
                    datbases_directory_path.c_str());
            }
        }
    }

    /*TODO: Tweak DB performance
        db_->set_page_size(2048);
        db_->set_cache_size(32);
        //Run the database in exclusive mode. Nobody else should be accessing the
        //database while we're running, and this will give somewhat improved perf.
        db_->set_exclusive_locking();
    */
    db_ = new sql::Connection();
    if (!db_->Open(base::FilePath(db_path_.c_str()))) {
        LIBNETXT_LOGE("STAT_HUB - Unable to open DB %s", db_path_.c_str());
        ReleaseDB();
        return false;
    }

    // Scope initialization in a transaction so we can't be partially initialized.
    if (!STAT_HUB_API_CPP(sql, Connection, BeginTransaction)(db_)) {
        LIBNETXT_LOGE("STAT_HUB - Unable to start transaction");
        ReleaseDB();
        return false;
    }

    // Create tables.
    if (!InitTables()) {
        LIBNETXT_LOGE("STAT_HUB - Unable to initialize DB tables");
        ReleaseDB();
        return false;
    }

    // Initialization is complete.
    if (!STAT_HUB_API_CPP(sql, Connection, CommitTransaction)(db_)) {
        LIBNETXT_LOGE("STAT_HUB - Unable to commit transaction");
        ReleaseDB();
        return false;
    }
    flush_db_required_ = true;
    if(IsVerboseEnabled()) {
        LIBNETXT_LOGI("STAT_HUB - Init DB Time: %d" ,LIBNETXT_API(GetTimeDeltaInMs)(start, LIBNETXT_API(GetSystemTime)()));
    }

    return true;
}

//=========================================================================
void StatHub::ReleaseDB() {
    if(IsVerboseEnabled()) {
        LIBNETXT_LOGI("STAT_HUB - Release DB");
    }

    if (db_) {
        db_->Close();
        delete db_;
        db_ = NULL;
    }
    flush_db_required_ = false;
    flush_db_scheduled_ = false;
}

//=========================================================================
void StatHub::Release() {
    if(IsVerboseEnabled()) {
        LIBNETXT_LOGI("STAT_HUB - Release");
    }

    //thread
    if(NULL!=thread_) {
        delete thread_;
        thread_ = NULL;
    }

    //processors
    StatProcessor* next_processor;
    for (StatProcessor* processor=first_processor_; processor!=NULL; processor=next_processor ) {
        next_processor = processor->next_;
        delete processor;
    }
    first_processor_ = NULL;

    //DataBase
    ReleaseDB();

    //Rest
    ready_ = false;
}

//=========================================================================
bool StatHub::InitTables() {
    if (!STAT_HUB_API_CPP(sql, Connection, DoesTableExist)(db_, "meta")) {
        if (!STAT_HUB_API_CPP(sql, Connection, Execute)(db_, "CREATE TABLE meta ("
            "key LONGVARCHAR NOT NULL UNIQUE PRIMARY KEY,"
            "value LONGVARCHAR"
            ")")) {
                return false;
        }
    }
    return true;
}

//=========================================================================
bool StatHub::GetDBmetaData(const char* key, std::string& val) {
    bool ret = false;

    InitDBOnce(this);
    if (db_) {
        sql::Statement* statement = STAT_HUB_API(GetStatement)(db_, SQL_FROM_HERE,
            "SELECT * FROM meta WHERE key=?");
        STAT_HUB_API_CPP(sql, Statement, BindCString)(statement, 0 , key);
        if(STAT_HUB_API_CPP(sql, Statement, Step)(statement)) {
            ret = true;
            val = STAT_HUB_API_CPP(sql, Statement, ColumnString)(statement, 1);
        }
        STAT_HUB_API(ReleaseStatement)(statement);
    }
    return ret;
}

//=========================================================================
bool StatHub::SetDBmetaData(const char* key, const char* val) {
    bool ret = true;

    InitDBOnce(this);
    if (db_) {
        sql::Statement* statement = STAT_HUB_API(GetStatement)(db_, SQL_FROM_HERE,
            "INSERT OR REPLACE INTO meta "
            "(key, value) "
            "VALUES (?,?)");
        STAT_HUB_API_CPP(sql, Statement, BindCString)(statement, 0 , key);
        STAT_HUB_API_CPP(sql, Statement, BindCString)(statement, 1 , val);
        ret = STAT_HUB_API_CPP(sql, Statement, Run)(statement);
        STAT_HUB_API(ReleaseStatement)(statement);
    }
    return ret;
}

//=========================================================================
void StatHub::MainUrlLoaded(const char* url) {
    flush_db_request_time_ = LIBNETXT_API(GetSystemTime)();
    if (flush_db_required_) {
        if (!flush_db_scheduled_) {
            flush_db_scheduled_ = true;
            flush_db_initial_request_time_ = flush_db_request_time_;
            if(IsVerboseEnabled()) {
                LIBNETXT_LOGI("STAT_HUB - Request DB flush (%d)", flush_delay_ );
            }
            thread_->message_loop()->PostDelayedTask(FROM_HERE ,base::Bind(&DoFlushDB, this),
                base::TimeDelta::FromMilliseconds(flush_delay_));
        }
        else {
            int delta = LIBNETXT_API(GetTimeDeltaInMs)(flush_db_initial_request_time_, flush_db_request_time_);
            if (delta>=dos_prevention_threshold_ && !dos_prevention_on_) {
                //continues page load - go to dos prevention mode
                cmd_mask_copy_ = cmd_mask_;
                cmd_mask_ = dos_prevention_cmd_mask_;
                dos_prevention_on_ = true;
                if(IsVerboseEnabled()) {
                    LIBNETXT_LOGI("STAT_HUB - Enable DOS prevention mode (%d)", delta);
                }
            }
        }
    }
}

//=========================================================================
bool StatHub::Cmd(StatHubCmd* cmd, bool sync) {
    if(NULL!=cmd) {
        if (IsPerfEnabled()) {
            StatHubTimeStamp time_stamp = LIBNETXT_API(GetSystemTime)();
            if (LIBNETXT_API(GetTimeDeltaInMs)(GetPerfTimeStamp(), time_stamp)>=50)
            {
                SetPerfTimeStamp(time_stamp);
                //pID
                char path[512] = {'\0'};
                pid_t pid = getpid();
                //stat
                snprintf(path, sizeof(path), "/proc/%d/stat", pid);
                int fd = open(path, O_RDONLY);
                if (-1!=fd) {
                    int rd_len = read(fd, path , sizeof(path)-1);
                    if (0 > rd_len) {
                        rd_len = 0;
                    }
                    path[rd_len] = 0;
                    cmd->SetStat(path);
                    close(fd);
                }
            }
        }
        StatHubCmdType cmd_id = cmd->GetCmd();
        StatHubActionType action_id = cmd->GetAction();
        if (STAT_HUB_IS_VERBOSE_LEVEL_DEBUG && STAT_HUB_DEV_LOG_ENABLED) {
            LIBNETXT_LOGD("STAT_HUB - StatHub::Cmd CMD:%d Action:%d", cmd->GetCmd(), cmd->GetAction());
        }
        if (dos_prevention_on_) {
            pl_processor_->OnCmd(cmd);
        }
        else {
            if (clear_enabled_ && SH_CMD_WK_MEMORY_CACHE==cmd_id && SH_ACTION_CLEAR==action_id) {
                for (StatProcessor* processor=first_processor_; processor!=NULL; processor=processor->next_ ) {
                    processor->OnClearDb(db_);
                }
            }
            for (StatProcessor* processor=first_processor_; processor!=NULL; processor=processor->next_ ) {
                if (processor->OnCmd(cmd) && sync) {
                    return true;
                }
            }
        }
        if (SH_CMD_WK_MAIN_URL==cmd_id && SH_ACTION_DID_FINISH==action_id) {
            const char* url = cmd->GetParamAsString(0);
            MainUrlLoaded(url);
        }
    }
    return false;
}

//=========================================================================
void StatHub::FlushDBrequest() {
    if (STAT_HUB_IS_VERBOSE_LEVEL_DEBUG) {
        LIBNETXT_LOGD("STAT_HUB - Flush DB Request");
    }

    if (flush_db_required_) {
        int delta = LIBNETXT_API(GetTimeDeltaInMs)(flush_db_request_time_, LIBNETXT_API(GetSystemTime)());

        if(IsVerboseEnabled()) {
            LIBNETXT_LOGI("STAT_HUB - Time since last Flush DB request is %d msec", delta);
        }
        if (delta>=flush_delay_) {
           flush_db_scheduled_ = false;
           FlushDB();
        }
        else {
            if(IsVerboseEnabled()) {
                LIBNETXT_LOGI("STAT_HUB - Flush DB postponed for %d msec", flush_delay_ - delta);
            }
            thread_->message_loop()->PostDelayedTask(FROM_HERE, base::Bind(&DoFlushDB, this),
                base::TimeDelta::FromMilliseconds(flush_delay_ - delta));
        }
    }
}

//=========================================================================
bool StatHub::FlushDB() {
    if (STAT_HUB_IS_VERBOSE_LEVEL_DEBUG) {
        LIBNETXT_LOGD("STAT_HUB - Flush DB...");
    }
    //(unused) required to suppress cross-platform compilation warning/ error
    __attribute__((unused)) base::Time start = LIBNETXT_API(GetSystemTime)();
    if (dos_prevention_on_) {
        //restore original cmd_mask
        cmd_mask_ = cmd_mask_copy_;
        dos_prevention_on_ = false;
        if(IsVerboseEnabled()) {
            LIBNETXT_LOGI("STAT_HUB - Disable DOS prevention mode");
        }
    }
    for (StatProcessor* processor=first_processor_; processor!=NULL; processor=processor->next_ ) {
        processor->OnFlushDb(db_);
    }

    if(IsVerboseEnabled()) {
        LIBNETXT_LOGI("STAT_HUB - Flush DB completed in %d msec", LIBNETXT_API(GetTimeDeltaInMs)(start, LIBNETXT_API(GetSystemTime)()));
    }
    return true;
}

}  // namespace stat_hub
