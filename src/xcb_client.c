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
#include "proto.h"
#include <xcb/xcbext.h>
#include <assert.h>
#include <sys/types.h>

static pid_t server_pid;

static struct xcb_extension_t xcb_xhiv_id = {
    .name = "XHIV",
    .global_id = X_XHIV_PROTO_REQTYPE
};

static const xcb_protocol_request_t xcb_req = {
    .count  = 1,
    .ext    = &xcb_xhiv_id,
    .opcode = XhivSeqStart,
    .isvoid = 1
};

xcb_connection_t *
xhiv_connect(xhiv_response *responses) {
    char *displayname;
    xcb_connection_t *conn;
    int screen;

    displayname = XhivOpenServer(responses, &server_pid);
    assert(displayname != NULL);

    conn = xcb_connect(displayname, &screen);
    assert(conn != NULL);
    assert(screen == 0);

    xhiv_sequence_sync(conn, 0);

    return conn;
}

void
xhiv_sequence_sync(xcb_connection_t *conn, uint32_t seq) {
    xXhivSeqStartReq xssreq = {
        .reqType  = X_XHIV_PROTO_REQTYPE,
        .reqMinor = XhivSeqStart,
        .length   = 2,
        .sequence = seq
    };
    struct iovec xcb_parts[4] = {
        [2] = { .iov_base = &xssreq, .iov_len = sizeof(xssreq) },
        [3] = { .iov_base = 0, .iov_len = 0 /* no padding needed */ }
    };

    xcb_send_request(conn, XCB_REQUEST_RAW, xcb_parts + 2, &xcb_req);
}

int
xhiv_disconnect(xcb_connection_t *conn) {
    pid_t waitfor;

    assert(xcb_connection_has_error(conn) == 0);
    xcb_disconnect(conn);

    waitfor = server_pid;
    server_pid = -1;
    return XhivWaitServer(waitfor);
}
