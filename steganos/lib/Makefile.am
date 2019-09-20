## Process this file with automake to produce Makefile.in

INCLUDES = -I$(top_srcdir)/include -I$(top_srcdir)/steganos/include -I$(top_srcdir)/steganos/lib @OGG_CFLAGS@

lib_LTLIBRARIES = libsteganos.la

libsteganos_la_SOURCES = protocols.c steganos_channel.c cryptos_channel.c\
		         miscellaneous.c numbers.c global_types.h steganos_types.h\
                         cryptos_types.h protocols.h steganos_channel.h\
			 cryptos_channel.h miscellaneous.h numbers.h codec.h

libsteganos_la_LDFLAGS = -no-undefined -version-info @VE_LIB_CURRENT@:@VE_LIB_REVISION@:@VE_LIB_AGE@
libsteganos_la_LIBADD = @OGG_LIBS@ @STEGANOS_LIBS@


debug:
	$(MAKE) all CFLAGS="@DEBUG@"

profile:
	$(MAKE) all CFLAGS="@PROFILE@"

vorbistegdebug:
	$(MAKE) all CFLAGS="@VORBISTEGDEBUG@"

vorbisteg:
	$(MAKE) all CFLAGS="@VORBISTEG@"