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
#include <X11/extensions/XInput.h>
#include <X11/extensions/XIproto.h>
#include "xhiv-Xi.h"
#include <assert.h>
#include <stdio.h>
#include <limits.h>

static void
testOverflowFields(void)
{
    const xGetFeedbackControlReply rep1 = {
        .repType = X_Reply,
        .RepType = X_GetFeedbackControl,
        .length  = (sizeof(xStringFeedbackState) * USHRT_MAX) >> 2,
        .num_feedbacks = USHRT_MAX
    };
    const xStringFeedbackState sfs = {
        .class = StringFeedbackClass,
        .length = sizeof(xStringFeedbackState),
        .max_symbols = 256,
        .num_syms_supported = USHRT_MAX,
    };
    xhiv_response sfs_response = {
        .length = (sizeof(xStringFeedbackState) * USHRT_MAX) >> 2,
        .repeat = USHRT_MAX - 1,
        .response_data = &sfs,
        .response_datalen = sizeof(sfs),
        .flags = XHIV_NO_SET_SEQUENCE
    };
    xhiv_response response1 = {
        .next = &xi_opendev_response,
        .chain = &sfs_response,
        .reqType = MY_XI_EXT_CODE,
        .reqMinor = X_GetFeedbackControl,
        .sequence = 101,
        .length = sizeof(rep1) >> 2,
        .response_data = &rep1,
        .response_datalen = sizeof(rep1)
    };
    Display *dpy = XhivOpenDisplay(&response1);
    XDevice *device;
    XFeedbackState *feeds;
    int num_feeds;

    device = XOpenDevice(dpy, MY_DEFAULT_DEVICE);
    assert(device != NULL);
    assert(device != (XDevice *) NoSuchExtension);

    /* set sequence to known value so that later calls match by sequence num */
    XhivSequenceSync(dpy, 100);

    printf("XGetFeedbackControl: overflow total size of states test\n");
    feeds = XGetFeedbackControl(dpy, device, &num_feeds);
    XFreeFeedbackList(feeds);
    assert(feeds == NULL);

    XhivCloseDisplay(dpy);
}

int
main(int argc, char **argv)
{
    testOverflowFields();
    printf("XGetFeedbackControl: all tests passed\n");
    return 0;
}
