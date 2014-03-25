/*
Copyright (c) 2014, The Linux Foundation. All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are
met:
    * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above
      copyright notice, this list of conditions and the following
      disclaimer in the documentation and/or other materials provided
      with the distribution.
    * Neither the name of The Linux Foundation nor the names of its
      contributors may be used to endorse or promote products derived
      from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED "AS IS" AND ANY EXPRESS OR IMPLIED
WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT
ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS
BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef NET_EXTERNAL_TYPES_H_
#define NET_EXTERNAL_TYPES_H_

#include "base/basictypes.h"
#include "base/format_macros.h"
#include "base/strings/stringprintf.h"
#include "base/supports_user_data.h"

namespace net
{

class STARequestMetaData : public base::SupportsUserData::Data
{
public:

    typedef enum {
      META_DATA_TYPE_BASIC = 0,
      META_DATA_TYPE_EXTENDED
    } STARequestMetaDataType;

    STARequestMetaData(uint64 issue_id,
                   int client_id,
                   int queue_id)
      : issue_id_(issue_id),
        client_id_(client_id),
        queue_id_(queue_id),
        type_(META_DATA_TYPE_BASIC)
      { }

   virtual ~STARequestMetaData()
      { }

   const uint64      issue_id_;
   const int         client_id_;
   const int         queue_id_;

   static const char * USER_DATA_KEY;

   STARequestMetaDataType type() { return type_; }

   virtual std::string ToString() {
      return base::StringPrintf("issue_id %"PRIu64" client_id %d queue_id %d "
              "type %u", issue_id_, client_id_, queue_id_, type_);
   }

protected:
   STARequestMetaDataType type_;

private:
   STARequestMetaData();
   DISALLOW_COPY_AND_ASSIGN(STARequestMetaData);
};

} //namespace net

#endif //NET_EXTERNAL_TYPES_H_
