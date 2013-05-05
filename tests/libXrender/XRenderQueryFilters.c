/*
 * Copyright (c) 2013, Oracle and/or its affiliates. All rights reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */
#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

/* X_RenderQueryFilters was added in XRender 0.6 */
#define MY_XRENDER_MAJOR_VERSION 0
#define MY_XRENDER_MINOR_VERSION 6

#include "xhiv.h"
#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include "xhiv-Xrender.h"
#include <assert.h>
#include <stdio.h>
#include <limits.h>

static void
testOverflowFields(void)
{
    const xRenderQueryFiltersReply rep1 = {
        .type           = X_Reply,
        .length         = 256,
        .numAliases     = 256,
        .numFilters     = (UINT_MAX / sizeof (char *)) - 16
    };
    const char filter_data[16] = { 0 };
    xhiv_response filter_response = {
        .length = rep1.length,
        .repeat = ((rep1.length << 2) / sizeof(filter_data)) - 1,
        .response_data = filter_data,
        .response_datalen = sizeof(filter_data)
    };
    xhiv_response response1 = {
        .next = &xrender_formats_response,
        .chain = &filter_response,
        .reqType = MY_XRENDER_EXT_CODE,
        .reqMinor = X_RenderQueryFilters,
        .sequence = XHIV_SEQ_IGNORE,
        .length = (sizeof(rep1) >> 2),
        .response_data = &rep1,
        .response_datalen = sizeof(rep1)
    };
    Display *dpy = XhivOpenDisplay(&response1);
    int major = MY_XRENDER_MAJOR_VERSION;
    int minor = MY_XRENDER_MINOR_VERSION;
    int status;
    XFilters *filters;
    
    status = XRenderQueryVersion(dpy, &major, &minor);
    assert(status != 0);

    printf("XRenderQueryFilters: overflow number of filters test: 0x%lx\n",
           (unsigned long) rep1.numFilters);
    filters = XRenderQueryFilters(dpy, DefaultRootWindow(dpy));
    XFree(filters);
    assert(filters == NULL);

    XhivCloseDisplay(dpy);
}

int
main(int argc, char **argv)
{
    testOverflowFields();
    printf("XRenderQueryFilters: all tests passed\n");
    return 0;
}
