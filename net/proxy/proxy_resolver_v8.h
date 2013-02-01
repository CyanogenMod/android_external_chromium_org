// Copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef NET_PROXY_PROXY_RESOLVER_V8_H_
#define NET_PROXY_PROXY_RESOLVER_V8_H_

#include "base/compiler_specific.h"
#include "base/memory/scoped_ptr.h"
#include "net/base/net_export.h"
#include "net/proxy/proxy_resolver.h"

namespace net {

// Implementation of ProxyResolver that uses V8 to evaluate PAC scripts.
//
// ----------------------------------------------------------------------------
// !!! Important note on threading model:
// ----------------------------------------------------------------------------
// There can be only one instance of V8 running at a time. To enforce this
// constraint, ProxyResolverV8 holds a v8::Locker during execution. Therefore
// it is OK to run multiple instances of ProxyResolverV8 on different threads,
// since only one will be running inside V8 at a time.
//
// It is important that *ALL* instances of V8 in the process be using
// v8::Locker. If not there can be race conditions between the non-locked V8
// instances and the locked V8 instances used by ProxyResolverV8 (assuming they
// run on different threads).
//
// This is the case with the V8 instance used by chromium's renderer -- it runs
// on a different thread from ProxyResolver (renderer thread vs PAC thread),
// and does not use locking since it expects to be alone.
class NET_EXPORT_PRIVATE ProxyResolverV8 : public ProxyResolver {
 public:
  // Interface for the javascript bindings.
  class NET_EXPORT_PRIVATE JSBindings {
   public:
    enum ResolveDnsOperation {
      DNS_RESOLVE,
      DNS_RESOLVE_EX,
      MY_IP_ADDRESS,
      MY_IP_ADDRESS_EX,
      NUM_DNS_OPERATIONS,
    };

    JSBindings() {}

    // Handler for "dnsResolve()", "dnsResolveEx()", "myIpAddress()",
    // "myIpAddressEx()". Returns true on success and fills |*output| with the
    // result.
    virtual bool ResolveDns(const std::string& host,
                            ResolveDnsOperation op,
                            std::string* output) = 0;

    // Handler for "alert(message)"
    virtual void Alert(const string16& message) = 0;

    // Handler for when an error is encountered. |line_number| may be -1
    // if a line number is not applicable to this error.
    virtual void OnError(int line_number, const string16& error) = 0;

   protected:
    virtual ~JSBindings() {}
  };

  // Constructs a ProxyResolverV8.
  ProxyResolverV8();

  virtual ~ProxyResolverV8();

  JSBindings* js_bindings() const { return js_bindings_; }
  void set_js_bindings(JSBindings* js_bindings) { js_bindings_ = js_bindings; }

  // ProxyResolver implementation:
  virtual int GetProxyForURL(const GURL& url,
                             ProxyInfo* results,
                             const net::CompletionCallback& /*callback*/,
                             RequestHandle* /*request*/,
                             const BoundNetLog& net_log) OVERRIDE;
  virtual void CancelRequest(RequestHandle request) OVERRIDE;
  virtual LoadState GetLoadState(RequestHandle request) const OVERRIDE;
  virtual void CancelSetPacScript() OVERRIDE;
  virtual void PurgeMemory() OVERRIDE;
  virtual int SetPacScript(
      const scoped_refptr<ProxyResolverScriptData>& script_data,
      const net::CompletionCallback& /*callback*/) OVERRIDE;

 private:
  // Context holds the Javascript state for the most recently loaded PAC
  // script. It corresponds with the data from the last call to
  // SetPacScript().
  class Context;

  scoped_ptr<Context> context_;

  JSBindings* js_bindings_;

  DISALLOW_COPY_AND_ASSIGN(ProxyResolverV8);
};

}  // namespace net

#endif  // NET_PROXY_PROXY_RESOLVER_V8_H_
