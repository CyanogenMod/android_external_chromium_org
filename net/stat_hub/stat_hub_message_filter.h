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

#ifndef STAT_HUB_MESSAGE_FILTER_H_
#define STAT_HUB_MESSAGE_FILTER_H_

#include "content/public/browser/browser_message_filter.h"
#include "content/public/common/process_type.h"

namespace stat_hub {

    class StatHubMessageFilter : public content::BrowserMessageFilter {
public:
    // Create the filter.
    StatHubMessageFilter();

    // BrowserMessageFilter methods:
    virtual void OnChannelConnected(int32 peer_pid) OVERRIDE;
    virtual bool OnMessageReceived(const IPC::Message& message) OVERRIDE;

private:

    virtual ~StatHubMessageFilter();

    void OnCommit(const IPC::Message& msg);
    void OnGetCmdMask(unsigned int* ret);
    void OnIsReady(bool* ret);
    void OnIsPerfEnabled(bool* ret);
    void OnIsCacheEnabled(bool* ret);
    void OnIsPreloaderEnabled(bool* ret);
    void OnIsPreloaded(std::string url, unsigned int* ret);
    void OnGetPreloaded(std::string url, unsigned int get_from,
        std::string* headers, std::string* data, unsigned int* size, bool* ret);
    void OnReleasePreloaded(std::string url, bool* ret);

    DISALLOW_COPY_AND_ASSIGN(StatHubMessageFilter);
};

}  // namespace stat_hub

#endif  // STAT_HUB_MESSAGE_FILTER_H_
