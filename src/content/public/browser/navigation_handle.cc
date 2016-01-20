// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "content/public/browser/navigation_handle.h"

#include "content/browser/frame_host/navigation_handle_impl.h"
#include "content/browser/frame_host/navigator.h"
#include "content/browser/web_contents/web_contents_impl.h"

namespace content {

WebContents* NavigationHandle::GetWebContents() {
  // The NavigationHandleImpl cannot access the WebContentsImpl as it would be
  // a layering violation, hence the cast here.
  return static_cast<WebContentsImpl*>(
      static_cast<NavigationHandleImpl*>(this)->delegate());
}

// static
scoped_ptr<NavigationHandle> NavigationHandle::CreateNavigationHandleForTesting(
    const GURL& url,
    bool is_main_frame,
    WebContents* web_contents) {
  scoped_ptr<NavigationHandleImpl> handle_impl = NavigationHandleImpl::Create(
      url, is_main_frame, static_cast<WebContentsImpl*>(web_contents));
  return scoped_ptr<NavigationHandle>(handle_impl.Pass());
}

}  // namespace content
