// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "net/spdy/hpack_header_table.h"

#include <algorithm>

#include "base/logging.h"
#include "net/spdy/hpack_constants.h"
#include "net/spdy/hpack_string_util.h"

namespace net {

using base::StringPiece;

namespace {

// An entry in the static table. Must be a POD in order to avoid
// static initializers, i.e. no user-defined constructors or
// destructors.
struct StaticEntry {
  const char* const name;
  const size_t name_len;
  const char* const value;
  const size_t value_len;
};

// The "constructor" for a StaticEntry that computes the lengths at
// compile time.
#define STATIC_ENTRY(name, value) \
  { name, arraysize(name) - 1, value, arraysize(value) - 1 }

const StaticEntry kStaticTable[] = {
  STATIC_ENTRY(":authority"                  , ""),             // 1
  STATIC_ENTRY(":method"                     , "GET"),          // 2
  STATIC_ENTRY(":method"                     , "POST"),         // 3
  STATIC_ENTRY(":path"                       , "/"),            // 4
  STATIC_ENTRY(":path"                       , "/index.html"),  // 5
  STATIC_ENTRY(":scheme"                     , "http"),         // 6
  STATIC_ENTRY(":scheme"                     , "https"),        // 7
  STATIC_ENTRY(":status"                     , "200"),          // 8
  STATIC_ENTRY(":status"                     , "500"),          // 9
  STATIC_ENTRY(":status"                     , "404"),          // 10
  STATIC_ENTRY(":status"                     , "403"),          // 11
  STATIC_ENTRY(":status"                     , "400"),          // 12
  STATIC_ENTRY(":status"                     , "401"),          // 13
  STATIC_ENTRY("accept-charset"              , ""),             // 14
  STATIC_ENTRY("accept-encoding"             , ""),             // 15
  STATIC_ENTRY("accept-language"             , ""),             // 16
  STATIC_ENTRY("accept-ranges"               , ""),             // 17
  STATIC_ENTRY("accept"                      , ""),             // 18
  STATIC_ENTRY("access-control-allow-origin" , ""),             // 19
  STATIC_ENTRY("age"                         , ""),             // 20
  STATIC_ENTRY("allow"                       , ""),             // 21
  STATIC_ENTRY("authorization"               , ""),             // 22
  STATIC_ENTRY("cache-control"               , ""),             // 23
  STATIC_ENTRY("content-disposition"         , ""),             // 24
  STATIC_ENTRY("content-encoding"            , ""),             // 25
  STATIC_ENTRY("content-language"            , ""),             // 26
  STATIC_ENTRY("content-length"              , ""),             // 27
  STATIC_ENTRY("content-location"            , ""),             // 28
  STATIC_ENTRY("content-range"               , ""),             // 29
  STATIC_ENTRY("content-type"                , ""),             // 30
  STATIC_ENTRY("cookie"                      , ""),             // 31
  STATIC_ENTRY("date"                        , ""),             // 32
  STATIC_ENTRY("etag"                        , ""),             // 33
  STATIC_ENTRY("expect"                      , ""),             // 34
  STATIC_ENTRY("expires"                     , ""),             // 35
  STATIC_ENTRY("from"                        , ""),             // 36
  STATIC_ENTRY("host"                        , ""),             // 37
  STATIC_ENTRY("if-match"                    , ""),             // 38
  STATIC_ENTRY("if-modified-since"           , ""),             // 39
  STATIC_ENTRY("if-none-match"               , ""),             // 40
  STATIC_ENTRY("if-range"                    , ""),             // 41
  STATIC_ENTRY("if-unmodified-since"         , ""),             // 42
  STATIC_ENTRY("last-modified"               , ""),             // 43
  STATIC_ENTRY("link"                        , ""),             // 44
  STATIC_ENTRY("location"                    , ""),             // 45
  STATIC_ENTRY("max-forwards"                , ""),             // 46
  STATIC_ENTRY("proxy-authenticate"          , ""),             // 47
  STATIC_ENTRY("proxy-authorization"         , ""),             // 48
  STATIC_ENTRY("range"                       , ""),             // 49
  STATIC_ENTRY("referer"                     , ""),             // 50
  STATIC_ENTRY("refresh"                     , ""),             // 51
  STATIC_ENTRY("retry-after"                 , ""),             // 52
  STATIC_ENTRY("server"                      , ""),             // 53
  STATIC_ENTRY("set-cookie"                  , ""),             // 54
  STATIC_ENTRY("strict-transport-security"   , ""),             // 55
  STATIC_ENTRY("transfer-encoding"           , ""),             // 56
  STATIC_ENTRY("user-agent"                  , ""),             // 57
  STATIC_ENTRY("vary"                        , ""),             // 58
  STATIC_ENTRY("via"                         , ""),             // 59
  STATIC_ENTRY("www-authenticate"            , ""),             // 60
};

#undef STATIC_ENTRY

}  // namespace

HpackHeaderTable::HpackHeaderTable()
    : settings_size_bound_(kDefaultHeaderTableSizeSetting),
      size_(0),
      max_size_(kDefaultHeaderTableSizeSetting),
      total_insertions_(0),
      dynamic_entries_count_(0) {
  for (const StaticEntry* it = kStaticTable;
       it != kStaticTable + arraysize(kStaticTable); ++it) {
    static_entries_.push_back(
        HpackEntry(StringPiece(it->name, it->name_len),
                   StringPiece(it->value, it->value_len),
                   true,  // is_static
                   total_insertions_,
                   &dynamic_entries_count_));
    CHECK(index_.insert(&static_entries_.back()).second);

    ++total_insertions_;
  }
}

HpackHeaderTable::~HpackHeaderTable() {}

HpackEntry* HpackHeaderTable::GetByIndex(size_t index) {
  if (index == 0) {
    return NULL;
  }
  index -= 1;
  if (index < dynamic_entries_.size()) {
    return &dynamic_entries_[index];
  }
  index -= dynamic_entries_.size();
  if (index < static_entries_.size()) {
    return &static_entries_[index];
  }
  return NULL;
}

HpackEntry* HpackHeaderTable::GetByName(StringPiece name) {
  HpackEntry query(name, "");
  HpackEntry::OrderedSet::const_iterator it = index_.lower_bound(&query);
  if (it != index_.end() && (*it)->name() == name) {
    return *it;
  }
  return NULL;
}

HpackEntry* HpackHeaderTable::GetByNameAndValue(StringPiece name,
                                                StringPiece value) {
  HpackEntry query(name, value);
  HpackEntry::OrderedSet::const_iterator it = index_.lower_bound(&query);
  if (it != index_.end() && (*it)->name() == name && (*it)->value() == value) {
    return *it;
  }
  return NULL;
}

void HpackHeaderTable::SetMaxSize(size_t max_size) {
  CHECK_LE(max_size, settings_size_bound_);

  max_size_ = max_size;
  if (size_ > max_size_) {
    Evict(EvictionCountToReclaim(size_ - max_size_));
    CHECK_LE(size_, max_size_);
  }
}

void HpackHeaderTable::SetSettingsHeaderTableSize(size_t settings_size) {
  settings_size_bound_ = settings_size;
  if (settings_size_bound_ < max_size_) {
    SetMaxSize(settings_size_bound_);
  }
}

void HpackHeaderTable::EvictionSet(StringPiece name,
                                   StringPiece value,
                                   EntryTable::iterator* begin_out,
                                   EntryTable::iterator* end_out) {
  size_t eviction_count = EvictionCountForEntry(name, value);
  *begin_out = dynamic_entries_.end() - eviction_count;
  *end_out = dynamic_entries_.end();
}

size_t HpackHeaderTable::EvictionCountForEntry(StringPiece name,
                                               StringPiece value) const {
  size_t available_size = max_size_ - size_;
  size_t entry_size = HpackEntry::Size(name, value);

  if (entry_size <= available_size) {
    // No evictions are required.
    return 0;
  }
  return EvictionCountToReclaim(entry_size - available_size);
}

size_t HpackHeaderTable::EvictionCountToReclaim(size_t reclaim_size) const {
  size_t count = 0;
  for (EntryTable::const_reverse_iterator it = dynamic_entries_.rbegin();
       it != dynamic_entries_.rend() && reclaim_size != 0; ++it, ++count) {
    reclaim_size -= std::min(reclaim_size, it->Size());
  }
  return count;
}

void HpackHeaderTable::Evict(size_t count) {
  for (size_t i = 0; i != count; ++i) {
    CHECK(!dynamic_entries_.empty());
    HpackEntry* entry = &dynamic_entries_.back();

    size_ -= entry->Size();
    CHECK_EQ(1u, index_.erase(entry));
    reference_set_.erase(entry);
    dynamic_entries_.pop_back();

    --dynamic_entries_count_;
    DCHECK_EQ(dynamic_entries_count_, dynamic_entries_.size());
  }
}

HpackEntry* HpackHeaderTable::TryAddEntry(StringPiece name, StringPiece value) {
  Evict(EvictionCountForEntry(name, value));

  size_t entry_size = HpackEntry::Size(name, value);
  if (entry_size > (max_size_ - size_)) {
    // Entire table has been emptied, but there's still insufficient room.
    DCHECK(dynamic_entries_.empty());
    DCHECK_EQ(0u, size_);
    return NULL;
  }
  dynamic_entries_.push_front(HpackEntry(name,
                                         value,
                                         false,  // is_static
                                         total_insertions_,
                                         &total_insertions_));
  CHECK(index_.insert(&dynamic_entries_.front()).second);

  size_ += entry_size;
  ++dynamic_entries_count_;
  ++total_insertions_;

  DCHECK_EQ(dynamic_entries_count_, dynamic_entries_.size());
  return &dynamic_entries_.front();
}

void HpackHeaderTable::ClearReferenceSet() {
  for (HpackEntry::OrderedSet::iterator it = reference_set_.begin();
       it != reference_set_.end(); ++it) {
    (*it)->set_state(0);
  }
  reference_set_.clear();
}

bool HpackHeaderTable::Toggle(HpackEntry* entry) {
  CHECK(!entry->IsStatic());
  CHECK_EQ(0u, entry->state());

  std::pair<HpackEntry::OrderedSet::iterator, bool> insert_result =
      reference_set_.insert(entry);
  if (insert_result.second) {
    return true;
  } else {
    reference_set_.erase(insert_result.first);
    return false;
  }
}

void HpackHeaderTable::DebugLogTableState() const {
  DVLOG(2) << "Reference Set:";
  for (HpackEntry::OrderedSet::const_iterator it = reference_set_.begin();
      it != reference_set_.end(); ++it) {
    DVLOG(2) << "  " << (*it)->GetDebugString();
  }
  DVLOG(2) << "Dynamic table:";
  for (EntryTable::const_iterator it = dynamic_entries_.begin();
      it != dynamic_entries_.end(); ++it) {
    DVLOG(2) << "  " << it->GetDebugString();
  }
  DVLOG(2) << "Full Index:";
  for (HpackEntry::OrderedSet::const_iterator it = index_.begin();
      it != index_.end(); ++it) {
    DVLOG(2) << "  " << (*it)->GetDebugString();
  }
}

}  // namespace net
