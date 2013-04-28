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
#include <X11/extensions/XInput2.h>
#include <X11/extensions/XI2proto.h>
#include "xhiv-Xi.h"
#include <assert.h>
#include <stdio.h>



static void
testOverflowFields(void)
{
    const xXIPassiveGrabDeviceReply rep1 = {
        .repType = X_Reply,
        .RepType = X_XIPassiveGrabDevice,
        .num_modifiers = 16,
        .length  = (16 * sizeof(xXIGrabModifierInfo)) >> 2
    };
    xhiv_response response1 = {
        .next = &xi2_vers_response,
        .reqType = MY_XI_EXT_CODE,
        .reqMinor = X_XIPassiveGrabDevice,
        .sequence = 101,
        .length = rep1.length + (sizeof(rep1) >> 2),
        .response_data = &rep1,
        .response_datalen = sizeof(rep1)
    };
    Display *dpy = XhivOpenDisplay(&response1);
    int major = MY_XI_MAJOR_VERSION;
    int minor = MY_XI_MINOR_VERSION;
    int status;
    unsigned char mask_data[4] = { 1, 2, 3, 4};
    XIEventMask mask = { MY_DEFAULT_DEVICE, 1, mask_data } ;
    XIGrabModifiers mods[2] = {
        { 0, 0 },
        { (int) 0xdeadbeef, (int) 0xabadf00d }
    };

    status = XIQueryVersion(dpy, &major, &minor);
    assert(status == Success);

    /* set sequence to known value so that later calls match by sequence num */
    XhivSequenceSync(dpy, 100);

    printf("XIGrabButton: overflow caller buffer test\n");
    status = XIGrabButton(dpy, MY_DEFAULT_DEVICE, /* button */ 2,
                          /* grab_window */ 3, /* cursor */ 4,
                          /* grab_mode */ 5, /* paired_device_mode */ 6,
                          /* owner_events */ xTrue, &mask,
                          /* num_modifiers */ 1, mods);
    assert(status > 0);
    assert(mods[1].modifiers == (int) 0xdeadbeef);
    assert(mods[1].status == (int) 0xabadf00d);

    XhivCloseDisplay(dpy);
}

int
main(int argc, char **argv)
{
    testOverflowFields();
    printf("XIPassiveGrabDevice: all tests passed\n");
    return 0;
}
