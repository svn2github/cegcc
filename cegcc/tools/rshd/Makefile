TARGET=arm-wince-mingw32ce

LDFLAGS=
CFLAGS=-O0 -g3 
WARNFLAGS=-Wall -Wextra

PIPELIBDIR=../PipeLib
LIBS=-lws2 -L$(PIPELIBDIR) -lPipeLib
INCLUDES=-I$(PIPELIBDIR)

ALLFLAGS=$(CFLAGS) $(INCLUDES) $(WARNFLAGS)

CC=$(TARGET)-gcc
STRIP=$(TARGET)-strip

UNSTRIPPED_BINS = rshd_unstripped.exe tester_unstripped.exe
BINS = rshd.exe tester.exe

OBJECTS=rshd.o tester.o

srcdir=.
distdir=rshd-0.1.0
TAR = tar
TARFLAGS = z
TARFILEEXT = .tar.gz

SRCDIST_FILES=\
	rshd.c tester.c Makefile README TODO COPYING ChangeLog

BINDIST_FILES=\
	rshd.exe

all: $(BINS)

rshd_unstripped.exe: rshd.c Makefile
	$(CC) -o $@ $< $(ALLFLAGS) $(LDFLAGS) $(LIBS)

rshd.exe: rshd_unstripped.exe
	$(STRIP) $< -o $@

tester_unstripped.exe: tester.c Makefile
	$(CC) -o $@ $< $(ALLFLAGS)

tester.exe: tester_unstripped.exe
	$(TARGET)-strip $< -o $@

download: download-rshd

download-rshd: rshd.exe
	pcp $< ":/rshd.exe"

download-tester: tester.exe
	pcp $< ":/tester.exe"

clean:
	rm -f $(OBJECTS) $(BINS) $(UNSTRIPPED_BINS)

dist: srcdist bindist

srcdist: all
	rm -rf $(distdir)
	mkdir $(distdir)
	chmod 755 $(distdir)
	for i in $(SRCDIST_FILES); do \
		cp -p $(srcdir)/$$i $(distdir)/$$i ; \
	done
	rm -f $(distdir).tar.gz
	$(TAR) $(TARFLAGS)cf $(distdir)-src$(TARFILEEXT) $(distdir)

bindist: all
	rm -rf $(distdir)
	mkdir $(distdir)
	chmod 755 $(distdir)
	for i in $(BINDIST_FILES); do \
		cp -p $(srcdir)/$$i $(distdir)/$$i ; \
	done
	rm -f $(distdir).tar.gz
	$(TAR) $(TARFLAGS)cf $(distdir)-bin$(TARFILEEXT) $(distdir)


.PHONY: all install download download-rshd download-tester clean dist bindist srcdist
