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

#define MY_XI_MAJOR_VERSION 2
#define MY_XI_MINOR_VERSION 0

#include "xhiv.h"
#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/extensions/XInput2.h>
#include <X11/extensions/XI2proto.h>
#include "xhiv-Xi.h"
#include <assert.h>
#include <stdio.h>
#include <limits.h>

#define NUM_MASKS ((USHRT_MAX >> 2) + 4)
#define MASK_LEN USHRT_MAX

typedef struct {
    xXIEventMask header;
    uint32_t mask[MASK_LEN];
} myXIEventMask;

static const myXIEventMask mask_rep = {
    .header.deviceid = 0,
    .header.mask_len = MASK_LEN
};

static void
testOverflowFields(void)
{
    const xXIGetSelectedEventsReply rep1 = {
        .repType = X_Reply,
        .RepType = X_XIGetSelectedEvents,
        .length  = (NUM_MASKS * (sizeof(myXIEventMask) >> 2)),
        .num_masks = NUM_MASKS
    };
    xhiv_response mask_response = {
        .length = (NUM_MASKS * (sizeof(myXIEventMask) >> 2)),
        .repeat = NUM_MASKS - 1,
        .response_data = &mask_rep,
        .response_datalen = sizeof(mask_rep),
        .flags = XHIV_NO_SET_SEQUENCE
    };
    xhiv_response response1 = {
        .next = &xi2_vers_response,
        .chain = &mask_response,
        .reqType = MY_XI_EXT_CODE,
        .reqMinor = X_XIGetSelectedEvents,
        .sequence = XHIV_SEQ_IGNORE,
        .length = sizeof(rep1) >> 2,
        .response_data = &rep1,
        .response_datalen = sizeof(rep1)
    };
    Display *dpy = XhivOpenDisplay(&response1);
    int major = MY_XI_MAJOR_VERSION;
    int minor = MY_XI_MINOR_VERSION;
    int status;
    Window w = DefaultRootWindow(dpy);
    int num_masks_return;
    XIEventMask *masks;
    
    status = XIQueryVersion(dpy, &major, &minor);
    assert(status == Success);

    printf("XIGetSelectedEvents: overflow number of items test\n");
    masks = XIGetSelectedEvents(dpy, w, &num_masks_return);
    XFree(masks);
    assert(masks == NULL);

    /* Currently hits XIOError that prevents distcheck: */
#ifdef DEBUG
       XhivCloseDisplay(dpy);
#endif
}

int
main(int argc, char **argv)
{
    testOverflowFields();
    printf("XIGetSelectedEvents: all tests passed\n");
    return 0;
}
