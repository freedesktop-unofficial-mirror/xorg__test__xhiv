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

#define XSERV_t
#define TRANS_SERVER
#include <X11/Xtrans/Xtrans.h>
#include <X11/Xtrans/Xtransint.h>
#include <X11/X.h>
#include <X11/Xproto.h>
#include <X11/extensions/bigreqsproto.h>

#include <sys/types.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <poll.h>
#include <assert.h>
#include <errno.h>
#include <limits.h>
#include <sys/wait.h>

#ifndef O_CLOEXEC
# define O_CLOEXEC 0
#endif

/* Data waiting to be written to clients */
typedef struct client_response_buffer {
    struct client_response_buffer *next;
    const void *response_data;  /* Data to return */
    uint32_t response_datalen;  /* Length of response_data, in bytes */
    uint32_t length;      /* Total length of reply packet, in 4-byte words */
    uint64_t response_written;  /* Number of bytes written so far */
    uint32_t response_sequence; /* Sequence number to set or XHIV_SEQ_IGNORE */
} client_response_buffer;

typedef struct client_state {
    XtransConnInfo conn;
    struct client_response_buffer *crb;
    uint32_t sequence;          /* sequence reported to client */
    uint32_t match_sequence;    /* sequence used for matching responses */
    uint32_t req_len_remaining; /* remaining length to discard from last req */
} client_state;

/*************************************************************************
 * Data for server/client handshake
 */

#define ARRAY_SIZE(a)           (sizeof((a)) / sizeof((a)[0]))
#define bytes_to_int32(b)       (((b) + 3) >> 2)

static const char default_vendor_string[] = PACKAGE_STRING;

static const xConnSetup default_conn_setup = {
    .release = (PACKAGE_VERSION_MAJOR << 24) |
               (PACKAGE_VERSION_MINOR << 16) |
               (PACKAGE_VERSION_PATCHLEVEL << 8),
    .ridBase = 0x00200000,
    .ridMask = 0x001fffff,
    .motionBufferSize = 256,
    .nbytesVendor = sizeof(default_vendor_string),
    .maxRequestSize = SHRT_MAX,
    .numRoots = 1,
    .numFormats = 7, /* must be same as default_pixmap_formats below */
    .imageByteOrder = LSBFirst,
    .bitmapBitOrder = LSBFirst,
    .bitmapScanlineUnit = 32,
    .bitmapScanlinePad = 32,
    .minKeyCode = 8,
    .maxKeyCode = 255,
};

static const xPixmapFormat default_pixmap_formats[7] = {
    /* depth, bits-per-pixel, scanline-pad */
    {  1,  1, 32 },
    {  4,  4, 32 },
    {  8,  8, 32 },
    { 15, 15, 32 },
    { 16, 16, 32 },
    { 24, 24, 32 },
    { 32, 32, 32 }
};

static const xWindowRoot default_root_window = {
    .windowId = 0x47,
    .defaultColormap = 0x20,
    .whitePixel = 0x00ffffffU,
    .blackPixel = 0,
    .currentInputMask = 0,
    .pixWidth = 1024,
    .pixHeight = 768,
#define pixels_to_mm(p) (((p) * 25.4) / 96)
    .mmWidth = pixels_to_mm(1024),
    .mmHeight = pixels_to_mm(768),
    .minInstalledMaps = 1,
    .maxInstalledMaps = 1,
    .rootVisualID = 0x21,
    .backingStore = NotUseful,
    .saveUnders = xFalse,
    .rootDepth = 24,
    .nDepths = 7 /* must be same as default_depths below */
};

static const xDepth default_depths[7] = {
    { .depth =  1, .nVisuals = 0 },
    { .depth =  4, .nVisuals = 0 },
    { .depth =  8, .nVisuals = 0 },
    { .depth = 15, .nVisuals = 0 },
    { .depth = 16, .nVisuals = 0 },
    { .depth = 32, .nVisuals = 0 },
    { .depth = 24, .nVisuals = 1 }, /* must be same as default_visuals below */
};

static const xVisualType default_visuals[1] = {
    {
        .visualID = 0x21,
        .class = TrueColor,
        .bitsPerRGB = 8,
        .colormapEntries = 256,
        .redMask   = 0x000000ff,
        .greenMask = 0x0000ff00,
        .blueMask  = 0x00ff0000
    }
};

static const xConnSetupPrefix default_conn_setup_prefix = {
    .success = xTrue,
    .lengthReason = 0,
    .majorVersion = X_PROTOCOL,
    .minorVersion = X_PROTOCOL_REVISION,
    .length = bytes_to_int32(sizeof(xConnSetup) +
                             sizeof(default_pixmap_formats) +
                             sizeof(default_root_window) +
                             sizeof(default_depths) +
                             sizeof(default_visuals) ) +
              bytes_to_int32(sizeof(default_vendor_string))
};

static const xhiv_response default_conn_response[] = {
    {
        .length = bytes_to_int32(sz_xConnSetupPrefix),
        .response_data = &default_conn_setup_prefix,
        .response_datalen = sizeof(default_conn_setup_prefix),
        .flags = XHIV_NO_SET_SEQUENCE
    },

    {
        .length = bytes_to_int32(sz_xConnSetup),
        .response_data = &default_conn_setup,
        .response_datalen = sizeof(default_conn_setup),
        .flags = XHIV_NO_SET_SEQUENCE
    },

    {
        .length = bytes_to_int32(sizeof(default_vendor_string)),
        .response_data = default_vendor_string,
        .response_datalen = sizeof(default_vendor_string),
        .flags = XHIV_NO_SET_SEQUENCE
    },

    {
        .length = bytes_to_int32(sizeof(default_pixmap_formats)),
        .response_data = default_pixmap_formats,
        .response_datalen = sizeof(default_pixmap_formats),
        .flags = XHIV_NO_SET_SEQUENCE
    },

    {
        .length = bytes_to_int32(sizeof(default_root_window)),
        .response_data = &default_root_window,
        .response_datalen = sizeof(default_root_window),
        .flags = XHIV_NO_SET_SEQUENCE
    },

    {
        .length = bytes_to_int32(sizeof(default_depths)),
        .response_data = default_depths,
        .response_datalen = sizeof(default_depths),
        .flags = XHIV_NO_SET_SEQUENCE
    },

    {
        .length = bytes_to_int32(sizeof(default_visuals)),
        .response_data = default_visuals,
        .response_datalen = sizeof(default_visuals),
        .flags = XHIV_NO_SET_SEQUENCE
    },
};

/* In order to simulate BigRequests, we simply implement it with a hardcoded
   extension request value */
#define BIGREQ_REQTYPE          255

/*************************************************************************
 * Data management functions
 */

static client_response_buffer *
AddResponseToBuffer(client_response_buffer *crb, const xhiv_response *response,
                    uint32_t sequence)
{
    client_response_buffer *new_crb;

    uint64_t total_bytes= ((uint64_t) response->length) << 2;
    assert(total_bytes >= response->response_datalen);

    new_crb = calloc(1, sizeof(client_response_buffer));
    assert(new_crb != NULL);

    new_crb->response_data = response->response_data;
    new_crb->response_datalen = response->response_datalen;
    new_crb->length = response->length;
    new_crb->response_sequence = (response->flags & XHIV_NO_SET_SEQUENCE)
        ? XHIV_SEQ_IGNORE : sequence;

    if (crb == NULL)
        crb = new_crb;
    else {
        client_response_buffer *n;

        for (n = crb ; n->next != NULL; n = n->next) {
            /* find end of list */
        }
        n->next = new_crb;
    }
    return crb;
}

/* Find the first response matching the criteria */
static xhiv_response *
FindXhivResponse(xhiv_response *xrlist, uint16_t reqType, uint16_t reqMinor,
                 uint32_t sequence)
{
    xhiv_response *r;

    for (r = xrlist; r != NULL ; r = r->next) {
        if (((r->reqType == reqType) || (r->reqType == XHIV_REQ_IGNORE)) &&
            ((r->reqMinor == reqMinor) || (r->reqMinor == XHIV_REQ_IGNORE)) &&
            ((r->sequence == sequence) || (r->sequence == XHIV_SEQ_IGNORE)))
            return r;
    }
    return NULL;
}


/*************************************************************************
 * Client communication functions
 */

static void
CloseListenTrans(XtransConnInfo *ListenTransConns, int ListenTransCount)
{
    int i;

    for (i = 0; i < ListenTransCount; i++)
        _XSERVTransClose(ListenTransConns[i]);
}

static XtransConnInfo
WaitForClient(XtransConnInfo *ListenTransConns, int ListenTransCount)
{
    struct pollfd *pollfds;
    int i;
    XtransConnInfo ClientTransConn = NULL;

    pollfds = calloc(ListenTransCount, sizeof(struct pollfd));
    assert (pollfds != NULL);

    for (i = 0; i < ListenTransCount; i++) {
        pollfds[i].fd = _XSERVTransGetConnectionNumber(ListenTransConns[i]);
        pollfds[i].events = POLLIN;
    }

    while (ClientTransConn == NULL) {
        int readyfds = poll(pollfds, ListenTransCount, -1);

        if (readyfds > 0) {
            for (i = 0; i < ListenTransCount; i++) {
                if (pollfds[i].revents & (POLLERR | POLLHUP | POLLNVAL)) {
                    perror("bad state polling for client connection");
                    exit(11);
                }
                else if (pollfds[i].revents) {
                    int status;
                    ClientTransConn =
                        _XSERVTransAccept(ListenTransConns[i], &status);
                    if (ClientTransConn)
                        break;
                }
            }
        }
        else if (readyfds < 0) { /* error */
            if (errno != EAGAIN && errno != EINTR) {
                perror("polling for client connection");
                exit(11);
            }
        }
    }

    free(pollfds);
    CloseListenTrans(ListenTransConns, ListenTransCount);
    return ClientTransConn;
}

static int urandom_fd = -1;

static void
HandleClientResponses(client_state *client)
{
    while (client->crb != NULL) {
        client_response_buffer *crb = client->crb;
        uint64_t nbytes, wbytes, total_bytes;

        if ((crb->response_written == 0) &&
            (crb->response_sequence != XHIV_SEQ_IGNORE)) {
            /* Set sequence in initial bytes if needed */
            xGenericReply rep;

#ifdef DEBUG
            printf("Sending reply: seq = %d, length = %d\n",
                   crb->response_sequence, crb->length);
#endif

            nbytes = sizeof(rep);
            if (nbytes > crb->response_datalen)
                nbytes = crb->response_datalen;

            memcpy(&rep, crb->response_data, nbytes);
            rep.sequenceNumber = (CARD16) crb->response_sequence;
            wbytes = _XSERVTransWrite(client->conn, (char *) &rep, nbytes);
            if (wbytes > 0)
                crb->response_written += wbytes;
        }

        if (crb->response_written < crb->response_datalen) {
            nbytes = crb->response_datalen - crb->response_written;
            wbytes = _XSERVTransWrite(client->conn,
                (const char *) crb->response_data + crb->response_written,
                nbytes);
            if (wbytes > 0)
                crb->response_written += wbytes;
            if (wbytes != nbytes) /* pipe is full, try again later */
                return;
        }

        total_bytes = ((uint64_t) crb->length) << 2;
        nbytes = total_bytes - crb->response_written;
        if (nbytes > 0) {
            char ranbuf[32768];

            if (urandom_fd < 0) {
                urandom_fd = open("/dev/urandom",
                                  O_RDONLY | O_NONBLOCK | O_CLOEXEC);
                if (urandom_fd < 0) {
                    perror("Could not open /dev/urandom");
                    exit(11);
                }
            }

            /*
             * Read some random bytes to write to fill buffer.
             * If it fails, ignore the error and continue with whatever
             * (not truly random) uninitialized data is on our stack.
             */
            if (nbytes > sizeof(ranbuf))
                nbytes = sizeof(ranbuf);
            read(urandom_fd, ranbuf, nbytes);

            do {
                wbytes = _XSERVTransWrite(client->conn, ranbuf, nbytes);
                if (wbytes > 0)
                    crb->response_written += wbytes;
                if (wbytes != nbytes) /* pipe is full, try again later */
                    return;
                nbytes = total_bytes - crb->response_written;
                if (nbytes > sizeof(ranbuf))
                    nbytes = sizeof(ranbuf);
            } while (nbytes > 0);
        }

        /* Are we done with this response buffer?  If so, nuke it & move on. */
        if (total_bytes == crb->response_written) {
            client->crb = crb->next;
            free(crb);
        }
        else {
            assert(total_bytes > crb->response_written);
            return;  /* if we couldn't finish this one, wait for poll to say
                        the client is ready for more */
        }
    }
}

static unsigned char readbuf[65536];

static void
DiscardRequestData(client_state *client)
{
    /* Read & ignore all the data in the request we don't care about */
    while (client->req_len_remaining) {
        int nbytes = (client->req_len_remaining > sizeof(readbuf)) ?
            sizeof(readbuf) : client->req_len_remaining;
        int rbytes = _XSERVTransRead(client->conn, (char *)readbuf, nbytes);
        if (rbytes <= 0)
            break;
        client->req_len_remaining -= rbytes;
#ifdef DEBUG
        printf("Discarded %d bytes of request data\n", rbytes);
#endif
    }
}

static void
HandleClientRequest(client_state *client, xhiv_response *responses)
{
    if (client->sequence == 0) { /* handshaking */
        xhiv_response *r;
        int i = 0;

        client->req_len_remaining = sizeof(xConnClientPrefix);
        DiscardRequestData(client);

        while ((r = FindXhivResponse(responses, XHIV_REQ_CONN_SETUP,
                                     XHIV_REQ_IGNORE, i))) {
            client->crb = AddResponseToBuffer(client->crb, r, 0);
            i++;
        }

        if (i == 0) { /* use default connection sequence if none provided */
            for (i = 0; i < ARRAY_SIZE(default_conn_response); i++)
                client->crb = AddResponseToBuffer(client->crb,
                                                  &default_conn_response[i],
                                                  0);
        }
    } else { /* normal protocol request/reply cycle */
        xhiv_response *r;
        xReq req;
        int rbytes;
        uint32_t length;

        if (client->req_len_remaining) /* still reading last request */
            DiscardRequestData(client);
        if (client->req_len_remaining) /* not all there yet */
            return; /* back to poll again for more data */

        errno = 0;
        rbytes = _XSERVTransRead(client->conn, (char *)&req, sizeof(req));
        if ((rbytes == 0) && (errno == 0)) {
            /* client disconnected */
            _XSERVTransClose(client->conn);
            return;
        }
        if (rbytes <= 0) {
            if ((errno == EINTR) || (errno == EAGAIN))
                return;
            perror("Reading from client");
            exit(11);
        }
        assert(rbytes == sizeof(req));
        if (req.length == 0) { /* BIG Request */
            rbytes = _XSERVTransRead(client->conn, (char *)&length, 4);
            assert(rbytes == 4);
        }
        else
            length = req.length;

#ifdef DEBUG
        printf("Request %d (match seq %d): %d/%d\n", client->sequence,
               client->match_sequence, req.reqType, req.data);
#endif

        /* X11 packets count the initial header as part of their length */
        client->req_len_remaining = (length << 2) - sizeof(req);

        r = FindXhivResponse(responses, req.reqType, req.data,
                             client->match_sequence);
        if (r != NULL)
            client->crb = AddResponseToBuffer(client->crb, r,
                                              client->sequence);
        else {  /* If match not found, check against builtin responses */
            switch (req.reqType) {
            case X_QueryExtension:
                /* XOpenDisplay checks for BIG-REQUESTS & XKB extensions.
                   We only simulate BIG-REQUESTS for now */
                {
                    int nbytes = client->req_len_remaining;
                    char extension[32] = "";
                    xQueryExtensionReply qext_reply = {
                        .type = X_Reply,
                        .length = 0,
                        .present = xFalse,
                        .major_opcode = 0,
                        .first_event = 0,
                        .first_error = 0
                    };
                    xhiv_response qext_response = {
                        .length = bytes_to_int32(sz_xQueryExtensionReply),
                        .response_data = &qext_reply,
                        .response_datalen = sizeof(qext_reply)
                    };

                    if (nbytes > sizeof(extension))
                        nbytes = sizeof(extension);
                    rbytes = _XSERVTransRead(client->conn, (char *)&extension,
                                           nbytes);
                    if (rbytes > 0) {
                        assert(client->req_len_remaining >= rbytes);
                        client->req_len_remaining -= rbytes;
                        if (strncmp(extension + 4, XBigReqExtensionName,
                                    sizeof(XBigReqExtensionName)) == 0) {
                            qext_reply.present = xTrue;
                            qext_reply.major_opcode = BIGREQ_REQTYPE;
                        }
                    }
                    else {
                        assert(rbytes == 0);
                    }
                    client->crb =
                        AddResponseToBuffer(client->crb, &qext_response,
                                            client->sequence);
                }
                break;

            case X_GetProperty:
                /* XOpenDisplay requests the root window resource property.
                   We just claim all properties don't exist. */
                {
                    const xGetPropertyReply getp_reply = {
                        .type = X_Reply,
                        .format = 0,
                        .length = 0,
                        .propertyType = None,
                        .bytesAfter = 0,
                        .nItems = 0
                    };
                    xhiv_response getp_response = {
                        .length = bytes_to_int32(sz_xGetPropertyReply),
                        .response_data = &getp_reply,
                        .response_datalen = sizeof(getp_reply)
                    };
                    client->crb =
                        AddResponseToBuffer(client->crb, &getp_response,
                                            client->sequence);
                }
                break;

            case X_GetInputFocus:
                /* XSync() sends this request to force a quick reply */
                {
                    const xGetInputFocusReply getif_reply = {
                        .type = X_Reply,
                        .revertTo = None,
                        .length = 0,
                        .focus = None
                    };
                    xhiv_response getif_response = {
                        .length = bytes_to_int32(sz_xGetInputFocusReply),
                        .response_data = &getif_reply,
                        .response_datalen = sizeof(getif_reply)
                    };
                    client->crb =
                        AddResponseToBuffer(client->crb, &getif_response,
                                            client->sequence);
                }
                break;

            case X_XHIV_PROTO_REQTYPE: /* our fake extension */
                if (req.data == XhivSeqStart) {
                    uint32_t newseq;
                    rbytes = _XSERVTransRead(client->conn, (char *)&newseq,
                                             sizeof(uint32_t));
                    assert(rbytes == sizeof(uint32_t));
                    client->req_len_remaining -= rbytes;

                    client->match_sequence = newseq;
#ifdef DEBUG
                    printf("Set match sequence to %d\n", newseq);
#endif
                }
                break;

            case BIGREQ_REQTYPE: /* our fake BIG-REQUESTS extension */
                if (req.data == X_BigReqEnable) {
                    const xBigReqEnableReply bigreq_reply = {
                        .type = X_Reply,
                        .length = 0,
                        .max_request_size = UINT32_MAX
                    };
                    xhiv_response bigreq_response = {
                        .length = bytes_to_int32(sz_xBigReqEnableReply),
                        .response_data = &bigreq_reply,
                        .response_datalen = sizeof(bigreq_reply)
                    };
                    client->crb =
                        AddResponseToBuffer(client->crb, &bigreq_response,
                                            client->sequence);
                }
                break;

            default:
#ifdef DEBUG
                printf("Discarded unhandled request type %d.%d (%d/%d)\n",
                       req.reqType, req.data,
                       client->match_sequence, client->sequence);
#endif
                break;
            }
        }
        /* don't need any more data from the request now */
        DiscardRequestData(client);
    }
    client->sequence++;
    client->match_sequence++;

    if (client->crb != NULL)
        HandleClientResponses(client);
}

/* main I/O loop of the server */
static void _X_NORETURN
XhivRunServer(XtransConnInfo *ListenTransConns, int ListenTransCount,
              xhiv_response *responses)
{
    struct pollfd clientfd = { .events = POLLIN };
    client_state client = { 0 };

    /* Just in case connection transport signals hangups w/ SIGPIPE */
    signal(SIGPIPE, SIG_IGN);

    /* Wait for a client to connect - when it does, the connections
       passed in are closed, and only the client socket remains open */
    client.conn = WaitForClient(ListenTransConns, ListenTransCount);
    clientfd.fd = _XSERVTransGetConnectionNumber(client.conn);
    _XSERVTransSetOption(client.conn, TRANS_NONBLOCKING, 1);

    for (;;) { /* repeat until client hangs up on us */
        int readyfds = poll(&clientfd, 1, -1);

        if (readyfds > 0) {
            if (clientfd.revents & (POLLERR | POLLHUP | POLLNVAL)) {
                /* assume client hungup, so we do too */
                break;
            }
            else {
                if (clientfd.revents & (POLLIN | POLLRDNORM))
                    HandleClientRequest(&client, responses);
                if (clientfd.revents & (POLLOUT | POLLWRNORM))
                    HandleClientResponses(&client);
            }
        }
        else if (readyfds < 0) { /* error */
            if (errno != EAGAIN && errno != EINTR) {
                perror("polling for client requests");
                exit(11);
            }
        }

        if (client.crb == NULL)
            clientfd.events = POLLIN;
        else
            clientfd.events = POLLIN | POLLOUT;
    }
    _XSERVTransClose(client.conn);
    exit(0);
}

int
XhivWaitServer(pid_t server_pid) {
    int pidstat;
    pid_t waitret;

    assert(server_pid > 0);

    while ((waitret = waitpid(server_pid, &pidstat, 0)) == -1) {
        if (errno != EAGAIN && errno != EINTR) {
            perror("waiting for server to exit");
            exit(11);
        }
    }
    if (WIFEXITED(pidstat)) {
        int exitstat = WEXITSTATUS(pidstat);

        if (exitstat != 0)
            fprintf(stderr, "Server %ld exited with %d.\n",
                    (long) server_pid, exitstat);

        return exitstat;
    } else if (WIFSIGNALED(pidstat)) {
        int sig = WTERMSIG(pidstat);
        char signame[SIG2STR_MAX];

        if (sig2str(sig, signame) == -1)
            snprintf(signame, sizeof(signame), "unknown");

        fprintf(stderr, "Server %ld killed by signal %d (%s)%s.\n",
                (long) server_pid, sig, signame,
                WCOREDUMP(pidstat) ? " -- core dumped" : "");
        return sig;
    } else {
        assert(pidstat);
    }
    return 0;
}

/*************************************************************************
 * External interface to this framework
 */

char *
XhivOpenServer(xhiv_response *responses, pid_t *return_pid)
{
    char *display;
    pid_t kidpid;
    int i, found;
    XtransConnInfo *ListenTransConns = NULL;
    int ListenTransCount = 0;

    /* TODO: configure nolisten options */

    /* find an unused socket to run on */
    for (i = 20, found = 0; i < 65535 - X_TCP_PORT; i++) {
        char port[8];
        int partial = 0;

        snprintf(port, sizeof(port), "%d", i);

        if (_XSERVTransMakeAllCOTSServerListeners(
                port, &partial, &ListenTransCount, &ListenTransConns) >= 0)
        {
            snprintf(port, sizeof(port), ":%d", i);
            display = strdup(port);
            found = 1;
            break;
        }
    }
    if (found == 0) {
        perror("Could not open server socket on any port");
        exit(11);
    }

    fflush(stdout);
    fflush(stderr);

    /* All tests should finish in less than 5 minutes - set timer to kill
       any processes that fail to notice their other half crashed */
    alarm(300);

    kidpid = fork();
    if (kidpid == 0) { /* child */
        free(display);
        XhivRunServer(ListenTransConns, ListenTransCount, responses);
    } else if (kidpid == -1) { /* error */
        perror("Fork of server process failed");
        exit(11);
    }
    /* else parent */
    CloseListenTrans(ListenTransConns, ListenTransCount);
    printf("Client %ld forked server child %ld on display %s\n",
           (long) getpid(), (long) kidpid, display);

    /* hack to allow sleeping long enough to attach debugger when needed */
    if (getenv("XHIV_SLEEP"))
        sleep(atoi(getenv("XHIV_SLEEP")));

    if (return_pid != NULL)
        *return_pid = kidpid;
    return display;
}
