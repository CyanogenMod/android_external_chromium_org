/*
 * Copyright (C) 2010 Google Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 *     * Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above
 * copyright notice, this list of conditions and the following disclaimer
 * in the documentation and/or other materials provided with the
 * distribution.
 *     * Neither the name of Google Inc. nor the names of its
 * contributors may be used to endorse or promote products derived from
 * this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "config.h"

#include <string>

#include "Document.h"
#include "EventListener.h"
#include "InjectedScriptHost.h"
#include "InspectorBackend.h"
#include "InspectorController.h"
#include "InspectorFrontend.h"
#include "InspectorResource.h"
#include "Node.h"
#include "Page.h"
#include "PlatformString.h"
#include "ResourceError.h"
#include "ResourceRequest.h"
#include "ResourceResponse.h"
#include "ScriptObject.h"
#include "ScriptState.h"
#include "ScriptValue.h"
#include "V8Binding.h"
#include "V8Proxy.h"
#include "V8Utilities.h"
#include <wtf/Noncopyable.h>
#include <wtf/OwnPtr.h>
#undef LOG

#include "grit/webkit_resources.h"
#include "third_party/WebKit/WebKit/chromium/public/WebDataSource.h"
#include "third_party/WebKit/WebKit/chromium/public/WebDevToolsAgentClient.h"
#include "third_party/WebKit/WebKit/chromium/public/WebDevToolsMessageData.h"
#include "third_party/WebKit/WebKit/chromium/public/WebFrame.h"
#include "third_party/WebKit/WebKit/chromium/public/WebString.h"
#include "third_party/WebKit/WebKit/chromium/public/WebURL.h"
#include "third_party/WebKit/WebKit/chromium/public/WebURLError.h"
#include "third_party/WebKit/WebKit/chromium/public/WebURLRequest.h"
#include "third_party/WebKit/WebKit/chromium/public/WebURLResponse.h"
#include "third_party/WebKit/WebKit/chromium/src/WebFrameImpl.h"
#include "third_party/WebKit/WebKit/chromium/src/WebViewImpl.h"
#include "webkit/glue/devtools/bound_object.h"
#include "webkit/glue/devtools/debugger_agent_impl.h"
#include "webkit/glue/devtools/debugger_agent_manager.h"
#include "webkit/glue/devtools/profiler_agent_impl.h"
#include "webkit/glue/glue_util.h"
#include "webkit/glue/webdevtoolsagent_impl.h"
#include "webkit/glue/webkit_glue.h"

using WebCore::Document;
using WebCore::DocumentLoader;
using WebCore::FrameLoader;
using WebCore::InjectedScriptHost;
using WebCore::InspectorBackend;
using WebCore::InspectorController;
using WebCore::InspectorFrontend;
using WebCore::InspectorResource;
using WebCore::Node;
using WebCore::Page;
using WebCore::ResourceError;
using WebCore::ResourceRequest;
using WebCore::ResourceResponse;
using WebCore::SafeAllocation;
using WebCore::ScriptObject;
using WebCore::ScriptState;
using WebCore::ScriptValue;
using WebCore::String;
using WebCore::V8ClassIndex;
using WebCore::V8DOMWrapper;
using WebCore::V8Proxy;
using WebKit::WebDataSource;
using WebKit::WebDevToolsAgentClient;
using WebKit::WebDevToolsMessageData;
using WebKit::WebFrame;
using WebKit::WebFrameImpl;
using WebKit::WebPoint;
using WebKit::WebString;
using WebKit::WebURL;
using WebKit::WebURLError;
using WebKit::WebURLRequest;
using WebKit::WebURLResponse;
using WebKit::WebViewImpl;

namespace {

void InjectedScriptHostWeakReferenceCallback(v8::Persistent<v8::Value> object, void* parameter)
{
    InjectedScriptHost* host = static_cast<InjectedScriptHost*>(parameter);
    host->deref();
    object.Dispose();
}

void InspectorBackendWeakReferenceCallback(v8::Persistent<v8::Value> object, void* parameter)
{
    InspectorBackend* backend = static_cast<InspectorBackend*>(parameter);
    backend->deref();
    object.Dispose();
}

void SetApuAgentEnabledInUtilityContext(v8::Handle<v8::Context> context, bool enabled)
{
    v8::HandleScope handleScope;
    v8::Context::Scope contextScope(context);
    v8::Handle<v8::Object> dispatcher = v8::Local<v8::Object>::Cast(
        context->Global()->Get(v8::String::New("ApuAgentDispatcher")));
    if (dispatcher.IsEmpty())
        return;
    dispatcher->Set(v8::String::New("enabled"), v8::Boolean::New(enabled));
}

// TODO(pfeldman): Make this public in WebDevToolsAgent API.
static const char kApuAgentFeatureName[] = "apu-agent";

// Keep these in sync with the ones in inject_dispatch.js.
static const char kTimelineFeatureName[] = "timeline-profiler";
static const char kResourceTrackingFeatureName[] = "resource-tracking";

class IoRpcDelegate : public DevToolsRpc::Delegate, public Noncopyable {
public:
    IoRpcDelegate() {}
    virtual ~IoRpcDelegate() {}
    virtual void sendRpcMessage(const WebKit::WebDevToolsMessageData& data)
    {
        WebDevToolsAgentClient::sendMessageToFrontendOnIOThread(data);
    }
};

} //  namespace

WebDevToolsAgentImpl::WebDevToolsAgentImpl(
    WebViewImpl* webViewImpl,
    WebDevToolsAgentClient* client)
    : m_hostId(client->hostIdentifier())
    , m_client(client)
    , m_webViewImpl(webViewImpl)
    , m_apuAgentEnabled(false)
    , m_resourceTrackingWasEnabled(false)
    , m_attached(false)
{
    m_debuggerAgentDelegateStub.set(new DebuggerAgentDelegateStub(this));
    m_toolsAgentDelegateStub.set(new ToolsAgentDelegateStub(this));
    m_toolsAgentNativeDelegateStub.set(new ToolsAgentNativeDelegateStub(this));
    m_apuAgentDelegateStub.set(new ApuAgentDelegateStub(this));
}

WebDevToolsAgentImpl::~WebDevToolsAgentImpl()
{
    DebuggerAgentManager::onWebViewClosed(m_webViewImpl);
    disposeUtilityContext();
}

void WebDevToolsAgentImpl::disposeUtilityContext()
{
    if (!m_utilityContext.IsEmpty()) {
        m_utilityContext.Dispose();
        m_utilityContext.Clear();
    }
}

void WebDevToolsAgentImpl::unhideResourcesPanelIfNecessary()
{
    InspectorController* ic = m_webViewImpl->page()->inspectorController();
    ic->ensureResourceTrackingSettingsLoaded();
    String command = String::format("[\"setResourcesPanelEnabled\", %s]",
        ic->resourceTrackingEnabled() ? "true" : "false");
    m_toolsAgentDelegateStub->dispatchOnClient(command);
}

void WebDevToolsAgentImpl::attach()
{
    if (m_attached)
        return;
    m_debuggerAgentImpl.set(
        new DebuggerAgentImpl(m_webViewImpl,
                              m_debuggerAgentDelegateStub.get(),
                              this));
    resetInspectorFrontendProxy();
    unhideResourcesPanelIfNecessary();
    // Allow controller to send messages to the frontend.
    InspectorController* ic = m_webViewImpl->page()->inspectorController();

    { // TODO(yurys): the source should have already been pushed by the frontend.
        v8::HandleScope scope;
        v8::Context::Scope contextScope(m_utilityContext);
        v8::Handle<v8::Value> constructorValue = m_utilityContext->Global()->Get(
            v8::String::New("injectedScriptConstructor"));
        if (constructorValue->IsFunction()) {
            String source = WebCore::toWebCoreString(constructorValue);
            ic->injectedScriptHost()->setInjectedScriptSource("(" + source + ")");
        }
    }

    ic->setWindowVisible(true, false);
    m_attached = true;
}

void WebDevToolsAgentImpl::detach()
{
    // Prevent controller from sending messages to the frontend.
    InspectorController* ic = m_webViewImpl->page()->inspectorController();
    ic->hideHighlight();
    ic->close();
    disposeUtilityContext();
    m_inspectorFrontendScriptState.clear();
    m_debuggerAgentImpl.set(0);
    m_attached = false;
    m_apuAgentEnabled = false;
}

void WebDevToolsAgentImpl::didNavigate()
{
    DebuggerAgentManager::onNavigate();
}

void WebDevToolsAgentImpl::didCommitProvisionalLoad(WebFrameImpl* webframe, bool isNewNavigation)
{
    if (!m_attached)
        return;
    WebDataSource* ds = webframe->dataSource();
    const WebURLRequest& request = ds->request();
    WebURL url = ds->hasUnreachableURL() ?
        ds->unreachableURL() :
        request.url();
    if (!webframe->parent()) {
        resetInspectorFrontendProxy();
        m_toolsAgentDelegateStub->frameNavigate(
            webkit_glue::WebURLToKURL(url).string());
        SetApuAgentEnabledInUtilityContext(m_utilityContext, m_apuAgentEnabled);
        unhideResourcesPanelIfNecessary();
    }
}

void WebDevToolsAgentImpl::didClearWindowObject(WebFrameImpl* webframe)
{
    DebuggerAgentManager::setHostId(webframe, m_hostId);
    if (m_attached) {
        // Push context id into the client if it is already attached.
        m_debuggerAgentDelegateStub->setContextId(m_hostId);
    }
}

void WebDevToolsAgentImpl::forceRepaint()
{
    m_client->forceRepaint();
}

void WebDevToolsAgentImpl::dispatchOnInspectorController(
      int callId,
      const String& functionName,
      const String& jsonArgs)
{
    String result;
    String exception;
    result = m_debuggerAgentImpl->executeUtilityFunction(m_utilityContext, callId,
        "InspectorControllerDispatcher", functionName, jsonArgs, false /* is sync */, &exception);
    m_toolsAgentDelegateStub->didDispatchOn(callId, result, exception);
}

void WebDevToolsAgentImpl::dispatchOnInjectedScript(
      int callId,
      int injectedScriptId,
      const String& functionName,
      const String& jsonArgs,
      bool async)
{
    // TODO(yurys): get rid of this code once WebKit change is rolled.
    if (injectedScriptId != 1000000) {
        inspectorController()->inspectorBackend()->dispatchOnInjectedScript(
            callId,
            injectedScriptId,
            functionName,
            jsonArgs,
            async);
        return;
    }
    String result;
    String exception;
    result = m_debuggerAgentImpl->executeUtilityFunction(m_utilityContext,
        callId, "InjectedScript", functionName, jsonArgs, async, &exception);
    if (!async)
        m_toolsAgentDelegateStub->didDispatchOn(callId, result, exception);
}

void WebDevToolsAgentImpl::executeVoidJavaScript()
{
    m_debuggerAgentImpl->executeVoidJavaScript(m_utilityContext);
}

void WebDevToolsAgentImpl::getResourceContent(int callId, int identifier)
{
    String content;
    Page* page = m_webViewImpl->page();
    if (page) {
        RefPtr<InspectorResource> resource = page->inspectorController()->resources().get(identifier);
        if (resource.get())
            content = resource->sourceString();
    }
    m_toolsAgentNativeDelegateStub->didGetResourceContent(callId, content);
}

void WebDevToolsAgentImpl::dispatchMessageFromFrontend(const WebDevToolsMessageData& data)
{
    if (ToolsAgentDispatch::dispatch(this, data))
        return;

    if (!m_attached)
        return;

    if (m_debuggerAgentImpl.get() && DebuggerAgentDispatch::dispatch(m_debuggerAgentImpl.get(), data))
        return;
}

void WebDevToolsAgentImpl::inspectElementAt(const WebPoint& point)
{
    m_webViewImpl->inspectElementAt(point);
}

void WebDevToolsAgentImpl::setRuntimeFeatureEnabled(const WebString& wfeature, bool enabled)
{
    String feature = webkit_glue::WebStringToString(wfeature);
    if (feature == kApuAgentFeatureName)
        setApuAgentEnabled(enabled);
    else if (feature == kTimelineFeatureName)
        setTimelineProfilingEnabled(enabled);
    else if (feature == kResourceTrackingFeatureName) {
        InspectorController* ic = m_webViewImpl->page()->inspectorController();
        if (enabled)
          ic->enableResourceTracking(false /* not sticky */, false /* no reload */);
        else
          ic->disableResourceTracking(false /* not sticky */);
    }
}

void WebDevToolsAgentImpl::sendRpcMessage(const WebKit::WebDevToolsMessageData& data)
{
    m_client->sendMessageToFrontend(data);
}

void WebDevToolsAgentImpl::initDevToolsAgentHost()
{
    BoundObject devtoolsAgentHost(m_utilityContext, this, "DevToolsAgentHost");
    devtoolsAgentHost.addProtoFunction(
        "dispatch",
        WebDevToolsAgentImpl::jsDispatchOnClient);
    devtoolsAgentHost.addProtoFunction(
        "dispatchToApu",
        WebDevToolsAgentImpl::jsDispatchToApu);
    devtoolsAgentHost.addProtoFunction(
        "evaluateOnSelf",
        WebDevToolsAgentImpl::jsEvaluateOnSelf);
    devtoolsAgentHost.addProtoFunction(
        "runtimeFeatureStateChanged",
        WebDevToolsAgentImpl::jsOnRuntimeFeatureStateChanged);
    devtoolsAgentHost.build();

    v8::HandleScope scope;
    v8::Context::Scope utilityScope(m_utilityContext);
    // Call custom code to create inspector backend wrapper in the utility context
    // instead of calling V8DOMWrapper::convertToV8Object that would create the
    // wrapper in the Page main frame context.
    v8::Handle<v8::Object> scriptHostWrapper = createInjectedScriptHostV8Wrapper();
    if (scriptHostWrapper.IsEmpty())
        return;
    m_utilityContext->Global()->Set(v8::String::New("InjectedScriptHost"), scriptHostWrapper);

    v8::Handle<v8::Object> backendWrapper = createInspectorBackendV8Wrapper();
    if (backendWrapper.IsEmpty())
        return;
    m_utilityContext->Global()->Set(v8::String::New("InspectorBackend"), backendWrapper);
}

v8::Local<v8::Object> WebDevToolsAgentImpl::createInjectedScriptHostV8Wrapper()
{
    V8ClassIndex::V8WrapperType descriptorType = V8ClassIndex::INJECTEDSCRIPTHOST;
    v8::Handle<v8::Function> function = V8DOMWrapper::getTemplate(descriptorType)->GetFunction();
    if (function.IsEmpty()) {
        // Return if allocation failed.
        return v8::Local<v8::Object>();
    }
    v8::Local<v8::Object> instance = SafeAllocation::newInstance(function);
    if (instance.IsEmpty()) {
        // Avoid setting the wrapper if allocation failed.
        return v8::Local<v8::Object>();
    }
    InjectedScriptHost* host = m_webViewImpl->page()->inspectorController()->injectedScriptHost();
    V8DOMWrapper::setDOMWrapper(instance, V8ClassIndex::ToInt(descriptorType), host);
    // Create a weak reference to the v8 wrapper of InspectorBackend to deref
    // InspectorBackend when the wrapper is garbage collected.
    host->ref();
    v8::Persistent<v8::Object> weakHandle = v8::Persistent<v8::Object>::New(instance);
    weakHandle.MakeWeak(host, &InjectedScriptHostWeakReferenceCallback);
    return instance;
}

v8::Local<v8::Object> WebDevToolsAgentImpl::createInspectorBackendV8Wrapper()
{
    V8ClassIndex::V8WrapperType descriptorType = V8ClassIndex::INSPECTORBACKEND;
    v8::Handle<v8::Function> function = V8DOMWrapper::getTemplate(descriptorType)->GetFunction();
    if (function.IsEmpty()) {
        // Return if allocation failed.
        return v8::Local<v8::Object>();
    }
    v8::Local<v8::Object> instance = SafeAllocation::newInstance(function);
    if (instance.IsEmpty()) {
        // Avoid setting the wrapper if allocation failed.
        return v8::Local<v8::Object>();
    }
    InspectorBackend* backend = m_webViewImpl->page()->inspectorController()->inspectorBackend();
    V8DOMWrapper::setDOMWrapper(instance, V8ClassIndex::ToInt(descriptorType), backend);
    // Create a weak reference to the v8 wrapper of InspectorBackend to deref
    // InspectorBackend when the wrapper is garbage collected.
    backend->ref();
    v8::Persistent<v8::Object> weakHandle = v8::Persistent<v8::Object>::New(instance);
    weakHandle.MakeWeak(backend, &InspectorBackendWeakReferenceCallback);
    return instance;
}

void WebDevToolsAgentImpl::resetInspectorFrontendProxy()
{
    disposeUtilityContext();
    m_debuggerAgentImpl->createUtilityContext(m_webViewImpl->page()->mainFrame(), &m_utilityContext);
    initDevToolsAgentHost();

    v8::HandleScope scope;
    v8::Context::Scope contextScope(m_utilityContext);
    m_inspectorFrontendScriptState.set(new ScriptState(
        m_webViewImpl->page()->mainFrame(),
        m_utilityContext));

    v8::Local<v8::Value> injectedScriptValue = m_utilityContext->Global()->Get(v8::String::New("InjectedScript"));
    v8::Local<v8::Object> injectedScript;
    // TODO(yurys): get rid of the 'if' once WebKit is rolled.
    if (injectedScriptValue->IsObject())
        injectedScript = v8::Local<v8::Object>::Cast(injectedScriptValue);
    else {
        v8::Local<v8::Object> global = m_utilityContext->Global();
        v8::Handle<v8::Function> injectedScriptConstructor =
            v8::Local<v8::Function>::Cast(global->Get(v8::String::New("injectedScriptConstructor")));
        v8::Local<v8::Value> injectedScriptHost = global->Get(v8::String::New("InjectedScriptHost"));
        ASSERT(!injectedScriptHost->IsUndefined());
        v8::Local<v8::Value> inspectedWindow = global->Get(v8::String::New("contentWindow"));
        ASSERT(!inspectedWindow->IsUndefined());
        v8::Local<v8::Number> id = v8::Number::New(0);

        v8::Handle<v8::Value> args[] = {
          injectedScriptHost,
          inspectedWindow,
          id
        };
        v8::Local<v8::Value> injectedScriptValue = injectedScriptConstructor->Call(global, 3, args);
        injectedScript = v8::Local<v8::Object>::Cast(injectedScriptValue);
        m_utilityContext->Global()->Set(v8::String::New("InjectedScript"), injectedScript);
    }

    ScriptState* state = m_inspectorFrontendScriptState.get();
    InspectorController* ic = m_webViewImpl->page()->inspectorController();
    ic->setFrontendProxyObject(
        state,
        ScriptObject(state, m_utilityContext->Global()),
        ScriptObject(state, injectedScript));
}

void WebDevToolsAgentImpl::setApuAgentEnabled(bool enabled)
{
    m_apuAgentEnabled = enabled;
    SetApuAgentEnabledInUtilityContext(m_utilityContext, enabled);
    InspectorController* ic = m_webViewImpl->page()->inspectorController();
    if (enabled) {
        m_resourceTrackingWasEnabled = ic->resourceTrackingEnabled();
        ic->startTimelineProfiler();
        if (!m_resourceTrackingWasEnabled) {
            // TODO(knorton): Introduce some kind of agents dependency here so that
            // user could turn off resource tracking while apu agent is on.
            ic->enableResourceTracking(false, false);
        }
        m_debuggerAgentImpl->setAutoContinueOnException(true);
    } else {
      ic->stopTimelineProfiler();
      if (!m_resourceTrackingWasEnabled)
          ic->disableResourceTracking(false);
      m_resourceTrackingWasEnabled = false;
    }
    m_client->runtimeFeatureStateChanged(
        webkit_glue::StringToWebString(kApuAgentFeatureName),
        enabled);
}

// static
v8::Handle<v8::Value> WebDevToolsAgentImpl::jsDispatchOnClient(const v8::Arguments& args)
{
    v8::TryCatch exceptionCatcher;
    String message = WebCore::toWebCoreStringWithNullCheck(args[0]);
    if (message.isEmpty() || exceptionCatcher.HasCaught())
        return v8::Undefined();
    WebDevToolsAgentImpl* agent = static_cast<WebDevToolsAgentImpl*>(v8::External::Cast(*args.Data())->Value());
    agent->m_toolsAgentDelegateStub->dispatchOnClient(message);
    return v8::Undefined();
}

// static
v8::Handle<v8::Value> WebDevToolsAgentImpl::jsDispatchToApu(const v8::Arguments& args)
{
    v8::TryCatch exceptionCatcher;
    String message = WebCore::toWebCoreStringWithNullCheck(args[0]);
    if (message.isEmpty() || exceptionCatcher.HasCaught())
        return v8::Undefined();
    WebDevToolsAgentImpl* agent = static_cast<WebDevToolsAgentImpl*>(
        v8::External::Cast(*args.Data())->Value());
    agent->m_apuAgentDelegateStub->dispatchToApu(message);
    return v8::Undefined();
}

// static
v8::Handle<v8::Value> WebDevToolsAgentImpl::jsEvaluateOnSelf(const v8::Arguments& args)
{
    String code;
    {
        v8::TryCatch exceptionCatcher;
        code = WebCore::toWebCoreStringWithNullCheck(args[0]);
        if (code.isEmpty() || exceptionCatcher.HasCaught())
            return v8::Undefined();
    }
    WebDevToolsAgentImpl* agent = static_cast<WebDevToolsAgentImpl*>(v8::External::Cast(*args.Data())->Value());
    v8::Context::Scope(agent->m_utilityContext);
    V8Proxy* proxy = V8Proxy::retrieve(agent->m_webViewImpl->page()->mainFrame());
    v8::Local<v8::Value> result = proxy->runScript(v8::Script::Compile(v8::String::New(code.utf8().data())), true);
    return result;
}

// static
v8::Handle<v8::Value> WebDevToolsAgentImpl::jsOnRuntimeFeatureStateChanged(const v8::Arguments& args)
{
  v8::TryCatch exceptionCatcher;
  String feature = WebCore::toWebCoreStringWithNullCheck(args[0]);
  bool enabled = args[1]->ToBoolean()->Value();
  if (feature.isEmpty() || exceptionCatcher.HasCaught())
      return v8::Undefined();
  WebDevToolsAgentImpl* agent = static_cast<WebDevToolsAgentImpl*>(v8::External::Cast(*args.Data())->Value());
  agent->m_client->runtimeFeatureStateChanged(webkit_glue::StringToWebString(feature), enabled);
  return v8::Undefined();
}


WebCore::InspectorController* WebDevToolsAgentImpl::inspectorController()
{
    if (Page* page = m_webViewImpl->page())
        return page->inspectorController();
    return 0;
}


//------- plugin resource load notifications ---------------
void WebDevToolsAgentImpl::identifierForInitialRequest(
    unsigned long resourceId,
    WebKit::WebFrame* frame,
    const WebURLRequest& request)
{
    if (InspectorController* ic = inspectorController()) {
        WebFrameImpl* webFrameImpl = static_cast<WebFrameImpl*>(frame);
        FrameLoader* frameLoader = webFrameImpl->frame()->loader();
        DocumentLoader* loader = frameLoader->activeDocumentLoader();
        ic->identifierForInitialRequest(resourceId, loader, request.toResourceRequest());
    }
}

void WebDevToolsAgentImpl::willSendRequest(unsigned long resourceId, const WebURLRequest& request)
{
  if (InspectorController* ic = inspectorController())
      ic->willSendRequest(resourceId, request.toResourceRequest(), ResourceResponse());
}

void WebDevToolsAgentImpl::didReceiveData(unsigned long resourceId, int length)
{
    if (InspectorController* ic = inspectorController())
        ic->didReceiveContentLength(resourceId, length);
}

void WebDevToolsAgentImpl::didReceiveResponse(unsigned long resourceId, const WebURLResponse& response)
{
    if (InspectorController* ic = inspectorController())
        ic->didReceiveResponse(resourceId, response.toResourceResponse());
}

void WebDevToolsAgentImpl::didFinishLoading(unsigned long resourceId)
{
    if (InspectorController* ic = inspectorController())
        ic->didFinishLoading(resourceId);
}

void WebDevToolsAgentImpl::didFailLoading(unsigned long resourceId, const WebURLError& error)
{
    ResourceError resourceError;
    if (InspectorController* ic = inspectorController())
        ic->didFailLoading(resourceId, resourceError);
}

void WebDevToolsAgentImpl::evaluateInWebInspector(long callId, const WebString& script)
{
    InspectorController* ic = inspectorController();
    ic->evaluateForTestInFrontend(callId, webkit_glue::WebStringToString(script));
}

void WebDevToolsAgentImpl::setTimelineProfilingEnabled(bool enabled)
{
    InspectorController* ic = inspectorController();
    if (enabled)
        ic->startTimelineProfiler();
    else
        ic->stopTimelineProfiler();
}


namespace WebKit {

// static
WebDevToolsAgent* WebDevToolsAgent::create(WebView* webview, WebDevToolsAgentClient* client)
{
    return new WebDevToolsAgentImpl(static_cast<WebViewImpl*>(webview), client);
}

// static
void WebDevToolsAgent::executeDebuggerCommand(const WebString& command, int callerId)
{
    DebuggerAgentManager::executeDebuggerCommand(webkit_glue::WebStringToString(command), callerId);
}

// static
void WebDevToolsAgent::debuggerPauseScript()
{
    DebuggerAgentManager::pauseScript();
}

// static
void WebDevToolsAgent::setMessageLoopDispatchHandler(MessageLoopDispatchHandler handler)
{
    DebuggerAgentManager::setMessageLoopDispatchHandler(handler);
}

// static
bool WebDevToolsAgent::dispatchMessageFromFrontendOnIOThread(const WebDevToolsMessageData& data)
{
    IoRpcDelegate transport;
    ProfilerAgentDelegateStub stub(&transport);
    ProfilerAgentImpl agent(&stub);
    return ProfilerAgentDispatch::dispatch(&agent, data);
}


} // namespace WebKit
