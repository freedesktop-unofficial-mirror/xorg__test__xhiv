noinst_LIBRARIES = libXhiv.a

libXhiv_a_SOURCES = \
        os.h \
        proto.h \
        xlib_client.c \
        xstrans.c \
        server.c

if HAVE_XCB
 libXhiv_a_SOURCES += xcb_client.c
endif

AM_CFLAGS = $(XHIV_CFLAGS) $(CWARNFLAGS) $(LIBX11_CFLAGS) $(LIBXCB_CFLAGS)

AM_CPPFLAGS = -I$(top_builddir) -I$(top_srcdir)/include
