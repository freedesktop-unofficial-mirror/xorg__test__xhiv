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
#include <X11/extensions/XI.h>
#include <X11/extensions/XIproto.h>
#include <X11/extensions/XI2proto.h>

/* Common definitions for Xinput extension tests */

#define MY_XI_EXT_CODE	218

/* Report that Xi is present */
static const xQueryExtensionReply xi_qext_reply = {
    .type = X_Reply,
    .length = 0,
    .present = xTrue,
    .major_opcode = MY_XI_EXT_CODE,
    .first_event = 100,
    .first_error = 150
};
static xhiv_response xi_qext_response = {
    .next = NULL,
    .reqType = X_QueryExtension,
    .reqMinor = XHIV_REQ_IGNORE,
    .sequence = XHIV_SEQ_MATCHDATA,
    .match_data = INAME,
    .length = (SIZEOF(xQueryExtensionReply) >> 2),
    .response_data = &xi_qext_reply,
    .response_datalen = sizeof(xi_qext_reply)
};

/* Report default Xinput 1.x version */
#ifndef MY_XI_MAJOR_VERSION
# define MY_XI_MAJOR_VERSION 1
#endif

#ifndef MY_XI_MINOR_VERSION
# define MY_XI_MINOR_VERSION 5
#endif

static const xGetExtensionVersionReply xi_vers_reply = {
    .repType = X_Reply,
    .RepType = X_GetExtensionVersion,
    .length = 0,
    .major_version = MY_XI_MAJOR_VERSION,
    .minor_version = MY_XI_MINOR_VERSION,
    .present = xTrue
};
static xhiv_response xi_vers_response = {
    .next = &xi_qext_response,
    .reqType = MY_XI_EXT_CODE,
    .reqMinor = X_GetExtensionVersion,
    .sequence = XHIV_SEQ_IGNORE,
    .length = (SIZEOF(xQueryExtensionReply) >> 2),
    .response_data = &xi_vers_reply,
    .response_datalen = sizeof(xi_vers_reply)
};


/* Report default Xinput 2.x version */
#ifndef MY_XI2_MAJOR_VERSION
# define MY_XI2_MAJOR_VERSION 2
#endif

#ifndef MY_XI2_MINOR_VERSION
# define MY_XI2_MINOR_VERSION 2
#endif

static const xXIQueryVersionReply xi2_vers_reply = {
    .repType = X_Reply,
    .RepType = X_XIQueryVersion,
    .length = 0,
    .major_version = MY_XI2_MAJOR_VERSION,
    .minor_version = MY_XI2_MINOR_VERSION
};
static xhiv_response xi2_vers_response = {
    .next = &xi_vers_response,
    .reqType = MY_XI_EXT_CODE,
    .reqMinor = X_XIQueryVersion,
    .sequence = XHIV_SEQ_IGNORE,
    .length = (SIZEOF(xXIQueryVersionReply) >> 2),
    .response_data = &xi2_vers_reply,
    .response_datalen = sizeof(xi2_vers_reply)
};


/* Provide a simple default device for XOpenDevice */
#define MY_DEFAULT_DEVICE 1
static const xOpenDeviceReply xi_opendev_reply = {
    .repType = X_Reply,
    .RepType = X_OpenDevice,
    .length = sizeof(xInputClassInfo),
    .num_classes = 1
};
static xhiv_response xi_opendev_response = {
    .next = &xi_vers_response,
    .reqType = MY_XI_EXT_CODE,
    .reqMinor = X_OpenDevice,
    .sequence = XHIV_SEQ_IGNORE,
    .length = (SIZEOF(xOpenDeviceReply) >> 2) + sizeof(xInputClassInfo),
    .response_data = &xi_opendev_reply,
    .response_datalen = sizeof(xi_opendev_reply)
};
