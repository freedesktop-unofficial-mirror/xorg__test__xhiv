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
    const xAllocColorCellsReply rep1 = {
        .type    = X_Reply,
        .nPixels = 16,
        .nMasks  =  4,
        .length  = rep1.nPixels + rep1.nMasks
    };
    const xAllocColorCellsReply rep2 = {
        .type    = X_Reply,
        .nPixels =  4,
        .nMasks  = 16,
        .length  = rep2.nPixels + rep2.nMasks
    };
    xhiv_response response2 = {
        .next = NULL,
        .reqType = X_AllocColorCells,
        .reqMinor = XHIV_REQ_IGNORE,
        .sequence = 2,
        .length = rep2.length + (sizeof(rep2) >> 2),
        .response_data = &rep2,
        .response_datalen = sizeof(rep2)
    };
    xhiv_response response1 = {
        .next = &response2,
        .reqType = X_AllocColorCells,
        .reqMinor = XHIV_REQ_IGNORE,
        .sequence = 1,
        .length = rep1.length + (sizeof(rep1) >> 2),
        .response_data = &rep1,
        .response_datalen = sizeof(rep1)
    };
    Display *dpy = XhivOpenDisplay(&response1);
    int i;

    /* Send twice to test both overflow possibilities */
    for (i = 0; i < 2; i++) {
        unsigned long plane_masks_return[16] = { 0L };
        unsigned long pixels_return[16] = { 0L };
        int err;

        printf("XAllocColorCells: overflow test #%d\n", i);
        err = XAllocColorCells(dpy, DefaultColormap(dpy, 0), False,
                               plane_masks_return, 4, pixels_return, 4);
        assert(plane_masks_return[4] == 0L);
        assert(pixels_return[4] == 0L);
        assert(err == 0);  /* 0 is failure for XAllocColorCells */
    }
    XhivCloseDisplay(dpy);
}

int
main(int argc, char **argv)
{
    testOverflowFields();
    printf("XAllocColorCells: all tests passed\n");
    return 0;
}
