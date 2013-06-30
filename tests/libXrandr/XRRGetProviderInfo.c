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
testCapabilityList(void)
{
    const xRRGetProviderInfoReply rep1 = {
        .type           = X_Reply,
        .length         = 36,
        .capabilities   = 0x0f,
        .nCrtcs         = 1,
        .nOutputs       = 1,
        .nAssociatedProviders = 3,
        .nameLength     = 1
    };
    int32_t provider_data[] = {
        /* crtcs */
        0x00cc00cc,
        /* outputs */
        0xdeadbeef,
        /* associated providers */
        0xbabeface,
        0xcafef00d,
        0xbabef00d,
        /* capabilities */
        0xca4ab171,
        0xca4ab172,
        0xca4ab173,
    };
    xhiv_response provider_response = {
        .length = rep1.length,
        .flags = XHIV_NO_SET_SEQUENCE,
        .response_data = provider_data,
        .response_datalen = sizeof(provider_data)
    };
    xhiv_response response1 = {
        .next = &xrandr_vers_response,
        .chain = &provider_response,
        .reqType = MY_XRANDR_EXT_CODE,
        .reqMinor = X_RRGetProviderInfo,
        .sequence = XHIV_SEQ_IGNORE,
        .length = (sizeof(rep1) >> 2),
        .response_data = &rep1,
        .response_datalen = sizeof(rep1)
    };
    Display *dpy = XhivOpenDisplay(&response1);
    int major = MY_XRANDR_MAJOR_VERSION;
    int minor = MY_XRANDR_MINOR_VERSION;
    int status;
    XRRScreenResources resources = { .configTimestamp = 0 };
    XRRProviderInfo *info;

    status = XRRQueryVersion(dpy, &major, &minor);
    assert(status != 0);

    printf("XRRGetProviderInfo: overflow number of items test\n");
    info = XRRGetProviderInfo(dpy, &resources, /* provider */ 1);

    assert(info != NULL);
    assert(info->crtcs[0] == (XID) provider_data[0]);
    assert(info->outputs[0] == (XID) provider_data[1]);
    assert(info->associated_providers[0] == (XID) provider_data[2]);
    assert(info->associated_providers[1] == (XID) provider_data[3]);
    assert(info->associated_providers[2] == (XID) provider_data[4]);
    assert(info->associated_capability[0] == provider_data[5]);
    assert(info->associated_capability[1] == provider_data[6]);
    assert(info->associated_capability[2] == provider_data[7]);

    XFree(info);

    XhivCloseDisplay(dpy);
}

int
main(int argc, char **argv)
{
    testCapabilityList();
    printf("XRRGetProviderInfo: all tests passed\n");
    return 0;
}
