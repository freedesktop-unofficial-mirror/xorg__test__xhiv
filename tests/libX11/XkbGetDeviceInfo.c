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
#include <X11/XKBlib.h>
#include <X11/Xproto.h>
#include <X11/extensions/XKBproto.h>
#include <assert.h>
#include <stdio.h>

#define MY_XKB_EXT_CODE         200

static void
testOverflowFields(void)
{
    /* Report that XKB is present */
    const xQueryExtensionReply qext_reply = {
        .type = X_Reply,
        .length = 0,
        .present = xTrue,
        .major_opcode = MY_XKB_EXT_CODE,
        .first_event = 100,
        .first_error = 150
    };
    xhiv_response qext_response = {
        .reqType = X_QueryExtension,
        .reqMinor = XHIV_REQ_IGNORE,
        .sequence = XHIV_SEQ_MATCHDATA,
        .match_data = XkbName,
        .length = (SIZEOF(xQueryExtensionReply) >> 2),
        .response_data = &qext_reply,
        .response_datalen = sizeof(qext_reply)
    };
    /* Report which version of XKB we support */
    const xkbUseExtensionReply xkb_use_reply = {
        .type = X_Reply,
        .supported = xTrue,
        .serverMajor = 1,
        .serverMinor = 0
    };
    xhiv_response xkb_use_response = {
        .next = &qext_response,
        .reqType = MY_XKB_EXT_CODE,
        .reqMinor = X_kbUseExtension,
        .sequence = XHIV_SEQ_IGNORE,
        .length = (SIZEOF(xkbUseExtensionReply) >> 2),
        .response_data = &xkb_use_reply,
        .response_datalen = sizeof(xkb_use_reply)
    };
    /* xkbGetDeviceInfo requires a length-counted name string follows */
    struct {
        uint16_t length;
        const char name[6];
    } xkbGetDeviceName = { 4, "name" };
    xhiv_response device_name = {
        .length = 2, /* bytes_to_int32(strlen(xkbGetDeviceName)) */
        .response_data = &xkbGetDeviceName,
        .response_datalen = sizeof(xkbGetDeviceName),
        .flags = XHIV_NO_SET_SEQUENCE
    };
    /* Test overflow of .firstBtnWanted + .nBtnsWanted > .totalBtns */
    const xkbGetDeviceInfoReply rep1 = {
        .type = X_Reply,
        .length = device_name.length,
        .firstBtnWanted = 254,
        .nBtnsWanted = 254,
        .totalBtns = 1
    };
    xhiv_response response1 = {
        .next = &xkb_use_response,
        .chain = &device_name,
        .reqType = MY_XKB_EXT_CODE,
        .reqMinor = X_kbGetDeviceInfo,
        .sequence = 1,
        .length = (SIZEOF(xkbGetDeviceInfoReply) >> 2),
        .response_data = &rep1,
        .response_datalen = sizeof(rep1)
    };
    /* Test overflow of .firstBtnRtrn + .nBtnsRtrn > .totalBtns */
    const xkbGetDeviceInfoReply rep2 = {
        .type = X_Reply,
        .length = device_name.length,
        .firstBtnRtrn = 254,
        .nBtnsRtrn = 254,
        .totalBtns = 1
    };
    xhiv_response response2 = {
        .next = &response1,
        .chain = &device_name,
        .reqType = MY_XKB_EXT_CODE,
        .reqMinor = X_kbGetDeviceInfo,
        .sequence = 2,
        .length = (SIZEOF(xkbGetDeviceInfoReply) >> 2),
        .response_data = &rep2,
        .response_datalen = sizeof(rep2)
    };
    Display *dpy = XhivOpenDisplay(&response2);
    XkbDeviceInfoPtr dev;

    printf("XkbGetDeviceInfo: overflow test - btnsWanted\n");
    dev = XkbGetDeviceInfo(dpy, XkbXI_AllFeaturesMask, XkbUseCoreKbd, 0, 0);
    XkbFreeDeviceInfo(dev, XkbXI_AllFeaturesMask, xTrue);
    assert(dev == NULL);

    printf("XkbGetDeviceInfo: overflow test - btnsRtrn\n");
    dev = XkbGetDeviceInfo(dpy, XkbXI_AllFeaturesMask, XkbUseCoreKbd, 0, 0);
    XkbFreeDeviceInfo(dev, XkbXI_AllFeaturesMask, xTrue);
    assert(dev == NULL);

    XhivCloseDisplay(dpy);
}

int
main(int argc, char **argv)
{
    testOverflowFields();
    printf("XkbGetDeviceInfo: all tests passed\n");
    return 0;
}
