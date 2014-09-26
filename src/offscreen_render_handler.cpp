// Copyright (c) 2011 The Chromium Embedded Framework Authors. All rights
// reserved. Use of this source code is governed by a BSD-style license that
// can be found in the LICENSE file.

#include "offscreen_render_handler.h"

#include <windowsx.h>
#include <iostream>

#include <include/cef_runnable.h>

#include "util.h"

// static
CefRefPtr<OffScreenRenderHandler> OffScreenRenderHandler::Create(
    bool transparent)
{
    return new OffScreenRenderHandler(transparent);
}

// static
CefRefPtr<OffScreenRenderHandler> OffScreenRenderHandler::From(
    CefRefPtr<ClientHandlerImpl::RenderHandler> renderHandler)
{
    return static_cast<OffScreenRenderHandler*>(renderHandler.get());
}

void OffScreenRenderHandler::OnBeforeClose(CefRefPtr<CefBrowser> browser)
{
    // @todo
}

bool OffScreenRenderHandler::GetRootScreenRect(CefRefPtr<CefBrowser> browser,
                                               CefRect& rect)
{
    if (window_ && window_->IsShown()) {
        window_->GetRootScreenRect(rect);
        return true;
    }
    return false;
}

bool OffScreenRenderHandler::GetViewRect(CefRefPtr<CefBrowser> browser,
                                         CefRect& rect)
{
    if (window_ && window_->IsShown()) {
        window_->GetViewRect(rect);
        return true;
    }
    return false;
}

bool OffScreenRenderHandler::GetScreenPoint(CefRefPtr<CefBrowser> browser,
                                            int viewX,
                                            int viewY,
                                            int& screenX,
                                            int& screenY)
{
    if (window_ && window_->IsShown()) {
        window_->GetScreenPoint(viewX, viewY, screenX, screenY);
        return true;
    }
    return false;
}

void OffScreenRenderHandler::OnPopupShow(CefRefPtr<CefBrowser> browser,
                                         bool show)
{
    if (!show) {
        CefRect dirty_rect = popup_rect_;
        popup_rect_.Set(0, 0, 0, 0);
        browser->GetHost()->Invalidate(dirty_rect, PET_VIEW);
    }
}

void OffScreenRenderHandler::OnPopupSize(CefRefPtr<CefBrowser> browser,
                                         const CefRect& rect)
{
    if (rect.width <= 0 || rect.height <= 0)
        return;
    popup_rect_ = rect;
}

void OffScreenRenderHandler::OnPaint(CefRefPtr<CefBrowser> browser,
                                     PaintElementType type,
                                     const RectList& dirty_rects,
                                     const void* buffer,
                                     int width, int height)
{
    if (popup_painting_)
        return;
    renderer_->Render(dirty_rects, buffer, width, height);
    if (type == PET_VIEW && !popup_rect_.IsEmpty()) {
        popup_painting_ = true;
        CefRect client_popup_rect(0, 0, popup_rect_.width, popup_rect_.height);
        browser->GetHost()->Invalidate(client_popup_rect, PET_POPUP);
        popup_painting_ = false;
    }
}

void OffScreenRenderHandler::OnCursorChange(CefRefPtr<CefBrowser> browser,
                                            CefCursorHandle cursor)
{
    if (window_)
        window_->SetCursor(cursor);
}

OffScreenRenderHandler::OffScreenRenderHandler(bool transparent)
    : transparent_(transparent),
      popup_painting_(false),
      render_task_pending_(false)
{
}

OffScreenRenderHandler::~OffScreenRenderHandler()
{
    // @todo
}

/// METHODS NOT USED
void OffScreenRenderHandler::Invalidate() {
    if (!CefCurrentlyOn(TID_UI)) {
        CefPostTask(TID_UI,
                    NewCefRunnableMethod(this, &OffScreenRenderHandler::Invalidate));
        return;
    }

    // Don't post another task if the previous task is still pending.
    if (render_task_pending_)
        return;

    render_task_pending_ = true;

    // @note This may have impact on the render loop
    // Render at 30fps.
    static const int kRenderDelay = 1000 / 30;
    CefPostDelayedTask(TID_UI,
                       NewCefRunnableMethod(this, &OffScreenRenderHandler::Render),
                       kRenderDelay);
}

void OffScreenRenderHandler::Render()
{
    ASSERT(CefCurrentlyOn(TID_UI));
    if (render_task_pending_)
        render_task_pending_ = false;
}

void OffScreenRenderHandler::OnDestroyed()
{
    Release();
}
