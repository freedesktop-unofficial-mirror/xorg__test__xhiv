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
#include <X11/extensions/Xrender.h>
#include <X11/extensions/renderproto.h>

/* Common definitions for RENDER extension tests */

#define MY_XRENDER_EXT_CODE	222

/* Report that RENDER is present */
static const xQueryExtensionReply xrender_qext_reply = {
    .type = X_Reply,
    .length = 0,
    .present = xTrue,
    .major_opcode = MY_XRENDER_EXT_CODE,
    .first_event = 100,
    .first_error = 150
};
static xhiv_response xrender_qext_response = {
    .next = NULL,
    .reqType = X_QueryExtension,
    .reqMinor = XHIV_REQ_IGNORE,
    .sequence = XHIV_SEQ_MATCHDATA,
    .match_data = RENDER_NAME,
    .length = (SIZEOF(xQueryExtensionReply) >> 2),
    .response_data = &xrender_qext_reply,
    .response_datalen = sizeof(xrender_qext_reply)
};

/* Report current RENDER version 0.11 by default */
#ifndef MY_XRENDER_MAJOR_VERSION
# define MY_XRENDER_MAJOR_VERSION 0
#endif

#ifndef MY_XRENDER_MINOR_VERSION
# define MY_XRENDER_MINOR_VERSION 11
#endif

static const xRenderQueryVersionReply xrender_vers_reply = {
    .type = X_Reply,
    .length = 0,
    .majorVersion = MY_XRENDER_MAJOR_VERSION,
    .minorVersion = MY_XRENDER_MINOR_VERSION
};
static xhiv_response xrender_vers_response = {
    .next = &xrender_qext_response,
    .reqType = MY_XRENDER_EXT_CODE,
    .reqMinor = X_RenderQueryVersion,
    .sequence = XHIV_SEQ_IGNORE,
    .length = (SIZEOF(xRenderQueryVersionReply) >> 2),
    .response_data = &xrender_vers_reply,
    .response_datalen = sizeof(xrender_vers_reply)
};

/* XRenderQueryVersion calls XRenderQueryFormats */
static const xRenderQueryPictFormatsReply xrender_formats_reply = {
    .type = X_Reply,
    .length = 0,
    .numFormats = 0,
    .numScreens = 0,
    .numDepths = 0,
    .numVisuals = 0,
    .numSubpixel = 0,
};
static xhiv_response xrender_formats_response = {
    .next = &xrender_vers_response,
    .reqType = MY_XRENDER_EXT_CODE,
    .reqMinor = X_RenderQueryPictFormats,
    .sequence = XHIV_SEQ_IGNORE,
    .length = (SIZEOF(xRenderQueryPictFormatsReply) >> 2),
    .response_data = &xrender_formats_reply,
    .response_datalen = sizeof(xrender_formats_reply)
};
