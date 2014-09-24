// Copyright (c) 2013 The Chromium Embedded Framework Authors. All rights
// reserved. Use of this source code is governed by a BSD-style license that
// can be found in the LICENSE file.

#include "client_handler.h"

#include <stdio.h>
#include <algorithm>
#include <set>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

#include <include/cef_browser.h>
#include <include/cef_frame.h>
#include <include/cef_path_util.h>
#include <include/cef_process_util.h>
#include <include/cef_runnable.h>
#include <include/cef_trace.h>
#include <include/cef_url.h>
#include <include/wrapper/cef_stream_resource_handler.h>

#include "client_renderer.h"
#include "client_switches.h"
#include "util.h"

namespace {

// Custom menu command Ids.
enum client_menu_ids {
    CLIENT_ID_SHOW_DEVTOOLS   = MENU_ID_USER_FIRST,
    CLIENT_ID_CLOSE_DEVTOOLS
};

}  // namespace

int ClientHandler::m_BrowserCount = 0;

ClientHandler::ClientHandler()
    : m_MainHwnd(NULL),
      m_BrowserId(0),
      m_bIsClosing(false),
      m_bFocusOnEditableField(false),
      m_bDevToolsShown(false),
      view_handler_(NULL), load_handler_(NULL), process_handler_(NULL),
      menu_handler_(NULL), dialog_handler_(NULL), print_handler_(NULL),
      download_handler_(NULL), input_method_editor_handler_(NULL)
{
    // Read command line settings.
    CefRefPtr<CefCommandLine> command_line =
        CefCommandLine::GetGlobalCommandLine();

    if (command_line->HasSwitch(cefclient::kUrl))
        m_StartupURL = command_line->GetSwitchValue(cefclient::kUrl);
    if (m_StartupURL.empty())
        m_StartupURL = "http://www.google.com/";
}

ClientHandler::~ClientHandler()
{
}

// View callbacks
/// CefDisplayHandler
void ClientHandler::OnAddressChange(CefRefPtr<CefBrowser> browser,
                                    CefRefPtr<CefFrame> frame,
                                    const CefString& url)
{
    if (!view_handler_.get())
        return;
    view_handler_->OnAddressChange(browser, frame, url);
}
void ClientHandler::OnTitleChange(CefRefPtr<CefBrowser> browser,
                                  const CefString& title)
{
    if (!view_handler_.get())
        return;
    view_handler_->OnTitleChange(browser, title);
}
bool ClientHandler::OnConsoleMessage(CefRefPtr<CefBrowser> browser,
                                     const CefString& message,
                                     const CefString& source,
                                     int line)
{
    if (!view_handler_.get())
        return;
    view_handler_->OnConsoleMessage(browser, message, source, line);
}
/// CefRenderHandler
void ClientHandler::OnCursorChange(CefRefPtr<CefBrowser> browser,
                                   CefCursorHandler cursor)
{
    if (!view_handler_.get())
        return;
    view_handler_->OnCursorChange(browser, cursor);
}
/// CefRenderProcessHandler [CefApp]
void ClientHandler::OnFocusedNodeChanged(CefRefPtr<CefBrowser> browser,
                                         CefRefPtr<CefFrame> frame,
                                         CefRefPtr<CefDOMNode> node)
{
    if (!view_handler_.get())
        return;
    view_handler_->OnFocusedNodeChanged(browser, frame, node);
}

bool ClientHandler::OnProcessMessageReceived(
    CefRefPtr<CefBrowser> browser,
    CefProcessId source_process,
    CefRefPtr<CefProcessMessage> message)
{
    if (message_router_->OnProcessMessageReceived(browser, source_process,
                                                  message)) {
        return true;
    }

    // Handle process messages
    for (auto it = message_delegates_.begin(); it != message_delegates_.end();
         ++it) {
        if ((*it)->OnProcessMessageReceived(browser, source_process, message))
            return true;
    }

    // Check for messages from the client renderer.
    std::string message_name = message->GetName();
    if (message_name == client_renderer::kFocusedNodeChangedMessage) {
        // A message is sent from ClientRenderDelegate to tell us whether the
        // currently focused DOM node is editable. Use of |m_bFocusOnEditableField|
        // is redundant with CefKeyEvent.focus_on_editable_field in OnPreKeyEvent
        // but is useful for demonstration purposes.
        m_bFocusOnEditableField = message->GetArgumentList()->GetBool(0);
        return true;
    }

    return false;
}

void ClientHandler::OnBeforeContextMenu(
    CefRefPtr<CefBrowser> browser,
    CefRefPtr<CefFrame> frame,
    CefRefPtr<CefContextMenuParams> params,
    CefRefPtr<CefMenuModel> model)
{
    // if ((params->GetTypeFlags() & (CM_TYPEFLAG_PAGE | CM_TYPEFLAG_FRAME)) != 0) {
    //     // Add a separator if the menu already has items.
    //     if (model->GetCount() > 0)
    //         model->AddSeparator();
    //
    //     // Add DevTools items to all context menus.
    //     model->AddItem(CLIENT_ID_SHOW_DEVTOOLS, "&Show DevTools");
    //     model->AddItem(CLIENT_ID_CLOSE_DEVTOOLS, "Close DevTools");
    // }
    model->Clear();
}

bool ClientHandler::OnContextMenuCommand(
    CefRefPtr<CefBrowser> browser,
    CefRefPtr<CefFrame> frame,
    CefRefPtr<CefContextMenuParams> params,
    int command_id,
    EventFlags event_flags)
{
    // switch (command_id) {
    // case CLIENT_ID_SHOW_DEVTOOLS:
    //     ShowDevTools(browser);
    //     return true;
    // case CLIENT_ID_CLOSE_DEVTOOLS:
    //     CloseDevTools(browser);
    //     return true;
    // }
    return true;
}

bool ClientHandler::OnConsoleMessage(CefRefPtr<CefBrowser> browser,
                                     const CefString& message,
                                     const CefString& source,
                                     int line)
{
    REQUIRE_UI_THREAD();

    return false;
}

void ClientHandler::OnBeforeDownload(
    CefRefPtr<CefBrowser> browser,
    CefRefPtr<CefDownloadItem> download_item,
    const CefString& suggested_name,
    CefRefPtr<CefBeforeDownloadCallback> callback)
{
    REQUIRE_UI_THREAD();
    // Continue the download and show the "Save As" dialog.
    callback->Continue(GetDownloadPath(suggested_name), true);
}

void ClientHandler::OnDownloadUpdated(
    CefRefPtr<CefBrowser> browser,
    CefRefPtr<CefDownloadItem> download_item,
    CefRefPtr<CefDownloadItemCallback> callback)
{
    REQUIRE_UI_THREAD();
    if (download_item->IsComplete()) {
        SetLastDownloadFile(download_item->GetFullPath());
        SendNotification(NOTIFY_DOWNLOAD_COMPLETE);
    }
}

bool ClientHandler::OnDragEnter(CefRefPtr<CefBrowser> browser,
                                CefRefPtr<CefDragData> dragData,
                                DragOperationsMask mask)
{
    REQUIRE_UI_THREAD();

    // Forbid dragging of link URLs.
    if (mask & DRAG_OPERATION_LINK)
        return true;

    return false;
}

void ClientHandler::OnRequestGeolocationPermission(
    CefRefPtr<CefBrowser> browser,
    const CefString& requesting_url,
    int request_id,
    CefRefPtr<CefGeolocationCallback> callback)
{
    // Allow geolocation access from all websites.
    callback->Continue(true);
}

bool ClientHandler::OnPreKeyEvent(CefRefPtr<CefBrowser> browser,
                                  const CefKeyEvent& event,
                                  CefEventHandle os_event,
                                  bool* is_keyboard_shortcut)
{
    if (event.type != KEYEVENT_RAWKEYDOWN)  // make sure to handle once
        return false;

    switch (event.windows_key_code) {
    case 0x7b:  // F12
        if (m_bDevToolsShown) {
            CloseDevTools(m_Browser);  // @note the is the main browser
        } else {
            ShowDevTools(m_Browser);
        }
        m_bDevToolsShown = !m_bDevToolsShown;
        return true;
    }

    return false;
}

bool ClientHandler::OnBeforePopup(CefRefPtr<CefBrowser> browser,
                                  CefRefPtr<CefFrame> frame,
                                  const CefString& target_url,
                                  const CefString& target_frame_name,
                                  const CefPopupFeatures& popupFeatures,
                                  CefWindowInfo& windowInfo,
                                  CefRefPtr<CefClient>& client,
                                  CefBrowserSettings& settings,
                                  bool* no_javascript_access)
{
    if (browser->GetHost()->IsWindowRenderingDisabled()) {
        // Cancel popups in off-screen rendering mode.
        return true;
    }
    return false;
}

void ClientHandler::OnAfterCreated(CefRefPtr<CefBrowser> browser)
{
    REQUIRE_UI_THREAD();

    if (!message_router_) {
        // Create the browser-side router for query handling.
        CefMessageRouterConfig config;
        message_router_ = CefMessageRouterBrowserSide::Create(config);

        // Register handlers with the router.
        CreateMessageHandlers(message_handlers_);
        MessageHandlerSet::const_iterator it = message_handlers_.begin();
        for (; it != message_handlers_.end(); ++it)
            message_router_->AddHandler(*(it), false);
    }

    // @note Important - Create message handlers for platform
    //CreateMessageDelegates(message_delegates_);

    AutoLock lock_scope(this);
    if (!m_Browser.get())   {
        // We need to keep the main child window, but not popup windows
        m_Browser = browser;
        m_BrowserId = browser->GetIdentifier();
    } else if (browser->IsPopup()) {
        // Add to the list of popup browsers.
        m_PopupBrowsers.push_back(browser);
    }

    m_BrowserCount++;
}

bool ClientHandler::DoClose(CefRefPtr<CefBrowser> browser)
{
    REQUIRE_UI_THREAD();

    // Closing the main window requires special handling. See the DoClose()
    // documentation in the CEF header for a detailed destription of this
    // process.
    if (m_BrowserId == browser->GetIdentifier()) {
        // Notify the browser that the parent window is about to close.
        browser->GetHost()->ParentWindowWillClose();

        // Set a flag to indicate that the window close should be allowed.
        m_bIsClosing = true;
    }

    // Allow the close. For windowed browsers this will result in the OS close
    // event being sent.
    return false;
}

void ClientHandler::OnBeforeClose(CefRefPtr<CefBrowser> browser)
{
    REQUIRE_UI_THREAD();

    message_router_->OnBeforeClose(browser);

    if (m_BrowserId == browser->GetIdentifier()) {
        // Free the browser pointer so that the browser can be destroyed
        m_Browser = NULL;

        if (m_OSRHandler.get()) {
            m_OSRHandler->OnBeforeClose(browser);
            m_OSRHandler = NULL;
        }
    } else if (browser->IsPopup()) {
        // Remove from the browser popup list.
        BrowserList::iterator it = m_PopupBrowsers.begin();
        for (; it != m_PopupBrowsers.end(); ++it) {
            if ((*it)->IsSame(browser)) {
                m_PopupBrowsers.erase(it);
                break;
            }
        }
    }

    if (--m_BrowserCount == 0) {
        // All browser windows have closed.
        // Remove and delete message router handlers.
        MessageHandlerSet::const_iterator it = message_handlers_.begin();
        for (; it != message_handlers_.end(); ++it) {
            message_router_->RemoveHandler(*(it));
            delete *(it);
        }
        message_handlers_.clear();
        message_router_ = NULL;

        // Quit the application message loop.
        //@todo AppQuitMessageLoop();
    }
}

void ClientHandler::OnLoadingStateChange(CefRefPtr<CefBrowser> browser,
                                         bool isLoading,
                                         bool canGoBack,
                                         bool canGoForward)
{
    REQUIRE_UI_THREAD();

    SetLoading(isLoading);
    SetNavState(canGoBack, canGoForward);
}

void ClientHandler::OnLoadError(CefRefPtr<CefBrowser> browser,
                                CefRefPtr<CefFrame> frame,
                                ErrorCode errorCode,
                                const CefString& errorText,
                                const CefString& failedUrl)
{
    REQUIRE_UI_THREAD();

    // Don't display an error for downloaded files.
    if (errorCode == ERR_ABORTED)
        return;

    // Don't display an error for external protocols that we allow the OS to
    // handle. See OnProtocolExecution().
    if (errorCode == ERR_UNKNOWN_URL_SCHEME) {
        std::string urlStr = frame->GetURL();
        if (urlStr.find("spotify:") == 0)
            return;
    }

    // Display a load error message.
    std::stringstream ss;
    ss << "<html><body bgcolor=\"white\">"
        "<h2>Failed to load URL " << std::string(failedUrl) <<
            " with error " << std::string(errorText) << " (" << errorCode <<
                ").</h2></body></html>";
    frame->LoadString(ss.str(), failedUrl);
}

bool ClientHandler::OnBeforeBrowse(CefRefPtr<CefBrowser> browser,
                                   CefRefPtr<CefFrame> frame,
                                   CefRefPtr<CefRequest> request,
                                   bool is_redirect)
{
    message_router_->OnBeforeBrowse(browser, frame);
    return false;
}

CefRefPtr<CefResourceHandler> ClientHandler::GetResourceHandler(
    CefRefPtr<CefBrowser> browser,
    CefRefPtr<CefFrame> frame,
    CefRefPtr<CefRequest> request)
{
    std::string url = request->GetURL();
    // @note This is the place to handle certain url specially

    return NULL;
}

bool ClientHandler::OnQuotaRequest(CefRefPtr<CefBrowser> browser,
                                   const CefString& origin_url,
                                   int64 new_size,
                                   CefRefPtr<CefQuotaCallback> callback)
{
    static const int64 max_size = 1024 * 1024 * 20;  // 20mb.

    // Grant the quota request if the size is reasonable.
    callback->Continue(new_size <= max_size);
    return true;
}

void ClientHandler::OnProtocolExecution(CefRefPtr<CefBrowser> browser,
                                        const CefString& url,
                                        bool& allow_os_execution)
{
    std::string urlStr = url;

    // Allow OS execution of Spotify URIs.
    if (urlStr.find("spotify:") == 0)
        allow_os_execution = true;
}

void ClientHandler::OnRenderProcessTerminated(CefRefPtr<CefBrowser> browser,
                                              TerminationStatus status)
{
    message_router_->OnRenderProcessTerminated(browser);

    // Load the startup URL if that's not the website that we terminated on.
    CefRefPtr<CefFrame> frame = browser->GetMainFrame();
    std::string url = frame->GetURL();
    std::transform(url.begin(), url.end(), url.begin(), tolower);

    std::string startupURL = GetStartupURL();
    if (startupURL != "chrome://crash" && !url.empty() &&
        url.find(startupURL) != 0) {
        frame->LoadURL(startupURL);
    }
}

/// *** BEGIN IMPORTANT *** ///
bool ClientHandler::GetRootScreenRect(CefRefPtr<CefBrowser> browser,
                                      CefRect& rect)
{
    if (!m_OSRHandler.get())
        return false;
    return m_OSRHandler->GetRootScreenRect(browser, rect);
}

bool ClientHandler::GetViewRect(CefRefPtr<CefBrowser> browser, CefRect& rect)
{
    if (!m_OSRHandler.get())
        return false;
    return m_OSRHandler->GetViewRect(browser, rect);
}

/// *** BEGIN IMPORTANT *** ///
bool ClientHandler::GetScreenPoint(CefRefPtr<CefBrowser> browser,
                                   int viewX,
                                   int viewY,
                                   int& screenX,
                                   int& screenY)
{
    if (!m_OSRHandler.get())
        return false;
    return m_OSRHandler->GetScreenPoint(browser, viewX, viewY, screenX, screenY);
}

bool ClientHandler::GetScreenInfo(CefRefPtr<CefBrowser> browser,
                                  CefScreenInfo& screen_info)
{
    if (!m_OSRHandler.get())
        return false;
    return m_OSRHandler->GetScreenInfo(browser, screen_info);
}

void ClientHandler::OnPopupShow(CefRefPtr<CefBrowser> browser,
                                bool show)
{
    if (!m_OSRHandler.get())
        return;
    return m_OSRHandler->OnPopupShow(browser, show);
}

void ClientHandler::OnPopupSize(CefRefPtr<CefBrowser> browser,
                                const CefRect& rect)
{
    if (!m_OSRHandler.get())
        return;
    return m_OSRHandler->OnPopupSize(browser, rect);
}

void ClientHandler::OnPaint(CefRefPtr<CefBrowser> browser,
                            PaintElementType type,
                            const RectList& dirtyRects,
                            const void* buffer,
                            int width,
                            int height)
{
    if (!m_OSRHandler.get())
        return;
    m_OSRHandler->OnPaint(browser, type, dirtyRects, buffer, width, height);
}

void ClientHandler::OnCursorChange(CefRefPtr<CefBrowser> browser,
                                   CefCursorHandle cursor)
{
    if (!m_OSRHandler.get())
        return;
    m_OSRHandler->OnCursorChange(browser, cursor);
}
/// *** END IMPORTANT *** ///

void ClientHandler::SetMainHwnd(CefWindowHandle hwnd)
{
    AutoLock lock_scope(this);
    m_MainHwnd = hwnd;
}

void ClientHandler::CloseAllBrowsers(bool force_close)
{
    if (!CefCurrentlyOn(TID_UI)) {
        // Execute on the UI thread.
        CefPostTask(TID_UI,
                    NewCefRunnableMethod(this, &ClientHandler::CloseAllBrowsers,
                                         force_close));
        return;
    }

    if (!m_PopupBrowsers.empty()) {
        // Request that any popup browsers close.
        BrowserList::const_iterator it = m_PopupBrowsers.begin();
        for (; it != m_PopupBrowsers.end(); ++it)
            (*it)->GetHost()->CloseBrowser(force_close);
    }

    if (m_Browser.get()) {
        // Request that the main browser close.
        m_Browser->GetHost()->CloseBrowser(force_close);
    }
}

void ClientHandler::SetLastDownloadFile(const std::string& fileName)
{
    AutoLock lock_scope(this);
    m_LastDownloadFile = fileName;
}

std::string ClientHandler::GetLastDownloadFile()
{
    AutoLock lock_scope(this);
    return m_LastDownloadFile;
}

void ClientHandler::ShowDevTools(CefRefPtr<CefBrowser> browser)
{
    CefWindowInfo windowInfo;
    CefBrowserSettings settings;

#ifdef OS_WIN
    windowInfo.SetAsPopup(browser->GetHost()->GetWindowHandle(), "DevTools");
#endif

    browser->GetHost()->ShowDevTools(windowInfo, this, settings);
}

void ClientHandler::CloseDevTools(CefRefPtr<CefBrowser> browser)
{
    browser->GetHost()->CloseDevTools();
}

void ClientHandler::BeginTracing()
{
    if (CefCurrentlyOn(TID_UI)) {
        CefBeginTracing(CefString());
    } else {
        CefPostTask(TID_UI,
                    NewCefRunnableMethod(this, &ClientHandler::BeginTracing));
    }
}

void ClientHandler::EndTracing()
{
    if (CefCurrentlyOn(TID_UI)) {
        class Client : public CefEndTracingCallback,
                       public CefRunFileDialogCallback
        {
        public:
            explicit Client(CefRefPtr<ClientHandler> handler)
                : handler_(handler) {
                    RunDialog();
            }

            void RunDialog() {
                static const char kDefaultFileName[] = "trace.txt";
                std::string path = handler_->GetDownloadPath(kDefaultFileName);
                if (path.empty())
                    path = kDefaultFileName;

                // Results in a call to OnFileDialogDismissed.
                handler_->GetBrowser()->GetHost()->RunFileDialog(
                    FILE_DIALOG_SAVE, CefString(), path, std::vector<CefString>(),
                    this);
            }

            virtual void OnFileDialogDismissed(
                CefRefPtr<CefBrowserHost> browser_host,
                const std::vector<CefString>& file_paths) OVERRIDE {
                if (!file_paths.empty()) {
                    // File selected. Results in a call to OnEndTracingComplete.
                    CefEndTracingAsync(file_paths.front(), this);
                } else {
                    // No file selected. Discard the trace data.
                    CefEndTracingAsync(CefString(), NULL);
                }
            }

            virtual void OnEndTracingComplete(const CefString& tracing_file) OVERRIDE {
                handler_->SetLastDownloadFile(tracing_file.ToString());
                handler_->SendNotification(NOTIFY_DOWNLOAD_COMPLETE);
            }

        private:
            CefRefPtr<ClientHandler> handler_;

            IMPLEMENT_REFCOUNTING(Callback);
        };

        new Client(this);
    } else {
        CefPostTask(TID_UI,
                    NewCefRunnableMethod(this, &ClientHandler::BeginTracing));
    }
}

bool ClientHandler::Save(const std::string& path, const std::string& data)
{
    FILE* f = fopen(path.c_str(), "w");
    if (!f)
        return false;
    size_t total = 0;
    do {
        size_t write = fwrite(data.c_str() + total, 1, data.size() - total, f);
        if (write == 0)
            break;
        total += write;
    } while (total < data.size());
    fclose(f);
    return true;
}

// static
void ClientHandler::CreateMessageHandlers(MessageHandlerSet& handlers)
{
    /// @todo
}
