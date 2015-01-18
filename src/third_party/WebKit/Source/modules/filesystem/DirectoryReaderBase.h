/*
 * Copyright (C) 2010 Google Inc. All rights reserved.
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

#ifndef DirectoryReaderBase_h
#define DirectoryReaderBase_h

#include "modules/filesystem/DOMFileSystemBase.h"
#include "platform/heap/Handle.h"
#include "wtf/text/WTFString.h"

namespace WebCore {

class DirectoryReaderBase : public GarbageCollectedFinalized<DirectoryReaderBase> {
public:
    DOMFileSystemBase* filesystem() const { return m_fileSystem.get(); }
    void setHasMoreEntries(bool hasMoreEntries) { m_hasMoreEntries = hasMoreEntries; }

    virtual ~DirectoryReaderBase() { }

    virtual void trace(Visitor* visitor)
    {
        visitor->trace(m_fileSystem);
    }

protected:
    DirectoryReaderBase(DOMFileSystemBase* fileSystem, const String& fullPath)
        : m_fileSystem(fileSystem)
        , m_fullPath(fullPath)
        , m_hasMoreEntries(true)
    {
    }

    Member<DOMFileSystemBase> m_fileSystem;

    // This is a virtual path.
    String m_fullPath;

    bool m_hasMoreEntries;
};

} // namespace WebCore

#endif // DirectoryReaderBase_h
