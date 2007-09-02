TARGET=arm-wince-mingw32ce
CFLAGS=-O0 -g3
WARNFLAGS=-Wall -Wextra

INCLUDES=-I../errno -I../libcwd -I.
LDFLAGS=-L../errno -L../libcwd
LIBS=-lcwd -lerrno -lws2

ALLFLAGS=$(CFLAGS) $(INCLUDES) $(WARNFLAGS)

CC=$(TARGET)-gcc
STRIP=$(TARGET)-strip

BINS = rcp_unstripped.exe
STRIPPED_BINS = rcp.exe

TARGETS = $(STRIPPED_BINS)

srcdir=.
distdir=rcp-0.2.0
TAR = tar
TARFLAGS = z
TARFILEEXT = .tar.gz

SRCDIST_FILES=\
	rcp.c Makefile README TODO COPYING ChangeLog

BINDIST_FILES=\
	rcp.exe

OBJECTS=rcp.o util.o

all:  $(TARGETS)
bins: $(BINS)

.c.o:
	$(CC) -o $@ $< -c $(ALLFLAGS)

rcp_unstripped.exe: $(OBJECTS) Makefile
	$(CC) $(LDFLAGS) -o $@ $(OBJECTS) $(ALLFLAGS) $(LIBS)

rcp.exe: rcp_unstripped.exe
	$(STRIP) $< -o $@

download: rcp.exe
	pcp rcp.exe ":/rcp.exe"

clean:
	rm -f $(BINS) $(STRIPPED_BINS) $(OBJECTS)

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
