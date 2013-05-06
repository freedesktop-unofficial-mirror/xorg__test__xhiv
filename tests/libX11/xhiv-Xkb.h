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
#include <X11/extensions/XKBproto.h>

/* Common definitions for XKB extension tests */

#define MY_XKB_EXT_CODE         200

/* Report that XKB is present */
static const xQueryExtensionReply xkb_qext_reply = {
    .type = X_Reply,
    .length = 0,
    .present = xTrue,
    .major_opcode = MY_XKB_EXT_CODE,
    .first_event = 100,
    .first_error = 150
};
static xhiv_response xkb_qext_response = {
    .next = NULL,
    .reqType = X_QueryExtension,
    .reqMinor = XHIV_REQ_IGNORE,
    .sequence = XHIV_SEQ_MATCHDATA,
    .match_data = XkbName,
    .length = (SIZEOF(xQueryExtensionReply) >> 2),
    .response_data = &xkb_qext_reply,
    .response_datalen = sizeof(xkb_qext_reply)
};

/* Report which version of XKB we support */
static const xkbUseExtensionReply xkb_use_reply = {
    .type = X_Reply,
    .supported = xTrue,
    .serverMajor = 1,
    .serverMinor = 0
};
static xhiv_response xkb_use_response = {
    .next = &xkb_qext_response,
    .reqType = MY_XKB_EXT_CODE,
    .reqMinor = X_kbUseExtension,
    .sequence = XHIV_SEQ_IGNORE,
    .length = (SIZEOF(xkbUseExtensionReply) >> 2),
    .response_data = &xkb_use_reply,
    .response_datalen = sizeof(xkb_use_reply)
};
