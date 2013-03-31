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
#include <assert.h>
#include <stdio.h>

static void
testOverflowFields(void)
{
    /* Test integer overflow of .nReplies * sizeof(XFontStruct) */
    const xListFontsWithInfoReply rep1 = {
        .type = X_Reply,
        .nameLength = 5, /* strlen("fixed"); */
        .length = (8 + SIZEOF(xListFontsWithInfoReply) - SIZEOF(xReply)) >> 2,
        .nReplies = 1
    };
    const xListFontsWithInfoReply rep2 = {
        .type = X_Reply,
        .nameLength = 5, /* strlen("fixed"); */
        .length = (8 + SIZEOF(xListFontsWithInfoReply) - SIZEOF(xReply)) >> 2,
        .nReplies = (UINT32_MAX / sizeof(XFontStruct)) - 1
    };
    const xListFontsWithInfoReply rep3 = {
        .type = X_Reply,
        .nameLength = 5, /* strlen("fixed"); */
        .length = (8 + SIZEOF(xListFontsWithInfoReply) - SIZEOF(xReply)) >> 2,
        .nReplies = (UINT32_MAX / sizeof(XFontStruct)) - 2
    };
    const xListFontsWithInfoReply rep4 = {
        .type = X_Reply,
        .nameLength = 0, /* end of replies */
        .length = (8 + SIZEOF(xListFontsWithInfoReply) - SIZEOF(xReply)) >> 2,
        .nReplies = 0
    };
    xhiv_response response4 = {
        .length = (8 + SIZEOF(xListFontsWithInfoReply)) >> 2,
        .response_data = &rep4,
        .response_datalen = sizeof(rep4)
    };
    xhiv_response response3 = {
        .chain = &response4,
        .length = (8 + SIZEOF(xListFontsWithInfoReply)) >> 2,
        .response_data = &rep3,
        .response_datalen = sizeof(rep3)
    };
    xhiv_response response2 = {
        .chain = &response3,
        .length = (8 + SIZEOF(xListFontsWithInfoReply)) >> 2,
        .response_data = &rep2,
        .response_datalen = sizeof(rep2)
    };
    xhiv_response response1 = {
        .chain = &response2,
        .reqType = X_ListFontsWithInfo,
        .reqMinor = XHIV_REQ_IGNORE,
        .sequence = XHIV_SEQ_IGNORE,
        .length = (8 + SIZEOF(xListFontsWithInfoReply)) >> 2,
        .response_data = &rep1,
        .response_datalen = sizeof(rep1)
    };
    Display *dpy = XhivOpenDisplay(&response1);
    char **names;
    int count;
    XFontStruct *info;

    printf("XListFontsWithInfo: overflow test - nReplies = 0x%lx\n",
           (unsigned long) rep2.nReplies);
    names = XListFontsWithInfo(dpy, "*", UINT16_MAX, &count, &info);
    /* If we're going to fail, see if we corrupted memory */
    XFreeFontInfo(names, info, count);

    assert(names == NULL);
    assert(count == 0);
    assert(info == NULL);

    XhivCloseDisplay(dpy);
}

int
main(int argc, char **argv)
{
    testOverflowFields();
    printf("XListFontsWithInfo: all tests passed\n");
    return 0;
}
