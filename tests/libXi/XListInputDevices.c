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
#include <X11/extensions/XInput.h>
#include <X11/extensions/XIproto.h>
#include "xhiv-Xi.h"
#include <assert.h>
#include <stdio.h>
#include <limits.h>

static void
testOverflowFields(void)
{
    const xListInputDevicesReply rep1 = {
        .repType = X_Reply,
        .RepType = X_ListInputDevices,
        .length  = (sizeof(xDeviceInfo) >> 2) + 1,
        .ndevices = 1,
    };
    const xDeviceInfo devi = {
        .type = OtherClass,
        .num_classes = 0,
    };
    char devstrings [] = { (char)(1 - sizeof(XDeviceInfo)), 0, 0, 0 };
    xhiv_response devs_response = {
        .length = (sizeof(devstrings) + 3) >> 2,
        .repeat = rep1.ndevices - 1,
        .response_data = &devstrings,
        .response_datalen = sizeof(devstrings),
        .flags = XHIV_NO_SET_SEQUENCE
    };
    xhiv_response devi_response = {
        .length = (sizeof(devi) >> 2) * rep1.ndevices,
        .chain = &devs_response,
        .repeat = rep1.ndevices - 1,
        .response_data = &devi,
        .response_datalen = sizeof(devi),
        .flags = XHIV_NO_SET_SEQUENCE
    };
    xhiv_response response1 = {
        .next = &xi_opendev_response,
        .chain = &devi_response,
        .reqType = MY_XI_EXT_CODE,
        .reqMinor = X_ListInputDevices,
        .sequence = XHIV_SEQ_IGNORE,
        .length = (sizeof(rep1) >> 2),
        .response_data = &rep1,
        .response_datalen = sizeof(rep1)
    };
    Display *dpy = XhivOpenDisplay(&response1);
    XDeviceInfo *devices;
    int num_return;
    
    printf("XListInputDevices: overflow number of items test\n");
    devices = XListInputDevices(dpy, &num_return);
    XFreeDeviceList(devices);
    assert(devices == NULL);

    XhivCloseDisplay(dpy);
}

int
main(int argc, char **argv)
{
    testOverflowFields();
    printf("XListInputDevices: all tests passed\n");
    return 0;
}
