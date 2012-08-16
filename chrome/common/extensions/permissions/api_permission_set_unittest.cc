// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "base/pickle.h"
#include "base/values.h"
#include "chrome/common/extensions/extension_messages.h"
#include "chrome/common/extensions/permissions/api_permission_set.h"
#include "chrome/common/extensions/permissions/permissions_info.h"
#include "ipc/ipc_message.h"
#include "testing/gtest/include/gtest/gtest.h"

namespace extensions {

class APIPermissionSetTest : public testing::Test {
};

TEST(APIPermissionSetTest, General) {
  APIPermissionSet apis;
  apis.insert(APIPermission::kTab);
  apis.insert(APIPermission::kBackground);
  apis.insert(APIPermission::kProxy);
  apis.insert(APIPermission::kClipboardWrite);
  apis.insert(APIPermission::kPlugin);

  EXPECT_EQ(apis.find(APIPermission::kProxy)->id(), APIPermission::kProxy);
  EXPECT_TRUE(apis.find(APIPermission::kSocket) == apis.end());

  EXPECT_EQ(apis.size(), 5u);

  EXPECT_EQ(apis.erase(APIPermission::kTab), 1u);
  EXPECT_EQ(apis.size(), 4u);

  EXPECT_EQ(apis.erase(APIPermission::kTab), 0u);
  EXPECT_EQ(apis.size(), 4u);
}

TEST(APIPermissionSetTest, CreateUnion) {
  scoped_refptr<APIPermissionDetail> detail;

  APIPermissionSet apis1;
  APIPermissionSet apis2;
  APIPermissionSet expected_apis;
  APIPermissionSet result;

  APIPermission* permission =
    PermissionsInfo::GetInstance()->GetByID(APIPermission::kSocket);
  detail = permission->CreateDetail();
  {
    scoped_ptr<ListValue> value(new ListValue());
    value->Append(Value::CreateStringValue("tcp-connect:*.example.com:80"));
    value->Append(Value::CreateStringValue("udp-bind::8080"));
    value->Append(Value::CreateStringValue("udp-send-to::8888"));
    if (!detail->FromValue(value.get())) {
      NOTREACHED();
    }
  }

  // Union with an empty set.
  apis1.insert(APIPermission::kTab);
  apis1.insert(APIPermission::kBackground);
  apis1.insert(detail);
  expected_apis.insert(APIPermission::kTab);
  expected_apis.insert(APIPermission::kBackground);
  expected_apis.insert(detail);

  APIPermissionSet::Union(apis1, apis2, &result);

  EXPECT_TRUE(apis1.Contains(apis2));
  EXPECT_TRUE(apis1.Contains(result));
  EXPECT_FALSE(apis2.Contains(apis1));
  EXPECT_FALSE(apis2.Contains(result));
  EXPECT_TRUE(result.Contains(apis1));
  EXPECT_TRUE(result.Contains(apis2));

  EXPECT_EQ(expected_apis, result);

  // Now use a real second set.
  apis2.insert(APIPermission::kTab);
  apis2.insert(APIPermission::kProxy);
  apis2.insert(APIPermission::kClipboardWrite);
  apis2.insert(APIPermission::kPlugin);

  detail = permission->CreateDetail();
  {
    scoped_ptr<ListValue> value(new ListValue());
    value->Append(Value::CreateStringValue("tcp-connect:*.example.com:80"));
    value->Append(Value::CreateStringValue("udp-send-to::8899"));
    if (!detail->FromValue(value.get())) {
      NOTREACHED();
    }
  }
  apis2.insert(detail);

  expected_apis.insert(APIPermission::kTab);
  expected_apis.insert(APIPermission::kProxy);
  expected_apis.insert(APIPermission::kClipboardWrite);
  expected_apis.insert(APIPermission::kPlugin);

  detail = permission->CreateDetail();
  {
    scoped_ptr<ListValue> value(new ListValue());
    value->Append(Value::CreateStringValue("tcp-connect:*.example.com:80"));
    value->Append(Value::CreateStringValue("udp-bind::8080"));
    value->Append(Value::CreateStringValue("udp-send-to::8888"));
    value->Append(Value::CreateStringValue("udp-send-to::8899"));
    if (!detail->FromValue(value.get())) {
      NOTREACHED();
    }
  }
  // Insert a new detail socket permission which will replace the old one.
  expected_apis.insert(detail);

  APIPermissionSet::Union(apis1, apis2, &result);

  EXPECT_FALSE(apis1.Contains(apis2));
  EXPECT_FALSE(apis1.Contains(result));
  EXPECT_FALSE(apis2.Contains(apis1));
  EXPECT_FALSE(apis2.Contains(result));
  EXPECT_TRUE(result.Contains(apis1));
  EXPECT_TRUE(result.Contains(apis2));

  EXPECT_EQ(expected_apis, result);
}

TEST(APIPermissionSetTest, CreateIntersection) {
  scoped_refptr<APIPermissionDetail> detail;

  APIPermissionSet apis1;
  APIPermissionSet apis2;
  APIPermissionSet expected_apis;
  APIPermissionSet result;

  APIPermission* permission =
    PermissionsInfo::GetInstance()->GetByID(APIPermission::kSocket);

  // Intersection with an empty set.
  apis1.insert(APIPermission::kTab);
  apis1.insert(APIPermission::kBackground);
  detail = permission->CreateDetail();
  {
    scoped_ptr<ListValue> value(new ListValue());
    value->Append(Value::CreateStringValue("tcp-connect:*.example.com:80"));
    value->Append(Value::CreateStringValue("udp-bind::8080"));
    value->Append(Value::CreateStringValue("udp-send-to::8888"));
    if (!detail->FromValue(value.get())) {
      NOTREACHED();
    }
  }
  apis1.insert(detail);

  APIPermissionSet::Intersection(apis1, apis2, &result);
  EXPECT_TRUE(apis1.Contains(result));
  EXPECT_TRUE(apis2.Contains(result));
  EXPECT_TRUE(apis1.Contains(apis2));
  EXPECT_FALSE(apis2.Contains(apis1));
  EXPECT_FALSE(result.Contains(apis1));
  EXPECT_TRUE(result.Contains(apis2));

  EXPECT_TRUE(result.empty());
  EXPECT_EQ(expected_apis, result);

  // Now use a real second set.
  apis2.insert(APIPermission::kTab);
  apis2.insert(APIPermission::kProxy);
  apis2.insert(APIPermission::kClipboardWrite);
  apis2.insert(APIPermission::kPlugin);
  detail = permission->CreateDetail();
  {
    scoped_ptr<ListValue> value(new ListValue());
    value->Append(Value::CreateStringValue("udp-bind::8080"));
    value->Append(Value::CreateStringValue("udp-send-to::8888"));
    value->Append(Value::CreateStringValue("udp-send-to::8899"));
    if (!detail->FromValue(value.get())) {
      NOTREACHED();
    }
  }
  apis2.insert(detail);

  expected_apis.insert(APIPermission::kTab);
  detail = permission->CreateDetail();
  {
    scoped_ptr<ListValue> value(new ListValue());
    value->Append(Value::CreateStringValue("udp-bind::8080"));
    value->Append(Value::CreateStringValue("udp-send-to::8888"));
    if (!detail->FromValue(value.get())) {
      NOTREACHED();
    }
  }
  expected_apis.insert(detail);

  APIPermissionSet::Intersection(apis1, apis2, &result);

  EXPECT_TRUE(apis1.Contains(result));
  EXPECT_TRUE(apis2.Contains(result));
  EXPECT_FALSE(apis1.Contains(apis2));
  EXPECT_FALSE(apis2.Contains(apis1));
  EXPECT_FALSE(result.Contains(apis1));
  EXPECT_FALSE(result.Contains(apis2));

  EXPECT_EQ(expected_apis, result);
}

TEST(APIPermissionSetTest, CreateDifference) {
  scoped_refptr<APIPermissionDetail> detail;

  APIPermissionSet apis1;
  APIPermissionSet apis2;
  APIPermissionSet expected_apis;
  APIPermissionSet result;

  APIPermission* permission =
    PermissionsInfo::GetInstance()->GetByID(APIPermission::kSocket);

  // Difference with an empty set.
  apis1.insert(APIPermission::kTab);
  apis1.insert(APIPermission::kBackground);
  detail = permission->CreateDetail();
  {
    scoped_ptr<ListValue> value(new ListValue());
    value->Append(Value::CreateStringValue("tcp-connect:*.example.com:80"));
    value->Append(Value::CreateStringValue("udp-bind::8080"));
    value->Append(Value::CreateStringValue("udp-send-to::8888"));
    if (!detail->FromValue(value.get())) {
      NOTREACHED();
    }
  }
  apis1.insert(detail);

  APIPermissionSet::Difference(apis1, apis2, &result);

  EXPECT_EQ(apis1, result);

  // Now use a real second set.
  apis2.insert(APIPermission::kTab);
  apis2.insert(APIPermission::kProxy);
  apis2.insert(APIPermission::kClipboardWrite);
  apis2.insert(APIPermission::kPlugin);
  detail = permission->CreateDetail();
  {
    scoped_ptr<ListValue> value(new ListValue());
    value->Append(Value::CreateStringValue("tcp-connect:*.example.com:80"));
    value->Append(Value::CreateStringValue("udp-send-to::8899"));
    if (!detail->FromValue(value.get())) {
      NOTREACHED();
    }
  }
  apis2.insert(detail);

  expected_apis.insert(APIPermission::kBackground);
  detail = permission->CreateDetail();
  {
    scoped_ptr<ListValue> value(new ListValue());
    value->Append(Value::CreateStringValue("udp-bind::8080"));
    value->Append(Value::CreateStringValue("udp-send-to::8888"));
    if (!detail->FromValue(value.get())) {
      NOTREACHED();
    }
  }
  expected_apis.insert(detail);

  APIPermissionSet::Difference(apis1, apis2, &result);

  EXPECT_TRUE(apis1.Contains(result));
  EXPECT_FALSE(apis2.Contains(result));

  EXPECT_EQ(expected_apis, result);

  // |result| = |apis1| - |apis2| --> |result| intersect |apis2| == empty_set
  APIPermissionSet result2;
  APIPermissionSet::Intersection(result, apis2, &result2);
  EXPECT_TRUE(result2.empty());
}

TEST(APIPermissionSetTest, IPC) {
  scoped_refptr<APIPermissionDetail> detail;

  APIPermissionSet apis;
  APIPermissionSet expected_apis;

  APIPermission* permission =
    PermissionsInfo::GetInstance()->GetByID(APIPermission::kSocket);

  apis.insert(APIPermission::kTab);
  apis.insert(APIPermission::kBackground);
  detail = permission->CreateDetail();
  {
    scoped_ptr<ListValue> value(new ListValue());
    value->Append(Value::CreateStringValue("tcp-connect:*.example.com:80"));
    value->Append(Value::CreateStringValue("udp-bind::8080"));
    value->Append(Value::CreateStringValue("udp-send-to::8888"));
    if (!detail->FromValue(value.get())) {
      NOTREACHED();
    }
  }
  apis.insert(detail);

  EXPECT_NE(apis, expected_apis);

  IPC::Message m;
  WriteParam(&m, apis);
  PickleIterator iter(m);
  CHECK(ReadParam(&m, &iter, &expected_apis));
  EXPECT_EQ(apis, expected_apis);
}

}  // namespace extensions
