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
    const xGetDevicePropertyReply rep1 = {
        .repType = X_Reply,
        .RepType = X_GetDeviceProperty,
        .length  = 32,
        .propertyType = XA_ATOM,
        .nItems  = (UINT_MAX / 4) + 1,
        .format  = 32,
    };
    xhiv_response response1 = {
        .next = &xi_opendev_response,
        .reqType = MY_XI_EXT_CODE,
        .reqMinor = X_GetDeviceProperty,
        .sequence = XHIV_SEQ_IGNORE,
        .length = (sizeof(rep1) >> 2) + rep1.length,
        .response_data = &rep1,
        .response_datalen = sizeof(rep1)
    };
    Display *dpy = XhivOpenDisplay(&response1);
    XDevice *device;
    int status;
    Atom type_return;
    int format_return;
    unsigned long num_items_return, bytes_after_return;
    unsigned char *data = NULL;
    
    device = XOpenDevice(dpy, MY_DEFAULT_DEVICE);
    assert(device != NULL);
    assert(device != (XDevice *) NoSuchExtension);    

    printf("XGetDeviceProperty: overflow number of items test\n");
    status = XGetDeviceProperty(dpy, device, /* property */ 0, /* offset */ 0,
                                /* length */ 0, /* delete_property */ False,
                                /* type */ XA_ATOM, &type_return,
                                &format_return, &num_items_return,
                                &bytes_after_return, &data);
    XFree(data);
    assert(status != Success);
    assert(data == NULL);

    XhivCloseDisplay(dpy);
}

int
main(int argc, char **argv)
{
    testOverflowFields();
    printf("XGetDeviceProperty: all tests passed\n");
    return 0;
}
