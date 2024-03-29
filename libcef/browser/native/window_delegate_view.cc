// Copyright 2014 The Chromium Embedded Framework Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be found
// in the LICENSE file.

#include "libcef/browser/native/window_delegate_view.h"

#include <utility>

#include "content/public/browser/web_contents.h"
#include "ui/views/background.h"
#include "ui/views/controls/webview/webview.h"
#include "ui/views/layout/fill_layout.h"
#include "ui/views/widget/widget.h"

CefWindowDelegateView::CefWindowDelegateView(
    SkColor background_color,
    bool always_on_top,
    base::RepeatingClosure on_bounds_changed)
    : background_color_(background_color),
      web_view_(NULL),
      always_on_top_(always_on_top),
      on_bounds_changed_(on_bounds_changed) {}

void CefWindowDelegateView::Init(gfx::AcceleratedWidget parent_widget,
                                 content::WebContents* web_contents,
                                 const gfx::Rect& bounds) {
  DCHECK(!web_view_);
  web_view_ = new views::WebView(web_contents->GetBrowserContext());
  web_view_->SetWebContents(web_contents);
  web_view_->SetPreferredSize(bounds.size());

  views::Widget* widget = new views::Widget;

  // See CalculateWindowStylesFromInitParams in
  // ui/views/widget/widget_hwnd_utils.cc for the conversion of |params| to
  // Windows style flags.
  views::Widget::InitParams params;
  params.parent_widget = parent_widget;
  params.bounds = bounds;
  params.delegate = this;
  // Set the WS_CHILD flag.
  params.child = true;
  // Set the WS_VISIBLE flag.
  params.type = views::Widget::InitParams::TYPE_CONTROL;
  // Don't set the WS_EX_COMPOSITED flag.
  params.opacity = views::Widget::InitParams::OPAQUE_WINDOW;
  // Tell Aura not to draw the window frame on resize.
  params.remove_standard_frame = true;
  // Cause WidgetDelegate::CanActivate to return true. See comments in
  // CefBrowserHostImpl::PlatformSetFocus.
  params.activatable = views::Widget::InitParams::ACTIVATABLE_YES;

  params.keep_on_top = always_on_top_;

  // Results in a call to InitContent().
  widget->Init(params);

  // |widget| should now be associated with |this|.
  DCHECK_EQ(widget, GetWidget());
  // |widget| must be top-level for focus handling to work correctly.
  DCHECK(widget->is_top_level());
  // |widget| must be activatable for focus handling to work correctly.
  DCHECK(widget->widget_delegate()->CanActivate());
}

void CefWindowDelegateView::InitContent() {
  SetBackground(views::CreateSolidBackground(background_color_));
  SetLayoutManager(std::make_unique<views::FillLayout>());
  AddChildView(web_view_);
}

void CefWindowDelegateView::ViewHierarchyChanged(
    const views::ViewHierarchyChangedDetails& details) {
  if (details.is_add && details.child == this)
    InitContent();
}

void CefWindowDelegateView::OnBoundsChanged(const gfx::Rect& previous_bounds) {
  views::WidgetDelegateView::OnBoundsChanged(previous_bounds);
  if (!on_bounds_changed_.is_null())
    on_bounds_changed_.Run();
}
