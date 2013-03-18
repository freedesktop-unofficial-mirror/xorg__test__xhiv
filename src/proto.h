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

/* Fake extension protocol between xhiv client & server */

#include <inttypes.h>

/* Major op code (reqType) */
#define X_XHIV_PROTO_REQTYPE  254

/* Minor op code (reqMinor) */
#define XhivSeqStart            0 /* start sequence counting */

/*
 * Force the sequence number used for matching canned request replies
 * to be set to the value specified in this request.  (i.e. if this
 * request has sequence 0, the next one will be treated as 1 & so on).
 */
typedef struct {
    uint8_t     reqType;        /* XHIV_PROTO_REQTYPE */
    uint8_t     reqMinor;       /* XHIV_PROTO_SEQSTART */
    uint16_t    length;         /* 2 - no more data needed */
    uint32_t    sequence;       /* treat this request as having this seq no. */
} xXhivSeqStartReq;
#define sz_xXhivSeqStartReq 4

/* internal API - not really proto */
extern int XhivWaitServer(pid_t server_pid);
