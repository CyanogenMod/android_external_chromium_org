// Copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "remoting/client/plugin/chromoting_instance.h"

#include <string>
#include <vector>

#include "base/bind.h"
#include "base/json/json_reader.h"
#include "base/logging.h"
#include "base/message_loop.h"
#include "base/stringprintf.h"
#include "base/synchronization/waitable_event.h"
#include "base/task.h"
#include "base/threading/thread.h"
#include "base/values.h"
// TODO(sergeyu): We should not depend on renderer here. Instead P2P
// Pepper API should be used. Remove this dependency.
// crbug.com/74951
#include "content/renderer/p2p/ipc_network_manager.h"
#include "content/renderer/p2p/ipc_socket_factory.h"
#include "media/base/media.h"
#include "ppapi/c/dev/ppb_query_policy_dev.h"
#include "ppapi/cpp/completion_callback.h"
#include "ppapi/cpp/input_event.h"
#include "ppapi/cpp/rect.h"
// TODO(wez): Remove this when crbug.com/86353 is complete.
#include "ppapi/cpp/private/var_private.h"
#include "remoting/base/task_thread_proxy.h"
#include "remoting/base/util.h"
#include "remoting/client/client_config.h"
#include "remoting/client/chromoting_client.h"
#include "remoting/client/rectangle_update_decoder.h"
#include "remoting/client/plugin/chromoting_scriptable_object.h"
#include "remoting/client/plugin/pepper_input_handler.h"
#include "remoting/client/plugin/pepper_port_allocator_session.h"
#include "remoting/client/plugin/pepper_view.h"
#include "remoting/client/plugin/pepper_view_proxy.h"
#include "remoting/client/plugin/pepper_util.h"
#include "remoting/client/plugin/pepper_xmpp_proxy.h"
#include "remoting/proto/auth.pb.h"
#include "remoting/protocol/connection_to_host.h"
#include "remoting/protocol/host_stub.h"
// TODO(sergeyu): This is a hack: plugin should not depend on webkit
// glue. It is used here to get P2PPacketDispatcher corresponding to
// the current RenderView. Use P2P Pepper API for connection and
// remove these includes.
// crbug.com/74951
#include "webkit/plugins/ppapi/ppapi_plugin_instance.h"
#include "webkit/plugins/ppapi/resource_tracker.h"

namespace remoting {

namespace {

const char kClientFirewallTraversalPolicyName[] =
    "remote_access.client_firewall_traversal";

}  // namespace

PPP_PolicyUpdate_Dev ChromotingInstance::kPolicyUpdatedInterface = {
  &ChromotingInstance::PolicyUpdatedThunk,
};

// This flag blocks LOGs to the UI if we're already in the middle of logging
// to the UI. This prevents a potential infinite loop if we encounter an error
// while sending the log message to the UI.
static bool g_logging_to_plugin = false;
static ChromotingInstance* g_logging_instance = NULL;
static logging::LogMessageHandlerFunction g_logging_old_handler = NULL;

const char* ChromotingInstance::kMimeType = "pepper-application/x-chromoting";

ChromotingInstance::ChromotingInstance(PP_Instance pp_instance)
    : pp::InstancePrivate(pp_instance),
      initialized_(false),
      scale_to_fit_(false),
      enable_client_nat_traversal_(false),
      initial_policy_received_(false),
      task_factory_(ALLOW_THIS_IN_INITIALIZER_LIST(this)) {
  RequestInputEvents(PP_INPUTEVENT_CLASS_MOUSE | PP_INPUTEVENT_CLASS_WHEEL);
  RequestFilteringInputEvents(PP_INPUTEVENT_CLASS_KEYBOARD);

  log_proxy_ = new TaskThreadProxy(MessageLoop::current());

  // Set up log message handler.
  // Note that this approach doesn't quite support having multiple instances
  // of Chromoting running. In that case, the most recently opened tab will
  // grab all the debug log messages, and when any Chromoting tab is closed
  // the logging handler will go away.
  // Since having multiple Chromoting tabs is not a primary use case, and this
  // is just debug logging, we're punting improving debug log support for that
  // case.
  if (g_logging_old_handler == NULL)
    g_logging_old_handler = logging::GetLogMessageHandler();
  logging::SetLogMessageHandler(&LogToUI);
  g_logging_instance = this;
}

ChromotingInstance::~ChromotingInstance() {
  DCHECK(CurrentlyOnPluginThread());

  // Detach the log proxy so we don't log anything else to the UI.
  log_proxy_->Detach();

  // Remove log message handler.
  logging::SetLogMessageHandler(g_logging_old_handler);
  g_logging_old_handler = NULL;
  g_logging_instance = NULL;

  if (client_.get()) {
    base::WaitableEvent done_event(true, false);
    client_->Stop(base::Bind(&base::WaitableEvent::Signal,
                             base::Unretained(&done_event)));
    done_event.Wait();
  }

  // Stopping the context shutdown all chromoting threads. This is a requirement
  // before we can call Detach() on |view_proxy_|.
  context_.Stop();

  if (view_proxy_.get()) {
    view_proxy_->Detach();
  }
}

bool ChromotingInstance::Init(uint32_t argc,
                              const char* argn[],
                              const char* argv[]) {
  CHECK(!initialized_);
  initialized_ = true;

  VLOG(1) << "Started ChromotingInstance::Init";

  // Check to make sure the media library is initialized.
  // http://crbug.com/91521.
  if (!media::IsMediaLibraryInitialized()) {
    LOG(ERROR) << "Media library not initialized.";
    return false;
  }

  // Start all the threads.
  context_.Start();

  SubscribeToNatTraversalPolicy();

  // Create the chromoting objects that don't depend on the network connection.
  view_.reset(new PepperView(this, &context_));
  view_proxy_ = new PepperViewProxy(this, view_.get());
  rectangle_decoder_ = new RectangleUpdateDecoder(
      context_.decode_message_loop(), view_proxy_);

  // Default to a medium grey.
  view_->SetSolidFill(0xFFCDCDCD);

  return true;
}

void ChromotingInstance::Connect(const ClientConfig& config) {
  DCHECK(CurrentlyOnPluginThread());

  // This can only happen at initialization if the Javascript connect call
  // occurs before the enterprise policy is read.  We are guaranteed that the
  // enterprise policy is pushed at least once, we we delay the connect call.
  if (!initial_policy_received_) {
    LOG(INFO) << "Delaying connect until initial policy is read.";
    delayed_connect_.reset(
        task_factory_.NewRunnableMethod(&ChromotingInstance::Connect,
                                          config));
    return;
  }

  webkit::ppapi::PluginInstance* plugin_instance =
      webkit::ppapi::ResourceTracker::Get()->GetInstance(pp_instance());

  P2PSocketDispatcher* socket_dispatcher =
      plugin_instance->delegate()->GetP2PSocketDispatcher();

  IpcNetworkManager* network_manager = NULL;
  IpcPacketSocketFactory* socket_factory = NULL;
  PortAllocatorSessionFactory* session_factory =
      CreatePepperPortAllocatorSessionFactory(this);

  // If we don't have socket dispatcher for IPC (e.g. P2P API is
  // disabled), then JingleSessionManager will try to use physical sockets.
  if (socket_dispatcher) {
    VLOG(1) << "Creating IpcNetworkManager and IpcPacketSocketFactory.";
    network_manager = new IpcNetworkManager(socket_dispatcher);
    socket_factory = new IpcPacketSocketFactory(socket_dispatcher);
  }

  host_connection_.reset(new protocol::ConnectionToHost(
      context_.network_message_loop(), network_manager, socket_factory,
      session_factory, enable_client_nat_traversal_));
  input_handler_.reset(new PepperInputHandler(&context_,
                                              host_connection_.get(),
                                              view_proxy_));

  client_.reset(new ChromotingClient(config, &context_, host_connection_.get(),
                                     view_proxy_, rectangle_decoder_.get(),
                                     input_handler_.get(), NULL));

  LOG(INFO) << "Connecting.";

  // Setup the XMPP Proxy.
  ChromotingScriptableObject* scriptable_object = GetScriptableObject();
  scoped_refptr<PepperXmppProxy> xmpp_proxy =
      new PepperXmppProxy(scriptable_object->AsWeakPtr(),
                          context_.network_message_loop());
  scriptable_object->AttachXmppProxy(xmpp_proxy);

  // Kick off the connection.
  client_->Start(xmpp_proxy);

  LOG(INFO) << "Connection status: Initializing";
  GetScriptableObject()->SetConnectionInfo(STATUS_INITIALIZING,
                                           QUALITY_UNKNOWN);
}

void ChromotingInstance::Disconnect() {
  DCHECK(CurrentlyOnPluginThread());

  LOG(INFO) << "Disconnecting from host.";
  if (client_.get()) {
    // TODO(sergeyu): Should we disconnect asynchronously?
    base::WaitableEvent done_event(true, false);
    client_->Stop(base::Bind(&base::WaitableEvent::Signal,
                             base::Unretained(&done_event)));
    done_event.Wait();
    client_.reset();
  }

  input_handler_.reset();
  host_connection_.reset();

  GetScriptableObject()->SetConnectionInfo(STATUS_CLOSED, QUALITY_UNKNOWN);
}

void ChromotingInstance::DidChangeView(const pp::Rect& position,
                                       const pp::Rect& clip) {
  DCHECK(CurrentlyOnPluginThread());

  bool size_changed =
      view_->SetPluginSize(gfx::Size(position.width(), position.height()));

  // If scale to fit there is no clipping so just update the scale ratio.
  if (scale_to_fit_) {
    rectangle_decoder_->SetScaleRatios(view_->GetHorizontalScaleRatio(),
                                       view_->GetVerticalScaleRatio());
  }

  if (size_changed) {
    // If plugin size has changed there may be regions uncovered so simply
    // request a full refresh.
    rectangle_decoder_->RefreshFullFrame();
  } else if (!scale_to_fit_) {
    rectangle_decoder_->UpdateClipRect(
        gfx::Rect(clip.x(), clip.y(), clip.width(), clip.height()));
  }
}

bool ChromotingInstance::HandleInputEvent(const pp::InputEvent& event) {
  DCHECK(CurrentlyOnPluginThread());
  if (!input_handler_.get()) {
    return false;
  }

  PepperInputHandler* pih
      = static_cast<PepperInputHandler*>(input_handler_.get());

  switch (event.GetType()) {
    case PP_INPUTEVENT_TYPE_MOUSEDOWN: {
      pih->HandleMouseButtonEvent(true, pp::MouseInputEvent(event));
      return true;
    }

    case PP_INPUTEVENT_TYPE_MOUSEUP: {
      pih->HandleMouseButtonEvent(false, pp::MouseInputEvent(event));
      return true;
    }

    case PP_INPUTEVENT_TYPE_MOUSEMOVE:
    case PP_INPUTEVENT_TYPE_MOUSEENTER:
    case PP_INPUTEVENT_TYPE_MOUSELEAVE: {
      pih->HandleMouseMoveEvent(pp::MouseInputEvent(event));
      return true;
    }

    case PP_INPUTEVENT_TYPE_CONTEXTMENU: {
      // We need to return true here or else we'll get a local (plugin) context
      // menu instead of the mouseup event for the right click.
      return true;
    }

    case PP_INPUTEVENT_TYPE_KEYDOWN: {
      pp::KeyboardInputEvent key = pp::KeyboardInputEvent(event);
      VLOG(3) << "PP_INPUTEVENT_TYPE_KEYDOWN" << " key=" << key.GetKeyCode();
      pih->HandleKeyEvent(true, key);
      return true;
    }

    case PP_INPUTEVENT_TYPE_KEYUP: {
      pp::KeyboardInputEvent key = pp::KeyboardInputEvent(event);
      VLOG(3) << "PP_INPUTEVENT_TYPE_KEYUP" << " key=" << key.GetKeyCode();
      pih->HandleKeyEvent(false, key);
      return true;
    }

    case PP_INPUTEVENT_TYPE_CHAR: {
      pih->HandleCharacterEvent(pp::KeyboardInputEvent(event));
      return true;
    }

    default: {
      LOG(INFO) << "Unhandled input event: " << event.GetType();
      break;
    }
  }

  return false;
}

ChromotingScriptableObject* ChromotingInstance::GetScriptableObject() {
  pp::VarPrivate object = GetInstanceObject();
  if (!object.is_undefined()) {
    pp::deprecated::ScriptableObject* so = object.AsScriptableObject();
    DCHECK(so != NULL);
    return static_cast<ChromotingScriptableObject*>(so);
  }
  LOG(ERROR) << "Unable to get ScriptableObject for Chromoting plugin.";
  return NULL;
}

void ChromotingInstance::SubmitLoginInfo(const std::string& username,
                                         const std::string& password) {
  if (host_connection_->state() !=
      protocol::ConnectionToHost::STATE_CONNECTED) {
    LOG(INFO) << "Client not connected or already authenticated.";
    return;
  }

  protocol::LocalLoginCredentials* credentials =
      new protocol::LocalLoginCredentials();
  credentials->set_type(protocol::PASSWORD);
  credentials->set_username(username);
  credentials->set_credential(password.data(), password.length());

  host_connection_->host_stub()->BeginSessionRequest(
      credentials,
      new DeleteTask<protocol::LocalLoginCredentials>(credentials));
}

void ChromotingInstance::SetScaleToFit(bool scale_to_fit) {
  DCHECK(CurrentlyOnPluginThread());

  if (scale_to_fit == scale_to_fit_)
    return;

  scale_to_fit_ = scale_to_fit;
  if (scale_to_fit) {
    rectangle_decoder_->SetScaleRatios(view_->GetHorizontalScaleRatio(),
                                       view_->GetVerticalScaleRatio());
  } else {
    rectangle_decoder_->SetScaleRatios(1.0, 1.0);
  }
  rectangle_decoder_->RefreshFullFrame();
}

// static
bool ChromotingInstance::LogToUI(int severity, const char* file, int line,
                                 size_t message_start,
                                 const std::string& str) {
  if (g_logging_instance) {
    std::string message = remoting::GetTimestampString();
    message += (str.c_str() + message_start);
    g_logging_instance->log_proxy_->Call(
        base::Bind(&ChromotingInstance::ProcessLogToUI,
                   base::Unretained(g_logging_instance), message));
  }

  if (g_logging_old_handler)
    return (g_logging_old_handler)(severity, file, line, message_start, str);
  return false;
}

void ChromotingInstance::ProcessLogToUI(const std::string& message) {
  if (!g_logging_to_plugin) {
    ChromotingScriptableObject* cso = GetScriptableObject();
    if (cso) {
      g_logging_to_plugin = true;
      cso->LogDebugInfo(message);
      g_logging_to_plugin = false;
    }
  }
}

pp::Var ChromotingInstance::GetInstanceObject() {
  if (instance_object_.is_undefined()) {
    ChromotingScriptableObject* object = new ChromotingScriptableObject(this);
    object->Init();

    // The pp::Var takes ownership of object here.
    instance_object_ = pp::VarPrivate(this, object);
  }

  return instance_object_;
}

ChromotingStats* ChromotingInstance::GetStats() {
  if (!client_.get())
    return NULL;
  return client_->GetStats();
}

void ChromotingInstance::ReleaseAllKeys() {
  if (!input_handler_.get()) {
    return;
  }

  input_handler_->ReleaseAllKeys();
}

// static
void ChromotingInstance::PolicyUpdatedThunk(PP_Instance pp_instance,
                                            PP_Var pp_policy_json) {
  ChromotingInstance* instance = static_cast<ChromotingInstance*>(
      pp::Module::Get()->InstanceForPPInstance(pp_instance));
  std::string policy_json =
      pp::Var(pp::Var::DontManage(), pp_policy_json).AsString();
  instance->HandlePolicyUpdate(policy_json);
}

void ChromotingInstance::SubscribeToNatTraversalPolicy() {
  pp::Module::Get()->AddPluginInterface(PPP_POLICY_UPDATE_DEV_INTERFACE,
                                        &kPolicyUpdatedInterface);
  const PPB_QueryPolicy_Dev* query_policy_interface =
      static_cast<PPB_QueryPolicy_Dev const*>(
          pp::Module::Get()->GetBrowserInterface(
              PPB_QUERY_POLICY_DEV_INTERFACE));
  query_policy_interface->SubscribeToPolicyUpdates(pp_instance());
}

bool ChromotingInstance::IsNatTraversalAllowed(
    const std::string& policy_json) {
  int error_code = base::JSONReader::JSON_NO_ERROR;
  std::string error_message;
  scoped_ptr<base::Value> policy(base::JSONReader::ReadAndReturnError(
      policy_json, true, &error_code, &error_message));

  if (!policy.get()) {
    LOG(ERROR) << "Error " << error_code << " parsing policy: "
               << error_message << ".";
    return false;
  }

  if (!policy->IsType(base::Value::TYPE_DICTIONARY)) {
    LOG(ERROR) << "Policy must be a dictionary";
    return false;
  }

  base::DictionaryValue* dictionary =
      static_cast<base::DictionaryValue*>(policy.get());
  bool traversal_policy = false;
  if (!dictionary->GetBoolean(kClientFirewallTraversalPolicyName,
                              &traversal_policy)) {
    // Disable NAT traversal on any failure of reading the policy.
    return false;
  }

  return traversal_policy;
}

void ChromotingInstance::HandlePolicyUpdate(const std::string policy_json) {
  DCHECK(CurrentlyOnPluginThread());
  bool traversal_policy = IsNatTraversalAllowed(policy_json);

  // If the policy changes from traversal allowed, to traversal denied, we
  // need to immediately drop all connections and redo the conneciton
  // preparation.
  if (traversal_policy == false &&
      traversal_policy != enable_client_nat_traversal_) {
    if (client_.get()) {
      // This will delete the client and network related objects.
      Disconnect();
    }
  }

  initial_policy_received_ = true;
  enable_client_nat_traversal_ = traversal_policy;

  if (delayed_connect_.get()) {
    RunTaskOnPluginThread(delayed_connect_.release());
  }
}

}  // namespace remoting
