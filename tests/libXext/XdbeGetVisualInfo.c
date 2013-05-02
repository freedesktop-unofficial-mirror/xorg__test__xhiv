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
#include <X11/extensions/Xdbe.h>
#include <X11/extensions/dbeproto.h>
#include <assert.h>
#include <stdio.h>
#include <limits.h>

#define MY_DBE_EXT_CODE 202

static void
testOverflowFields(void)
{
    /* Report that DBE is present */
    const xQueryExtensionReply dbeqext_reply = {
        .type = X_Reply,
        .length = 0,
        .present = xTrue,
        .major_opcode = MY_DBE_EXT_CODE,
        .first_event = 100,
        .first_error = 150
    };
    xhiv_response dbeqext_response = {
        .next = NULL,
        .reqType = X_QueryExtension,
        .reqMinor = XHIV_REQ_IGNORE,
        .sequence = XHIV_SEQ_MATCHDATA,
        .match_data = DBE_PROTOCOL_NAME,
        .length = (SIZEOF(xQueryExtensionReply) >> 2),
        .response_data = &dbeqext_reply,
        .response_datalen = sizeof(dbeqext_reply)
    };

    const xDbeGetVersionReply dbevers_reply = {
        .type = X_Reply,
        .length = 0,
        .majorVersion = DBE_MAJOR_VERSION,
        .minorVersion = DBE_MINOR_VERSION
    };
    xhiv_response dbevers_response = {
        .next     = &dbeqext_response,
        .reqType  = MY_DBE_EXT_CODE,
        .reqMinor = X_DbeGetVersion,
        .sequence = XHIV_SEQ_IGNORE,
        .length   = (SIZEOF(xQueryExtensionReply) >> 2),
        .response_data = &dbevers_reply,
        .response_datalen = sizeof(dbevers_reply)
    };
 
    const xDbeGetVisualInfoReply rep1 = {
        .type    = X_Reply,
        .pad1    = X_DbeGetVisualInfo,
        .length  = (UINT_MAX / sizeof(XdbeScreenVisualInfo)) + 2,
        .m       = (UINT_MAX / sizeof(XdbeScreenVisualInfo)) + 2,
    };
    const CARD32 dbeviscount = 0;
    xhiv_response dbeviscounts_response = {
        .reqType  = XHIV_REQ_IGNORE,
        .reqMinor = XHIV_REQ_IGNORE,
        .sequence = XHIV_SEQ_IGNORE,
        .length   = rep1.length,
        .repeat   = rep1.length - 1,
        .response_data = &dbeviscount,
        .response_datalen = sizeof(dbeviscount)
    };
    xhiv_response response1 = {
        .next     = &dbevers_response,
        .chain    = &dbeviscounts_response,
        .reqType  = MY_DBE_EXT_CODE,
        .reqMinor = X_DbeGetVisualInfo,
        .sequence = XHIV_SEQ_IGNORE,
        .length   = (sizeof(rep1) >> 2),
        .response_data = &rep1,
        .response_datalen = sizeof(rep1)
    };
    Display *dpy = XhivOpenDisplay(&response1);
    int major = DBE_MAJOR_VERSION, minor = DBE_MINOR_VERSION;
    int status, nscreens = 0;
    Drawable screens[] = { 0xdeadbeef };
    XdbeScreenVisualInfo *vi;

    status = XdbeQueryExtension(dpy, &major, &minor);
    assert(status == True);

    printf("XdbeGetVisualInfo: overflow number of screens test: 0x%lx\n",
           (unsigned long) rep1.m);
    vi = XdbeGetVisualInfo(dpy, screens, &nscreens);
    XdbeFreeVisualInfo(vi);
    assert(vi == NULL);

    XhivCloseDisplay(dpy);
}

int
main(int argc, char **argv)
{
    testOverflowFields();
    printf("XdbeGetVisualInfo: all tests passed\n");
    return 0;
}
