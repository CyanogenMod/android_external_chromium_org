// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/renderer/media/cast_udp_transport.h"

#include "base/logging.h"
#include "chrome/renderer/media/cast_session.h"

CastUdpTransport::CastUdpTransport(
    const scoped_refptr<CastSession>& session)
    : cast_session_(session) {
}

CastUdpTransport::~CastUdpTransport() {
}

void CastUdpTransport::Start(const net::HostPortPair& remote_address) {
  NOTIMPLEMENTED();
}
