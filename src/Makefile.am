noinst_LIBRARIES = libXhiv.a

libXhiv_a_SOURCES = \
        proto.h \
        xcb_client.c \
        xlib_client.c \
        xstrans.c \
        server.c

AM_CFLAGS = $(XHIV_CFLAGS) $(CWARNFLAGS) $(LIBX11_CFLAGS) $(LIBXCB_CFLAGS)

AM_CPPFLAGS = -I$(top_builddir) -I$(top_srcdir)/include
