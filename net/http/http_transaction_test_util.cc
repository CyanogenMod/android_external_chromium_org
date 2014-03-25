// Copyright 2014 The Chromium Authors. All rights reserved.
// Copyright (c) 2014, The Linux Foundation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "net/http/http_transaction_test_util.h"

#include <algorithm>

#include "base/bind.h"
#include "base/message_loop/message_loop.h"
#include "base/strings/stringprintf.h"
#include "base/time/time.h"
#include "net/base/load_flags.h"
#include "net/base/load_timing_info.h"
#include "net/base/net_errors.h"
#include "net/disk_cache/disk_cache.h"
#include "net/http/http_cache.h"
#include "net/http/http_request_info.h"
#include "net/http/http_response_info.h"
#include "net/http/http_transaction.h"
#include "testing/gtest/include/gtest/gtest.h"

namespace {
typedef base::hash_map<std::string, const MockTransaction*> MockTransactionMap;
static MockTransactionMap mock_transactions;

typedef base::hash_map<std::string, unsigned int> ReadMockTransactionMap;
static ReadMockTransactionMap read_mock_transactions;

struct TransactionData{
    TransactionData(std::string url, int return_code, const net::CompletionCallback& callback, MockNetworkTransaction* net_trans):
        url_(url),
        return_code_(return_code),
        callback_(callback),
        net_trans_(net_trans),
        is_called(false){}

    std::string url_;
    int return_code_;
    net::CompletionCallback callback_;
    MockNetworkTransaction* net_trans_;
    bool is_called;
};

typedef std::vector<TransactionData*> MockTransactionVector;
static MockTransactionVector mock_transactions_data;

}  // namespace

//-----------------------------------------------------------------------------
// mock transaction data

const MockTransaction kSimpleGET_Transaction = {
  "http://www.google.com/",
  "GET",
  base::Time(),
  "",
  net::LOAD_NORMAL,
  "HTTP/1.1 200 OK",
  "Cache-Control: max-age=10000\n",
  base::Time(),
  "<html><body>Google Blah Blah</body></html>",
  TEST_MODE_NORMAL,
  NULL,
  0,
  net::OK
};

const MockTransaction kSimplePOST_Transaction = {
  "http://bugdatabase.com/edit",
  "POST",
  base::Time(),
  "",
  net::LOAD_NORMAL,
  "HTTP/1.1 200 OK",
  "",
  base::Time(),
  "<html><body>Google Blah Blah</body></html>",
  TEST_MODE_NORMAL,
  NULL,
  0,
  net::OK
};

const MockTransaction kTypicalGET_Transaction = {
  "http://www.example.com/~foo/bar.html",
  "GET",
  base::Time(),
  "",
  net::LOAD_NORMAL,
  "HTTP/1.1 200 OK",
  "Date: Wed, 28 Nov 2007 09:40:09 GMT\n"
  "Last-Modified: Wed, 28 Nov 2007 00:40:09 GMT\n",
  base::Time(),
  "<html><body>Google Blah Blah</body></html>",
  TEST_MODE_NORMAL,
  NULL,
  0,
  net::OK
};

const MockTransaction kETagGET_Transaction = {
  "http://www.google.com/foopy",
  "GET",
  base::Time(),
  "",
  net::LOAD_NORMAL,
  "HTTP/1.1 200 OK",
  "Cache-Control: max-age=10000\n"
  "Etag: \"foopy\"\n",
  base::Time(),
  "<html><body>Google Blah Blah</body></html>",
  TEST_MODE_NORMAL,
  NULL,
  0,
  net::OK
};

const MockTransaction kRangeGET_Transaction = {
  "http://www.google.com/",
  "GET",
  base::Time(),
  "Range: 0-100\r\n",
  net::LOAD_NORMAL,
  "HTTP/1.1 200 OK",
  "Cache-Control: max-age=10000\n",
  base::Time(),
  "<html><body>Google Blah Blah</body></html>",
  TEST_MODE_NORMAL,
  NULL,
  0,
  net::OK
};

static const MockTransaction* const kBuiltinMockTransactions[] = {
  &kSimpleGET_Transaction,
  &kSimplePOST_Transaction,
  &kTypicalGET_Transaction,
  &kETagGET_Transaction,
  &kRangeGET_Transaction
};

const MockTransaction* FindMockTransaction(const GURL& url) {
  // look for overrides:
  MockTransactionMap::const_iterator it = mock_transactions.find(url.spec());
  if (it != mock_transactions.end())
    return it->second;

  // look for builtins:
  for (size_t i = 0; i < arraysize(kBuiltinMockTransactions); ++i) {
    if (url == GURL(kBuiltinMockTransactions[i]->url))
      return kBuiltinMockTransactions[i];
  }
  return NULL;
}

void AddMockTransaction(const MockTransaction* trans) {
  mock_transactions[GURL(trans->url).spec()] = trans;
}

void RemoveMockTransaction(const MockTransaction* trans) {
  mock_transactions.erase(GURL(trans->url).spec());
}

void sta::AddReadMockTransaction(const MockTransaction* trans, const  unsigned int read_call_num){
    read_mock_transactions[GURL(trans->url).spec()] = read_call_num;
}

void sta::RemoveReadMockTransaction(const MockTransaction* trans) {
    read_mock_transactions.erase(GURL(trans->url).spec());
}

void MockTransactionsDataClear(){
    mock_transactions_data.clear();
}

void CallbackMockTransactions(std::vector<int> order_vector){
    for (std::vector<int>::iterator it = order_vector.begin(); it != order_vector.end(); it++) {
        int& index = *it;
        TransactionData* td = mock_transactions_data[index];
        td->net_trans_->CallbackLater(td->callback_, td->return_code_);
        td->is_called = true;
    }

    for (std::vector<TransactionData*>::iterator it = mock_transactions_data.begin(); it != mock_transactions_data.end();) {
        if ((*it)->is_called){
            it = mock_transactions_data.erase(it);
        }
        else{
            it++;
        }
    }
}



MockHttpRequest::MockHttpRequest(const MockTransaction& t) {
  url = GURL(t.url);
  method = t.method;
  extra_headers.AddHeadersFromString(t.request_headers);
  load_flags = t.load_flags;
}

//-----------------------------------------------------------------------------

// static
int TestTransactionConsumer::quit_counter_ = 0;

TestTransactionConsumer::TestTransactionConsumer(
    net::RequestPriority priority,
    net::HttpTransactionFactory* factory)
    : state_(IDLE), error_(net::OK) {
  // Disregard the error code.
  factory->CreateTransaction(priority, &trans_);
  ++quit_counter_;
}

TestTransactionConsumer::~TestTransactionConsumer() {
}

void TestTransactionConsumer::Start(const net::HttpRequestInfo* request,
                                    const net::BoundNetLog& net_log) {
  state_ = STARTING;
  int result = trans_->Start(
      request, base::Bind(&TestTransactionConsumer::OnIOComplete,
                          base::Unretained(this)), net_log);
  if (result != net::ERR_IO_PENDING)
    DidStart(result);
}

void TestTransactionConsumer::DidStart(int result) {
  if (result != net::OK) {
    DidFinish(result);
  } else {
    Read();
  }
}

void TestTransactionConsumer::DidRead(int result) {
  if (result <= 0) {
    DidFinish(result);
  } else {
    content_.append(read_buf_->data(), result);
    Read();
  }
}

void TestTransactionConsumer::DidFinish(int result) {
  state_ = DONE;
  error_ = result;
  if (--quit_counter_ == 0)
    base::MessageLoop::current()->Quit();
}

void TestTransactionConsumer::Read() {
  state_ = READING;
  read_buf_ = new net::IOBuffer(1024);
  int result = trans_->Read(read_buf_.get(),
                            1024,
                            base::Bind(&TestTransactionConsumer::OnIOComplete,
                                       base::Unretained(this)));
  if (result != net::ERR_IO_PENDING)
    DidRead(result);
}

void TestTransactionConsumer::OnIOComplete(int result) {
  switch (state_) {
    case STARTING:
      DidStart(result);
      break;
    case READING:
      DidRead(result);
      break;
    default:
      NOTREACHED();
  }
}

MockNetworkTransaction::MockNetworkTransaction(
    net::RequestPriority priority,
    MockNetworkLayer* factory)
    : weak_factory_(this),
      request_(NULL),
      data_cursor_(0),
      priority_(priority),
      websocket_handshake_stream_create_helper_(NULL),
      transaction_factory_(factory->AsWeakPtr()),
      received_bytes_(0),
      socket_log_id_(net::NetLog::Source::kInvalidId) {
}

MockNetworkTransaction::~MockNetworkTransaction() {}

int MockNetworkTransaction::Start(const net::HttpRequestInfo* request,
                                  const net::CompletionCallback& callback,
                                  const net::BoundNetLog& net_log) {
  if (request_)
    return net::ERR_FAILED;

  request_ = request;
  return StartInternal(request, callback, net_log);
}

int MockNetworkTransaction::RestartIgnoringLastError(
    const net::CompletionCallback& callback) {
  return net::ERR_FAILED;
}

int MockNetworkTransaction::RestartWithCertificate(
    net::X509Certificate* client_cert,
    const net::CompletionCallback& callback) {
  return net::ERR_FAILED;
}

int MockNetworkTransaction::RestartWithAuth(
    const net::AuthCredentials& credentials,
    const net::CompletionCallback& callback) {
  if (!IsReadyToRestartForAuth())
    return net::ERR_FAILED;

  net::HttpRequestInfo auth_request_info = *request_;
  auth_request_info.extra_headers.AddHeaderFromString("Authorization: Bar");

  // Let the MockTransactionHandler worry about this: the only way for this
  // test to succeed is by using an explicit handler for the transaction so
  // that server behavior can be simulated.
  return StartInternal(&auth_request_info, callback, net::BoundNetLog());
}

bool MockNetworkTransaction::IsReadyToRestartForAuth() {
  if (!request_)
    return false;

  // Only mock auth when the test asks for it.
  return request_->extra_headers.HasHeader("X-Require-Mock-Auth");
}

int MockNetworkTransaction::Read(net::IOBuffer* buf, int buf_len,
                                 const net::CompletionCallback& callback) {
  int data_len = static_cast<int>(data_.size());
  int num = std::min(buf_len, data_len - data_cursor_);
  if (test_mode_ & TEST_MODE_SLOW_READ)
    num = std::min(num, 1);
  if (num) {
    memcpy(buf->data(), data_.data() + data_cursor_, num);
    data_cursor_ += num;
  }
  if (test_mode_ & TEST_MODE_SYNC_NET_READ)
    return num;

  CallbackLater(callback, num);
  return net::ERR_IO_PENDING;
}

void MockNetworkTransaction::StopCaching() {
  if (transaction_factory_.get())
    transaction_factory_->TransactionStopCaching();
}

bool MockNetworkTransaction::GetFullRequestHeaders(
    net::HttpRequestHeaders* headers) const {
  return false;
}

int64 MockNetworkTransaction::GetTotalReceivedBytes() const {
  return received_bytes_;
}

void MockNetworkTransaction::DoneReading() {
  if (transaction_factory_.get())
    transaction_factory_->TransactionDoneReading();
}

const net::HttpResponseInfo* MockNetworkTransaction::GetResponseInfo() const {
  return &response_;
}

net::LoadState MockNetworkTransaction::GetLoadState() const {
  if (data_cursor_)
    return net::LOAD_STATE_READING_RESPONSE;
  return net::LOAD_STATE_IDLE;
}

net::UploadProgress MockNetworkTransaction::GetUploadProgress() const {
  return net::UploadProgress();
}

void MockNetworkTransaction::SetQuicServerInfo(
    net::QuicServerInfo* quic_server_info) {}

bool MockNetworkTransaction::GetLoadTimingInfo(
    net::LoadTimingInfo* load_timing_info) const {
  if (socket_log_id_ != net::NetLog::Source::kInvalidId) {
    // The minimal set of times for a request that gets a response, assuming it
    // gets a new socket.
    load_timing_info->socket_reused = false;
    load_timing_info->socket_log_id = socket_log_id_;
    load_timing_info->connect_timing.connect_start = base::TimeTicks::Now();
    load_timing_info->connect_timing.connect_end = base::TimeTicks::Now();
    load_timing_info->send_start = base::TimeTicks::Now();
    load_timing_info->send_end = base::TimeTicks::Now();
  } else {
    // If there's no valid socket ID, just use the generic socket reused values.
    // No tests currently depend on this, just should not match the values set
    // by a cache hit.
    load_timing_info->socket_reused = true;
    load_timing_info->send_start = base::TimeTicks::Now();
    load_timing_info->send_end = base::TimeTicks::Now();
  }
  return true;
}

void MockNetworkTransaction::SetPriority(net::RequestPriority priority) {
  priority_ = priority;
}

void MockNetworkTransaction::SetWebSocketHandshakeStreamCreateHelper(
    net::WebSocketHandshakeStreamBase::CreateHelper* create_helper) {
  websocket_handshake_stream_create_helper_ = create_helper;
}

int MockNetworkTransaction::StartInternal(
    const net::HttpRequestInfo* request,
    const net::CompletionCallback& callback,
    const net::BoundNetLog& net_log) {
  const MockTransaction* t = FindMockTransaction(request->url);
  if (!t)
    return net::ERR_FAILED;

  test_mode_ = t->test_mode;

  // Return immediately if we're returning an error.
  if (net::OK != t->return_code) {
    if (test_mode_ & TEST_MODE_SYNC_NET_START)
      return t->return_code;
    if (test_mode_ & TEST_MODE_DELAYED_NET_START){
        mock_transactions_data.push_back(new TransactionData(request->url.spec(), t->return_code, callback, this));
    }
    else{
        CallbackLater(callback, t->return_code);
    }

    return net::ERR_IO_PENDING;
  }

  return_code_ = t->return_code;// can be modified later during Read()
  std::string resp_status = t->status;
  std::string resp_headers = t->response_headers;
  std::string resp_data = t->data;
  received_bytes_ = resp_status.size() + resp_headers.size() + resp_data.size();
  if (t->handler)
    (t->handler)(request, &resp_status, &resp_headers, &resp_data);

  std::string header_data = base::StringPrintf(
      "%s\n%s\n", resp_status.c_str(), resp_headers.c_str());
  std::replace(header_data.begin(), header_data.end(), '\n', '\0');

  response_.request_time = base::Time::Now();
  if (!t->request_time.is_null())
    response_.request_time = t->request_time;

  response_.was_cached = false;
  response_.network_accessed = true;

  response_.response_time = base::Time::Now();
  if (!t->response_time.is_null())
    response_.response_time = t->response_time;

  response_.headers = new net::HttpResponseHeaders(header_data);
  response_.vary_data.Init(*request, *response_.headers.get());
  response_.ssl_info.cert_status = t->cert_status;
  data_ = resp_data;

  if (net_log.net_log())
    socket_log_id_ = net_log.net_log()->NextID();

  if (test_mode_ & TEST_MODE_SYNC_NET_START)
    return net::OK;

  if (test_mode_ & TEST_MODE_DELAYED_NET_START){
    mock_transactions_data.push_back(new TransactionData(request->url.spec(), net::OK, callback, this));
  }
  else{
    CallbackLater(callback, net::OK);
  }
  return net::ERR_IO_PENDING;
}

void MockNetworkTransaction::SetBeforeNetworkStartCallback(
    const BeforeNetworkStartCallback& callback) {
}

void MockNetworkTransaction::SetBeforeProxyHeadersSentCallback(
    const BeforeProxyHeadersSentCallback& callback) {
}

int MockNetworkTransaction::ResumeNetworkStart() {
  // Should not get here.
  return net::ERR_FAILED;
}

void MockNetworkTransaction::CallbackLater(
    const net::CompletionCallback& callback, int result) {
  base::MessageLoop::current()->PostTask(
      FROM_HERE, base::Bind(&MockNetworkTransaction::RunCallback,
                            weak_factory_.GetWeakPtr(), callback, result));
}

void MockNetworkTransaction::RunCallback(
    const net::CompletionCallback& callback, int result) {
  callback.Run(result);
}

MockNetworkLayer::MockNetworkLayer()
    : transaction_count_(0),
      done_reading_called_(false),
      stop_caching_called_(false),
      last_create_transaction_priority_(net::DEFAULT_PRIORITY),
      use_sta_transaction_class_(false){}

MockNetworkLayer::~MockNetworkLayer() {}

void MockNetworkLayer::TransactionDoneReading() {
  done_reading_called_ = true;
}

void MockNetworkLayer::SetStaTransaction(){
  use_sta_transaction_class_ = true;
}

void MockNetworkLayer::TransactionStopCaching() {
  stop_caching_called_ = true;
}

int MockNetworkLayer::CreateTransaction(
    net::RequestPriority priority,
    scoped_ptr<net::HttpTransaction>* trans) {
  transaction_count_++;
  last_create_transaction_priority_ = priority;
  scoped_ptr<MockNetworkTransaction> mock_transaction(
          use_sta_transaction_class_ ? new sta::MockNetworkTransaction(priority, this)
          : new MockNetworkTransaction(priority, this));
  last_transaction_ = mock_transaction->AsWeakPtr();
  *trans = mock_transaction.Pass();
  return net::OK;
}

net::HttpCache* MockNetworkLayer::GetCache() {
  return NULL;
}

net::HttpNetworkSession* MockNetworkLayer::GetSession() {
  return NULL;
}

//
// ==== sta subclass impl ====
//
sta::MockNetworkTransaction::MockNetworkTransaction(
        net::RequestPriority priority, MockNetworkLayer* factory) :
        ::MockNetworkTransaction(priority, factory), max_read_return_bytes_(0), num_body_bytes_(
                0) {
}

const int sta::MockNetworkTransaction::FindReadMockTransactions(const GURL& url){
    ReadMockTransactionMap::iterator it = read_mock_transactions.find(url.spec());
    if (it != read_mock_transactions.end()){
        return it->second--;
    }
    return -1;
}

void sta::MockNetworkTransaction::SetAsyncRead(bool useAsyncRead) {
    if (!useAsyncRead)
        test_mode_ |= TEST_MODE_SYNC_NET_READ;  // set this bit
    else
        test_mode_ &= ~TEST_MODE_SYNC_NET_READ; // reset this bit
}

int sta::MockNetworkTransaction::SetReadSize(int numBytes){
    int tmp = max_read_return_bytes_;
    max_read_return_bytes_ = numBytes;
    return tmp;
}

int sta::MockNetworkTransaction::Read(net::IOBuffer* buf, int buf_len,
        const net::CompletionCallback& callback) {

    // Return immediately if we're returning an error.
    if (net::OK != return_code_) {
        if (test_mode_ & TEST_MODE_SYNC_NET_START)
            return return_code_;
        if (test_mode_ & TEST_MODE_DELAYED_NET_READ){
            if (FindReadMockTransactions(request_->url) > 0){
                CallbackLater(callback, return_code_);
            }
        }
        else{
            CallbackLater(callback, return_code_);
        }
        return net::ERR_IO_PENDING;
    }

    int data_len = static_cast<int>(data_.size());
    if(num_body_bytes_ >0) // enforce behavior suitable for byte-range response
        data_len = num_body_bytes_;

    int num = std::min(buf_len, data_len - data_cursor_);
    if (max_read_return_bytes_ > 0)
        num = std::min(num, max_read_return_bytes_);

    if (num) {
        memcpy(buf->data(), data_.data() + data_cursor_, num);
        data_cursor_ += num;
    }
    if (test_mode_ & TEST_MODE_SYNC_NET_READ)
        return num;

    if (test_mode_ & TEST_MODE_DELAYED_NET_READ){
        if (FindReadMockTransactions(request_->url) > 0){
            CallbackLater(callback, num);
        }
    }
    else{
        CallbackLater(callback, num);
    }

    return net::ERR_IO_PENDING;
}

//-----------------------------------------------------------------------------
// helpers

int ReadTransaction(net::HttpTransaction* trans, std::string* result) {
  int rv;

  net::TestCompletionCallback callback;

  std::string content;
  do {
    scoped_refptr<net::IOBuffer> buf(new net::IOBuffer(256));
    rv = trans->Read(buf.get(), 256, callback.callback());
    if (rv == net::ERR_IO_PENDING)
      rv = callback.WaitForResult();

    if (rv > 0)
      content.append(buf->data(), rv);
    else if (rv < 0)
      return rv;
  } while (rv > 0);

  result->swap(content);
  return net::OK;
}
