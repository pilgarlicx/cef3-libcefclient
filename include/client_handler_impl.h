// Copyright (c) 2011 The Chromium Embedded Framework Authors. All rights
// reserved. Use of this source code is governed by a BSD-style license that
// can be found in the LICENSE file.

#ifndef CEF_TESTS_CEFCLIENT_CLIENT_HANDLER_H_
#define CEF_TESTS_CEFCLIENT_CLIENT_HANDLER_H_
#pragma once

#include <list>
#include <map>
#include <set>
#include <string>

#include <include/wrapper/cef_message_router.h>

#include "client_handler.h"
#include "util.h"

// ClientHandler implementation.
class ClientHandlerImpl : public ClientHandler,
                          public CefContextMenuHandler,
                          public CefDisplayHandler,
                          public CefDownloadHandler,
                          public CefDragHandler,
                          public CefGeolocationHandler,
                          public CefKeyboardHandler,
                          public CefLifeSpanHandler,
                          public CefLoadHandler,
                          public CefRenderHandler,
                          public CefRequestHandler
{
public:
    // Interface implemented to handle off-screen rendering.
    class RenderHandler : public CefRenderHandler {
    public:
        virtual ~RenderHandler() {}
        // To inform browser close
        virtual void OnBeforeClose(CefRefPtr<CefBrowser> browser) =0;
    };

    typedef std::set<CefMessageRouterBrowserSide::Handler*> MessageHandlerSet;

    // Interface implemented to handle process message sent from renderer process
    class MessageDelegate: public virtual CefBase {
    public:
        virtual bool OnProcessMessageReceived(
            CefRefPtr<CefBrowser> browser,
            CefProcessId source_process,
            CefRefPtr<CefProcessMessage> message) {
            return false;
        };
    };
    typedef std::set<CefRefPtr<MessageDelegate> > MessageDelegateSet;

    ClientHandlerImpl();
    virtual ~ClientHandlerImpl();

    // CefClient methods
    virtual CefRefPtr<CefContextMenuHandler> GetContextMenuHandler() OVERRIDE {
        return this;
    }
    virtual CefRefPtr<CefDisplayHandler> GetDisplayHandler() OVERRIDE {
        return this;
    }
    virtual CefRefPtr<CefDownloadHandler> GetDownloadHandler() OVERRIDE {
        return this;
    }
    virtual CefRefPtr<CefDragHandler> GetDragHandler() OVERRIDE {
        return this;
    }
    virtual CefRefPtr<CefGeolocationHandler> GetGeolocationHandler() OVERRIDE {
        return this;
    }
    virtual CefRefPtr<CefKeyboardHandler> GetKeyboardHandler() OVERRIDE {
        return this;
    }
    virtual CefRefPtr<CefLifeSpanHandler> GetLifeSpanHandler() OVERRIDE {
        return this;
    }
    virtual CefRefPtr<CefLoadHandler> GetLoadHandler() OVERRIDE {
        return this;
    }
    virtual CefRefPtr<CefRenderHandler> GetRenderHandler() OVERRIDE {
        return this;
    }
    virtual CefRefPtr<CefRequestHandler> GetRequestHandler() OVERRIDE {
        return this;
    }
    virtual bool OnProcessMessageReceived(CefRefPtr<CefBrowser> browser,
                                          CefProcessId source_process,
                                          CefRefPtr<CefProcessMessage> message)
        OVERRIDE;

    // CefContextMenuHandler methods
    virtual void OnBeforeContextMenu(CefRefPtr<CefBrowser> browser,
                                     CefRefPtr<CefFrame> frame,
                                     CefRefPtr<CefContextMenuParams> params,
                                     CefRefPtr<CefMenuModel> model) OVERRIDE;
    virtual bool OnContextMenuCommand(CefRefPtr<CefBrowser> browser,
                                      CefRefPtr<CefFrame> frame,
                                      CefRefPtr<CefContextMenuParams> params,
                                      int command_id,
                                      EventFlags event_flags) OVERRIDE;

    // CefDisplayHandler methods
    virtual void OnAddressChange(CefRefPtr<CefBrowser> browser,
                                 CefRefPtr<CefFrame> frame,
                                 const CefString& url) OVERRIDE;
    virtual void OnTitleChange(CefRefPtr<CefBrowser> browser,
                               const CefString& title) OVERRIDE;
    virtual bool OnConsoleMessage(CefRefPtr<CefBrowser> browser,
                                  const CefString& message,
                                  const CefString& source,
                                  int line) OVERRIDE;

    // CefDownloadHandler methods
    virtual void OnBeforeDownload(
        CefRefPtr<CefBrowser> browser,
        CefRefPtr<CefDownloadItem> download_item,
        const CefString& suggested_name,
        CefRefPtr<CefBeforeDownloadCallback> callback) OVERRIDE;
    virtual void OnDownloadUpdated(
        CefRefPtr<CefBrowser> browser,
        CefRefPtr<CefDownloadItem> download_item,
        CefRefPtr<CefDownloadItemCallback> callback) OVERRIDE;

    // CefDragHandler methods
    virtual bool OnDragEnter(CefRefPtr<CefBrowser> browser,
                             CefRefPtr<CefDragData> dragData,
                             DragOperationsMask mask) OVERRIDE;

    // CefGeolocationHandler methods
    virtual void OnRequestGeolocationPermission(
        CefRefPtr<CefBrowser> browser,
        const CefString& requesting_url,
        int request_id,
        CefRefPtr<CefGeolocationCallback> callback) OVERRIDE;

    // CefKeyboardHandler methods
    virtual bool OnPreKeyEvent(CefRefPtr<CefBrowser> browser,
                               const CefKeyEvent& event,
                               CefEventHandle os_event,
                               bool* is_keyboard_shortcut) OVERRIDE;

    // CefLifeSpanHandler methods
    virtual bool OnBeforePopup(CefRefPtr<CefBrowser> browser,
                               CefRefPtr<CefFrame> frame,
                               const CefString& target_url,
                               const CefString& target_frame_name,
                               const CefPopupFeatures& popupFeatures,
                               CefWindowInfo& windowInfo,
                               CefRefPtr<CefClient>& client,
                               CefBrowserSettings& settings,
                               bool* no_javascript_access) OVERRIDE;
    virtual void OnAfterCreated(CefRefPtr<CefBrowser> browser) OVERRIDE;
    virtual bool DoClose(CefRefPtr<CefBrowser> browser) OVERRIDE;
    virtual void OnBeforeClose(CefRefPtr<CefBrowser> browser) OVERRIDE;

    // CefLoadHandler methods
    virtual void OnLoadingStateChange(CefRefPtr<CefBrowser> browser,
                                      bool isLoading,
                                      bool canGoBack,
                                      bool canGoForward) OVERRIDE;
    virtual void OnLoadError(CefRefPtr<CefBrowser> browser,
                             CefRefPtr<CefFrame> frame,
                             ErrorCode errorCode,
                             const CefString& errorText,
                             const CefString& failedUrl) OVERRIDE;

    // CefRequestHandler methods
    virtual bool OnBeforeBrowse(CefRefPtr<CefBrowser> browser,
                                CefRefPtr<CefFrame> frame,
                                CefRefPtr<CefRequest> request,
                                bool is_redirect) OVERRIDE;
    virtual CefRefPtr<CefResourceHandler> GetResourceHandler(
        CefRefPtr<CefBrowser> browser,
        CefRefPtr<CefFrame> frame,
        CefRefPtr<CefRequest> request) OVERRIDE;
    virtual bool OnQuotaRequest(CefRefPtr<CefBrowser> browser,
                                const CefString& origin_url,
                                int64 new_size,
                                CefRefPtr<CefQuotaCallback> callback) OVERRIDE;
    virtual void OnProtocolExecution(CefRefPtr<CefBrowser> browser,
                                     const CefString& url,
                                     bool& allow_os_execution) OVERRIDE;
    virtual void OnRenderProcessTerminated(CefRefPtr<CefBrowser> browser,
                                           TerminationStatus status) OVERRIDE;

    // @note [Important] CefRenderHandler methods
    virtual bool GetRootScreenRect(CefRefPtr<CefBrowser> browser,
                                   CefRect& rect) OVERRIDE;
    virtual bool GetViewRect(CefRefPtr<CefBrowser> browser,
                             CefRect& rect) OVERRIDE;
    virtual bool GetScreenPoint(CefRefPtr<CefBrowser> browser,
                                int viewX,
                                int viewY,
                                int& screenX,
                                int& screenY) OVERRIDE;
    virtual bool GetScreenInfo(CefRefPtr<CefBrowser> browser,
                               CefScreenInfo& screen_info) OVERRIDE;
    virtual void OnPopupShow(CefRefPtr<CefBrowser> browser, bool show) OVERRIDE;
    virtual void OnPopupSize(CefRefPtr<CefBrowser> browser,
                             const CefRect& rect) OVERRIDE;
    virtual void OnPaint(CefRefPtr<CefBrowser> browser,
                         PaintElementType type,
                         const RectList& dirtyRects,
                         const void* buffer,
                         int width,
                         int height) OVERRIDE;
    virtual void OnCursorChange(CefRefPtr<CefBrowser> browser,
                                CefCursorHandle cursor) OVERRIDE;

    void SetMainHwnd(CefWindowHandle hwnd);
    CefWindowHandle GetMainHwnd() { return m_MainHwnd; }
    void SetOSRHandler(CefRefPtr<RenderHandler> handler) {
        m_OSRHandler = handler;
    }
    CefRefPtr<RenderHandler> GetOSRHandler() { return m_OSRHandler; }

    CefRefPtr<CefBrowser> GetBrowser() { return m_Browser; }
    int GetBrowserId() { return m_BrowserId; }

    // Request that all existing browser windows close.
    void CloseAllBrowsers(bool force_close);

    // Returns true if the main browser window is currently closing. Used in
    // combination with DoClose() and the OS close notification to properly handle
    // 'onbeforeunload' JavaScript events during window close.
    bool IsClosing() { return m_bIsClosing; }

    void SetLastDownloadFile(const std::string& fileName);
    std::string GetLastDownloadFile();

    // Send a notification to the application. Notifications should not block the
    // caller.
    enum NotificationType {
        NOTIFY_CONSOLE_MESSAGE,
        NOTIFY_DOWNLOAD_COMPLETE,
        NOTIFY_DOWNLOAD_ERROR,
    };
    void SendNotification(NotificationType type);

    void ShowDevTools(CefRefPtr<CefBrowser> browser);
    void CloseDevTools(CefRefPtr<CefBrowser> browser);

    // Returns the startup URL.
    std::string GetStartupURL() { return m_StartupURL; }

    void BeginTracing();
    void EndTracing();

    bool Save(const std::string& path, const std::string& data);

    template<typename MDT> // Message Delegate Type
    void CreateMessageDelegate() {
        message_delegates_.insert(new MDT);
    }

protected:
    void SetLoading(bool isLoading);
    void SetNavState(bool canGoBack, bool canGoForward);

    // Create all CefMessageRouterBrowserSide::Handler objects. They will be
    // deleted when the ClientHandler is destroyed.
    static void CreateMessageHandlers(MessageHandlerSet& handlers);

    // Create all message delegates
    //static void CreateMessageDelegates(MessageDelegateSet& delegates);

    // Returns the full download path for the specified file, or an empty path to
    // use the default temp directory.
    std::string GetDownloadPath(const std::string& file_name);

    // The child browser window
    CefRefPtr<CefBrowser> m_Browser;

    // List of any popup browser windows. Only accessed on the CEF UI thread.
    typedef std::list<CefRefPtr<CefBrowser> > BrowserList;
    BrowserList m_PopupBrowsers;

    // The main frame window handle
    CefWindowHandle m_MainHwnd;

    // The child browser id
    int m_BrowserId;

    // True if the main browser window is currently closing.
    bool m_bIsClosing;

    CefRefPtr<RenderHandler> m_OSRHandler;

    // Support for downloading files.
    std::string m_LastDownloadFile;

    // True if an editable field currently has focus.
    bool m_bFocusOnEditableField;

    // True if the DevTools window shown
    bool m_bDevToolsShown;

    // The startup URL.
    std::string m_StartupURL;

    // Handles the browser side of query routing. The renderer side is handled
    // in client_renderer.cpp.
    CefRefPtr<CefMessageRouterBrowserSide> message_router_;

    // Set of Handlers registered with the message router.
    MessageHandlerSet message_handlers_;

    // Set of delegates(handlers) to process messages sent from renderer
    // process
    MessageDelegateSet message_delegates_;

    // Number of currently existing browser windows. The application will exit
    // when the number of windows reaches 0.
    static int m_BrowserCount;

    // Include the default reference counting implementation.
    IMPLEMENT_REFCOUNTING(ClientHandlerImpl);
    // Include the default locking implementation.
    IMPLEMENT_LOCKING(ClientHandlerImpl);
};

#endif  // CEF_TESTS_CEFCLIENT_CLIENT_HANDLER_H_
