// Copyright (c) 2012 The Chromium Embedded Framework Authors. All rights
// reserved. Use of this source code is governed by a BSD-style license that
// can be found in the LICENSE file.

#ifndef CEF_TESTS_CEFCLIENT_CLIENT_RENDERER_H_
#define CEF_TESTS_CEFCLIENT_CLIENT_RENDERER_H_
#pragma once

#include <map>
#include <string>

#include <include/cef_base.h>
#include <include/cef_v8.h>

#include "client_app.h"

namespace client_renderer {

// Message sent when the focused node changes.
extern const char kFocusedNodeChangedMessage[];
extern const char kTestMessage[];
extern const char kPlatformMessage[];

typedef std::map<std::pair<std::string, int>,
                 std::pair<CefRefPtr<CefV8Context>, CefRefPtr<CefV8Value> > >
                 CallbackMap;
extern CallbackMap g_callbacks;

// Create platform message with the specified event
std::string GenPlatformMsg(std::string event_name);
// Judge whether or not the specified message is of type platform
bool IsPlatformMsg(std::string message);
// Parse event name from message name
std::string GetEventFromMsg(std::string message);

// Create the render delegate.
void CreateRenderDelegates(ClientApp::RenderDelegateSet& delegates);

}  // namespace client_renderer

#endif  // CEF_TESTS_CEFCLIENT_CLIENT_RENDERER_H_
