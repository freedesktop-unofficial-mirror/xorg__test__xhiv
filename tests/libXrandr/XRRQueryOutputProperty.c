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

#include "xhiv.h"
#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include "xhiv-Xrandr.h"
#include <assert.h>
#include <stdio.h>
#include <limits.h>

static void
testOverflowFields(void)
{
    const xRRQueryOutputPropertyReply rep1 = {
        .type           = X_Reply,
        .length         = INT_MAX / sizeof(long),
    };
    xhiv_response response1 = {
        .next = &xrandr_vers_response,
        .reqType = MY_XRANDR_EXT_CODE,
        .reqMinor = X_RRQueryOutputProperty,
        .sequence = XHIV_SEQ_IGNORE,
        .length = (sizeof(rep1) >> 2) + rep1.length,
        .response_data = &rep1,
        .response_datalen = sizeof(rep1)
    };
    Display *dpy = XhivOpenDisplay(&response1);
    int major = MY_XRANDR_MAJOR_VERSION;
    int minor = MY_XRANDR_MINOR_VERSION;
    int status;
    XRRPropertyInfo *prop_info;
    
    status = XRRQueryVersion(dpy, &major, &minor);
    assert(status != 0);

    printf("XRRQueryOutputProperty: overflow length of property test\n");
    prop_info = XRRQueryOutputProperty(dpy, /* output */ 0, /* property */ 0);
    XFree(prop_info);
    assert(prop_info == NULL);

    XhivCloseDisplay(dpy);
}

int
main(int argc, char **argv)
{
    testOverflowFields();
    printf("XRRQueryOutputProperty: all tests passed\n");
    return 0;
}
