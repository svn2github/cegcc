TARGET=arm-wince-mingw32ce
LDFLAGS=-lws2 -lPipeLib
CFLAGS=-O0 -g3
WARNFLAGS=-Wall -Wextra

INCLUDES=

ALLFLAGS=$(CFLAGS) $(INCLUDES) $(WARNFLAGS)

CC=$(TARGET)-gcc

BINS = rshd.exe tester.exe
STRIPPED_BINS = rshd-stripped.exe tester-stripped.exe

LIBS    =
TARGETS = $(BINS) $(LIBS)

srcdir=.
distdir=rshd-0.1.0
TAR = tar
TARFLAGS = z
TARFILEEXT = .tar.gz

SRCDIST_FILES=\
	rshd.c tester.c Makefile README TODO COPYING ChangeLog

BINDIST_FILES=\
	rshd.exe

all:  $(TARGETS)
bins: $(BINS)
libs: $(LIBS)

rshd.exe: rshd.c Makefile
	$(CC) -o $@ $< $(ALLFLAGS) $(LDFLAGS)

rshd-stripped.exe: rshd.exe
	$(TARGET)-strip $< -o $@

tester.exe: tester.c Makefile
	$(CC) -o $@ $< $(ALLFLAGS)

tester-stripped.exe: tester.exe
	$(TARGET)-strip $< -o $@

download: rshd-stripped.exe tester-stripped.exe
	pcp rshd-stripped.exe ":/rshd.exe"
	pcp tester-stripped.exe ":/tester.exe"

clean:
	rm -f $(BINS) $(STRIPPED_BINS) $(LIBS)

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


.PHONY: all install download clean dist bindist srcdist
