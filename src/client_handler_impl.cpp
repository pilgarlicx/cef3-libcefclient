// Copyright (c) 2013 The Chromium Embedded Framework Authors. All rights
// reserved. Use of this source code is governed by a BSD-style license that
// can be found in the LICENSE file.

#include "client_handler_impl.h"

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

int ClientHandlerImpl::m_BrowserCount = 0;

ClientHandlerImpl::ClientHandlerImpl()
    : ClientHandler(),
      m_MainHwnd(NULL),
      m_BrowserId(0),
      m_bIsClosing(false),
      m_bCanGoBack(false),
      m_bCanGoForward(false),
      m_bIsCrashed(false),
      m_HistLinksPos(-1),
      m_bFocusOnEditableField(false),
      m_bDevToolsShown(false)
{
    // Read command line settings.
    CefRefPtr<CefCommandLine> command_line =
        CefCommandLine::GetGlobalCommandLine();

    if (command_line->HasSwitch(cefclient::kUrl))
        m_StartupURL = command_line->GetSwitchValue(cefclient::kUrl);
    if (m_StartupURL.empty())
        m_StartupURL = "http://www.google.com/";
}

ClientHandlerImpl::~ClientHandlerImpl()
{
}

// View
/// CefDisplayHandler
void ClientHandlerImpl::OnAddressChange(CefRefPtr<CefBrowser> browser,
                                    CefRefPtr<CefFrame> frame,
                                    const CefString& url)
{
    REQUIRE_UI_THREAD();
    if (view_handler_.get())
        view_handler_->OnAddressChange(browser, frame, url);
}
void ClientHandlerImpl::OnTitleChange(CefRefPtr<CefBrowser> browser,
                                  const CefString& title)
{
    REQUIRE_UI_THREAD();
    if (view_handler_.get())
        view_handler_->OnTitleChange(browser, title);

    m_Title = title;
}
bool ClientHandlerImpl::OnConsoleMessage(CefRefPtr<CefBrowser> browser,
                                     const CefString& message,
                                     const CefString& source,
                                     int line)
{
    if (view_handler_.get())
        return view_handler_->OnConsoleMessage(browser, message, source, line);
    return false;
}
/// CefRenderHandler
void ClientHandlerImpl::OnCursorChange(CefRefPtr<CefBrowser> browser,
                                   CefCursorHandle cursor)
{
    REQUIRE_UI_THREAD();
    if (view_handler_.get())
        view_handler_->OnCursorChange(browser, cursor);
}

/// Interfaces for outter wrapper
void ClientHandlerImpl::GoBack()
{
    if (m_Browser.get() && CanGoBack()) {
        m_Browser->GoBack();
        m_HistLinksPos--;
    }
}
void ClientHandlerImpl::GoForward()
{
    if (m_Browser.get() && CanGoForward()) {
        m_Browser->GoForward();
        m_HistLinksPos++;
    }
}
void ClientHandlerImpl::GoToHistoryOffset(int offset)
{
    if (m_Browser.get() && m_HistLinksPos + offset >= 0 &&
            m_HistLinksPos + offset < m_HistLinks.size()) {
        m_HistLinksPos += offset;
        m_Browser->GetMainFrame()->LoadURL(m_HistLinks[m_HistLinksPos]);
    }
}
void ClientHandlerImpl::Stop()
{
    if (m_Browser.get())
        m_Browser->StopLoad();
}
void ClientHandlerImpl::Reload(bool ignore_cache)
{
    if (m_Browser.get()) {
        if (ignore_cache)
            m_Browser->ReloadIgnoreCache();
        else
            m_Browser->Reload();
    }
}
void ClientHandlerImpl::Resize(int width, int height)
{
    //TODO
}
void ClientHandlerImpl::PauseRendering()
{
    //TODO
}
void ClientHandlerImpl::ResumeRendering()
{
    //TODO
}
void ClientHandlerImpl::Focus()
{
    if (m_Browser.get())
        m_Browser->GetHost()->SendFocusEvent(true);
}
void ClientHandlerImpl::Unfocus()
{
    if (m_Browser.get())
        m_Browser->GetHost()->SendFocusEvent(false);
}
double ClientHandlerImpl::GetZoomLevel()
{
    if (m_Browser.get())
        return m_Browser->GetHost()->GetZoomLevel();
    return 0;
}
void ClientHandlerImpl::SetZoomLevel(double zoom_level)
{
    if (m_Browser.get())
        m_Browser->GetHost()->SetZoomLevel(zoom_level);
}
///

bool ClientHandlerImpl::OnProcessMessageReceived(
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

    return false;
}

void ClientHandlerImpl::OnBeforeContextMenu(
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

    /// CEF3-Awesomium
    if (menu_handler_.get())
        menu_handler_->OnBeforeContextMenu(browser, frame, params, model);
}

bool ClientHandlerImpl::OnContextMenuCommand(
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

void ClientHandlerImpl::OnBeforeDownload(
    CefRefPtr<CefBrowser> browser,
    CefRefPtr<CefDownloadItem> download_item,
    const CefString& suggested_name,
    CefRefPtr<CefBeforeDownloadCallback> callback)
{
    REQUIRE_UI_THREAD();
    // // Continue the download and show the "Save As" dialog.
    // callback->Continue(GetDownloadPath(suggested_name), true);
    if (download_handler_.get())
        download_handler_->OnBeforeDownload(browser, download_item,
                suggested_name, callback);
}

void ClientHandlerImpl::OnDownloadUpdated(
    CefRefPtr<CefBrowser> browser,
    CefRefPtr<CefDownloadItem> download_item,
    CefRefPtr<CefDownloadItemCallback> callback)
{
    REQUIRE_UI_THREAD();
    // if (download_item->IsComplete()) {
    //     SetLastDownloadFile(download_item->GetFullPath());
    //     SendNotification(NOTIFY_DOWNLOAD_COMPLETE);
    // }
    if (download_handler_.get())
        download_handler_->OnDownloadUpdated(browser, download_item, callback);
}

bool ClientHandlerImpl::OnDragEnter(CefRefPtr<CefBrowser> browser,
                                CefRefPtr<CefDragData> dragData,
                                DragOperationsMask mask)
{
    REQUIRE_UI_THREAD();

    // Forbid dragging of link URLs.
    if (mask & DRAG_OPERATION_LINK)
        return true;

    return false;
}

void ClientHandlerImpl::OnRequestGeolocationPermission(
    CefRefPtr<CefBrowser> browser,
    const CefString& requesting_url,
    int request_id,
    CefRefPtr<CefGeolocationCallback> callback)
{
    // Allow geolocation access from all websites.
    callback->Continue(true);
}

bool ClientHandlerImpl::OnPreKeyEvent(CefRefPtr<CefBrowser> browser,
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

bool ClientHandlerImpl::OnBeforePopup(CefRefPtr<CefBrowser> browser,
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

void ClientHandlerImpl::OnAfterCreated(CefRefPtr<CefBrowser> browser)
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

bool ClientHandlerImpl::DoClose(CefRefPtr<CefBrowser> browser)
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

void ClientHandlerImpl::OnBeforeClose(CefRefPtr<CefBrowser> browser)
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

// CefLoadHandler
void ClientHandlerImpl::OnLoadingStateChange(CefRefPtr<CefBrowser> browser,
                                             bool isLoading,
                                             bool canGoBack,
                                             bool canGoForward)
{
    SetLoading(isLoading);
    SetNavState(canGoBack, canGoForward);
}
void ClientHandlerImpl::OnLoadStart(CefRefPtr<CefBrowser> browser,
                                    CefRefPtr<CefFrame> frame)
{
    if (load_handler_.get())
        load_handler_->OnLoadStart(browser, frame);
    if (frame->IsMain() && frame->GetURL() != m_HistLinks[m_HistLinksPos]) {
        // Update history links on main frame when opening new link
        m_HistLinks.resize(m_HistLinksPos + 1);
        m_HistLinks.push_back(frame->GetURL());
        m_HistLinksPos++;
    }
}
void ClientHandlerImpl::OnLoadError(CefRefPtr<CefBrowser> browser,
                                    CefRefPtr<CefFrame> frame,
                                    ErrorCode errorCode,
                                    const CefString& errorText,
                                    const CefString& failedUrl)
{
    REQUIRE_UI_THREAD();

    // Don't display an error for downloaded files.
    if (errorCode == ERR_ABORTED)
        return;

    if (load_handler_.get())
        load_handler_->OnLoadError(browser, frame, errorCode, errorText,
                failedUrl);
}
void ClientHandlerImpl::OnLoadEnd(CefRefPtr<CefBrowser> browser,
                                  CefRefPtr<CefFrame> frame,
                                  int httpStatusCode)
{
    if (load_handler_.get())
        load_handler_->OnLoadEnd(browser, frame, httpStatusCode);
}

bool ClientHandlerImpl::OnBeforeBrowse(CefRefPtr<CefBrowser> browser,
                                       CefRefPtr<CefFrame> frame,
                                       CefRefPtr<CefRequest> request,
                                       bool is_redirect)
{
    message_router_->OnBeforeBrowse(browser, frame);
    return false;
}

CefRefPtr<CefResourceHandler> ClientHandlerImpl::GetResourceHandler(
    CefRefPtr<CefBrowser> browser,
    CefRefPtr<CefFrame> frame,
    CefRefPtr<CefRequest> request)
{
    std::string url = request->GetURL();
    // @note This is the place to handle certain url specially

    return NULL;
}

bool ClientHandlerImpl::OnQuotaRequest(CefRefPtr<CefBrowser> browser,
                                       const CefString& origin_url,
                                       int64 new_size,
                                       CefRefPtr<CefQuotaCallback> callback)
{
    static const int64 max_size = 1024 * 1024 * 20;  // 20mb.

    // Grant the quota request if the size is reasonable.
    callback->Continue(new_size <= max_size);
    return true;
}

void ClientHandlerImpl::OnProtocolExecution(CefRefPtr<CefBrowser> browser,
                                            const CefString& url,
                                            bool& allow_os_execution)
{
    std::string urlStr = url;

    // Allow OS execution of Spotify URIs.
    if (urlStr.find("spotify:") == 0)
        allow_os_execution = true;
}

void ClientHandlerImpl::OnRenderProcessTerminated(CefRefPtr<CefBrowser> browser,
                                                  TerminationStatus status)
{
    message_router_->OnRenderProcessTerminated(browser);
    
    /// CEF3-Awesomium
    if (process_handler_.get())
        process_handler_->OnRenderProcessTerminated(browser, status);

    m_bIsCrashed = true;

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
bool ClientHandlerImpl::GetRootScreenRect(CefRefPtr<CefBrowser> browser,
                                          CefRect& rect)
{
    if (!m_OSRHandler.get())
        return false;
    return m_OSRHandler->GetRootScreenRect(browser, rect);
}

bool ClientHandlerImpl::GetViewRect(CefRefPtr<CefBrowser> browser, CefRect& rect)
{
    if (!m_OSRHandler.get())
        return false;
    return m_OSRHandler->GetViewRect(browser, rect);
}

/// *** BEGIN IMPORTANT *** ///
bool ClientHandlerImpl::GetScreenPoint(CefRefPtr<CefBrowser> browser,
                                   int viewX,
                                   int viewY,
                                   int& screenX,
                                   int& screenY)
{
    if (!m_OSRHandler.get())
        return false;
    return m_OSRHandler->GetScreenPoint(browser, viewX, viewY, screenX, screenY);
}

bool ClientHandlerImpl::GetScreenInfo(CefRefPtr<CefBrowser> browser,
                                  CefScreenInfo& screen_info)
{
    if (!m_OSRHandler.get())
        return false;
    return m_OSRHandler->GetScreenInfo(browser, screen_info);
}

void ClientHandlerImpl::OnPopupShow(CefRefPtr<CefBrowser> browser,
                                bool show)
{
    if (!m_OSRHandler.get())
        return;
    return m_OSRHandler->OnPopupShow(browser, show);
}

void ClientHandlerImpl::OnPopupSize(CefRefPtr<CefBrowser> browser,
                                const CefRect& rect)
{
    if (!m_OSRHandler.get())
        return;
    return m_OSRHandler->OnPopupSize(browser, rect);
}

void ClientHandlerImpl::OnPaint(CefRefPtr<CefBrowser> browser,
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

// void ClientHandlerImpl::OnCursorChange(CefRefPtr<CefBrowser> browser,
//                                    CefCursorHandle cursor)
// {
//     if (!m_OSRHandler.get())
//         return;
//     m_OSRHandler->OnCursorChange(browser, cursor);
// }
/// *** END IMPORTANT *** ///

bool ClientHandlerImpl::OnFileDialog(CefRefPtr<CefBrowser> browser,
                                     CefDialogHandler::FileDialogMode mode,
                                     const CefString& title,
                                     const CefString& default_file_name,
                                     const std::vector<CefString>& accept_types,
                                     CefRefPtr<CefFileDialogCallback> callback)
{
    REQUIRE_UI_THREAD();
    if (dialog_handler_.get())
        return dialog_handler_->OnFileDialog(browser, mode, title,
                default_file_name, accept_types, callback);
    return false;
}
bool ClientHandlerImpl::GetAuthCredentials(CefRefPtr<CefBrowser> browser,
                                           CefRefPtr<CefFrame> frame,
                                           bool isProxy,
                                           const CefString& host,
                                           int port,
                                           const CefString& realm,
                                           const CefString& scheme,
                                           CefRefPtr<CefAuthCallback> callback)
{
    REQUIRE_UI_THREAD();
    if (dialog_handler_.get())
        return dialog_handler_->GetAuthCredentials(browser, frame, isProxy,
                host, port, realm, scheme, callback);
    return false;
}
bool ClientHandlerImpl::OnCertificateError(cef_errorcode_t cert_error,
                                           const CefString& request_url,
                                           CefRefPtr<CefAllowCertificateErrorCallback> callback)
{
    REQUIRE_UI_THREAD();
    if (dialog_handler_.get())
        return dialog_handler_->OnCertificateError(cert_error, request_url,
                callback);
    return false;
}

void ClientHandlerImpl::SetMainHwnd(CefWindowHandle hwnd)
{
    AutoLock lock_scope(this);
    m_MainHwnd = hwnd;
}

void ClientHandlerImpl::CloseAllBrowsers(bool force_close)
{
    if (!CefCurrentlyOn(TID_UI)) {
        // Execute on the UI thread.
        CefPostTask(TID_UI,
                    NewCefRunnableMethod(this, &ClientHandlerImpl::CloseAllBrowsers,
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

void ClientHandlerImpl::SetLastDownloadFile(const std::string& fileName)
{
    AutoLock lock_scope(this);
    m_LastDownloadFile = fileName;
}

std::string ClientHandlerImpl::GetLastDownloadFile()
{
    AutoLock lock_scope(this);
    return m_LastDownloadFile;
}

void ClientHandlerImpl::ShowDevTools(CefRefPtr<CefBrowser> browser)
{
    CefWindowInfo windowInfo;
    CefBrowserSettings settings;

#ifdef OS_WIN
    windowInfo.SetAsPopup(browser->GetHost()->GetWindowHandle(), "DevTools");
#endif

    browser->GetHost()->ShowDevTools(windowInfo, this, settings);
}

void ClientHandlerImpl::CloseDevTools(CefRefPtr<CefBrowser> browser)
{
    browser->GetHost()->CloseDevTools();
}

void ClientHandlerImpl::BeginTracing()
{
    if (CefCurrentlyOn(TID_UI)) {
        CefBeginTracing(CefString());
    } else {
        CefPostTask(TID_UI,
                    NewCefRunnableMethod(this, &ClientHandlerImpl::BeginTracing));
    }
}

void ClientHandlerImpl::EndTracing()
{
    if (CefCurrentlyOn(TID_UI)) {
        class Callback : public CefEndTracingCallback,
                         public CefRunFileDialogCallback
        {
        public:
            explicit Callback(CefRefPtr<ClientHandlerImpl> handler)
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
            CefRefPtr<ClientHandlerImpl> handler_;

            IMPLEMENT_REFCOUNTING(Callback);
        };

        new Callback(this);
    } else {
        CefPostTask(TID_UI,
                    NewCefRunnableMethod(this, &ClientHandlerImpl::BeginTracing));
    }
}

bool ClientHandlerImpl::Save(const std::string& path, const std::string& data)
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
void ClientHandlerImpl::CreateMessageHandlers(MessageHandlerSet& handlers)
{
    /// @todo
}
