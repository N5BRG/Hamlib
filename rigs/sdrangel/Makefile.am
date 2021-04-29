AM_CFLAGS += $(LIBUSB_CFLAGS)

AM_LDFLAGS = -g -L/usr/local/lib -lcurl -lcurlpp -lftdi -Wl,-rpath -Wl,LIBDIR

SDRANGEL_SRC = sdrangel_rtlsdr.h sdrangel_rtlsdr.c sdrangel_hackrf.h sdrangel_hackrf.c sdrangel_pluto.h sdrangel_pluto.c relay.c sdrangel.h sdrangel.c

# automake gets confused and invokes the C++ linker via libtool regardless
# of whether or not HAVE_USRP enables the .cc source.  This override forces
# automake to invoke the C linker as no C++ is involved:
libhamlib_sdrangel_LINK = $(LINK) $(AM_LDFLAGS)

noinst_LTLIBRARIES = libhamlib-sdrangel.la
libhamlib_sdrangel_la_SOURCES = $(SDRANGEL_SRC) 
libhamlib_sdrangel_la_LIBADD = $(LIBUSB_LIBS) $(MATH_LIBS)

