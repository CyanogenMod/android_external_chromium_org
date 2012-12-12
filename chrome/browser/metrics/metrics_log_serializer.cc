// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/metrics/metrics_log_serializer.h"

#include "base/base64.h"
#include "base/md5.h"
#include "base/metrics/histogram.h"
#include "chrome/browser/browser_process.h"
#include "chrome/browser/prefs/pref_service.h"
#include "chrome/browser/prefs/scoped_user_pref_update.h"
#include "chrome/common/pref_names.h"

namespace {

// The number of "initial" logs to save, and hope to send during a future Chrome
// session.  Initial logs contain crash stats, and are pretty small.
const size_t kInitialLogsPersistLimit = 20;

// The number of ongoing logs to save persistently, and hope to
// send during a this or future sessions.  Note that each log may be pretty
// large, as presumably the related "initial" log wasn't sent (probably nothing
// was, as the user was probably off-line).  As a result, the log probably kept
// accumulating while the "initial" log was stalled, and couldn't be sent.  As a
// result, we don't want to save too many of these mega-logs.
// A "standard shutdown" will create a small log, including just the data that
// was not yet been transmitted, and that is normal (to have exactly one
// ongoing_log_ at startup).
const size_t kOngoingLogsPersistLimit = 8;

// The number of bytes each of initial and ongoing logs that must be stored.
// This ensures that a reasonable amount of history will be stored even if there
// is a long series of very small logs.
const size_t kStorageByteLimitPerLogType = 300000;

// We append (2) more elements to persisted lists: the size of the list and a
// checksum of the elements.
const size_t kChecksumEntryCount = 2;

MetricsLogSerializer::LogReadStatus MakeRecallStatusHistogram(
    MetricsLogSerializer::LogReadStatus status,
    bool is_xml) {
  if (is_xml) {
    UMA_HISTOGRAM_ENUMERATION("PrefService.PersistentLogRecall",
                              status, MetricsLogSerializer::END_RECALL_STATUS);
  } else {
    UMA_HISTOGRAM_ENUMERATION("PrefService.PersistentLogRecallProtobufs",
                              status, MetricsLogSerializer::END_RECALL_STATUS);
  }
  return status;
}

}  // namespace


MetricsLogSerializer::MetricsLogSerializer() {}

MetricsLogSerializer::~MetricsLogSerializer() {}

void MetricsLogSerializer::SerializeLogs(
    const std::vector<MetricsLogManager::SerializedLog>& logs,
    MetricsLogManager::LogType log_type) {
  PrefService* local_state = g_browser_process->local_state();
  DCHECK(local_state);
  const char* pref_xml = NULL;
  const char* pref_proto = NULL;
  size_t store_length_limit = 0;
  switch (log_type) {
    case MetricsLogManager::INITIAL_LOG:
      pref_xml = prefs::kMetricsInitialLogsXml;
      pref_proto = prefs::kMetricsInitialLogsProto;
      store_length_limit = kInitialLogsPersistLimit;
      break;
    case MetricsLogManager::ONGOING_LOG:
      pref_xml = prefs::kMetricsOngoingLogsXml;
      pref_proto = prefs::kMetricsOngoingLogsProto;
      store_length_limit = kOngoingLogsPersistLimit;
      break;
    default:
      NOTREACHED();
      return;
  };

  // Write the XML version.
  ListPrefUpdate update_xml(local_state, pref_xml);
  WriteLogsToPrefList(logs, true, store_length_limit,
                      kStorageByteLimitPerLogType, update_xml.Get());

  // Write the protobuf version.
  ListPrefUpdate update_proto(local_state, pref_proto);
  WriteLogsToPrefList(logs, false, store_length_limit,
                      kStorageByteLimitPerLogType, update_proto.Get());
}

void MetricsLogSerializer::DeserializeLogs(
    MetricsLogManager::LogType log_type,
    std::vector<MetricsLogManager::SerializedLog>* logs) {
  DCHECK(logs);
  PrefService* local_state = g_browser_process->local_state();
  DCHECK(local_state);

  const char* pref_xml;
  const char* pref_proto;
  if (log_type == MetricsLogManager::INITIAL_LOG) {
    pref_xml = prefs::kMetricsInitialLogsXml;
    pref_proto = prefs::kMetricsInitialLogsProto;
  } else {
    pref_xml = prefs::kMetricsOngoingLogsXml;
    pref_proto = prefs::kMetricsOngoingLogsProto;
  }

  const ListValue* unsent_logs_xml = local_state->GetList(pref_xml);
  const ListValue* unsent_logs_proto = local_state->GetList(pref_proto);
  if (ReadLogsFromPrefList(*unsent_logs_xml, true, logs) == RECALL_SUCCESS) {
    // In order to try to keep the data sent to both servers roughly in sync,
    // only read the protobuf data if we read the XML data successfully.
    ReadLogsFromPrefList(*unsent_logs_proto, false, logs);
  }
}

// static
void MetricsLogSerializer::WriteLogsToPrefList(
    const std::vector<MetricsLogManager::SerializedLog>& local_list,
    bool is_xml,
    size_t list_length_limit,
    size_t byte_limit,
    base::ListValue* list) {
  // One of the limit arguments must be non-zero.
  DCHECK(list_length_limit > 0 || byte_limit > 0);

  list->Clear();
  if (local_list.size() == 0)
    return;

  size_t start = 0;
  // If there are too many logs, keep the most recent logs up to the length
  // limit, and at least to the minimum number of bytes.
  if (local_list.size() > list_length_limit) {
    start = local_list.size();
    size_t bytes_used = 0;
    for (std::vector<MetricsLogManager::SerializedLog>::const_reverse_iterator
         it = local_list.rbegin(); it != local_list.rend(); ++it) {
      size_t log_size = it->length();
      if (bytes_used >= byte_limit &&
          (local_list.size() - start) >= list_length_limit)
        break;
      bytes_used += log_size;
      --start;
    }
  }
  DCHECK_LT(start, local_list.size());
  if (start >= local_list.size())
    return;

  // Store size at the beginning of the list.
  list->Append(Value::CreateIntegerValue(local_list.size() - start));

  base::MD5Context ctx;
  base::MD5Init(&ctx);
  std::string encoded_log;
  for (std::vector<MetricsLogManager::SerializedLog>::const_iterator it =
           local_list.begin() + start;
       it != local_list.end(); ++it) {
    const std::string& value = is_xml ? it->xml : it->proto;
    // We encode the compressed log as Value::CreateStringValue() expects to
    // take a valid UTF8 string.
    if (!base::Base64Encode(value, &encoded_log)) {
      list->Clear();
      return;
    }
    base::MD5Update(&ctx, encoded_log);
    list->Append(Value::CreateStringValue(encoded_log));
  }

  // Append hash to the end of the list.
  base::MD5Digest digest;
  base::MD5Final(&digest, &ctx);
  list->Append(Value::CreateStringValue(base::MD5DigestToBase16(digest)));
  DCHECK(list->GetSize() >= 3);  // Minimum of 3 elements (size, data, hash).
}

// static
MetricsLogSerializer::LogReadStatus MetricsLogSerializer::ReadLogsFromPrefList(
    const ListValue& list,
    bool is_xml,
    std::vector<MetricsLogManager::SerializedLog>* local_list) {
  if (list.GetSize() == 0)
    return MakeRecallStatusHistogram(LIST_EMPTY, is_xml);
  if (list.GetSize() < 3)
    return MakeRecallStatusHistogram(LIST_SIZE_TOO_SMALL, is_xml);

  // The size is stored at the beginning of the list.
  int size;
  bool valid = (*list.begin())->GetAsInteger(&size);
  if (!valid)
    return MakeRecallStatusHistogram(LIST_SIZE_MISSING, is_xml);
  // Account for checksum and size included in the list.
  if (static_cast<unsigned int>(size) !=
      list.GetSize() - kChecksumEntryCount) {
    return MakeRecallStatusHistogram(LIST_SIZE_CORRUPTION, is_xml);
  }

  // Allocate strings for all of the logs we are going to read in.
  // Do this ahead of time so that we can decode the string values directly into
  // the elements of |local_list|, and thereby avoid making copies of the
  // serialized logs, which can be fairly large.
  if (is_xml) {
    DCHECK(local_list->empty());
    local_list->resize(size);
  } else if (local_list->size() != static_cast<size_t>(size)) {
    return MakeRecallStatusHistogram(XML_PROTO_MISMATCH, false);
  }

  base::MD5Context ctx;
  base::MD5Init(&ctx);
  std::string encoded_log;
  size_t local_index = 0;
  for (ListValue::const_iterator it = list.begin() + 1;
       it != list.end() - 1;  // Last element is the checksum.
       ++it, ++local_index) {
    bool valid = (*it)->GetAsString(&encoded_log);
    if (!valid) {
      local_list->clear();
      return MakeRecallStatusHistogram(LOG_STRING_CORRUPTION, is_xml);
    }

    base::MD5Update(&ctx, encoded_log);

    DCHECK_LT(local_index, local_list->size());
    std::string& decoded_log = is_xml ?
        (*local_list)[local_index].xml :
        (*local_list)[local_index].proto;
    if (!base::Base64Decode(encoded_log, &decoded_log)) {
      local_list->clear();
      return MakeRecallStatusHistogram(DECODE_FAIL, is_xml);
    }
  }

  // Verify checksum.
  base::MD5Digest digest;
  base::MD5Final(&digest, &ctx);
  std::string recovered_md5;
  // We store the hash at the end of the list.
  valid = (*(list.end() - 1))->GetAsString(&recovered_md5);
  if (!valid) {
    local_list->clear();
    return MakeRecallStatusHistogram(CHECKSUM_STRING_CORRUPTION, is_xml);
  }
  if (recovered_md5 != base::MD5DigestToBase16(digest)) {
    local_list->clear();
    return MakeRecallStatusHistogram(CHECKSUM_CORRUPTION, is_xml);
  }
  return MakeRecallStatusHistogram(RECALL_SUCCESS, is_xml);
}
