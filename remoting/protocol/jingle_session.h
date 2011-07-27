// Copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef REMOTING_PROTOCOL_JINGLE_SESSION_H_
#define REMOTING_PROTOCOL_JINGLE_SESSION_H_

#include "base/memory/ref_counted.h"
#include "base/task.h"
#include "crypto/rsa_private_key.h"
#include "net/base/completion_callback.h"
#include "remoting/protocol/session.h"
#include "third_party/libjingle/source/talk/base/sigslot.h"
#include "third_party/libjingle/source/talk/p2p/base/session.h"

namespace remoting {

namespace protocol {

class JingleChannelConnector;
class JingleSessionManager;

// Implements protocol::Session that work over libjingle session (the
// cricket::Session object is passed to Init() method). Created
// by JingleSessionManager for incoming and outgoing connections.
class JingleSession : public protocol::Session,
                      public sigslot::has_slots<> {
 public:
  static const char kChromotingContentName[];

  // Session interface.
  virtual void SetStateChangeCallback(StateChangeCallback* callback) OVERRIDE;
  virtual void CreateStreamChannel(
      const std::string& name,
      const StreamChannelCallback& callback) OVERRIDE;
  virtual void CreateDatagramChannel(
      const std::string& name,
      const DatagramChannelCallback& callback) OVERRIDE;
  virtual net::Socket* control_channel() OVERRIDE;
  virtual net::Socket* event_channel() OVERRIDE;
  virtual net::Socket* video_channel() OVERRIDE;
  virtual net::Socket* video_rtp_channel() OVERRIDE;
  virtual net::Socket* video_rtcp_channel() OVERRIDE;
  virtual const std::string& jid() OVERRIDE;
  virtual const CandidateSessionConfig* candidate_config() OVERRIDE;
  virtual const SessionConfig* config() OVERRIDE;
  virtual void set_config(const SessionConfig* config) OVERRIDE;
  virtual const std::string& initiator_token() OVERRIDE;
  virtual void set_initiator_token(const std::string& initiator_token) OVERRIDE;
  virtual const std::string& receiver_token() OVERRIDE;
  virtual void set_receiver_token(const std::string& receiver_token) OVERRIDE;
  virtual void Close() OVERRIDE;

 private:
  friend class JingleDatagramConnector;
  friend class JingleSessionManager;
  friend class JingleStreamConnector;

  // Create a JingleSession used in client mode. A server certificate is
  // required.
  static JingleSession* CreateClientSession(JingleSessionManager* manager,
                                            const std::string& host_public_key);

  // Create a JingleSession used in server mode. A server certificate and
  // private key is provided. |key| is copied in the constructor.
  //
  // TODO(sergeyu): Remove |certificate| and |key| when we stop using TLS.
  static JingleSession* CreateServerSession(
      JingleSessionManager* manager,
      const std::string& certificate,
      crypto::RSAPrivateKey* key);

  // TODO(sergeyu): Change type of |peer_public_key| to RSAPublicKey.
  JingleSession(JingleSessionManager* jingle_session_manager,
                const std::string& local_cert,
                crypto::RSAPrivateKey* local_private_key,
                const std::string& peer_public_key);
  virtual ~JingleSession();

  // Called by JingleSessionManager.
  void set_candidate_config(const CandidateSessionConfig* candidate_config);
  const std::string& local_certificate() const;
  void Init(cricket::Session* cricket_session);
  std::string GetEncryptedMasterKey() const;

  // Close all the channels and terminate the session.
  void CloseInternal(int result, bool failed);

  bool HasSession(cricket::Session* cricket_session);
  cricket::Session* ReleaseSession();

  // Initialize the session configuration from a received connection response
  // stanza.
  bool InitializeConfigFromDescription(
      const cricket::SessionDescription* description);

  // Used for Session.SignalState sigslot.
  void OnSessionState(cricket::BaseSession* session,
                      cricket::BaseSession::State state);
  // Used for Session.SignalError sigslot.
  void OnSessionError(cricket::BaseSession* session,
                      cricket::BaseSession::Error error);

  void OnInitiate();
  void OnAccept();
  void OnTerminate();

  // Notifies upper layer about incoming connection and
  // accepts/rejects connection.
  void AcceptConnection();

  void AddChannelConnector(const std::string& name,
                           JingleChannelConnector* connector);

  // Called by JingleChannelConnector when it has finished connecting
  // the channel and needs to be destroyed.
  void OnChannelConnectorFinished(const std::string& name,
                                  JingleChannelConnector* connector);

  // Creates channels after session has been accepted.
  // TODO(sergeyu): Don't create channels in JingleSession.
  void CreateChannels();

  // Callbacks for the channels created in JingleSession.
  // TODO(sergeyu): Remove this method once *_channel() methods are
  // removed from Session interface.
  void OnStreamChannelConnected(
      const std::string& name, net::StreamSocket* socket);
  void OnChannelConnected(const std::string& name, net::Socket* socket);

  const cricket::ContentInfo* GetContentInfo() const;

  void SetState(State new_state);

  // JingleSessionManager that created this session. Guaranteed to
  // exist throughout the lifetime of the session.
  JingleSessionManager* jingle_session_manager_;

  // Certificates used for connection. Currently only receiving side
  // has a certificate.
  std::string local_cert_;
  std::string remote_cert_;

  // Private key used in SSL server sockets.
  scoped_ptr<crypto::RSAPrivateKey> local_private_key_;

  // Public key of the peer.
  std::string peer_public_key_;

  // Master key used to derive ice keys for each ice
  // session. Generated on the client and sent to the host in the
  // session-initiate message (encrypted with the host's key).
  std::string master_key_;

  State state_;
  scoped_ptr<StateChangeCallback> state_change_callback_;

  bool closed_;
  bool closing_;

  // JID of the other side. Set when the connection is initialized,
  // and never changed after that.
  std::string jid_;

  // The corresponding libjingle session.
  cricket::Session* cricket_session_;

  scoped_ptr<const SessionConfig> config_;

  std::string initiator_token_;
  std::string receiver_token_;

  // These data members are only set on the receiving side.
  scoped_ptr<const CandidateSessionConfig> candidate_config_;

  // Channels that are currently being connected.
  std::map<std::string, JingleChannelConnector*> channel_connectors_;

  scoped_ptr<net::Socket> control_channel_socket_;
  scoped_ptr<net::Socket> event_channel_socket_;
  scoped_ptr<net::Socket> video_channel_socket_;
  scoped_ptr<net::Socket> video_rtp_channel_socket_;
  scoped_ptr<net::Socket> video_rtcp_channel_socket_;

  ScopedRunnableMethodFactory<JingleSession> task_factory_;

  DISALLOW_COPY_AND_ASSIGN(JingleSession);
};

}  // namespace protocol

}  // namespace remoting

#endif  // REMOTING_PROTOCOL_JINGLE_SESSION_H_
