// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// This is the list of load states and their values. For the enum values,
// include the file "net/base/load_states.h".
//
// Here we define the values using a macro LOAD_STATE, so it can be
// expanded differently in some places (for example, to automatically
// map a load flag value to its symbolic name).

// This is the default state.  It corresponds to a resource load that has
// either not yet begun or is idle waiting for the consumer to do something
// to move things along (e.g., the consumer of an URLRequest may not have
// called Read yet).
LOAD_STATE(IDLE)

// This state indicates that the URLRequest delegate has chosen to block this
// request before it was sent over the network. When in this state, the
// delegate should set a load state parameter on the URLRequest describing
// the nature of the delay (i.e. "Waiting for <description given by
// delegate>").
LOAD_STATE(WAITING_FOR_DELEGATE)

// This state corresponds to a resource load that is blocked waiting for
// access to a resource in the cache.  If multiple requests are made for the
// same resource, the first request will be responsible for writing (or
// updating) the cache entry and the second request will be deferred until
// the first completes.  This may be done to optimize for cache reuse.
LOAD_STATE(WAITING_FOR_CACHE)

// This state corresponds to a resource load that is blocked waiting for
// access to a resource in the AppCache.
// Note: This is a layering violation, but being the only one it's not that
// bad. TODO(rvargas): Reconsider what to do if we need to add more.
LOAD_STATE(WAITING_FOR_APPCACHE)

// This state corresponds to a resource being blocked waiting for the
// PAC script to be downloaded.
LOAD_STATE(DOWNLOADING_PROXY_SCRIPT)

// This state corresponds to a resource load that is blocked waiting for a
// proxy autoconfig script to return a proxy server to use.
LOAD_STATE(RESOLVING_PROXY_FOR_URL)

// This state corresponds to a resource load that is blocked waiting for a
// proxy autoconfig script to return a proxy server to use, but that proxy
// script is busy resolving the IP address of a host.
LOAD_STATE(RESOLVING_HOST_IN_PROXY_SCRIPT)

// This state indicates that we're in the process of establishing a tunnel
// through the proxy server.
LOAD_STATE(ESTABLISHING_PROXY_TUNNEL)

// This state corresponds to a resource load that is blocked waiting for a
// host name to be resolved.  This could either indicate resolution of the
// origin server corresponding to the resource or to the host name of a proxy
// server used to fetch the resource.
LOAD_STATE(RESOLVING_HOST)

// This state corresponds to a resource load that is blocked waiting for a
// TCP connection (or other network connection) to be established.  HTTP
// requests that reuse a keep-alive connection skip this state.
LOAD_STATE(CONNECTING)

// This state corresponds to a resource load that is blocked waiting for the
// SSL handshake to complete.
LOAD_STATE(SSL_HANDSHAKE)

// This state corresponds to a resource load that is blocked waiting to
// completely upload a request to a server.  In the case of a HTTP POST
// request, this state includes the period of time during which the message
// body is being uploaded.
LOAD_STATE(SENDING_REQUEST)

// This state corresponds to a resource load that is blocked waiting for the
// response to a network request.  In the case of a HTTP transaction, this
// corresponds to the period after the request is sent and before all of the
// response headers have been received.
LOAD_STATE(WAITING_FOR_RESPONSE)

// This state corresponds to a resource load that is blocked waiting for a
// read to complete.  In the case of a HTTP transaction, this corresponds to
// the period after the response headers have been received and before all of
// the response body has been downloaded.  (NOTE: This state only applies for
// an URLRequest while there is an outstanding Read operation.)
LOAD_STATE(READING_RESPONSE)
