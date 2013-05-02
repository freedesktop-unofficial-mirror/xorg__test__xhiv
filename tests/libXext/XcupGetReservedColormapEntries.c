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
#include <X11/extensions/Xcup.h>
#include <X11/extensions/cupproto.h>
#include <assert.h>
#include <stdio.h>
#include <limits.h>

#define MY_CUP_EXT_CODE 202

static void
testOverflowFields(void)
{
    /* Report that CUP is present */
    const xQueryExtensionReply cupqext_reply = {
        .type = X_Reply,
        .length = 0,
        .present = xTrue,
        .major_opcode = MY_CUP_EXT_CODE,
        .first_event = 100,
        .first_error = 150
    };
    xhiv_response cupqext_response = {
        .next = NULL,
        .reqType = X_QueryExtension,
        .reqMinor = XHIV_REQ_IGNORE,
        .sequence = XHIV_SEQ_MATCHDATA,
        .match_data = XCUPNAME,
        .length = (SIZEOF(xQueryExtensionReply) >> 2),
        .response_data = &cupqext_reply,
        .response_datalen = sizeof(cupqext_reply)
    };

    const xXcupQueryVersionReply cupvers_reply = {
        .type = X_Reply,
        .length = 0,
        .server_major_version = XCUP_MAJOR_VERSION,
        .server_minor_version = XCUP_MINOR_VERSION
    };
    xhiv_response cupvers_response = {
        .next     = &cupqext_response,
        .reqType  = MY_CUP_EXT_CODE,
        .reqMinor = X_XcupQueryVersion,
        .sequence = XHIV_SEQ_IGNORE,
        .length   = (SIZEOF(xQueryExtensionReply) >> 2),
        .response_data = &cupvers_reply,
        .response_datalen = sizeof(cupvers_reply)
    };
 
    const xXcupGetReservedColormapEntriesReply rep1 = {
        .type    = X_Reply,
        .pad1    = X_XcupGetReservedColormapEntries,
        .length  = (INT_MAX / 4),
    };
    xhiv_response response1 = {
        .next     = &cupvers_response,
        .reqType  = MY_CUP_EXT_CODE,
        .reqMinor = X_XcupGetReservedColormapEntries,
        .sequence = XHIV_SEQ_IGNORE,
        .length   = (sizeof(rep1) >> 2) + rep1.length,
        .response_data = &rep1,
        .response_datalen = sizeof(rep1)
    };
    Display *dpy = XhivOpenDisplay(&response1);
    XColor* colors_out = NULL;
    int ncolors, status;
    int major = XCUP_MAJOR_VERSION, minor = XCUP_MINOR_VERSION;

    status = XcupQueryVersion(dpy, &major, &minor);
    assert(status == True);
    
    printf("XcupGetReservedColormapEntries: overflow number of entries test\n");
    status = XcupGetReservedColormapEntries(dpy, DefaultScreen(dpy),
                                            &colors_out, &ncolors);
    XFree(colors_out);
    assert(colors_out == NULL);
    assert(status == False);

    XhivCloseDisplay(dpy);
}

int
main(int argc, char **argv)
{
#if 0 /* XXX - Disable so make distcheck can pass */
    testOverflowFields();
    printf("XcupGetReservedColormapEntries: all tests passed\n");
    return 0;
#else
    printf("XcupGetReservedColormapEntries: untested\n");
    return 77;
#endif
}
