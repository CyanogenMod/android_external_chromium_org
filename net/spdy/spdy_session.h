// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef NET_SPDY_SPDY_SESSION_H_
#define NET_SPDY_SPDY_SESSION_H_

#include <deque>
#include <list>
#include <map>
#include <set>
#include <string>

#include "base/gtest_prod_util.h"
#include "base/memory/ref_counted.h"
#include "base/memory/weak_ptr.h"
#include "base/time.h"
#include "net/base/io_buffer.h"
#include "net/base/load_states.h"
#include "net/base/net_errors.h"
#include "net/base/request_priority.h"
#include "net/base/ssl_client_cert_type.h"
#include "net/base/ssl_config_service.h"
#include "net/socket/client_socket_handle.h"
#include "net/socket/ssl_client_socket.h"
#include "net/socket/stream_socket.h"
#include "net/spdy/buffered_spdy_framer.h"
#include "net/spdy/spdy_credential_state.h"
#include "net/spdy/spdy_header_block.h"
#include "net/spdy/spdy_io_buffer.h"
#include "net/spdy/spdy_protocol.h"
#include "net/spdy/spdy_session_pool.h"

namespace net {

// This is somewhat arbitrary and not really fixed, but it will always work
// reasonably with ethernet. Chop the world into 2-packet chunks.  This is
// somewhat arbitrary, but is reasonably small and ensures that we elicit
// ACKs quickly from TCP (because TCP tries to only ACK every other packet).
const int kMss = 1430;
// The 8 is the size of the SPDY frame header.
const int kMaxSpdyFrameChunkSize = (2 * kMss) - 8;

// Specifies the maxiumum concurrent streams server could send (via push).
const int kMaxConcurrentPushedStreams = 1000;

// Specifies the number of bytes read synchronously (without yielding) if the
// data is available.
const int kMaxReadBytes = 32 * 1024;

class BoundNetLog;
struct LoadTimingInfo;
class SpdyStream;
class SSLInfo;

enum SpdyProtocolErrorDetails {
  // SpdyFramer::SpdyErrors
  SPDY_ERROR_NO_ERROR,
  SPDY_ERROR_INVALID_CONTROL_FRAME,
  SPDY_ERROR_CONTROL_PAYLOAD_TOO_LARGE,
  SPDY_ERROR_ZLIB_INIT_FAILURE,
  SPDY_ERROR_UNSUPPORTED_VERSION,
  SPDY_ERROR_DECOMPRESS_FAILURE,
  SPDY_ERROR_COMPRESS_FAILURE,
  SPDY_ERROR_CREDENTIAL_FRAME_CORRUPT,
  SPDY_ERROR_INVALID_DATA_FRAME_FLAGS,
  SPDY_ERROR_INVALID_CONTROL_FRAME_FLAGS,

  // SpdyRstStreamStatus
  STATUS_CODE_INVALID,
  STATUS_CODE_PROTOCOL_ERROR,
  STATUS_CODE_INVALID_STREAM,
  STATUS_CODE_REFUSED_STREAM,
  STATUS_CODE_UNSUPPORTED_VERSION,
  STATUS_CODE_CANCEL,
  STATUS_CODE_INTERNAL_ERROR,
  STATUS_CODE_FLOW_CONTROL_ERROR,
  STATUS_CODE_STREAM_IN_USE,
  STATUS_CODE_STREAM_ALREADY_CLOSED,
  STATUS_CODE_INVALID_CREDENTIALS,
  STATUS_CODE_FRAME_TOO_LARGE,

  // SpdySession errors
  PROTOCOL_ERROR_UNEXPECTED_PING,
  PROTOCOL_ERROR_RST_STREAM_FOR_NON_ACTIVE_STREAM,
  PROTOCOL_ERROR_SPDY_COMPRESSION_FAILURE,
  PROTOCOL_ERROR_REQUEST_FOR_SECURE_CONTENT_OVER_INSECURE_SESSION,
  PROTOCOL_ERROR_SYN_REPLY_NOT_RECEIVED,
  NUM_SPDY_PROTOCOL_ERROR_DETAILS
};

COMPILE_ASSERT(STATUS_CODE_INVALID ==
               static_cast<SpdyProtocolErrorDetails>(SpdyFramer::LAST_ERROR),
               SpdyProtocolErrorDetails_SpdyErrors_mismatch);

COMPILE_ASSERT(PROTOCOL_ERROR_UNEXPECTED_PING ==
               static_cast<SpdyProtocolErrorDetails>(
                   RST_STREAM_NUM_STATUS_CODES + STATUS_CODE_INVALID),
               SpdyProtocolErrorDetails_SpdyErrors_mismatch);

// A helper class used to manage a request to create a stream.
class NET_EXPORT_PRIVATE SpdyStreamRequest {
 public:
  SpdyStreamRequest();
  // Calls CancelRequest().
  ~SpdyStreamRequest();

  // Starts the request to create a stream. If OK is returned, then
  // ReleaseStream() may be called. If ERR_IO_PENDING is returned,
  // then when the stream is created, |callback| will be called, at
  // which point ReleaseStream() may be called. Otherwise, the stream
  // is not created, an error is returned, and ReleaseStream() may not
  // be called.
  //
  // If OK is returned, must not be called again without
  // ReleaseStream() being called first. If ERR_IO_PENDING is
  // returned, must not be called again without CancelRequest() or
  // ReleaseStream() being called first. Otherwise, in case of an
  // immediate error, this may be called again.
  int StartRequest(const scoped_refptr<SpdySession>& session,
                   const GURL& url,
                   RequestPriority priority,
                   const BoundNetLog& net_log,
                   const CompletionCallback& callback);

  // Cancels any pending stream creation request. May be called
  // repeatedly.
  void CancelRequest();

  // Transfers the created stream (guaranteed to not be NULL) to the
  // caller. Must be called at most once after StartRequest() returns
  // OK or |callback| is called with OK. The caller must immediately
  // set a delegate for the returned stream (except for test code).
  scoped_refptr<SpdyStream> ReleaseStream();

 private:
  friend class SpdySession;

  // Called by |session_| when the stream attempt is
  // finished. |stream| is non-NULL exactly when |rv| is OK. Also
  // called with a NULL stream and ERR_ABORTED if |session_| is
  // destroyed while the stream attempt is still pending.
  void OnRequestComplete(const scoped_refptr<SpdyStream>& stream, int rv);

  // Accessors called by |session_|.
  const GURL& url() const { return url_; }
  RequestPriority priority() const { return priority_; }
  const BoundNetLog& net_log() const { return net_log_; }

  void Reset();

  scoped_refptr<SpdySession> session_;
  scoped_refptr<SpdyStream> stream_;
  GURL url_;
  RequestPriority priority_;
  BoundNetLog net_log_;
  CompletionCallback callback_;

  DISALLOW_COPY_AND_ASSIGN(SpdyStreamRequest);
};

class NET_EXPORT SpdySession : public base::RefCounted<SpdySession>,
                               public BufferedSpdyFramerVisitorInterface {
 public:
  // TODO(akalin): Use base::TickClock when it becomes available.
  typedef base::TimeTicks (*TimeFunc)(void);

  // How we handle flow control (version-dependent).
  enum FlowControlState {
    FLOW_CONTROL_NONE,
    FLOW_CONTROL_STREAM,
    FLOW_CONTROL_STREAM_AND_SESSION
  };

  // Defines an interface for producing SpdyIOBuffers.
  class NET_EXPORT_PRIVATE SpdyIOBufferProducer {
   public:
    SpdyIOBufferProducer() {}

    // Returns a newly created SpdyIOBuffer, owned by the caller, or NULL
    // if not buffer is ready to be produced.
    virtual SpdyIOBuffer* ProduceNextBuffer(SpdySession* session) = 0;

    virtual RequestPriority GetPriority() const = 0;

    virtual ~SpdyIOBufferProducer() {}

   protected:
    // Activates |spdy_stream| in |spdy_session|.
    static void ActivateStream(SpdySession* spdy_session,
                               SpdyStream* spdy_stream);

    static SpdyIOBuffer* CreateIOBuffer(SpdyFrame* frame,
                                        RequestPriority priority,
                                        SpdyStream* spdy_stream);

   private:
    DISALLOW_COPY_AND_ASSIGN(SpdyIOBufferProducer);
  };

  // Create a new SpdySession.
  // |host_port_proxy_pair| is the host/port that this session connects to, and
  // the proxy configuration settings that it's using.
  // |spdy_session_pool| is the SpdySessionPool that owns us.  Its lifetime must
  // strictly be greater than |this|.
  // |session| is the HttpNetworkSession.  |net_log| is the NetLog that we log
  // network events to.
  SpdySession(const HostPortProxyPair& host_port_proxy_pair,
              SpdySessionPool* spdy_session_pool,
              HttpServerProperties* http_server_properties,
              bool verify_domain_authentication,
              bool enable_sending_initial_settings,
              bool enable_credential_frames,
              bool enable_compression,
              bool enable_ping_based_connection_checking,
              NextProto default_protocol_,
              size_t stream_initial_recv_window_size,
              size_t initial_max_concurrent_streams,
              size_t max_concurrent_streams_limit,
              TimeFunc time_func,
              const HostPortPair& trusted_spdy_proxy,
              NetLog* net_log);

  const HostPortPair& host_port_pair() const {
    return host_port_proxy_pair_.first;
  }
  const HostPortProxyPair& host_port_proxy_pair() const {
    return host_port_proxy_pair_;
  }

  // Get a pushed stream for a given |url|.  If the server initiates a
  // stream, it might already exist for a given path.  The server
  // might also not have initiated the stream yet, but indicated it
  // will via X-Associated-Content.  Returns OK if a stream was found
  // and put into |spdy_stream|, or if one was not found but it is
  // okay to create a new stream.  Returns an error (not
  // ERR_IO_PENDING) otherwise.
  int GetPushStream(
      const GURL& url,
      scoped_refptr<SpdyStream>* spdy_stream,
      const BoundNetLog& stream_net_log);

  // Used by SpdySessionPool to initialize with a pre-existing SSL socket. For
  // testing, setting is_secure to false allows initialization with a
  // pre-existing TCP socket.
  // Returns OK on success, or an error on failure.
  net::Error InitializeWithSocket(ClientSocketHandle* connection,
                                  bool is_secure,
                                  int certificate_error_code);

  // Check to see if this SPDY session can support an additional domain.
  // If the session is un-authenticated, then this call always returns true.
  // For SSL-based sessions, verifies that the server certificate in use by
  // this session provides authentication for the domain and no client
  // certificate or channel ID was sent to the original server during the SSL
  // handshake.  NOTE:  This function can have false negatives on some
  // platforms.
  // TODO(wtc): rename this function and the Net.SpdyIPPoolDomainMatch
  // histogram because this function does more than verifying domain
  // authentication now.
  bool VerifyDomainAuthentication(const std::string& domain);

  // Records that |stream| has a write available from |producer|.
  // |producer| will be owned by this SpdySession.
  void SetStreamHasWriteAvailable(SpdyStream* stream,
                                  SpdyIOBufferProducer* producer);

  // Send the SYN frame for |stream_id|. This also sends PING message to check
  // the status of the connection.
  SpdyFrame* CreateSynStream(
      SpdyStreamId stream_id,
      RequestPriority priority,
      uint8 credential_slot,
      SpdyControlFlags flags,
      const SpdyHeaderBlock& headers);

  // Write a CREDENTIAL frame to the session.
  SpdyFrame* CreateCredentialFrame(const std::string& origin,
                                   SSLClientCertType type,
                                   const std::string& key,
                                   const std::string& cert,
                                   RequestPriority priority);

  // Write a HEADERS frame to the stream.
  SpdyFrame* CreateHeadersFrame(SpdyStreamId stream_id,
                                const SpdyHeaderBlock& headers,
                                SpdyControlFlags flags);

  // Write a data frame to the stream.
  // Used to create and queue a data frame for the given stream.
  SpdyFrame* CreateDataFrame(SpdyStreamId stream_id,
                             net::IOBuffer* data, int len,
                             SpdyDataFlags flags);

  // Close a stream.
  void CloseStream(SpdyStreamId stream_id, int status);

  // Close a stream that has been created but is not yet active.
  void CloseCreatedStream(SpdyStream* stream, int status);

  // Reset a stream by sending a RST_STREAM frame with given status code.
  // Also closes the stream.  Was not piggybacked to CloseStream since not
  // all of the calls to CloseStream necessitate sending a RST_STREAM.
  void ResetStream(SpdyStreamId stream_id,
                   SpdyRstStreamStatus status,
                   const std::string& description);

  // Check if a stream is active.
  bool IsStreamActive(SpdyStreamId stream_id) const;

  // The LoadState is used for informing the user of the current network
  // status, such as "resolving host", "connecting", etc.
  LoadState GetLoadState() const;

  // Fills SSL info in |ssl_info| and returns true when SSL is in use.
  bool GetSSLInfo(SSLInfo* ssl_info,
                  bool* was_npn_negotiated,
                  NextProto* protocol_negotiated);

  // Fills SSL Certificate Request info |cert_request_info| and returns
  // true when SSL is in use.
  bool GetSSLCertRequestInfo(SSLCertRequestInfo* cert_request_info);

  // Returns the ServerBoundCertService used by this Socket, or NULL
  // if server bound certs are not supported in this session.
  ServerBoundCertService* GetServerBoundCertService() const;

  // Send a WINDOW_UPDATE frame for a stream. Called by a stream
  // whenever receive window size is increased.
  void SendStreamWindowUpdate(SpdyStreamId stream_id,
                              uint32 delta_window_size);

  // Called by a stream to increase this session's receive window size
  // by |delta_window_size|, which must be at least 1 and must not
  // cause this session's receive window size to overflow, possibly
  // also sending a WINDOW_UPDATE frame. Does nothing if session flow
  // control is turned off.
  void IncreaseRecvWindowSize(int32 delta_window_size);

  // If session is closed, no new streams/transactions should be created.
  bool IsClosed() const { return state_ == STATE_CLOSED; }

  // Closes this session.  This will close all active streams and mark
  // the session as permanently closed.
  // |err| should not be OK; this function is intended to be called on
  // error.
  // |remove_from_pool| indicates whether to also remove the session from the
  // session pool.
  // |description| indicates the reason for the error.
  void CloseSessionOnError(net::Error err,
                           bool remove_from_pool,
                           const std::string& description);

  // Retrieves information on the current state of the SPDY session as a
  // Value.  Caller takes possession of the returned value.
  base::Value* GetInfoAsValue() const;

  // Indicates whether the session is being reused after having successfully
  // used to send/receive data in the past.
  bool IsReused() const;

  // Returns true if the underlying transport socket ever had any reads or
  // writes.
  bool WasEverUsed() const {
    return connection_->socket()->WasEverUsed();
  }

  // Returns the load timing information from the perspective of the given
  // stream.  If it's not the first stream, the connection is considered reused
  // for that stream.
  //
  // This uses a different notion of reuse than IsReused().  This function
  // sets |socket_reused| to false only if |stream_id| is the ID of the first
  // stream using the session.  IsReused(), on the other hand, indicates if the
  // session has been used to send/receive data at all.
  bool GetLoadTimingInfo(SpdyStreamId stream_id,
                         LoadTimingInfo* load_timing_info) const;

  void set_spdy_session_pool(SpdySessionPool* pool) {
    spdy_session_pool_ = NULL;
  }

  // Returns true if session is not currently active
  bool is_active() const {
    return !active_streams_.empty() || !created_streams_.empty();
  }

  // Access to the number of active and pending streams.  These are primarily
  // available for testing and diagnostics.
  size_t num_active_streams() const { return active_streams_.size(); }
  size_t num_unclaimed_pushed_streams() const {
      return unclaimed_pushed_streams_.size();
  }
  size_t num_created_streams() const { return created_streams_.size(); }

  size_t pending_create_stream_queues(int priority) {
    DCHECK_LT(priority, NUM_PRIORITIES);
    return pending_create_stream_queues_[priority].size();
  }

  // Returns the (version-dependent) flow control state.
  FlowControlState flow_control_state() const {
    return flow_control_state_;
  }

  // Returns the current |stream_initial_send_window_size_|.
  int32 stream_initial_send_window_size() const {
    return stream_initial_send_window_size_;
  }

  // Returns the current |stream_initial_recv_window_size_|.
  int32 stream_initial_recv_window_size() const {
    return stream_initial_recv_window_size_;
  }

  const BoundNetLog& net_log() const { return net_log_; }

  int GetPeerAddress(IPEndPoint* address) const;
  int GetLocalAddress(IPEndPoint* address) const;

  // Returns true if requests on this session require credentials.
  bool NeedsCredentials() const;

  SpdyCredentialState* credential_state() { return &credential_state_; }

  // Adds |alias| to set of aliases associated with this session.
  void AddPooledAlias(const HostPortProxyPair& alias);

  // Returns the set of aliases associated with this session.
  const std::set<HostPortProxyPair>& pooled_aliases() const {
    return pooled_aliases_;
  }

  int GetProtocolVersion() const;

 private:
  friend class base::RefCounted<SpdySession>;
  friend class SpdyStreamRequest;

  // Allow tests to access our innards for testing purposes.
  FRIEND_TEST_ALL_PREFIXES(SpdySessionSpdy2Test, ClientPing);
  FRIEND_TEST_ALL_PREFIXES(SpdySessionSpdy2Test, FailedPing);
  FRIEND_TEST_ALL_PREFIXES(SpdySessionSpdy2Test, GetActivePushStream);
  FRIEND_TEST_ALL_PREFIXES(SpdySessionSpdy2Test, DeleteExpiredPushStreams);
  FRIEND_TEST_ALL_PREFIXES(SpdySessionSpdy2Test, ProtocolNegotiation);
  FRIEND_TEST_ALL_PREFIXES(SpdySessionSpdy3Test, ClientPing);
  FRIEND_TEST_ALL_PREFIXES(SpdySessionSpdy3Test, FailedPing);
  FRIEND_TEST_ALL_PREFIXES(SpdySessionSpdy3Test, GetActivePushStream);
  FRIEND_TEST_ALL_PREFIXES(SpdySessionSpdy3Test, DeleteExpiredPushStreams);
  FRIEND_TEST_ALL_PREFIXES(SpdySessionSpdy3Test, ProtocolNegotiation);
  FRIEND_TEST_ALL_PREFIXES(SpdySessionSpdy3Test, ProtocolNegotiation31);
  FRIEND_TEST_ALL_PREFIXES(SpdySessionSpdy3Test, IncreaseRecvWindowSize);
  FRIEND_TEST_ALL_PREFIXES(SpdySessionSpdy3Test, AdjustRecvWindowSize31);
  FRIEND_TEST_ALL_PREFIXES(SpdySessionSpdy3Test, AdjustSendWindowSize31);
  FRIEND_TEST_ALL_PREFIXES(SpdySessionSpdy3Test,
                           SessionFlowControlInactiveStream31);
  FRIEND_TEST_ALL_PREFIXES(SpdySessionSpdy3Test, SessionFlowControlEndToEnd31);

  typedef std::deque<SpdyStreamRequest*> PendingStreamRequestQueue;
  typedef std::set<SpdyStreamRequest*> PendingStreamRequestCompletionSet;

  typedef std::map<int, scoped_refptr<SpdyStream> > ActiveStreamMap;
  typedef std::map<std::string,
      std::pair<scoped_refptr<SpdyStream>, base::TimeTicks> > PushedStreamMap;
  typedef std::priority_queue<SpdyIOBuffer> OutputQueue;

  typedef std::set<scoped_refptr<SpdyStream> > CreatedStreamSet;
  typedef std::map<SpdyIOBufferProducer*, SpdyStream*> StreamProducerMap;

  class SpdyIOBufferProducerCompare {
   public:
    bool operator() (const SpdyIOBufferProducer* lhs,
                     const SpdyIOBufferProducer* rhs) const {
      return lhs->GetPriority() < rhs->GetPriority();
    }
  };

  typedef std::priority_queue<SpdyIOBufferProducer*,
                              std::vector<SpdyIOBufferProducer*>,
                              SpdyIOBufferProducerCompare> WriteQueue;

  enum State {
    STATE_IDLE,
    STATE_CONNECTING,
    STATE_DO_READ,
    STATE_DO_READ_COMPLETE,
    STATE_CLOSED
  };

  virtual ~SpdySession();

  // Called by SpdyStreamRequest to start a request to create a
  // stream. If OK is returned, then |stream| will be filled in with a
  // valid stream. If ERR_IO_PENDING is returned, then
  // |request->OnRequestComplete()| will be called when the stream is
  // created (unless it is cancelled). Otherwise, no stream is created
  // and the error is returned.
  int TryCreateStream(SpdyStreamRequest* request,
                      scoped_refptr<SpdyStream>* stream);

  // Actually create a stream into |stream|. Returns OK if successful;
  // otherwise, returns an error and |stream| is not filled.
  int CreateStream(const SpdyStreamRequest& request,
                   scoped_refptr<SpdyStream>* stream);

  // Called by SpdyStreamRequest to remove |request| from the stream
  // creation queue.
  void CancelStreamRequest(SpdyStreamRequest* request);

  // Called when there is room to create more streams (e.g., a stream
  // was closed). Processes as many pending stream requests as
  // possible.
  void ProcessPendingStreamRequests();

  // Start the DoLoop to read data from socket.
  void StartRead();

  // Try to make progress by reading and processing data.
  int DoLoop(int result);
  // The implementations of STATE_DO_READ/STATE_DO_READ_COMPLETE state changes
  // of the state machine.
  int DoRead();
  int DoReadComplete(int bytes_read);

  // Check if session is connected or not.
  bool IsConnected() const {
    return state_ == STATE_DO_READ || state_ == STATE_DO_READ_COMPLETE;
  }

  // IO Callbacks
  void OnReadComplete(int result);
  void OnWriteComplete(int result);

  // Send relevant SETTINGS.  This is generally called on connection setup.
  void SendInitialSettings();

  // Helper method to send SETTINGS a frame.
  void SendSettings(const SettingsMap& settings);

  // Handle SETTING.  Either when we send settings, or when we receive a
  // SETTINGS control frame, update our SpdySession accordingly.
  void HandleSetting(uint32 id, uint32 value);

  // Adjust the send window size of all ActiveStreams and PendingStreamRequests.
  void UpdateStreamsSendWindowSize(int32 delta_window_size);

  // Send the PING (preface-PING) frame.
  void SendPrefacePingIfNoneInFlight();

  // Send PING if there are no PINGs in flight and we haven't heard from server.
  void SendPrefacePing();

  // Send a single WINDOW_UPDATE frame.
  void SendWindowUpdateFrame(SpdyStreamId stream_id, uint32 delta_window_size,
                             RequestPriority priority);

  // Send the PING frame.
  void WritePingFrame(uint32 unique_id);

  // Post a CheckPingStatus call after delay. Don't post if there is already
  // CheckPingStatus running.
  void PlanToCheckPingStatus();

  // Check the status of the connection. It calls |CloseSessionOnError| if we
  // haven't received any data in |kHungInterval| time period.
  void CheckPingStatus(base::TimeTicks last_check_time);

  // Write current data to the socket.
  void WriteSocketLater();
  void WriteSocket();

  // Get a new stream id.
  int GetNewStreamId();

  // Queue a frame for sending.
  // |frame| is the frame to send.
  // |priority| is the priority for insertion into the queue.
  void QueueFrame(SpdyFrame* frame, RequestPriority priority);

  // Track active streams in the active stream list.
  void ActivateStream(SpdyStream* stream);
  void DeleteStream(SpdyStreamId id, int status);

  // Removes this session from the session pool.
  void RemoveFromPool();

  // Check if we have a pending pushed-stream for this url
  // Returns the stream if found (and returns it from the pending
  // list), returns NULL otherwise.
  scoped_refptr<SpdyStream> GetActivePushStream(const std::string& url);

  // Calls OnResponseReceived().
  // Returns true if successful.
  bool Respond(const SpdyHeaderBlock& headers,
               const scoped_refptr<SpdyStream> stream);

  void RecordPingRTTHistogram(base::TimeDelta duration);
  void RecordHistograms();
  void RecordProtocolErrorHistogram(SpdyProtocolErrorDetails details);

  // Closes all streams.  Used as part of shutdown.
  void CloseAllStreams(net::Error status);

  void LogAbandonedStream(const scoped_refptr<SpdyStream>& stream,
                          net::Error status);

  // Invokes a user callback for stream creation.  We provide this method so it
  // can be deferred to the MessageLoop, so we avoid re-entrancy problems.
  void CompleteStreamRequest(SpdyStreamRequest* pending_request);

  // Remove old unclaimed pushed streams.
  void DeleteExpiredPushedStreams();

  // BufferedSpdyFramerVisitorInterface:
  virtual void OnError(SpdyFramer::SpdyError error_code) OVERRIDE;
  virtual void OnStreamError(SpdyStreamId stream_id,
                             const std::string& description) OVERRIDE;
  virtual void OnPing(uint32 unique_id) OVERRIDE;
  virtual void OnRstStream(SpdyStreamId stream_id,
                           SpdyRstStreamStatus status) OVERRIDE;
  virtual void OnGoAway(SpdyStreamId last_accepted_stream_id,
                        SpdyGoAwayStatus status) OVERRIDE;
  virtual void OnStreamFrameData(SpdyStreamId stream_id,
                                 const char* data,
                                 size_t len,
                                 bool fin) OVERRIDE;
  virtual void OnSetting(
      SpdySettingsIds id, uint8 flags, uint32 value) OVERRIDE;
  virtual void OnWindowUpdate(SpdyStreamId stream_id,
                              uint32 delta_window_size) OVERRIDE;
  virtual void OnSynStreamCompressed(
      size_t uncompressed_size,
      size_t compressed_size) OVERRIDE;
  virtual void OnSynStream(SpdyStreamId stream_id,
                           SpdyStreamId associated_stream_id,
                           SpdyPriority priority,
                           uint8 credential_slot,
                           bool fin,
                           bool unidirectional,
                           const SpdyHeaderBlock& headers) OVERRIDE;
  virtual void OnSynReply(
      SpdyStreamId stream_id,
      bool fin,
      const SpdyHeaderBlock& headers) OVERRIDE;
  virtual void OnHeaders(
      SpdyStreamId stream_id,
      bool fin,
      const SpdyHeaderBlock& headers) OVERRIDE;

  // If session flow control is turned on, called by OnWindowUpdate()
  // (which is in turn called by the framer) to increase this
  // session's send window size by |delta_window_size| from a
  // WINDOW_UPDATE frome, which must be at least 1. If
  // |delta_window_size| would cause this session's send window size
  // to overflow, does nothing.
  void IncreaseSendWindowSize(int32 delta_window_size);

  // If session flow control is turned on, called by CreateDataFrame()
  // (which is in turn called by a stream) to decrease this session's
  // send window size by |delta_window_size|, which must be at least 1
  // and at most kMaxSpdyFrameChunkSize.  |delta_window_size| must not
  // cause this session's send window size to go negative.
  void DecreaseSendWindowSize(int32 delta_window_size);

  // If session flow control is turned on, called by OnStreamFrameData
  // (which is in turn called by the framer) to decrease this
  // session's receive window size by |delta_window_size|, which must
  // be at least 1 and must not cause this session's receive window
  // size to go negative.
  void DecreaseRecvWindowSize(int32 delta_window_size);

  // --------------------------
  // Helper methods for testing
  // --------------------------

  void set_connection_at_risk_of_loss_time(base::TimeDelta duration) {
    connection_at_risk_of_loss_time_ = duration;
  }

  void set_hung_interval(base::TimeDelta duration) {
    hung_interval_ = duration;
  }

  int64 pings_in_flight() const { return pings_in_flight_; }

  uint32 next_ping_id() const { return next_ping_id_; }

  base::TimeTicks last_activity_time() const { return last_activity_time_; }

  bool check_ping_status_pending() const { return check_ping_status_pending_; }

  // Returns the SSLClientSocket that this SPDY session sits on top of,
  // or NULL, if the transport is not SSL.
  SSLClientSocket* GetSSLClientSocket() const;

  // Used for posting asynchronous IO tasks.  We use this even though
  // SpdySession is refcounted because we don't need to keep the SpdySession
  // alive if the last reference is within a RunnableMethod.  Just revoke the
  // method.
  base::WeakPtrFactory<SpdySession> weak_factory_;

  // The domain this session is connected to.
  const HostPortProxyPair host_port_proxy_pair_;

  // Set set of HostPortProxyPairs for which this session has serviced
  // requests.
  std::set<HostPortProxyPair> pooled_aliases_;

  // |spdy_session_pool_| owns us, therefore its lifetime must exceed ours.  We
  // set this to NULL after we are removed from the pool.
  SpdySessionPool* spdy_session_pool_;
  HttpServerProperties* const http_server_properties_;

  // The socket handle for this session.
  scoped_ptr<ClientSocketHandle> connection_;

  // The read buffer used to read data from the socket.
  scoped_refptr<IOBuffer> read_buffer_;

  int stream_hi_water_mark_;  // The next stream id to use.

  // Queue, for each priority, of pending stream requests that have
  // not yet been satisfied.
  PendingStreamRequestQueue pending_create_stream_queues_[NUM_PRIORITIES];

  // A set of requests that are waiting to be completed (i.e., for the
  // stream to actually be created). This is necessary since we kick
  // off the stream creation asynchronously, and so the request may be
  // cancelled before the asynchronous task to create the stream runs.
  PendingStreamRequestCompletionSet pending_stream_request_completions_;

  // Map from stream id to all active streams.  Streams are active in the sense
  // that they have a consumer (typically SpdyNetworkTransaction and regardless
  // of whether or not there is currently any ongoing IO [might be waiting for
  // the server to start pushing the stream]) or there are still network events
  // incoming even though the consumer has already gone away (cancellation).
  // TODO(willchan): Perhaps we should separate out cancelled streams and move
  // them into a separate ActiveStreamMap, and not deliver network events to
  // them?
  ActiveStreamMap active_streams_;
  // Map of all the streams that have already started to be pushed by the
  // server, but do not have consumers yet.
  PushedStreamMap unclaimed_pushed_streams_;

  // Set of all created streams but that have not yet sent any frames.
  CreatedStreamSet created_streams_;

  // As streams have data to be sent, we put them into the write queue.
  WriteQueue write_queue_;

  // Mapping from SpdyIOBufferProducers to their corresponding SpdyStream
  // so that when a stream is destroyed, we can remove the corresponding
  // producer from |write_queue_|.
  StreamProducerMap stream_producers_;

  // The packet we are currently sending.
  bool write_pending_;            // Will be true when a write is in progress.
  SpdyIOBuffer in_flight_write_;  // This is the write buffer in progress.

  // Flag if we have a pending message scheduled for WriteSocket.
  bool delayed_write_pending_;

  // Flag if we're using an SSL connection for this SpdySession.
  bool is_secure_;

  // Certificate error code when using a secure connection.
  int certificate_error_code_;

  // Spdy Frame state.
  scoped_ptr<BufferedSpdyFramer> buffered_spdy_framer_;

  // If an error has occurred on the session, the session is effectively
  // dead.  Record this error here.  When no error has occurred, |error_| will
  // be OK.
  net::Error error_;
  State state_;

  // Limits
  size_t max_concurrent_streams_;  // 0 if no limit
  size_t max_concurrent_streams_limit_;

  // Some statistics counters for the session.
  int streams_initiated_count_;
  int streams_pushed_count_;
  int streams_pushed_and_claimed_count_;
  int streams_abandoned_count_;

  // |total_bytes_received_| keeps track of all the bytes read by the
  // SpdySession. It is used by the |Net.SpdySettingsCwnd...| histograms.
  int total_bytes_received_;

  // |bytes_read_| keeps track of number of bytes read continously in the
  // DoLoop() without yielding.
  int bytes_read_;

  bool sent_settings_;      // Did this session send settings when it started.
  bool received_settings_;  // Did this session receive at least one settings
                            // frame.
  int stalled_streams_;     // Count of streams that were ever stalled.

  // Count of all pings on the wire, for which we have not gotten a response.
  int64 pings_in_flight_;

  // This is the next ping_id (unique_id) to be sent in PING frame.
  uint32 next_ping_id_;

  // This is the last time we have sent a PING.
  base::TimeTicks last_ping_sent_time_;

  // This is the last time we had activity in the session.
  base::TimeTicks last_activity_time_;

  // This is the next time that unclaimed push streams should be checked for
  // expirations.
  base::TimeTicks next_unclaimed_push_stream_sweep_time_;

  // Indicate if we have already scheduled a delayed task to check the ping
  // status.
  bool check_ping_status_pending_;

  // The (version-dependent) flow control state.
  FlowControlState flow_control_state_;

  // Initial send window size for this session's streams. Can be
  // changed by an arriving SETTINGS frame. Newly created streams use
  // this value for the initial send window size.
  int32 stream_initial_send_window_size_;

  // Initial receive window size for this session's streams. There are
  // plans to add a command line switch that would cause a SETTINGS
  // frame with window size announcement to be sent on startup. Newly
  // created streams will use this value for the initial receive
  // window size.
  int32 stream_initial_recv_window_size_;

  // Session flow control variables. All zero unless session flow
  // control is turned on.
  int32 session_send_window_size_;
  int32 session_recv_window_size_;
  int32 session_unacked_recv_window_bytes_;

  BoundNetLog net_log_;

  // Outside of tests, these should always be true.
  bool verify_domain_authentication_;
  bool enable_sending_initial_settings_;
  bool enable_credential_frames_;
  bool enable_compression_;
  bool enable_ping_based_connection_checking_;
  NextProto default_protocol_;

  SpdyCredentialState credential_state_;

  // |connection_at_risk_of_loss_time_| is an optimization to avoid sending
  // wasteful preface pings (when we just got some data).
  //
  // If it is zero (the most conservative figure), then we always send the
  // preface ping (when none are in flight).
  //
  // It is common for TCP/IP sessions to time out in about 3-5 minutes.
  // Certainly if it has been more than 3 minutes, we do want to send a preface
  // ping.
  //
  // We don't think any connection will time out in under about 10 seconds. So
  // this might as well be set to something conservative like 10 seconds. Later,
  // we could adjust it to send fewer pings perhaps.
  base::TimeDelta connection_at_risk_of_loss_time_;

  // The amount of time that we are willing to tolerate with no activity (of any
  // form), while there is a ping in flight, before we declare the connection to
  // be hung. TODO(rtenneti): When hung, instead of resetting connection, race
  // to build a new connection, and see if that completes before we (finally)
  // get a PING response (http://crbug.com/127812).
  base::TimeDelta hung_interval_;

  // This SPDY proxy is allowed to push resources from origins that are
  // different from those of their associated streams.
  HostPortPair trusted_spdy_proxy_;

  TimeFunc time_func_;
};

}  // namespace net

#endif  // NET_SPDY_SPDY_SESSION_H_
