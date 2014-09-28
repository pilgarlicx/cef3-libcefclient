/**
 * @file client_handler.h
 *
 * @breif
 */
#ifndef CLIENT_HANDLER_H_
#define CLIENT_HANDLER_H_
#pragma once

#include <include/cef_base.h>
#include <include/cef_load_handler.h>
#include <include/cef_request_handler.h>
#include <include/cef_context_menu_handler.h>
#include <include/cef_dialog_handler.h>
#include <include/cef_client.h>

class CefBrowser;
class CefFrame;
class CefDOMNode;
class CefV8Context;
class CefDownloadItem;
class CefBeforeDownloadCallback;
class CefDownloadItemCallback;

class ClientHandler;
class ClientViewHandler;
class ClientLoadHandler;
class ClientProcessHandler;
class ClientMenuHandler;
class ClientDialogHandler;
class ClientPrintHandler;
class ClientDownloadHandler;
class ClientInputMethodEditorHandler;

/// Exposed interfaces to retreive the actual object defined outside of this lib
extern CefRefPtr<ClientHandler> GetClientHandler();

class ClientHandler : public CefClient {
public:
    typedef ClientViewHandler View;
    typedef ClientLoadHandler Load;
    typedef ClientProcessHandler Process;
    typedef ClientMenuHandler Menu;
    typedef ClientDialogHandler Dialog;
    typedef ClientPrintHandler Print;
    typedef ClientDownloadHandler Download;
    typedef ClientInputMethodEditorHandler InputMethodEditor;

    ClientHandler()
        : view_handler_(NULL), load_handler_(NULL), process_handler_(NULL),
          menu_handler_(NULL), dialog_handler_(NULL), print_handler_(NULL),
          download_handler_(NULL), input_method_editor_handler_(NULL) {}
    virtual ~ClientHandler() {}

    void set_view_handler(CefRefPtr<View> handler) { view_handler_ = handler; }
    void set_load_handler(CefRefPtr<Load> handler) { load_handler_ = handler; }
    void set_process_handler(CefRefPtr<Process> handler) {
        process_handler_ = handler;
    }
    void set_menu_handler(CefRefPtr<Menu> handler) { menu_handler_ = handler; }
    void set_dialog_handler(CefRefPtr<Dialog> handler) {
        dialog_handler_ = handler;
    }
    void set_print_handler(CefRefPtr<Print> handler) {
        print_handler_ = handler;
    }
    void set_download_handler(CefRefPtr<Download> handler) {
        download_handler_ = handler;
    }
    void set_input_method_editor_handler(CefRefPtr<InputMethodEditor> handler) {
        input_method_editor_handler_ = handler;
    }
    CefRefPtr<View> view_handler() const { return view_handler_; }
    CefRefPtr<Load> load_handler() const { return load_handler_; }
    CefRefPtr<Process> process_handler() const { return process_handler_; }
    CefRefPtr<Menu> menu_handler() const { return menu_handler_; }
    CefRefPtr<Dialog> dialog_handler() const { return dialog_handler_; }
    CefRefPtr<Print> print_handler() const { return print_handler_; }
    CefRefPtr<Download> download_handler() const { return download_handler_; }
    CefRefPtr<InputMethodEditor> input_method_editor_handler() const {
        return input_method_editor_handler_;
    }
protected:
    CefRefPtr<View> view_handler_;
    CefRefPtr<Load> load_handler_;
    CefRefPtr<Process> process_handler_;
    CefRefPtr<Menu> menu_handler_;
    CefRefPtr<Dialog> dialog_handler_;
    CefRefPtr<Print> print_handler_;
    CefRefPtr<Download> download_handler_;
    CefRefPtr<InputMethodEditor> input_method_editor_handler_;
};

class ClientViewHandler : public virtual CefBase {
public:
    /// CefDisplayHandler [CefClient]
    /// @note default to be of 'CefClient'
    virtual void OnAddressChange(CefRefPtr<CefBrowser> browser,
                                 CefRefPtr<CefFrame> frame,
                                 const CefString& url) = 0;
    virtual void OnTitleChange(CefRefPtr<CefBrowser> browser,
                               const CefString& title) = 0;
    virtual bool OnConsoleMessage(CefRefPtr<CefBrowser> browser,
                                  const CefString& message,
                                  const CefString& source,
                                  int line) = 0;
    /// CefRenderHandler
    virtual void OnCursorChange(CefRefPtr<CefBrowser> browser,
                                CefCursorHandle cursor) = 0;
    /// CefRenderProcessHandler [CefApp]
    virtual void OnFocusedNodeChanged(CefRefPtr<CefBrowser> browser,
                                      CefRefPtr<CefFrame> frame,
                                      CefRefPtr<CefDOMNode> node) = 0;
    // TODO - the following interfaces of WebViewListener::View are not
    // implemented yet:
    // 'OnChangeTooltip', 'OnChangeTargetUrl', 'OnChangeFocus',
    // 'OnShowCreatedWebView'
};

class ClientLoadHandler : public virtual CefBase {
public:
    /// CefLoadHandler [CefApp][CefRenderProcessHandler]
    virtual void OnLoadStart(CefRefPtr<CefBrowser> browser,
                             CefRefPtr<CefFrame> frame) = 0;
    virtual void OnLoadError(CefRefPtr<CefBrowser> browser,
                             CefRefPtr<CefFrame> frame,
                             CefLoadHandler::ErrorCode errorCode,
                             const CefString& errorText,
                             const CefString& failedUrl) = 0;
    virtual void OnLoadEnd(CefRefPtr<CefBrowser> browser,
                           CefRefPtr<CefFrame> frame,
                           int httpStatusCode) = 0;
    /// CefRenderProcessHandler [CefApp]
    virtual void OnContextCreated(CefRefPtr<CefBrowser> browser,
                                  CefRefPtr<CefFrame> frame,
                                  CefRefPtr<CefV8Context> context) = 0;
};

class ClientProcessHandler : public virtual CefBase {
public:
    /// CefRequestHandler
    virtual void OnRenderProcessTerminated(CefRefPtr<CefBrowser> browser,
                                           CefRequestHandler::TerminationStatus status) = 0;
    // TODO - the following interfaces of WebViewListener::Process are not
    // implemented yet:
    // 'OnUnresponsive', 'OnResponsive'
};

class ClientMenuHandler : public virtual CefBase {
public:
    /// CefContextMenuHandler
    virtual void OnBeforeContextMenu(CefRefPtr<CefBrowser> browser,
                                     CefRefPtr<CefFrame> frame,
                                     CefRefPtr<CefContextMenuParams> params,
                                     CefRefPtr<CefMenuModel> model) = 0;
    virtual void OnContextMenuCommand(CefRefPtr<CefBrowser> browser,
                                      CefRefPtr<CefFrame> frame,
                                      CefRefPtr<CefContextMenuParams> params,
                                      int command_id,
                                      CefContextMenuHandler::EventFlags event_flags) = 0;
    // TODO - the following interfaces of WebViewListener::Menu are not
    // implemented yet:
    // 'OnShowPopupMenu'
};

class ClientDialogHandler : public CefBase {
public:
    /// CefDialogHandler
    virtual bool OnFileDialog(CefRefPtr<CefBrowser> browser,
                              CefDialogHandler::FileDialogMode mode,
                              const CefString& title,
                              const CefString& default_file_name,
                              const std::vector<CefString>& accept_types,
                              CefRefPtr<CefFileDialogCallback> callback) = 0;
    /// CefRequestHandler
    virtual bool GetAuthCredentials(CefRefPtr<CefBrowser> browser,
                                    CefRefPtr<CefFrame> frame,
                                    bool isProxy,
                                    const CefString& host,
                                    int port,
                                    const CefString& realm,
                                    const CefString& scheme,
                                    CefRefPtr<CefAuthCallback> callback) = 0;
    virtual bool OnCertificateError(cef_errorcode_t cert_error,
                                    const CefString& request_url,
                                    CefRefPtr<CefAllowCertificateErrorCallback> callback) = 0;
    // TODO - the following interfaces of WebViewListener::Dialog are not
    // implemented yet:
    // 'OnShowPageInfoDialog'
};

class ClientPrintHandler : public virtual CefBase {
public:
    // TODO - the following interfaces WebViewListener::Print are not
    // implemented yet:
    // 'OnRequestPrint', 'OnFailPrint', 'OnFinishPrint'
};

class ClientDownloadHandler : public virtual CefBase {
public:
    /// CefDownloadHandler
    virtual void OnBeforeDownload(CefRefPtr<CefBrowser> browser,
                                  CefRefPtr<CefDownloadItem> download_item,
                                  const CefString& suggested_name,
                                  CefRefPtr<CefBeforeDownloadCallback> callback);
    virtual void OnDownloadUpdated(CefRefPtr<CefBrowser> browser,
                                   CefRefPtr<CefDownloadItem> download_item,
                                   CefRefPtr<CefDownloadItemCallback> callback);
    // NOTE - 'OnFinisheDownload' can be implemented in 'OnDownloadUpdated'
};

class ClientInputMethodEditorHandler : public virtual CefBase {
public:
    // TODO - the following interfaces WebViewListener::InputMethodEditor are
    // not implemented yet:
    // 'OnUpdateIME', 'OnCancelIME', 'OnChangeIMERange'
};

#endif // CLIENT_HANDLER_H_
