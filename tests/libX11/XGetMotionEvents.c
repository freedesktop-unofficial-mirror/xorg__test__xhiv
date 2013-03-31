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
#include <X11/Xproto.h>
#include <assert.h>
#include <stdio.h>

static void
testOverflowFields(void)
{
    /* Test integer overflow of .nEvents * sizeof(xTimecoord) */
    const xGetMotionEventsReply rep1 = {
        .type = X_Reply,
        .length = ((SIZEOF(xGetMotionEventsReply) - SIZEOF(xReply)) >> 2)
                + (UINT32_MAX / sizeof(XTimeCoord)) + 1,
        .nEvents = (UINT32_MAX / sizeof(XTimeCoord)) + 1
    };
    xhiv_response response1 = {
        .reqType = X_GetMotionEvents,
        .reqMinor = XHIV_REQ_IGNORE,
        .sequence = XHIV_SEQ_IGNORE,
        .length = (SIZEOF(xGetMotionEventsReply) >> 2) + rep1.nEvents,
        .response_data = &rep1,
        .response_datalen = sizeof(rep1)
    };
    Display *dpy = XhivOpenDisplay(&response1);
    XTimeCoord *xtc;
    int n;

    printf("XGetMotionEvents: overflow test - nEvents = 0x%lx\n",
           (unsigned long) rep1.nEvents);
    xtc = XGetMotionEvents(dpy, PointerWindow, 0, CurrentTime, &n);

    assert(xtc == NULL);
    assert(n == 0);

    XhivCloseDisplay(dpy);
}

int
main(int argc, char **argv)
{
    testOverflowFields();
    printf("XGetMotionEvents: all tests passed\n");
    return 0;
}
