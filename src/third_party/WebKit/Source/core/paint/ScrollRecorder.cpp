// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "config.h"
#include "core/paint/ScrollRecorder.h"

#include "platform/graphics/GraphicsContext.h"
#include "platform/graphics/paint/DisplayItemList.h"
#include "platform/graphics/paint/ScrollDisplayItem.h"

namespace blink {

ScrollRecorder::ScrollRecorder(GraphicsContext& context, const DisplayItemClientWrapper& client, PaintPhase phase, const IntSize& currentOffset)
    : m_client(client)
    , m_beginItemType(DisplayItem::paintPhaseToScrollType(phase))
    , m_context(context)
{
    ASSERT(m_context.displayItemList());
    if (m_context.displayItemList()->displayItemConstructionIsDisabled())
        return;
    m_context.displayItemList()->createAndAppend<BeginScrollDisplayItem>(m_client, m_beginItemType, currentOffset);
}

ScrollRecorder::~ScrollRecorder()
{
    ASSERT(m_context.displayItemList());
    if (!m_context.displayItemList()->displayItemConstructionIsDisabled()) {
        if (m_context.displayItemList()->lastDisplayItemIsNoopBegin())
            m_context.displayItemList()->removeLastDisplayItem();
        else
            m_context.displayItemList()->createAndAppend<EndScrollDisplayItem>(m_client, DisplayItem::scrollTypeToEndScrollType(m_beginItemType));
    }
}

} // namespace blink
