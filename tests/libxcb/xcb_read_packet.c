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
#include <xcb/xcb.h>
#include <xcb/xproto.h>
#include <assert.h>
#include <stdio.h>
#include <limits.h>

/* Tests the basic packet reading code in libxcb's xcb_in.c */

static void
testOverflowFields(void)
{
    const xcb_get_motion_events_reply_t rep1 = {
        .response_type  = XCB_GET_MOTION_EVENTS,
        .length         = (UINT_MAX / 4) + 12,
        .events_len     = (UINT_MAX / 2) + 6
    };
    xhiv_response response1 = {
        .reqType  = XCB_GET_MOTION_EVENTS,
        .reqMinor = XHIV_REQ_IGNORE,
        .sequence = XHIV_SEQ_IGNORE,
        .length   = (sizeof(rep1) >> 2) + rep1.length,
        .response_data = &rep1,
        .response_datalen = sizeof(rep1)
    };
    xcb_connection_t *conn = xhiv_connect(&response1);
    xcb_get_motion_events_cookie_t cookie;
    xcb_get_motion_events_reply_t *rep;

    printf("xcb read_packet: overflow reply length test\n");
    cookie = xcb_get_motion_events (conn, 0, 0, 0);
    rep = xcb_get_motion_events_reply(conn, cookie, NULL);
    assert(rep == NULL);

    xhiv_disconnect(conn, xhiv_conn_error_allowed);
}

int
main(int argc, char **argv)
{
    testOverflowFields();
    printf("xcb read_packet: all tests passed\n");
    return 0;
}
