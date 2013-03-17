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

#ifndef XHIV_H
#define XHIV_H

#include <inttypes.h>
#include <sys/types.h>

/* Structures definining data to send to client after given requests */
#define XHIV_REQ_IGNORE        1024   /* Match only seq number, not req code */
#define XHIV_REQ_CONN_SETUP    1025   /* Initial handshake */

#define XHIV_SEQ_IGNORE  0xFFFFFFFF   /* Match only req code, not seq number */

#define XHIV_NO_SET_SEQUENCE   0x01   /* Don't set sequence number in reply */

typedef struct xhiv_response {
    struct xhiv_response *next;
    uint16_t reqType;  /* Request code, extension number or XHIV_REQ constant */
    uint16_t reqMinor; /* Extension minor request code */
    uint32_t sequence; /* Sequence number, or XHIV_SEQ_IGNORE */
    uint32_t length;   /* Total length of reply packet, in 4-byte words */
    const void *response_data;  /* Data to return */
    uint32_t response_datalen;  /* Length of response_data, in bytes */
    /* Response will be filled with random data to fill the difference
       between response_datalen & length */
    uint32_t flags;   /* flags controlling various options */
} xhiv_response;

/* Fork a server process and return the string needed to connect to it. */
extern char *XhivOpenServer(xhiv_response *responses, pid_t *return_pid);

/* Open a Xlib display connection to a new Xhiv server */
#include <X11/Xlib.h>
extern Display *XhivOpenDisplay(xhiv_response *responses);
extern int XhivCloseDisplay(Display *dpy);

#ifdef HAVE_XCB
/* Open an xcb display connection to a new Xhiv server */
#include <xcb/xcb.h>
extern xcb_connection_t *xhiv_connect(xhiv_response *responses);
extern int xhiv_disconnect(xcb_connection_t *conn);
#endif

#endif /* XHIV_H */
