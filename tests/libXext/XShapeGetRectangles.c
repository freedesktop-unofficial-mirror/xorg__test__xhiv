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
#include <X11/extensions/shape.h>
#include <X11/extensions/shapeproto.h>
#include <assert.h>
#include <stdio.h>
#include <limits.h>

#define MY_SHAPE_EXT_CODE 204

static void
testOverflowFields(void)
{
    /* Report that SHAPE is present */
    const xQueryExtensionReply shapeqext_reply = {
        .type = X_Reply,
        .length = 0,
        .present = xTrue,
        .major_opcode = MY_SHAPE_EXT_CODE,
        .first_event = 100,
        .first_error = 150
    };
    xhiv_response shapeqext_response = {
        .next = NULL,
        .reqType = X_QueryExtension,
        .reqMinor = XHIV_REQ_IGNORE,
        .sequence = XHIV_SEQ_MATCHDATA,
        .match_data = SHAPENAME,
        .length = (SIZEOF(xQueryExtensionReply) >> 2),
        .response_data = &shapeqext_reply,
        .response_datalen = sizeof(shapeqext_reply)
    };

    const xShapeQueryVersionReply shapevers_reply = {
        .type = X_Reply,
        .length = 0,
        .majorVersion = SHAPE_MAJOR_VERSION,
        .minorVersion = SHAPE_MINOR_VERSION
    };
    xhiv_response shapevers_response = {
        .next     = &shapeqext_response,
        .reqType  = MY_SHAPE_EXT_CODE,
        .reqMinor = X_ShapeQueryVersion,
        .sequence = XHIV_SEQ_IGNORE,
        .length   = (SIZEOF(xQueryExtensionReply) >> 2),
        .response_data = &shapevers_reply,
        .response_datalen = sizeof(shapevers_reply)
    };
 
    const xShapeGetRectanglesReply rep1 = {
        .type    = X_Reply,
        .length  = 128,
        .nrects  = (INT_MAX / sizeof(XRectangle)) + 64,
    };
    xhiv_response response1 = {
        .next     = &shapevers_response,
        .reqType  = MY_SHAPE_EXT_CODE,
        .reqMinor = X_ShapeGetRectangles,
        .sequence = XHIV_SEQ_IGNORE,
        .length   = (sizeof(rep1) >> 2) + rep1.length,
        .response_data = &rep1,
        .response_datalen = sizeof(rep1)
    };
    Display *dpy = XhivOpenDisplay(&response1);
    int major = SHAPE_MAJOR_VERSION, minor = SHAPE_MINOR_VERSION;
    int status, nrects = 0, ordering;
    XRectangle *rects;

    status = XShapeQueryVersion(dpy, &major, &minor);
    assert(status == True);

    printf("XShapeGetRectangles: overflow number of rects test: 0x%lx\n",
           (unsigned long) rep1.nrects);
    rects = XShapeGetRectangles(dpy, DefaultRootWindow(dpy), 0,
                                &nrects, &ordering);
    XFree(rects);
    assert(rects == NULL);

    XhivCloseDisplay(dpy);
}

int
main(int argc, char **argv)
{
    testOverflowFields();
    printf("XShapeGetRectangles: all tests passed\n");
    return 0;
}
