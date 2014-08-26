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

#ifndef STAT_HUB_CMD_H_
#define STAT_HUB_CMD_H_

#include "base/compiler_specific.h"
#include "build/build_config.h"

#include <string>
#include <algorithm>
#include <vector>
#include "base/time/time.h"
#include "stat_hub_cmd_def.h"

#define STAT_HUB_MAX_NUM_OF_ACTIONS     10

// ========================================================================
#define STAT_HUB_CMD_HANDLER_INIT(handlers_list, cmd_mask, cmd_id, handler, ...)    \
    {                                                                               \
        StatHubActionType act[] = {__VA_ARGS__, SH_ACTION_LAST};                    \
        handlers_list.push_back(new StatHubCmdHandlerFunction(cmd_id, &handler, act));   \
        cmd_mask |= (1<<cmd_id);                                                    \
    }

// ========================================================================
#define STAT_HUB_CMD_HANDLER_CONTAINER_INIT(handlers_list, cmd_mask, cmd_id, handler, ...) \
    {                                                                   \
        StatHubActionType act[] = {__VA_ARGS__, SH_ACTION_LAST};        \
        handlers_list.push_back(new StatHubCmdHandlerMethod(cmd_id,     \
            new StatHubCmdHandlerContainer##handler (*this), act));     \
        cmd_mask |= (1<<cmd_id);                                        \
    }

// ========================================================================
#define STAT_HUB_CMD_HANDLER_CONTAINER_IMPL(container, handler)                     \
    class StatHubCmdHandlerContainer##handler : public StatHubCmdHandlerContainer { \
    public:                                                                         \
        StatHubCmdHandlerContainer##handler(container& object):                     \
            object_(object) {};                                                     \
        virtual bool Invoke(StatHubCmd* cmd) OVERRIDE;                              \
    private:                                                                        \
        container& object_;                                                         \
    };                                                                              \
    bool StatHubCmdHandlerContainer##handler::Invoke(StatHubCmd* cmd) {             \
        return object_.handler(cmd);                                                \
    }                                                                               \
    bool container::handler(StatHubCmd* cmd)


typedef base::Time StatHubTimeStamp;

template <typename T> void DeleteStatHubParam(T *p) {
    delete p;
}

namespace base {
    class MessageLoop;
}

namespace net {
    class URLRequestContext;
    class HttpCache;
}

class StatHubCmd {
public:

inline StatHubCmdType GetCmd() {
        return cmd_;
    }

inline StatHubActionType GetAction() {
        return action_;
    }

inline unsigned int GetCookie() {
        return cookie_;
    }

inline StatHubTimeStamp& GetStartTimeStamp() {
        return start_timestamp_;
    }

inline StatHubTimeStamp& GetCommitTimeStamp() {
        return commit_timestamp_;
    }

inline void AddParamAsUint32(unsigned int param) {
        params_.push_back(new StatHubCmdParam(param));
    }

inline void AddParamAsString(const char* param) {
        params_.push_back(new StatHubCmdParam(param));
    }

inline void AddParamAsPtr(void* param) {
        params_.push_back(new StatHubCmdParam(param));
    }

inline void AddParamAsBuf(const void* param, unsigned int size) {
        params_.push_back(new StatHubCmdParam(param, size));
    }

inline void AddParamAsBool(bool param) {
        params_.push_back(new StatHubCmdParam(param));
    }

inline void SetStartTimeStamp(StatHubTimeStamp timestamp) {
        start_timestamp_ = timestamp;
    }

inline void SetCommitTimeStamp(StatHubTimeStamp timestamp) {
        commit_timestamp_ = timestamp;
    }

inline void SetStat(char* stat) {
        stat_ = stat;
    }

inline std::string& GetStat() {
        return stat_;
    }

    unsigned int GetParamAsUint32(unsigned int param_index) {
        if (param_index<params_.size()) {
            return params_[param_index]->param_uint32_;
        }
        return 0;
    }

    const char* GetParamAsString(unsigned int param_index) {
        if (param_index<params_.size()) {
            return (const char*)params_[param_index]->param_;
        }
        return NULL;
    }

    void* GetParamAsPtr(unsigned int param_index) {
        if (param_index<params_.size()) {
            return params_[param_index]->param_;
        }
        return NULL;
    }

    void* GetParamAsBuf(unsigned int param_index, unsigned int& size) {
        if (param_index<params_.size()) {
            size = params_[param_index]->param_size_;
            return params_[param_index]->param_;
        }
        size = 0;
        return NULL;
    }

    bool GetParamAsBool(unsigned int param_index) {
        if (param_index<params_.size()) {
            return (bool)params_[param_index]->param_uint32_;
        }
        return false;
    }

static void Release(StatHubCmd* cmd) {
        if (NULL!=cmd) {
            cmd->referenced_--;
            if(0==cmd->referenced_) {
                delete cmd;
            }
        }
    }

    void IncReference() {
        referenced_++;
    }

    StatHubCmd(StatHubCmdType cmd, StatHubActionType action, unsigned int cookie);

    void ResetParams() {
        std::for_each(params_.begin(), params_.end(), DeleteStatHubParam<StatHubCmdParam>);
        params_.clear();
    }

    StatHubCmd();

    virtual ~StatHubCmd();

    class StatHubCmdParam
    {
    public:
        StatHubCmdParam(unsigned int param_uint32, void* param): param_size_(0) {
            param_uint32_ = param_uint32;
            param_ = param;
        }

        StatHubCmdParam(unsigned int param): param_(0), param_size_(0) {
            param_uint32_ = param;
        }

        StatHubCmdParam(const char* param): param_(0), param_uint32_(0), param_size_(0) {
            if(NULL!=param) {
                Init(param, strlen(param)+1);
            }
        }

        StatHubCmdParam(void* param): param_uint32_(0), param_size_(0) {
            param_ = param;
        }

        StatHubCmdParam(const void* param, unsigned int param_size): param_(0), param_uint32_(0), param_size_(0) {
            Init(param, param_size);
        }


        StatHubCmdParam(bool param): param_(0), param_size_(0) {
            param_uint32_ = (unsigned int)param;
        }

        ~StatHubCmdParam() {
            if (0!=param_size_ && NULL!=param_) {
                delete[] (char*)param_;
            }
        }

        void Init(const void* param, unsigned int param_size) {
            if(NULL!=param && 0!=param_size) {
                param_size_ = param_size;
                param_ = new char[param_size_];
                memcpy(param_, param, param_size_);
            }
        }

        friend class StatHubCmd;
        StatHubCmdParam() {}

        void*           param_;
        unsigned int    param_uint32_;
        unsigned int    param_size_;
    };

    typedef std::vector<StatHubCmdParam*> StatHubCmdParamsType;

    StatHubCmdType          cmd_;
    StatHubActionType       action_;
    unsigned int            cookie_;
    StatHubTimeStamp        start_timestamp_;
    StatHubTimeStamp        commit_timestamp_;
    StatHubCmdParamsType    params_;

    unsigned int            referenced_;

    //performance
    std::string             stat_;
};

typedef bool (*StatHubCmdHandlerFuncType)(StatHubCmd* cmd);

// ========================================================================
class StatHubCmdHandlerContainer {
public:
    virtual ~StatHubCmdHandlerContainer() {};
    virtual bool Invoke(StatHubCmd* cmd) = 0;
};

// ========================================================================
class StatHubCmdHandler {
public:

    StatHubCmdHandler(StatHubCmdType cmd, StatHubActionType act[]);

    bool Invoke(StatHubCmd* cmd) {
        StatHubCmdType cmd_id = cmd->GetCmd();

        if (cmd_==cmd_id) {
            StatHubActionType action_id = cmd->GetAction();

            for (unsigned int index = 0; index<act_size_; index++) {
                if (act_[index]==action_id) {
                    return InvokeInternal(cmd);
                }
            }
        }
        return false;
    }

protected:
    virtual bool InvokeInternal(StatHubCmd* cmd) = 0;

    StatHubCmdType      cmd_;
    StatHubActionType   act_[STAT_HUB_MAX_NUM_OF_ACTIONS];
    unsigned int        act_size_;

};

// ========================================================================
class StatHubCmdHandlerFunction : public StatHubCmdHandler {
public:
    StatHubCmdHandlerFunction(StatHubCmdType cmd, StatHubCmdHandlerFuncType handler, StatHubActionType act[]);
    virtual ~StatHubCmdHandlerFunction();
    virtual bool InvokeInternal(StatHubCmd* cmd) OVERRIDE;

    StatHubCmdHandlerFuncType   handler_;
};

// ========================================================================
class StatHubCmdHandlerMethod : public StatHubCmdHandler {
public:
    StatHubCmdHandlerMethod(StatHubCmdType cmd, StatHubCmdHandlerContainer* handler, StatHubActionType act[]);
    virtual ~StatHubCmdHandlerMethod();
    virtual bool InvokeInternal(StatHubCmd* cmd) OVERRIDE;

    StatHubCmdHandlerContainer*   handler_;
};

// ========================================================================
#define STAT_HUB_CMD_HANDLER_IMPL() \
    StatHubCmdHandler::StatHubCmdHandler(StatHubCmdType cmd, StatHubActionType act[]): cmd_(cmd) {       \
        for (act_size_=0; act[act_size_]!=SH_ACTION_LAST; act_size_++);                                  \
        if (act_size_>0) {                                                                               \
            for (unsigned int index = 0; index<act_size_ && index<STAT_HUB_MAX_NUM_OF_ACTIONS; index++) {\
                act_[index] = act[index];                                                                \
            }                                                                                            \
        }                                                                                                \
    }                                                                                                    \
    StatHubCmdHandlerFunction::StatHubCmdHandlerFunction(                                                \
        StatHubCmdType cmd, StatHubCmdHandlerFuncType handler, StatHubActionType act[]):                 \
        StatHubCmdHandler(cmd, act), handler_(handler) {}                                                \
    StatHubCmdHandlerFunction::~StatHubCmdHandlerFunction() {}                                           \
    bool StatHubCmdHandlerFunction::InvokeInternal(StatHubCmd* cmd) { return handler_(cmd);}             \
    StatHubCmdHandlerMethod::StatHubCmdHandlerMethod(                                                    \
        StatHubCmdType cmd, StatHubCmdHandlerContainer* handler, StatHubActionType act[]):               \
        StatHubCmdHandler(cmd, act), handler_(handler) {}                                                \
    StatHubCmdHandlerMethod::~StatHubCmdHandlerMethod() { if (handler_) { delete handler_; }}            \
    bool StatHubCmdHandlerMethod::InvokeInternal(StatHubCmd* cmd) { return handler_->Invoke(cmd); }

typedef std::vector<StatHubCmdHandler*> StatHubCmdHandlersList;

#endif /* STAT_HUB_CMD_H_ */
