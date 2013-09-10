// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "sql/test/test_helpers.h"

#include <string>

#include "base/file_util.h"
#include "sql/connection.h"
#include "sql/statement.h"
#include "testing/gtest/include/gtest/gtest.h"

namespace {

size_t CountSQLItemsOfType(sql::Connection* db, const char* type) {
  const char kTypeSQL[] = "SELECT COUNT(*) FROM sqlite_master WHERE type = ?";
  sql::Statement s(db->GetUniqueStatement(kTypeSQL));
  s.BindCString(0, type);
  EXPECT_TRUE(s.Step());
  return s.ColumnInt(0);
}

}  // namespace

namespace sql {
namespace test {

size_t CountSQLTables(sql::Connection* db) {
  return CountSQLItemsOfType(db, "table");
}

size_t CountSQLIndices(sql::Connection* db) {
  return CountSQLItemsOfType(db, "index");
}

size_t CountTableColumns(sql::Connection* db, const char* table) {
  // TODO(shess): sql::Connection::QuoteForSQL() would make sense.
  std::string quoted_table;
  {
    const char kQuoteSQL[] = "SELECT quote(?)";
    sql::Statement s(db->GetUniqueStatement(kQuoteSQL));
    s.BindCString(0, table);
    EXPECT_TRUE(s.Step());
    quoted_table = s.ColumnString(0);
  }

  std::string sql = "PRAGMA table_info(" + quoted_table + ")";
  sql::Statement s(db->GetUniqueStatement(sql.c_str()));
  size_t rows = 0;
  while (s.Step()) {
    ++rows;
  }
  EXPECT_TRUE(s.Succeeded());
  return rows;
}

bool CreateDatabaseFromSQL(const base::FilePath& db_path,
                           const base::FilePath& sql_path) {
  if (base::PathExists(db_path) || !base::PathExists(sql_path))
    return false;

  std::string sql;
  if (!base::ReadFileToString(sql_path, &sql))
    return false;

  sql::Connection db;
  if (!db.Open(db_path))
    return false;

  return db.Execute(sql.c_str());
}

}  // namespace test
}  // namespace sql
