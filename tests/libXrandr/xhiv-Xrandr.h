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
#include <X11/Xproto.h>
#include <X11/extensions/Xrandr.h>
#include <X11/extensions/randrproto.h>

/* Common definitions for RANDR extension tests */

#define MY_XRANDR_EXT_CODE	220

/* Report that RANDR is present */
static const xQueryExtensionReply xrandr_qext_reply = {
    .type = X_Reply,
    .length = 0,
    .present = xTrue,
    .major_opcode = MY_XRANDR_EXT_CODE,
    .first_event = 100,
    .first_error = 150
};
static xhiv_response xrandr_qext_response = {
    .next = NULL,
    .reqType = X_QueryExtension,
    .reqMinor = XHIV_REQ_IGNORE,
    .sequence = XHIV_SEQ_MATCHDATA,
    .match_data = RANDR_NAME,
    .length = (SIZEOF(xQueryExtensionReply) >> 2),
    .response_data = &xrandr_qext_reply,
    .response_datalen = sizeof(xrandr_qext_reply)
};

/* Report current RANDR version 1.4 by default */
#ifndef MY_XRANDR_MAJOR_VERSION
# define MY_XRANDR_MAJOR_VERSION 1
#endif

#ifndef MY_XRANDR_MINOR_VERSION
# define MY_XRANDR_MINOR_VERSION 4
#endif

static const xRRQueryVersionReply xrandr_vers_reply = {
    .type = X_Reply,
    .length = 0,
    .majorVersion = MY_XRANDR_MAJOR_VERSION,
    .minorVersion = MY_XRANDR_MINOR_VERSION
};
static xhiv_response xrandr_vers_response = {
    .next = &xrandr_qext_response,
    .reqType = MY_XRANDR_EXT_CODE,
    .reqMinor = X_RRQueryVersion,
    .sequence = XHIV_SEQ_IGNORE,
    .length = (SIZEOF(xRRQueryVersionReply) >> 2),
    .response_data = &xrandr_vers_reply,
    .response_datalen = sizeof(xrandr_vers_reply)
};
