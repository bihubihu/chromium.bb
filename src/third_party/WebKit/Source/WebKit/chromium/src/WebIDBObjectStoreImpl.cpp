/*
 * Copyright (C) 2011 Google Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1.  Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 * 2.  Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE AND ITS CONTRIBUTORS "AS IS" AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL APPLE OR ITS CONTRIBUTORS BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "config.h"
#include "WebIDBObjectStoreImpl.h"

#if ENABLE(INDEXED_DATABASE)

#include "DOMStringList.h"
#include "IDBCallbacksProxy.h"
#include "IDBIndexBackendInterface.h"
#include "IDBKeyPath.h"
#include "IDBKeyRange.h"
#include "IDBObjectStoreBackendInterface.h"
#include "WebIDBIndexImpl.h"
#include "WebIDBKey.h"
#include "WebIDBKeyRange.h"
#include "WebIDBTransaction.h"
#include "platform/WebSerializedScriptValue.h"

using namespace WebCore;

namespace WebKit {

WebIDBObjectStoreImpl::WebIDBObjectStoreImpl(PassRefPtr<IDBObjectStoreBackendInterface> objectStore)
    : m_objectStore(objectStore)
{
}

WebIDBObjectStoreImpl::~WebIDBObjectStoreImpl()
{
}

void WebIDBObjectStoreImpl::get(const WebIDBKeyRange& keyRange, WebIDBCallbacks* callbacks, const WebIDBTransaction& transaction, WebExceptionCode& ec)
{
    m_objectStore->get(keyRange, IDBCallbacksProxy::create(adoptPtr(callbacks)), transaction.getIDBTransactionBackendInterface(), ec);
}

void WebIDBObjectStoreImpl::put(const WebSerializedScriptValue& value, const WebIDBKey& key, PutMode putMode, WebIDBCallbacks* callbacks, const WebIDBTransaction& transaction, const WebVector<long long>& webIndexIds, const WebVector<WebIndexKeys>& webIndexKeys)
{
    ASSERT(webIndexIds.size() == webIndexKeys.size());
    Vector<int64_t> indexIds(webIndexIds.size());
    Vector<IDBObjectStoreBackendInterface::IndexKeys> indexKeys(webIndexKeys.size());

    for (size_t i = 0; i < webIndexIds.size(); ++i) {
        indexIds[i] = webIndexIds[i];
        Vector<RefPtr<IDBKey> > indexKeyList(webIndexKeys[i].size());
        for (size_t j = 0; j < webIndexKeys[i].size(); ++j)
            indexKeyList[j] = webIndexKeys[i][j];
        indexKeys[i] = indexKeyList;
    }

    m_objectStore->put(value, key, static_cast<IDBObjectStoreBackendInterface::PutMode>(putMode), IDBCallbacksProxy::create(adoptPtr(callbacks)), transaction.getIDBTransactionBackendInterface(), indexIds, indexKeys);
}

void WebIDBObjectStoreImpl::setIndexKeys(const WebIDBKey& primaryKey, const WebVector<long long>& webIndexIds, const WebVector<WebIndexKeys>& webIndexKeys, const WebIDBTransaction& transaction)
{
    ASSERT(webIndexIds.size() == webIndexKeys.size());
    Vector<int64_t> indexIds(webIndexIds.size());
    Vector<IDBObjectStoreBackendInterface::IndexKeys> indexKeys(webIndexKeys.size());

    for (size_t i = 0; i < webIndexIds.size(); ++i) {
        indexIds[i] = webIndexIds[i];
        Vector<RefPtr<IDBKey> > indexKeyList(webIndexKeys[i].size());
        for (size_t j = 0; j < webIndexKeys[i].size(); ++j)
            indexKeyList[j] = webIndexKeys[i][j];
        indexKeys[i] = indexKeyList;
    }
    m_objectStore->setIndexKeys(primaryKey, indexIds, indexKeys, transaction.getIDBTransactionBackendInterface());
}

void WebIDBObjectStoreImpl::setIndexesReady(const WebVector<long long>& webIndexNames, const WebIDBTransaction& transaction)
{
    Vector<int64_t> indexIds(webIndexNames.size());
    for (size_t i = 0; i < webIndexNames.size(); ++i)
        indexIds[i] = webIndexNames[i];
    m_objectStore->setIndexesReady(indexIds, transaction.getIDBTransactionBackendInterface());
}

void WebIDBObjectStoreImpl::deleteFunction(const WebIDBKeyRange& keyRange, WebIDBCallbacks* callbacks, const WebIDBTransaction& transaction, WebExceptionCode& ec)
{
    m_objectStore->deleteFunction(keyRange, IDBCallbacksProxy::create(adoptPtr(callbacks)), transaction.getIDBTransactionBackendInterface(), ec);
}

void WebIDBObjectStoreImpl::clear(WebIDBCallbacks* callbacks, const WebIDBTransaction& transaction, WebExceptionCode& ec)
{
    m_objectStore->clear(IDBCallbacksProxy::create(adoptPtr(callbacks)), transaction.getIDBTransactionBackendInterface(), ec);
}

WebIDBIndex* WebIDBObjectStoreImpl::createIndex(long long id, const WebString& name, const WebIDBKeyPath& keyPath, bool unique, bool multiEntry, const WebIDBTransaction& transaction, WebExceptionCode& ec)
{
    RefPtr<IDBIndexBackendInterface> index = m_objectStore->createIndex(id, name, keyPath, unique, multiEntry, transaction.getIDBTransactionBackendInterface(), ec);
    if (!index)
        return 0;
    return new WebIDBIndexImpl(index);
}

WebIDBIndex* WebIDBObjectStoreImpl::index(long long objectStoreId)
{
    RefPtr<IDBIndexBackendInterface> index = m_objectStore->index(objectStoreId);
    if (!index)
        return 0;
    return new WebIDBIndexImpl(index);
}

void WebIDBObjectStoreImpl::deleteIndex(long long objectStoreId, const WebIDBTransaction& transaction, WebExceptionCode& ec)
{
    m_objectStore->deleteIndex(objectStoreId, transaction.getIDBTransactionBackendInterface(), ec);
}

void WebIDBObjectStoreImpl::openCursor(const WebIDBKeyRange& keyRange, WebIDBCursor::Direction direction, WebIDBCallbacks* callbacks, WebIDBTransaction::TaskType taskType, const WebIDBTransaction& transaction, WebExceptionCode& ec)
{
    m_objectStore->openCursor(keyRange, static_cast<IDBCursor::Direction>(direction), IDBCallbacksProxy::create(adoptPtr(callbacks)), static_cast<IDBTransactionBackendInterface::TaskType>(taskType), transaction.getIDBTransactionBackendInterface(), ec);
}

void WebIDBObjectStoreImpl::count(const WebIDBKeyRange& keyRange, WebIDBCallbacks* callbacks, const WebIDBTransaction& transaction, WebExceptionCode& ec)
{
    m_objectStore->count(keyRange, IDBCallbacksProxy::create(adoptPtr(callbacks)), transaction.getIDBTransactionBackendInterface(), ec);
}

} // namespace WebKit

#endif // ENABLE(INDEXED_DATABASE)
