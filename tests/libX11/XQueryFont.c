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
    /* Test integer overflow of .nCharInfos * sizeof(XCharStruct) */
    const xQueryFontReply rep1 = {
        .type = X_Reply,
        .length = ((2 * SIZEOF(xCharInfo)) + SIZEOF(xQueryFontReply)
                   - SIZEOF(xReply)) >> 2,
        .nFontProps = 0,
        .nCharInfos = (UINT32_MAX / sizeof(XCharStruct)) + 2
    };
    /* Test reporting more properties than length allows for */
    const xQueryFontReply rep2 = {
        .type = X_Reply,
        .length = (SIZEOF(xQueryFontReply) - SIZEOF(xReply)) >> 2,
        .nFontProps = 32,
        .nCharInfos = 0
    };
    /* Test reporting more characters than length allows for */
    const xQueryFontReply rep3 = {
        .type = X_Reply,
        .length = (SIZEOF(xQueryFontReply) - SIZEOF(xReply)) >> 2,
        .nFontProps = 0,
        .nCharInfos = 32
    };
    /* Empty lists for initialization query */
    const xQueryFontReply rep_init = {
        .type = X_Reply,
        .length = (SIZEOF(xQueryFontReply) - SIZEOF(xReply)) >> 2,
        .nFontProps = 0,
        .nCharInfos = 0
    };
    xhiv_response response_init = {
        .next = NULL,
        .reqType = X_QueryFont,
        .reqMinor = XHIV_REQ_IGNORE,
        .sequence = XHIV_SEQ_IGNORE,
        .length = (SIZEOF(xQueryFontReply)) >> 2,
        .response_data = &rep_init,
        .response_datalen = sizeof(rep_init)
    };
    xhiv_response response3 = {
        .next = &response_init,
        .reqType = X_QueryFont,
        .reqMinor = XHIV_REQ_IGNORE,
        .sequence = 103,
        .length = (SIZEOF(xQueryFontReply)) >> 2,
        .response_data = &rep3,
        .response_datalen = sizeof(rep3)
    };
    xhiv_response response2 = {
        .next = &response3,
        .reqType = X_QueryFont,
        .reqMinor = XHIV_REQ_IGNORE,
        .sequence = 102,
        .length = (SIZEOF(xQueryFontReply)) >> 2,
        .response_data = &rep2,
        .response_datalen = sizeof(rep2)
    };
    xhiv_response response1 = {
        .next = &response2,
        .reqType = X_QueryFont,
        .reqMinor = XHIV_REQ_IGNORE,
        .sequence = 101,
        .length = (SIZEOF(xQueryFontReply) + (2 * SIZEOF(xCharInfo))) >> 2,
        .response_data = &rep1,
        .response_datalen = sizeof(rep1)
    };
    Display *dpy = XhivOpenDisplay(&response1);
    XFontStruct *font;

    /* initial query to get Xlib to initialize XF86Bigfont extension hooks
       so we don't have to count those requests in later sequences */
    font = XQueryFont(dpy, 1);
    assert(font != NULL);

    /* set sequence to known value so that later calls match by sequence num */
    XhivSequenceSync(dpy, 100);

    printf("XQueryFont: overflow test - nCharInfos = 0x%lx\n",
           (unsigned long) rep1.nCharInfos);
    font = XQueryFont(dpy, 1);
    assert(font == NULL);

    printf("XQueryFont: overlength test - nFontProps = %d / length = %d\n",
           rep2.nFontProps, rep2.length);
    font = XQueryFont(dpy, 1);
    assert(font == NULL);

    printf("XQueryFont: overlength test - nCharInfos = %d / length = %d\n",
           rep3.nCharInfos, rep3.length);
    font = XQueryFont(dpy, 1);
    assert(font == NULL);

    XhivCloseDisplay(dpy);
}

int
main(int argc, char **argv)
{
    testOverflowFields();
    printf("XQueryFont: all tests passed\n");
    return 0;
}
