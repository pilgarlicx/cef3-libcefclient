// Copyright (c) 2011 The Chromium Embedded Framework Authors. All rights
// reserved. Use of this source code is governed by a BSD-style license that
// can be found in the LICENSE file.

#ifndef CEF_TESTS_CEFCLIENT_CEFCLIENT_OSR_WIDGET_WIN_H_
#define CEF_TESTS_CEFCLIENT_CEFCLIENT_OSR_WIDGET_WIN_H_
#pragma once

#include <vector>

#include <include/cef_render_handler.h>

#include "client_handler_impl.h"


class OffScreenRenderHandler : public ClientHandlerImpl::RenderHandler
{
public:
    // Wrapper class for the underlying window of this offscreen browser, used
    // to provide window/client size, do view-to-screen position translation,
    // and change cursor style.
    class WindowWrapper {
    public:
        virtual bool IsShown() = 0;
        virtual void GetRootScreenRect(CefRect& rect) = 0;
        virtual void GetViewRect(CefRect& rect) = 0;
        virtual void GetScreenPoint(int viewX, int viewY, int& screenX,
                                    int& screenY) = 0;
        virtual void SetCursor(CefCursorHandle cursor) = 0;
    };

    // Wrapper class for the offscreen renderer, to update and/or render new
    // pixel buffer of the browser.
    class RendererWrapper {
    public:
        virtual void Render(const std::vector<CefRect>& dirty_rects,
                            const void* buffer, int width, int height) = 0;
    };

    // Create a new OffScreenRenderHandler instance.
    static CefRefPtr<OffScreenRenderHandler> Create(bool transparent=true);

    static CefRefPtr<OffScreenRenderHandler> From(
        CefRefPtr<ClientHandlerImpl::RenderHandler> render_handler);

    // ClientHandler::RenderHandler methods
    virtual void OnBeforeClose(CefRefPtr<CefBrowser> browser) OVERRIDE;

    // CefRenderHandler methods
    virtual bool GetRootScreenRect(CefRefPtr<CefBrowser> browser,
                                   CefRect& rect) OVERRIDE;
    virtual bool GetViewRect(CefRefPtr<CefBrowser> browser,
                             CefRect& rect) OVERRIDE;
    virtual bool GetScreenPoint(CefRefPtr<CefBrowser> browser,
                                int viewX,
                                int viewY,
                                int& screenX,
                                int& screenY) OVERRIDE;
    virtual void OnPopupShow(CefRefPtr<CefBrowser> browser,
                             bool show) OVERRIDE;
    virtual void OnPopupSize(CefRefPtr<CefBrowser> browser,
                             const CefRect& rect) OVERRIDE;
    virtual void OnPaint(CefRefPtr<CefBrowser> browser,
                         PaintElementType type,
                         const RectList& dirty_rects,
                         const void* buffer,
                         int width,
                         int height) OVERRIDE;
    virtual void OnCursorChange(CefRefPtr<CefBrowser> browser,
                                CefCursorHandle cursor) OVERRIDE;

    void Invalidate();

    void SetWindow(WindowWrapper* window) { window_ = window; }
    void SetRenderer(RendererWrapper* renderer) { renderer_ = renderer; }

private:
    OffScreenRenderHandler(bool transparent=true);
    virtual ~OffScreenRenderHandler();

    void Render();
    void OnDestroyed();
    bool IsOverPopupWidget(int x, int y) const;
    int GetPopupXOffset() const;
    int GetPopupYOffset() const;
    void ApplyPopupOffset(int& x, int& y) const;

    bool transparent_;
    bool popup_painting_;
    bool render_task_pending_;
    CefRect popup_rect_;

    WindowWrapper* window_;
    RendererWrapper* renderer_;

    IMPLEMENT_REFCOUNTING(OffScreenRenderHandler);
};

#endif  // CEF_TESTS_CEFCLIENT_CEFCLIENT_OSR_WIDGET_WIN_H_
