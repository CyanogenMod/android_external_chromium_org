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

#ifndef STAT_HUB_DEF_H_
#define STAT_HUB_DEF_H_

#define PROP_VAL_TO_STR(val) PROP_VAL_TO_STR_HELPER(val)
#define PROP_VAL_TO_STR_HELPER(val) #val
#define STAT_MAX_PARAM_LEN  2048

#define STAT_HUB_DEV_LOG_ENABLED    (false)

#define SH_PREFETCH_PRIO_OPTIONAL   65535
#define SH_PREFETCH_PRIO_LOW        1024
#define SH_PREFETCH_PRIO_NORMAL     512
#define SH_PREFETCH_PRIO_HIGH       0

#define SH_PRELOADED_TO_MEMCACHE    0x00000001
#define SH_PRELOADED_TO_DISKCACHE   0x00000002
#define SH_PRELOADED_TO_INTCACHE    0x00000004

typedef enum StatHubVerboseLevel {
    STAT_HUB_VERBOSE_LEVEL_DISABLED,// 0
    STAT_HUB_VERBOSE_LEVEL_ERROR,   // 1
    STAT_HUB_VERBOSE_LEVEL_WARNING, // 2
    STAT_HUB_VERBOSE_LEVEL_INFO,    // 3
    STAT_HUB_VERBOSE_LEVEL_DEBUG    // 4
} StatHubVerboseLevel;

typedef enum StatHubMimeType {
    StatHubMimeMainResource,       // 0
    StatHubMimeImageResource,      // 1
    StatHubMimeCSSStyleSheet,      // 2
    StatHubMimeScript,             // 3
    StatHubMimeFontResource,       // 4
    StatHubMimeRawResource,        // 5
    StatHubMimeUnDefined = 1024    // 1024
} StatHubMimeType;

#endif /* STAT_HUB_DEF_H_ */
