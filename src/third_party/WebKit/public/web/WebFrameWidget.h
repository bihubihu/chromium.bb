/*
 * Copyright (C) 2014 Google Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 *     * Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above
 * copyright notice, this list of conditions and the following disclaimer
 * in the documentation and/or other materials provided with the
 * distribution.
 *     * Neither the name of Google Inc. nor the names of its
 * contributors may be used to endorse or promote products derived from
 * this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef WebFrameWidget_h
#define WebFrameWidget_h

#include "../platform/WebCommon.h"
#include "../platform/WebPageVisibilityState.h"
#include "public/web/WebWidget.h"

namespace blink {

class WebLocalFrame;
class WebView;
class WebWidgetClient;

class WebFrameWidget : public WebWidget {
public:
    BLINK_EXPORT static WebFrameWidget* create(WebWidgetClient*, WebLocalFrame*);
    // Creates a frame widget for a WebView. Temporary helper to help transition
    // away from WebView inheriting WebWidget.
    // TODO(dcheng): Remove once transition is complete.
    BLINK_EXPORT static WebFrameWidget* create(WebWidgetClient*, WebView*, WebLocalFrame* mainFrame);

    // Sets the visibility of the WebFrameWidget.
    // We still track page-level visibility, but additionally we need to notify a WebFrameWidget
    // when its owning RenderWidget receives a Show or Hide directive, so that it knows whether
    // it needs to draw or not.
    virtual void setVisibilityState(WebPageVisibilityState visibilityState, bool isInitialState) { }

    // Temporary: there should only be one WebFrameWidget implementation but the
    // Blink API is currently in a transition state. See https://goo.gl/7yVrnb
    virtual bool forSubframe() const { return true; }
};

} // namespace blink

#endif
