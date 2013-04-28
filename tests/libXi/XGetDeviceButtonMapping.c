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



static void
testOverflowFields(void)
{
    const xGetDeviceButtonMappingReply rep1 = {
        .repType = X_Reply,
        .RepType = X_GetDeviceButtonMapping,
        .nElts   = 255,
        .length  = 256 >> 2
    };
    const xGetDeviceButtonMappingReply rep2 = {
        .repType = X_Reply,
        .RepType = X_GetDeviceButtonMapping,
        .nElts   = 255,
        .length  = 1024
    };
    xhiv_response response2 = {
        .next = &xi_opendev_response,
        .reqType = MY_XI_EXT_CODE,
        .reqMinor = X_GetDeviceButtonMapping,
        .sequence = 102,
        .length = rep2.length + (sizeof(rep2) >> 2),
        .response_data = &rep2,
        .response_datalen = sizeof(rep2)
    };
    xhiv_response response1 = {
        .next = &response2,
        .reqType = MY_XI_EXT_CODE,
        .reqMinor = X_GetDeviceButtonMapping,
        .sequence = 101,
        .length = rep1.length + (sizeof(rep1) >> 2),
        .response_data = &rep1,
        .response_datalen = sizeof(rep1)
    };
    Display *dpy = XhivOpenDisplay(&response1);
    XDevice *device;
    int status;
    unsigned char map[260];

    device = XOpenDevice(dpy, MY_DEFAULT_DEVICE);
    assert(device != NULL);
    assert(device != (XDevice *) NoSuchExtension);

    /* set sequence to known value so that later calls match by sequence num */
    XhivSequenceSync(dpy, 100);

    printf("XGetDeviceButtonMapping: overflow caller buffer test\n");
    map[16] = 0xde;
    map[17] = 0xad;
    map[18] = 0xbe;
    map[19] = 0xef;
    status = XGetDeviceButtonMapping(dpy, device, map, 16);
    assert(status == 255);
    assert(map[16] == 0xde);
    assert(map[17] == 0xad);
    assert(map[18] == 0xbe);
    assert(map[19] == 0xef);

    /* libXi had a hardcoded 256 char buffer it assumed the server wouldn't
       overflow */
    printf("XGetDeviceButtonMapping: overflow library buffer test\n");
    map[256] = 0xde;
    map[257] = 0xad;
    map[258] = 0xbe;
    map[259] = 0xef;
    status = XGetDeviceButtonMapping(dpy, device, map, 256);
    assert(status == 0);
    assert(map[256] == 0xde);
    assert(map[257] == 0xad);
    assert(map[258] == 0xbe);
    assert(map[259] == 0xef);

    XhivCloseDisplay(dpy);
}

int
main(int argc, char **argv)
{
    testOverflowFields();
    printf("XGetDeviceButtonMapping: all tests passed\n");
    return 0;
}
