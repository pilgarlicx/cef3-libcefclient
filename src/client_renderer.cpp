// Copyright (c) 2012 The Chromium Embedded Framework Authors. All rights
// reserved. Use of this source code is governed by a BSD-style license that
// can be found in the LICENSE file.

#include "client_renderer.h"

#include <sstream>
#include <string>

#include <include/cef_dom.h>
#include <include/cef_v8.h>
#include <include/wrapper/cef_message_router.h>

#include "util.h"
#include "v8_util.h"

namespace client_renderer {

    const char kFocusedNodeChangedMessage[] = "ClientRenderer.FocusedNodeChanged";
    const char kTestMessage[] = "ClientRenderer.TestMsg";
    const char kPlatformMessage[] = "ClientRenderer.PlatformMsg";

    CallbackMap g_callbacks;

    std::string GenPlatformMsg(std::string event_name) {
        std::ostringstream oss;
        oss << kPlatformMessage << ':' << event_name;
        return oss.str();
    }

    bool IsPlatformMsg(std::string message) {
        return message.substr(0, message.find_first_of(':')) == std::string(kPlatformMessage);
    }

    std::string GetEventFromMsg(std::string message) {
        return message.substr(message.find_first_of(':') + 1);
    }

    namespace {

        /// @note Test platform javascript callbacks
        class PlatformV8Handler : public CefV8Handler
        {
        public:
            PlatformV8Handler() {}
            virtual bool Execute(const CefString& name,
                                 CefRefPtr<CefV8Value> object,
                                 const CefV8ValueList& arguments,
                                 CefRefPtr<CefV8Value>& retval,
                                 CefString& exception) OVERRIDE {
                CefRefPtr<CefV8Context> context = CefV8Context::GetCurrentContext();
                if (name == "bind") {
                    // Save the callback, together with its context
                    if (arguments.size() == 2 && arguments[0]->IsString()
                        && arguments[1]->IsFunction()) {
                        std::string event_name = arguments[0]->GetStringValue();
                        std::string message_name = GenPlatformMsg(event_name);
                        int browser_id = context->GetBrowser()->GetIdentifier();
                        g_callbacks[std::make_pair(message_name, browser_id)] =
                            std::make_pair(context, arguments[1]);
                        return true;
                    }
                } else if (name == "emit") {
                    // Send IPC message to the browser process
                    if (arguments.size() >= 1 && arguments[0]->IsString()) {
                        std::string event_name = arguments[0]->GetStringValue();
                        std::string message_name = GenPlatformMsg(event_name);
                        CefRefPtr<CefProcessMessage> message =
                            CefProcessMessage::Create(message_name);
                        CefRefPtr<CefListValue> args =
                            message->GetArgumentList();
                        util::SetList(arguments, args);
                        context->GetBrowser()->SendProcessMessage(PID_BROWSER,
                                                                  message);
                        return true;
                    }
                }
                return false;
            }

            IMPLEMENT_REFCOUNTING(PlatformV8Handler);
        };


        class ClientRenderDelegate : public ClientApp::RenderDelegate
        {
        public:
            ClientRenderDelegate()
                : last_node_is_editable_(false) {
            }

            virtual void OnWebKitInitialized(CefRefPtr<ClientApp> app) OVERRIDE
            {
                // Create the renderer-side router for query handling.
                CefMessageRouterConfig config;
                message_router_ = CefMessageRouterRendererSide::Create(config);
            }

            virtual void OnContextCreated(CefRefPtr<ClientApp> app,
                                          CefRefPtr<CefBrowser> browser,
                                          CefRefPtr<CefFrame> frame,
                                          CefRefPtr<CefV8Context> context)
                OVERRIDE
            {
                message_router_->OnContextCreated(browser,  frame, context);
                /// test JavaScript functions and window bindings
                CefRefPtr<CefV8Value> global = context->GetGlobal();
                CefRefPtr<CefV8Handler> platform_handler =
                    new PlatformV8Handler();
                CefRefPtr<CefV8Value> platform = CefV8Value::CreateObject(NULL);
                // JavaScript callbacks binding
                CefRefPtr<CefV8Value> bind_fn =
                    CefV8Value::CreateFunction("bind", platform_handler);
                platform->SetValue("bind", bind_fn, V8_PROPERTY_ATTRIBUTE_NONE);
                CefRefPtr<CefV8Value> emit_fn =
                    CefV8Value::CreateFunction("emit", platform_handler);
                platform->SetValue("emit", emit_fn, V8_PROPERTY_ATTRIBUTE_NONE);
                global->SetValue("platform", platform,
                                 V8_PROPERTY_ATTRIBUTE_NONE);
            }

            virtual void OnContextReleased(CefRefPtr<ClientApp> app,
                                           CefRefPtr<CefBrowser> browser,
                                           CefRefPtr<CefFrame> frame,
                                           CefRefPtr<CefV8Context> context)
                OVERRIDE
            {
                message_router_->OnContextReleased(browser,  frame, context);
                // Remove any JavaScript callbacks registered for the context that
                // is being released
                for (auto it = g_callbacks.begin(); it != g_callbacks.end();) {
                    if (it->second.first->IsSame(context))
                        g_callbacks.erase(it++);
                    else
                        ++it;
                }
            }

            virtual void OnFocusedNodeChanged(CefRefPtr<ClientApp> app,
                                              CefRefPtr<CefBrowser> browser,
                                              CefRefPtr<CefFrame> frame,
                                              CefRefPtr<CefDOMNode> node)
                OVERRIDE
            {
                bool is_editable = (node.get() && node->IsEditable());
                if (is_editable != last_node_is_editable_) {
                    // Notify the browser of the change in focused element type.
                    // @note This is the demo for sending message to the browser
                    // process.
                    last_node_is_editable_ = is_editable;
                    CefRefPtr<CefProcessMessage> message =
                        CefProcessMessage::Create(kFocusedNodeChangedMessage);
                    message->GetArgumentList()->SetBool(0, is_editable);
                    browser->SendProcessMessage(PID_BROWSER, message);
                }
            }

            virtual bool OnProcessMessageReceived(
                CefRefPtr<ClientApp> app,
                CefRefPtr<CefBrowser> browser,
                CefProcessId source_process,
                CefRefPtr<CefProcessMessage> message) OVERRIDE
            {
                if (message_router_->OnProcessMessageReceived(browser,
                                                              source_process,
                                                              message)) {
                    return true;
                }
                bool handled = false;
                std::string message_name = message->GetName();
                int browser_id = browser->GetIdentifier();
                auto it = g_callbacks.find(std::make_pair(message_name, browser_id));
                if (it != g_callbacks.end()) {
                    CefRefPtr<CefV8Context> context = it->second.first;
                    CefRefPtr<CefV8Value> callback = it->second.second;
                    context->Enter();
                    CefV8ValueList arguments;
                    util::SetList(message->GetArgumentList(), arguments);
                    std::string event_name = GetEventFromMsg(message_name);
                    CefRefPtr<CefV8Value> retval = callback->ExecuteFunction(NULL, arguments);
                    if (retval.get()) {
                        if (retval->IsBool())
                            handled = retval->GetBoolValue();
                    }
                    context->Exit();
                }
                return handled;
            }

        private:
            bool last_node_is_editable_;

            // Handles the renderer side of query routing.
            CefRefPtr<CefMessageRouterRendererSide> message_router_;

            IMPLEMENT_REFCOUNTING(ClientRenderDelegate);
        };

    }  // namespace

    void CreateRenderDelegates(ClientApp::RenderDelegateSet& delegates)
    {
        delegates.insert(new ClientRenderDelegate);
    }

}  // namespace client_renderer
