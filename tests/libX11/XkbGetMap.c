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
#include "xhiv-Xkb.h"
#include <X11/Xlib.h>
#include <X11/XKBlib.h>
#include <X11/Xproto.h>
#include <X11/extensions/XKBproto.h>
#include <assert.h>
#include <stdio.h>

#define MY_XKB_EXT_CODE         200

static void
testOverflowFields(void)
{
    /* Test overflow of .firstKeySym + .nKeySyms > .maxKeyCode */
    const xkbGetMapReply rep1 = {
        .type = X_Reply,
        .length = 1024,
        .minKeyCode = 1,
        .maxKeyCode = 2,
        .firstKeySym = 128,
        .totalSyms = 128,
        .nKeySyms = 128,
    };
    xhiv_response response1 = {
        .next = &xkb_use_response,
        .reqType = MY_XKB_EXT_CODE,
        .reqMinor = X_kbGetMap,
        .sequence = 101,
        .length = (SIZEOF(xkbGetMapReply) >> 2),
        .response_data = &rep1,
        .response_datalen = sizeof(rep1)
    };
    /* Test overflow of .firstKeyAct + .nKeyActs > .maxKeyCode */
    const xkbGetMapReply rep2 = {
        .type = X_Reply,
        .length = 1024,
        .minKeyCode = 1,
        .maxKeyCode = 2,
        .firstKeyAct = 128,
        .totalActs = 128,
        .nKeyActs = 128,
    };
    xhiv_response response2 = {
        .next = &response1,
        .reqType = MY_XKB_EXT_CODE,
        .reqMinor = X_kbGetMap,
        .sequence = 102,
        .length = (SIZEOF(xkbGetMapReply) >> 2),
        .response_data = &rep2,
        .response_datalen = sizeof(rep2)
    };
    Display *dpy = XhivOpenDisplay(&response2);
    XkbDescPtr desc;

    /* set sequence to known value so that later calls match by sequence num */
    XhivSequenceSync(dpy, 100);

    printf("XkbGetMap: overflow test - firstKeySym\n");
    desc = XkbGetMap(dpy, XkbXI_AllFeaturesMask, XkbUseCoreKbd);
    /* Which function to use to free ? */
    assert(desc == NULL);

    printf("XkbGetMap: overflow test - firstKeyAct\n");
    desc = XkbGetMap(dpy, XkbXI_AllFeaturesMask, XkbUseCoreKbd);
    /* Which function to use to free ? */
    assert(desc == NULL);

    XhivCloseDisplay(dpy);
}

int
main(int argc, char **argv)
{
#if 0
    testOverflowFields();
    printf("XkbGetMap: all tests passed\n");
    return 0;
#else
    printf("XkbGetMap: untested\n");
    return 77;
#endif
}
