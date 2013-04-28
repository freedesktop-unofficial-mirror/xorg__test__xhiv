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
    const xGetDeviceControlReply rep1 = {
        .repType = X_Reply,
        .RepType = X_GetDeviceControl,
        .length  = sizeof(xDeviceResolutionState) >> 2,
    };
    const xDeviceResolutionState drs = {
        .control = DEVICE_RESOLUTION,
        .length  = sizeof(xDeviceResolutionState),
        .num_valuators = (UINT_MAX / (3 * sizeof(int))) + 1
    };
    xhiv_response drs_response = {
        .length = sizeof(xDeviceResolutionState) >> 2,
        .response_data = &drs,
        .response_datalen = sizeof(drs),
        .flags = XHIV_NO_SET_SEQUENCE
    };
    xhiv_response response1 = {
        .next = &xi_opendev_response,
        .chain = &drs_response,
        .reqType = MY_XI_EXT_CODE,
        .reqMinor = X_GetDeviceControl,
        .sequence = 101,
        .length = sizeof(rep1) >> 2,
        .response_data = &rep1,
        .response_datalen = sizeof(rep1)
    };
    Display *dpy = XhivOpenDisplay(&response1);
    XDevice *device;
    XDeviceControl *control;

    device = XOpenDevice(dpy, MY_DEFAULT_DEVICE);
    assert(device != NULL);
    assert(device != (XDevice *) NoSuchExtension);

    /* set sequence to known value so that later calls match by sequence num */
    XhivSequenceSync(dpy, 100);

    printf("XGetDeviceControl: overflow number of valuators test\n");
    control = XGetDeviceControl(dpy, device, DEVICE_RESOLUTION);
    XFreeDeviceControl(control);
    assert(control == NULL);

    XhivCloseDisplay(dpy);
}

int
main(int argc, char **argv)
{
    testOverflowFields();
    printf("XGetDeviceControl: all tests passed\n");
    return 0;
}
