// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Multiply-included message file, no traditonal include guard.
#include <string>
#include <vector>

#include "base/basictypes.h"
#include "base/string16.h"
#include "base/values.h"
#include "chrome/browser/history/history_types.h"
#include "chrome/browser/importer/importer_data_types.h"
#include "chrome/browser/importer/profile_writer.h"
#include "chrome/browser/search_engines/template_url.h"
#include "content/public/common/common_param_traits.h"
#include "content/public/common/webkit_param_traits.h"
#include "ipc/ipc_message_macros.h"
#include "ipc/ipc_message_utils.h"
#include "webkit/forms/password_form.h"

#ifndef CHROME_BROWSER_IMPORTER_PROFILE_IMPORT_PROCESS_MESSAGES_H_
#define CHROME_BROWSER_IMPORTER_PROFILE_IMPORT_PROCESS_MESSAGES_H_

namespace IPC {

// Traits for importer::SourceProfile struct to pack/unpack.
template <>
struct ParamTraits<importer::SourceProfile> {
  typedef importer::SourceProfile param_type;
  static void Write(Message* m, const param_type& p) {
    WriteParam(m, p.importer_name);
    WriteParam(m, static_cast<int>(p.importer_type));
    WriteParam(m, p.source_path);
    WriteParam(m, p.app_path);
    WriteParam(m, static_cast<int>(p.services_supported));
  }
  static bool Read(const Message* m, PickleIterator* iter, param_type* p) {
    if (!ReadParam(m, iter, &p->importer_name))
      return false;

    int importer_type = 0;
    if (!ReadParam(m, iter, &importer_type))
      return false;
    p->importer_type = static_cast<importer::ImporterType>(importer_type);

    if (!ReadParam(m, iter, &p->source_path) ||
        !ReadParam(m, iter, &p->app_path))
        return false;

    int services_supported = 0;
    if (!ReadParam(m, iter, &services_supported))
      return false;
    p->services_supported = static_cast<uint16>(services_supported);

    return true;
  }
  static void Log(const param_type& p, std::string* l) {
    l->append("(");
    LogParam(p.importer_name, l);
    l->append(", ");
    LogParam(static_cast<int>(p.importer_type), l);
    l->append(", ");
    LogParam(p.source_path, l);
    l->append(", ");
    LogParam(p.app_path, l);
    l->append(", ");
    LogParam(static_cast<int>(p.services_supported), l);
    l->append(")");
  }
};  // ParamTraits<importer::SourceProfile>

// Traits for history::URLRow to pack/unpack.
template <>
struct ParamTraits<history::URLRow> {
  typedef history::URLRow param_type;
  static void Write(Message* m, const param_type& p) {
    WriteParam(m, p.id());
    WriteParam(m, p.url());
    WriteParam(m, p.title());
    WriteParam(m, p.visit_count());
    WriteParam(m, p.typed_count());
    WriteParam(m, p.last_visit());
    WriteParam(m, p.hidden());
  }
  static bool Read(const Message* m, PickleIterator* iter, param_type* p) {
    history::URLID id;
    GURL url;
    string16 title;
    int visit_count, typed_count;
    base::Time last_visit;
    bool hidden;
    if (!ReadParam(m, iter, &id) ||
        !ReadParam(m, iter, &url) ||
        !ReadParam(m, iter, &title) ||
        !ReadParam(m, iter, &visit_count) ||
        !ReadParam(m, iter, &typed_count) ||
        !ReadParam(m, iter, &last_visit) ||
        !ReadParam(m, iter, &hidden))
      return false;
    *p = history::URLRow(url, id);
    p->set_title(title);
    p->set_visit_count(visit_count);
    p->set_typed_count(typed_count);
    p->set_last_visit(last_visit);
    p->set_hidden(hidden);
    return true;
  }
  static void Log(const param_type& p, std::string* l) {
    l->append("(");
    LogParam(p.id(), l);
    l->append(", ");
    LogParam(p.url(), l);
    l->append(", ");
    LogParam(p.title(), l);
    l->append(", ");
    LogParam(p.visit_count(), l);
    l->append(", ");
    LogParam(p.typed_count(), l);
    l->append(", ");
    LogParam(p.last_visit(), l);
    l->append(", ");
    LogParam(p.hidden(), l);
    l->append(")");
  }
};  // ParamTraits<history::URLRow>

// Traits for ProfileWriter::BookmarkEntry to pack/unpack.
template <>
struct ParamTraits<ProfileWriter::BookmarkEntry> {
  typedef ProfileWriter::BookmarkEntry param_type;
  static void Write(Message* m, const param_type& p) {
    WriteParam(m, p.in_toolbar);
    WriteParam(m, p.is_folder);
    WriteParam(m, p.url);
    WriteParam(m, p.path);
    WriteParam(m, p.title);
    WriteParam(m, p.creation_time);
  }
  static bool Read(const Message* m, PickleIterator* iter, param_type* p) {
    return
        (ReadParam(m, iter, &p->in_toolbar)) &&
        (ReadParam(m, iter, &p->is_folder)) &&
        (ReadParam(m, iter, &p->url)) &&
        (ReadParam(m, iter, &p->path)) &&
        (ReadParam(m, iter, &p->title)) &&
        (ReadParam(m, iter, &p->creation_time));
  }
  static void Log(const param_type& p, std::string* l) {
    l->append("(");
    LogParam(p.in_toolbar, l);
    l->append(", ");
    LogParam(p.is_folder, l);
    l->append(", ");
    LogParam(p.url, l);
    l->append(", ");
    LogParam(p.path, l);
    l->append(", ");
    LogParam(p.title, l);
    l->append(", ");
    LogParam(p.creation_time, l);
    l->append(")");
  }
};  // ParamTraits<ProfileWriter::BookmarkEntry>

// Traits for history::ImportedFaviconUsage.
template <>
struct ParamTraits<history::ImportedFaviconUsage> {
  typedef history::ImportedFaviconUsage param_type;
  static void Write(Message* m, const param_type& p) {
    WriteParam(m, p.favicon_url);
    WriteParam(m, p.png_data);
    WriteParam(m, p.urls);
  }
  static bool Read(const Message* m, PickleIterator* iter, param_type* p) {
    return
        ReadParam(m, iter, &p->favicon_url) &&
        ReadParam(m, iter, &p->png_data) &&
        ReadParam(m, iter, &p->urls);
  }
  static void Log(const param_type& p, std::string* l) {
    l->append("(");
    LogParam(p.favicon_url, l);
    l->append(", ");
    LogParam(p.png_data, l);
    l->append(", ");
    LogParam(p.urls, l);
    l->append(")");
  }
};  // ParamTraits<history::ImportedFaviconUsage

// Traits for TemplateURL*.
// WARNING: These will cause us to allocate a new TemplateURL on the heap on the
// receiver side.  Any messages using this type must have handlers that are
// careful to properly take ownership and avoid leaks!  See warning below on
// ProfileImportProcessHostMsg_NotifyKeywordsReady.
template <>
struct ParamTraits<TemplateURL*> {
  typedef TemplateURL* param_type;
  static void Write(Message* m, const param_type& p) {
    WriteParam(m, p->short_name());
    if (p->suggestions_url()) {
      WriteParam(m, true);
      WriteParam(m, p->suggestions_url()->url());
    } else {
      WriteParam(m, false);
    }
    WriteParam(m, p->url()->url());
    WriteParam(m, p->originating_url());
    WriteParam(m, p->keyword());
    WriteParam(m, p->autogenerate_keyword());
    WriteParam(m, p->show_in_default_list());
    WriteParam(m, p->safe_for_autoreplace());
    WriteParam(m, p->favicon_url());
    WriteParam(m, p->input_encodings());
    WriteParam(m, p->date_created());
    WriteParam(m, p->last_modified());
    WriteParam(m, p->usage_count());
    WriteParam(m, p->prepopulate_id());
  }
  static bool Read(const Message* m, PickleIterator* iter, param_type* p) {
    *p = NULL;

    string16 short_name;
    bool includes_suggestions_url;
    std::string suggestions_url;
    std::string url;
    GURL originating_url;
    string16 keyword;
    bool autogenerate_keyword;
    bool show_in_default_list;
    bool safe_for_autoreplace;
    GURL favicon_url;
    base::Time date_created;
    base::Time last_modified;
    int usage_count;
    int prepopulate_id;

    if (!ReadParam(m, iter, &short_name))
      return false;

    if (!ReadParam(m, iter, &includes_suggestions_url))
      return false;
    if (includes_suggestions_url) {
        if (!ReadParam(m, iter, &suggestions_url))
          return false;
    }

    if (!ReadParam(m, iter, &url) ||
        !ReadParam(m, iter, &originating_url) ||
        !ReadParam(m, iter, &keyword) ||
        !ReadParam(m, iter, &autogenerate_keyword) ||
        !ReadParam(m, iter, &show_in_default_list) ||
        !ReadParam(m, iter, &safe_for_autoreplace))
      return false;

    scoped_ptr<TemplateURL> turl(new TemplateURL());
    if (!ReadParam(m, iter, &favicon_url) ||
        !ReadParam(m, iter, &date_created) ||
        !ReadParam(m, iter, &last_modified) ||
        !ReadParam(m, iter, &usage_count) ||
        !ReadParam(m, iter, &prepopulate_id))
      return false;

    turl->set_short_name(short_name);
    turl->SetSuggestionsURL(suggestions_url);
    turl->SetURL(url);
    turl->set_originating_url(originating_url);
    turl->set_keyword(keyword);
    turl->set_autogenerate_keyword(autogenerate_keyword);
    turl->set_show_in_default_list(show_in_default_list);
    turl->set_safe_for_autoreplace(safe_for_autoreplace);
    turl->set_favicon_url(favicon_url);
    turl->set_date_created(date_created);
    turl->set_last_modified(last_modified);
    turl->set_usage_count(usage_count);
    turl->SetPrepopulateId(prepopulate_id);
    *p = turl.release();
    return true;
  }
  static void Log(const param_type& p, std::string* l) {
    l->append("<TemplateURL>");
  }
};

}  // namespace IPC

#endif  // CHROME_BROWSER_IMPORTER_PROFILE_IMPORT_PROCESS_MESSAGES_H_

#define IPC_MESSAGE_START ProfileImportMsgStart

//-----------------------------------------------------------------------------
// ProfileImportProcess messages
// These are messages sent from the browser to the profile import process.
IPC_MESSAGE_CONTROL3(ProfileImportProcessMsg_StartImport,
                     importer::SourceProfile,
                     int                     /* Bitmask of items to import. */,
                     DictionaryValue         /* Localized strings. */)

IPC_MESSAGE_CONTROL0(ProfileImportProcessMsg_CancelImport)

IPC_MESSAGE_CONTROL1(ProfileImportProcessMsg_ReportImportItemFinished,
                     int  /* ImportItem */)

//---------------------------------------------------------------------------
// ProfileImportProcessHost messages
// These are messages sent from the profile import process to the browser.
// These messages send information about the status of the import and
// individual import tasks.
IPC_MESSAGE_CONTROL0(ProfileImportProcessHostMsg_Import_Started)

IPC_MESSAGE_CONTROL2(ProfileImportProcessHostMsg_Import_Finished,
                     bool         /* was import successful? */,
                     std::string  /* error message, if any */)

IPC_MESSAGE_CONTROL1(ProfileImportProcessHostMsg_ImportItem_Started,
                     int  /* ImportItem */)

IPC_MESSAGE_CONTROL1(ProfileImportProcessHostMsg_ImportItem_Finished,
                     int  /* ImportItem */)

// These messages send data from the external importer process back to
// the process host so it can be written to the profile.
IPC_MESSAGE_CONTROL1(ProfileImportProcessHostMsg_NotifyHistoryImportStart,
                     int  /* total number of history::URLRow items */)

IPC_MESSAGE_CONTROL2(ProfileImportProcessHostMsg_NotifyHistoryImportGroup,
                     history::URLRows,
                     int  /* the source of URLs as in history::VisitSource.*/
                          /* To simplify IPC call, pass as an integer */)

IPC_MESSAGE_CONTROL1(ProfileImportProcessHostMsg_NotifyHomePageImportReady,
                     GURL  /* GURL of home page */)

IPC_MESSAGE_CONTROL2(ProfileImportProcessHostMsg_NotifyBookmarksImportStart,
                     string16  /* first folder name */,
                     int       /* total number of bookmarks */)

IPC_MESSAGE_CONTROL1(ProfileImportProcessHostMsg_NotifyBookmarksImportGroup,
                     std::vector<ProfileWriter::BookmarkEntry>)

IPC_MESSAGE_CONTROL1(ProfileImportProcessHostMsg_NotifyFaviconsImportStart,
                     int  /* total number of favicons */)

IPC_MESSAGE_CONTROL1(ProfileImportProcessHostMsg_NotifyFaviconsImportGroup,
                     std::vector<history::ImportedFaviconUsage>)

IPC_MESSAGE_CONTROL1(ProfileImportProcessHostMsg_NotifyPasswordFormReady,
                     webkit::forms::PasswordForm)

// WARNING: The TemplateURL*s in the following message get heap-allocated on the
// receiving end.  The message handler for this message MUST take ownership of
// these pointers and ensure they're properly freed!
IPC_MESSAGE_CONTROL2(ProfileImportProcessHostMsg_NotifyKeywordsReady,
                     std::vector<TemplateURL*>,
                     bool  /* unique on host and path */)
