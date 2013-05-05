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
#include <X11/extensions/Xinerama.h>
#include <X11/extensions/panoramiXproto.h>
#include <assert.h>
#include <stdio.h>
#include <limits.h>

#define MY_XINERAMA_EXT_CODE 202

static void
testOverflowFields(void)
{
    /* Report that XINERAMA is present */
    const xQueryExtensionReply xineramaqext_reply = {
        .type = X_Reply,
        .length = 0,
        .present = xTrue,
        .major_opcode = MY_XINERAMA_EXT_CODE,
        .first_event = 100,
        .first_error = 150
    };
    xhiv_response xineramaqext_response = {
        .next = NULL,
        .reqType = X_QueryExtension,
        .reqMinor = XHIV_REQ_IGNORE,
        .sequence = XHIV_SEQ_MATCHDATA,
        .match_data = PANORAMIX_PROTOCOL_NAME,
        .length = (SIZEOF(xQueryExtensionReply) >> 2),
        .response_data = &xineramaqext_reply,
        .response_datalen = sizeof(xineramaqext_reply)
    };

    const xPanoramiXQueryVersionReply xineramavers_reply = {
        .type = X_Reply,
        .length = 0,
        .majorVersion = PANORAMIX_MAJOR_VERSION,
        .minorVersion = PANORAMIX_MINOR_VERSION
    };
    xhiv_response xineramavers_response = {
        .next     = &xineramaqext_response,
        .reqType  = MY_XINERAMA_EXT_CODE,
        .reqMinor = X_PanoramiXQueryVersion,
        .sequence = XHIV_SEQ_IGNORE,
        .length   = (SIZEOF(xPanoramiXQueryVersionReply) >> 2),
        .response_data = &xineramavers_reply,
        .response_datalen = sizeof(xineramavers_reply)
    };

    const xXineramaQueryScreensReply rep1 = {
        .type    = X_Reply,
        .pad1    = X_XineramaQueryScreens,
        .length  = 65536,
        .number  = (UINT_MAX / sizeof(XineramaScreenInfo)) + 4
    };
    xhiv_response response1 = {
        .next     = &xineramavers_response,
        .reqType  = MY_XINERAMA_EXT_CODE,
        .reqMinor = X_XineramaQueryScreens,
        .sequence = XHIV_SEQ_IGNORE,
        .length   = (sizeof(rep1) >> 2) + rep1.length,
        .response_data = &rep1,
        .response_datalen = sizeof(rep1)
    };

    Display *dpy = XhivOpenDisplay(&response1);
    int nscreens, status;
    int major = PANORAMIX_MAJOR_VERSION, minor = PANORAMIX_MINOR_VERSION;
    XineramaScreenInfo *xsi;

    status = XineramaQueryVersion(dpy, &major, &minor);
    assert(status == True);

    printf("XineramaQueryScreens: overflow number of screens test: 0x%x\n",
        rep1.number);
    xsi = XineramaQueryScreens(dpy, &nscreens);
    XFree(xsi);
    assert(xsi == NULL);

    XhivCloseDisplay(dpy);
}

int
main(int argc, char **argv)
{
    testOverflowFields();
    printf("XineramaQueryScreens: all tests passed\n");
    return 0;
}
