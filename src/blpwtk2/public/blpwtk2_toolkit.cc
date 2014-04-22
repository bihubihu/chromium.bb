/*
 * Copyright (C) 2013 Bloomberg Finance L.P.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS," WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

#include <blpwtk2_toolkit.h>

#include <third_party/WebKit/public/web/WebKit.h>

// dependencies needed for SubProcessMain()
#include <blpwtk2_contentmaindelegateimpl.h>
#include <blpwtk2_products.h>
#include <content/public/app/content_main.h>
#include <ui/gl/gl_implementation.h>


namespace blpwtk2 {

void Toolkit::allowNonWindowContexts_Unsafe()
{
    WebKit::allowNonWindowContexts_Unsafe();
}

Toolkit::~Toolkit()
{
}

}  // close namespace blpwtk2

// This is the entry point for blpwtk2_subprocess.exe
// Do not call this from anywhere else!!
extern "C" __declspec(dllexport)
int SubProcessMain(HINSTANCE hInstance,
                   sandbox::SandboxInterfaceInfo* sandboxInfo)
{
    gfx::SetBLPAngleDLLName(BLPANGLE_DLL_NAME);
    blpwtk2::ContentMainDelegateImpl delegate(true, false);
    return content::ContentMain(hInstance, sandboxInfo, &delegate);
}
