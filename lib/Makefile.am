## Process this file with automake to produce Makefile.in

SUBDIRS = modes books

INCLUDES = -I$(top_srcdir)/include -I$(top_srcdir)/steganos/include -I$(top_srcdir)/steganos/lib @OGG_CFLAGS@

lib_LTLIBRARIES = libvorbis.la libvorbisfile.la libvorbisenc.la #libsteganos.la

libvorbis_la_SOURCES = mdct.c smallft.c block.c envelope.c window.c lsp.c \
			lpc.c analysis.c synthesis.c psy.c info.c \
			floor1.c floor0.c\
			res0.c mapping0.c registry.c codebook.c sharedbook.c\
			lookup.c bitrate.c\
			envelope.h lpc.h lsp.h codebook.h misc.h psy.h\
			masking.h os.h mdct.h smallft.h highlevel.h\
			registry.h scales.h window.h lookup.h lookup_data.h\
			codec_internal.h backends.h bitrate.h

libvorbis_la_LDFLAGS = -no-undefined -version-info @V_LIB_CURRENT@:@V_LIB_REVISION@:@V_LIB_AGE@ -DSTEGO
libvorbis_la_LIBADD = $(top_srcdir)/steganos/lib/libsteganos.la @VORBIS_LIBS@ @OGG_LIBS@ @STEGANOS_LIBS@

libvorbisfile_la_SOURCES = vorbisfile.c
libvorbisfile_la_LDFLAGS = -no-undefined -version-info @VF_LIB_CURRENT@:@VF_LIB_REVISION@:@VF_LIB_AGE@
libvorbisfile_la_LIBADD = libvorbis.la @OGG_LIBS@

libvorbisenc_la_SOURCES = vorbisenc.c 
libvorbisenc_la_LDFLAGS = -no-undefined -version-info @VE_LIB_CURRENT@:@VE_LIB_REVISION@:@VE_LIB_AGE@
libvorbisenc_la_LIBADD = libvorbis.la @OGG_LIBS@

EXTRA_PROGRAMS = barkmel tone psytune
CLEANFILES = $(EXTRA_PROGRAMS)

barkmel_SOURCES = barkmel.c
tone_SOURCES = tone.c
psytune_SOURCES = psytune.c
psytune_LDFLAGS = -static
psytune_LDADD = libvorbis.la

EXTRA_DIST = lookups.pl 

# build and run the self tests on 'make check'

#vorbis_selftests = test_codebook test_sharedbook
vorbis_selftests = test_sharedbook

noinst_PROGRAMS = $(vorbis_selftests)

check: $(noinst_PROGRAMS)
	./test_sharedbook$(EXEEXT)

#test_codebook_SOURCES = codebook.c
#test_codebook_CFLAGS = -D_V_SELFTEST

test_sharedbook_SOURCES = sharedbook.c
test_sharedbook_CFLAGS = -D_V_SELFTEST
test_sharedbook_LDADD = @VORBIS_LIBS@

# recurse for alternate targets

debug:
	$(MAKE) all CFLAGS="@DEBUG@"

profile:
	$(MAKE) all CFLAGS="@PROFILE@"

vorbistegdebug:
	$(MAKE) all CFLAGS="@VORBISTEGDEBUG@"

vorbisteg:
	$(MAKE) all CFLAGS="@VORBISTEG@"