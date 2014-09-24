/**
 * @file client_handler_intf.h
 *
 * @breif
 */
#ifndef CLIENT_HANDLER_INTF_H_
#define CLIENT_HANDLER_INTF_H_
#pragma once

#include <include/cef_base.h>
#include <include/cef_load_handler.h>
#include <include/cef_request_handler.h>
#include <include/cef_menu_context_handler.h>
#include <include/cef_dialog_handler.h>

class CefBrowser;
class CefFrame;
class CefDOMNode;
class CefV8Context;
class CefDownloadItem;
class CefBeforeDownloadCallback;
class CefDownloadItemCallback;

class ClientViewHandler : public virtual CefBase {
public:
    /// CefDisplayHandler [CefClient]
    /// @note default to be of 'CefClient'
    virtual void OnAddressChange(CefRefPtr<CefBrowser> browser,
                                 CefRefPtr<CefFrame> frame,
                                 const CefString& url) = 0;
    virtual void OnTitleChange(CefRefPtr<CefBrowser> browser,
                               const CefString& title) = 0;
    virtual void OnConsoleMessage(CefRefPtr<CefBrowser> browser,
                                  const CefString& message,
                                  const CefString& source,
                                  int line) = 0;
    /// CefRenderHandler
    virtual void OnCursorChange(CefRefPtr<CefBrowser> browser,
                                CefCursorHandler cursor) = 0;
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
    virtual void OnFileDialog(CefRefPtr<CefBrowser> browser,
                              CefDialogHandler::FileDialogMode mode,
                              const CefString& title,
                              const CefString& default_file_name,
                              const std::vector<CefString>& accept_types,
                              CefRefPtr<CefFileDialogCallback> callback) = 0;
    /// CefRequestHandler
    virtual bool GetAuthCrendentials(CefRefPtr<CefBrowser> browser,
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

#endif // CLIENT_HANDLER_INTF_H_
