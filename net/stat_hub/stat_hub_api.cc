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

#include <unistd.h>
#include <string>
#include <set>
#include <stdio.h>

#include "url/gurl.h"
#include "base/hash.h"
#include "base/memory/ref_counted.h"
#include "base/threading/thread.h"
#include "net/base/completion_callback.h"
#include "net/filter/filter.h"
#include "net/base/net_log.h"
#include "net/base/net_errors.h"
#include "net/base/io_buffer.h"
#include "net/http/http_cache_transaction.h"
#include "net/http/http_request_info.h"
#include "net/http/http_response_headers.h"
#include "net/http/preconnect.h"
#include "net/url_request/url_request.h"
#include "net/url_request/redirect_info.h"
#include "net/socket/client_socket_pool_manager.h"
#include "base/time/time.h"
#include "sql/connection.h"
#include "sql/statement.h"

#include "stat_hub.h"
#include "stat_hub_api.h"
#include "stat_hub_cmd_api.h"

typedef std::set<net::URLRequestContext*> StatHubContextSetType;

// ======================================= Fetch Interface ==============================================
#define READ_BUF_SIZE               (50*1024)

class FetchRequest : public net::URLRequest::Delegate,public base::RefCountedThreadSafe<FetchRequest> {
public:
    explicit FetchRequest(unsigned int cookie, net::HttpRequestInfo* request_info):
        cookie_(cookie),
        read_in_progress_(false),
        buf_(NULL)
    {
          request_info_.reset(request_info);
    }

    //=========================================================================
    bool StartFetch(net::URLRequestContext* context) {
        StatHubContextSetType::iterator context_iter = ContextSet().find(context);
        if (context_iter == ContextSet().end()) {
            LIBNETXT_LOGE("STAT_HUB - Undefined context %p for %s",
                context, request_info_->url.spec().c_str());
            delete this;
            return false;
        }
        request_.reset(new net::URLRequest(request_info_->url, net::DEFAULT_PRIORITY, this, context));
        if(STAT_HUB_IS_VERBOSE_LEVEL_DEBUG) {
            LIBNETXT_LOGD("STAT_HUB - Fetch with context: %s (%p)", request_info_->url.spec().c_str(), context);
        }
        request_->SetExtraRequestHeaders(request_info_->extra_headers);
        request_->set_method(request_info_->method);
        request_->SetLoadFlags(request_info_->load_flags);
        //TODO: request_->set_priority(request_info_->priority);
        request_->Start();
        return true;
    }

    // Called upon a server-initiated redirect.  The delegate may call the
    // request's Cancel method to prevent the redirect from being followed.
    // Since there may be multiple chained redirects, there may also be more
    // than one redirect call.
    //
    // When this function is called, the request will still contain the
    // original URL, the destination of the redirect is provided in 'new_url'.
    // If the delegate does not cancel the request and |*defer_redirect| is
    // false, then the redirect will be followed, and the request's URL will be
    // changed to the new URL.  Otherwise if the delegate does not cancel the
    // request and |*defer_redirect| is true, then the redirect will be
    // followed once FollowDeferredRedirect is called on the URLRequest.
    //
    // The caller must set |*defer_redirect| to false, so that delegates do not
    // need to set it if they are happy with the default behavior of not
    // deferring redirect.
virtual void OnReceivedRedirect(net::URLRequest* new_request, const net::RedirectInfo& new_url, bool* defer_redirect)
    {
        if(STAT_HUB_IS_VERBOSE_LEVEL_DEBUG) {
            LIBNETXT_LOGD("STAT_HUB - Fetch redirect canceled: %s -> %s",
                request_info_->url.spec().c_str(), new_url.new_url.spec().c_str());
        }
        if (NULL!=new_request) {
            new_request->Cancel();
        }
    }

    // After calling Start(), the delegate will receive an OnResponseStarted
    // callback when the request has completed.  If an error occurred, the
    // request->status() will be set.  On success, all redirects have been
    // followed and the final response is beginning to arrive.  At this point,
    // meta data about the response is available, including for example HTTP
    // response headers if this is a request for a HTTP resource.
virtual void OnResponseStarted(net::URLRequest* request) {
        int error_code = net::ERR_UNEXPECTED;
        const net::HttpResponseInfo* response_info = NULL;
        if (NULL!=request &&request->status().is_success()) {
            error_code = net::OK;
            response_info = &request->response_info();
        }
        OnStartCompleteHelper(error_code, response_info);
        if (error_code == net::OK) {
            StartReadFromRequest();
        }
    }

    // Called when the a Read of the response body is completed after an
    // IO_PENDING status from a Read() call.
    // The data read is filled into the buffer which the caller passed
    // to Read() previously.
    //
    // If an error occurred, request->status() will contain the error,
    // and bytes read will be -1.
virtual void OnReadCompleted(net::URLRequest* request, int bytesRead) {
        if (NULL!=request && request->status().is_success() && -1!=bytesRead) {
            ReadDone(bytesRead);
            StartReadFromRequest();
        }
        else {
            Finish(net::ERR_UNEXPECTED);
        }
    }

    //=========================================================================
    static StatHubContextSetType& ContextSet() {
        CR_DEFINE_STATIC_LOCAL(StatHubContextSetType, stat_hub_context_set_, ());
        return stat_hub_context_set_;
    }

private:
    friend class base::RefCountedThreadSafe<FetchRequest>;

    //=========================================================================
    ~FetchRequest() {
    }

    //=========================================================================
    void OnStartCompleteHelper(int error_code, const net::HttpResponseInfo* response_info) {
        if(STAT_HUB_IS_VERBOSE_LEVEL_DEBUG) {
            LIBNETXT_LOGD("STAT_HUB - Fetch transaction started: %s (%d)", request_info_->url.spec().c_str(), error_code);
        }

        if (error_code == net::OK) {
            StatHubCmd* cmd = STAT_HUB_API(CmdCreate)(SH_CMD_PRELOADER, SH_ACTION_DID_START_LOAD, 0);
            if (NULL!=cmd) {
                cmd->AddParamAsUint32(cookie_);
                cmd->AddParamAsUint32((unsigned int)error_code);
                cmd->AddParamAsUint32((unsigned int)response_info->headers->response_code());
                cmd->AddParamAsUint32((unsigned int)response_info->headers->GetContentLength());
                cmd->AddParamAsBuf(response_info->headers->raw_headers().data(), response_info->headers->raw_headers().size());
                STAT_HUB_API(CmdCommit)(cmd);
            }
            buf_ = new net::IOBuffer(READ_BUF_SIZE);
        }
        else {
            if (net::ERR_UNEXPECTED!=error_code) {
                LIBNETXT_LOGE("STAT_HUB - Fetch ERROR while starting transaction %d : %s",
                    error_code, request_info_->url.spec().c_str());
            }
            Finish(error_code);
        }
    }

    //=========================================================================
    void Finish(int error_code) {
        if(STAT_HUB_IS_VERBOSE_LEVEL_DEBUG) {
            LIBNETXT_LOGD("STAT_HUB - Fetch done: %s (%d)", request_info_->url.spec().c_str(), error_code);
        }
        StatHubCmd* cmd = STAT_HUB_API(CmdCreate)(SH_CMD_PRELOADER, SH_ACTION_DID_FINISH_LOAD, 0);
        if (NULL!=cmd) {
            cmd->AddParamAsUint32(cookie_);
            cmd->AddParamAsUint32((unsigned int)error_code);
            STAT_HUB_API(CmdCommit)(cmd);
        }
        delete this;
    }

    //=========================================================================
    void ReadDone(int bytes_to_read) {
        if (bytes_to_read > 0) {
            if(STAT_HUB_IS_VERBOSE_LEVEL_DEBUG) {
                LIBNETXT_LOGD("STAT_HUB - Fetch read: %s (%d)", request_info_->url.spec().c_str(), bytes_to_read);
            }
            StatHubCmd* cmd = STAT_HUB_API(CmdCreate)(SH_CMD_PRELOADER, SH_ACTION_DID_FINISH_READ, 0);
            if (NULL!=cmd) {
                cmd->AddParamAsUint32(cookie_);
                cmd->AddParamAsBuf((void *)buf_->data(), bytes_to_read);
                STAT_HUB_API(CmdCommit)(cmd);
            }
        }
    }

    //=========================================================================
    void StartReadFromRequest()
    {
        int bytes_read = 0;

        if(!request_->Read(buf_, READ_BUF_SIZE, &bytes_read)) {
            if (request_->status().is_io_pending()) {
                // Wait for OnReadCompleted()
                return;
            }
            LIBNETXT_LOGE("STAT_HUB - Fetch read from request ERROR: %s", request_info_->url.spec().c_str());
            Finish(net::ERR_UNEXPECTED);
            return;
        }
        if(bytes_read) {
            ReadDone(bytes_read);
            StartReadFromRequest();
        }
        else {
            //Done: bytes_read == 0 indicates finished
            Finish(net::OK);
        }
    }

    unsigned int cookie_;
    bool read_in_progress_;

    scoped_refptr<net::IOBuffer>        buf_;
    scoped_ptr<net::HttpRequestInfo>    request_info_;

    scoped_ptr<net::URLRequest> request_;

    DISALLOW_COPY_AND_ASSIGN(FetchRequest);
};

static void DoFetch(unsigned int cookie, net::HttpRequestInfo* request_info, net::URLRequestContext* context) {
    FetchRequest* fetch = new FetchRequest(cookie, request_info);
    if (context) {
        fetch->StartFetch(context);
    }
}

static void DoPreconnect(std::string* url, unsigned int count) {
    if(STAT_HUB_IS_VERBOSE_LEVEL_DEBUG) {
        LIBNETXT_LOGD("STAT_HUB - Preconnect: %s (%d) ", url->c_str(), count);
    }
    net::HttpCache* cache = StatHubGetHttpCache();
    if (cache) {
        net::HttpNetworkSession* session = cache->GetSession();
        if (session) {
            net::Preconnect::DoPreconnect(session, GURL(*url), count);
        }
    }
    delete url;
}

bool StatHubURLRequestContextCreated(net::URLRequestContext* context) {
    if(STAT_HUB_IS_VERBOSE_LEVEL_DEBUG) {
        LIBNETXT_LOGD("STAT_HUB - URL request context created: %p ", context);
    }
    FetchRequest::ContextSet().insert(context);
    return true;
}

bool StatHubURLRequestContextDestroyed(net::URLRequestContext* context) {
    if(STAT_HUB_IS_VERBOSE_LEVEL_DEBUG) {
        LIBNETXT_LOGD("STAT_HUB - URL request context destroyed: %p ", context);
    }
    FetchRequest::ContextSet().erase(context);
    return true;
}

bool StatHubIsCacheEnabled() {
    bool ret = false;

    StatHubCmd* cmd = STAT_HUB_API(CmdCreate)(SH_CMD_PRELOADER, SH_ACTION_IS_CACHE_ENABLED, 0);
    if (cmd) {
        if (STAT_HUB_API(CmdCommitSync)(cmd)) {
            ret = cmd->GetParamAsBool(0);
        }
        STAT_HUB_API(CmdRelease)(cmd);
    }
    return ret;
}

bool StatHubIsPreloaderEnabled() {
    bool ret = false;

    StatHubCmd* cmd = STAT_HUB_API(CmdCreate)(SH_CMD_PRELOADER, SH_ACTION_IS_PRELOADER_ENABLED, 0);
    if (cmd) {
        if (STAT_HUB_API(CmdCommitSync)(cmd)) {
            ret = cmd->GetParamAsBool(0);
        }
        STAT_HUB_API(CmdRelease)(cmd);
    }
    return ret;
}

unsigned int StatHubIsPreloaded(const char* url) {
    unsigned int ret = 0;

    StatHubCmd* cmd = STAT_HUB_API(CmdCreate)(SH_CMD_PRELOADER, SH_ACTION_IS_PRELOADED, 0);
    if (cmd) {
        cmd->AddParamAsString(url);
        if (STAT_HUB_API(CmdCommitSync)(cmd)) {
            ret = cmd->GetParamAsUint32(0);
        }
        STAT_HUB_API(CmdRelease)(cmd);
    }
    return ret;
}

bool StatHubGetPreloaded(const char* url, unsigned int get_from, std::string& headers, std::string& data, unsigned int& size) {
    bool ret = false;

    StatHubCmd* cmd = STAT_HUB_API(CmdCreate)(SH_CMD_PRELOADER, SH_ACTION_GET_PRELOADED, 0);
    if (cmd) {
        cmd->AddParamAsString(url);
        cmd->AddParamAsUint32(get_from);
        if (STAT_HUB_API(CmdCommitSync)(cmd)) {
            ret = cmd->GetParamAsBool(0);
            if (ret) {
                char* tmp_ptr = (char*)cmd->GetParamAsBuf(1, size);
                if (tmp_ptr && size) {
                    headers.assign(tmp_ptr, size);
                }
                else {
                    headers = "";
                }
                tmp_ptr = (char*)cmd->GetParamAsBuf(2, size);
                if (tmp_ptr && size) {
                    data.assign(tmp_ptr, size);
                }
                else {
                    data = "";
                }
            }
        }
        STAT_HUB_API(CmdRelease)(cmd);
    }
    return ret;
}

bool StatHubReleasePreloaded(const char* url) {
    bool ret = false;

    StatHubCmd* cmd = STAT_HUB_API(CmdCreate)(SH_CMD_PRELOADER, SH_ACTION_RELEASE_PRELOADED, 0);
    if (cmd) {
        cmd->AddParamAsString(url);
        STAT_HUB_API(CmdCommitSync)(cmd);
        STAT_HUB_API(CmdRelease)(cmd);
        ret = true;
    }
    return ret;
}

// ======================================= Exports ==============================================

bool StatHubIsVerboseEnabled() {
    return stat_hub::StatHub::GetInstance()->IsVerboseEnabled();
}

bool StatHubIsPerfEnabled() {
    return stat_hub::StatHub::GetInstance()->IsPerfEnabled();
}

StatHubVerboseLevel StatHubGetVerboseLevel() {
    return stat_hub::StatHub::GetInstance()->GetVerboseLevel();
}

unsigned int StatHubHash(const char* str) {
    if (str) {
        return base::Hash(str, strlen(str));
    }
    return 0;
}

bool StatHubFetch(unsigned int cookie, net::HttpRequestInfo* request_info, net::URLRequestContext* context) {
    base::MessageLoop* message_loop = StatHubGetIoMessageLoop();
    if (message_loop && request_info) {
        message_loop->PostTask(FROM_HERE, base::Bind(&DoFetch, cookie, request_info, context));
        return true;
    }
    return false;
}

bool StatHubPreconnect(const char* url, unsigned int count) {
    base::MessageLoop* message_loop = StatHubGetIoMessageLoop();
    if (message_loop && url) {
        message_loop->PostTask(FROM_HERE, base::Bind(&DoPreconnect, new std::string(url), count));
        return true;
    }
    return false;
}

bool StatHubGetDBmetaData(const char* key, std::string& val) {
    return stat_hub::StatHub::GetInstance()->GetDBmetaData(key, val);
}

bool StatHubSetDBmetaData(const char* key, const char* val) {
    return stat_hub::StatHub::GetInstance()->SetDBmetaData(key, val);
}

net::HttpCache* StatHubGetHttpCache() {
    return stat_hub::StatHub::GetInstance()->GetHttpCache();
}

base::MessageLoop* StatHubGetIoMessageLoop() {
    return stat_hub::StatHub::GetInstance()->GetIoMessageLoop();
}

bool StatHubSetIoMessageLoop(base::MessageLoop* message_loop) {
    stat_hub::StatHub::GetInstance()->SetIoMessageLoop(message_loop);
    if (NULL!=StatHubGetHttpCache()) {
        StatHubCmd* cmd = STAT_HUB_API(CmdCreate)(SH_CMD_CH_URL_REQUEST, SH_ACTION_FETCH_DELAYED, 0);
        if (NULL!=cmd) {
            STAT_HUB_API(CmdCommit)(cmd);
            return true;
        }
    }
    return false;
}

bool StatHubSetHttpCache(net::HttpCache* cache) {
    stat_hub::StatHub::GetInstance()->SetHttpCache(cache);
    if (NULL!=StatHubGetIoMessageLoop()) {
        StatHubCmd* cmd = STAT_HUB_API(CmdCreate)(SH_CMD_CH_URL_REQUEST, SH_ACTION_FETCH_DELAYED, 0);
        if (NULL!=cmd) {
            STAT_HUB_API(CmdCommit)(cmd);
            return true;
        }
    }
    return false;
}

// ================================ StatHub SQL Interface ====================================
LIBNETXT_API_CPP_FORWARDER_0(StatHub, sql, Connection, BeginTransaction, bool)
LIBNETXT_API_CPP_FORWARDER_0(StatHub, sql, Connection, CommitTransaction, bool)
LIBNETXT_API_CPP_FORWARDER_1(StatHub, sql, Connection, DoesTableExist, bool, const char*)
LIBNETXT_API_CPP_FORWARDER_1(StatHub, sql, Connection, Execute, bool, const char*)

LIBNETXT_API_CPP_FORWARDER_0(StatHub, sql, Statement, Step, bool)
LIBNETXT_API_CPP_FORWARDER_0(StatHub, sql, Statement, Run, bool)
LIBNETXT_API_CPP_FORWARDER_1V(StatHub, sql, Statement, Reset, void, bool)

LIBNETXT_API_CPP_FORWARDER_1(StatHub, sql, Statement, ColumnInt, int, int)
LIBNETXT_API_CPP_FORWARDER_1(StatHub, sql, Statement, ColumnInt64, int64, int)
LIBNETXT_API_CPP_FORWARDER_1(StatHub, sql, Statement, ColumnBool, bool, int)
LIBNETXT_API_CPP_FORWARDER_1(StatHub, sql, Statement, ColumnString, std::string, int)
LIBNETXT_API_CPP_FORWARDER_2(StatHub, sql, Statement, BindInt, bool, int, int)
LIBNETXT_API_CPP_FORWARDER_2(StatHub, sql, Statement, BindInt64, bool, int, int64)
LIBNETXT_API_CPP_FORWARDER_2(StatHub, sql, Statement, BindBool, bool, int, bool)
LIBNETXT_API_CPP_FORWARDER_2(StatHub, sql, Statement, BindCString, bool, int, const char*)

sql::Statement* StatHubGetStatement(sql::Connection* db, const sql::StatementID& id, const char* sql) {
    if(NULL!=db && NULL!=sql) {
        return new sql::Statement(db->GetCachedStatement(id, sql));
    }
    return NULL;
}

bool StatHubReleaseStatement(sql::Statement* st) {
    if (NULL!=st) {
        delete st;
        return true;
    }
    return false;
}

// ================================ StatHub Interface ====================================
bool StatHubIsReady() {
    return stat_hub::StatHub::GetInstance()->IsReady();
}

bool StatHubIsProcReady(const char* name) {
    return stat_hub::StatHub::GetInstance()->IsProcReady(name);
}
