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
#include <X11/extensions/sync.h>
#include <X11/extensions/syncproto.h>
#include <assert.h>
#include <stdio.h>
#include <limits.h>

#define MY_SYNC_EXT_CODE 208

static void
testOverflowFields(void)
{
    /* Report that SYNC is present */
    const xQueryExtensionReply syncqext_reply = {
        .type = X_Reply,
        .length = 0,
        .present = xTrue,
        .major_opcode = MY_SYNC_EXT_CODE,
        .first_event = 100,
        .first_error = 150
    };
    xhiv_response syncqext_response = {
        .next = NULL,
        .reqType = X_QueryExtension,
        .reqMinor = XHIV_REQ_IGNORE,
        .sequence = XHIV_SEQ_MATCHDATA,
        .match_data = SYNC_NAME,
        .length = (SIZEOF(xQueryExtensionReply) >> 2),
        .response_data = &syncqext_reply,
        .response_datalen = sizeof(syncqext_reply)
    };

    const xSyncInitializeReply syncvers_reply = {
        .type = X_Reply,
        .length = 0,
        .majorVersion = SYNC_MAJOR_VERSION,
        .minorVersion = SYNC_MINOR_VERSION
    };
    xhiv_response syncvers_response = {
        .next     = &syncqext_response,
        .reqType  = MY_SYNC_EXT_CODE,
        .reqMinor = X_SyncInitialize,
        .sequence = XHIV_SEQ_IGNORE,
        .length   = (SIZEOF(xSyncInitializeReply) >> 2),
        .response_data = &syncvers_reply,
        .response_datalen = sizeof(syncvers_reply)
    };
 
    const xSyncListSystemCountersReply rep1 = {
        .type    = X_Reply,
        .length  = 128,
        .nCounters = (INT_MAX / sizeof(XSyncSystemCounter)) + 4
    };
    xhiv_response response1 = {
        .next     = &syncvers_response,
        .reqType  = MY_SYNC_EXT_CODE,
        .reqMinor = X_SyncListSystemCounters,
        .sequence = XHIV_SEQ_IGNORE,
        .length   = (sizeof(rep1) >> 2) + rep1.length,
        .response_data = &rep1,
        .response_datalen = sizeof(rep1)
    };
    Display *dpy = XhivOpenDisplay(&response1);
    int major = SYNC_MAJOR_VERSION, minor = SYNC_MINOR_VERSION;
    int status, ncounters;
    XSyncSystemCounter *counters;

    status = XSyncInitialize(dpy, &major, &minor);
    assert(status == True);

    printf("XSyncListSystemCounters: overflow number of counters test: 0x%lx\n",
           (unsigned long) rep1.nCounters);
    counters = XSyncListSystemCounters(dpy, &ncounters);
    XSyncFreeSystemCounterList(counters);
    assert(counters == NULL);

    XhivCloseDisplay(dpy);
}

int
main(int argc, char **argv)
{
    testOverflowFields();
    printf("XSyncListSystemCounters: all tests passed\n");
    return 0;
}
