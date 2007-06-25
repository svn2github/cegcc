TARGET=arm-wince-mingw32ce
LDFLAGS=
INCLUDES=-I.
CFLAGS=-O0 -g3
WARNFLAGS=-Wall -Wextra

ALLFLAGS=$(WARNFLAGS) $(CFLAGS) $(INCLUDES)

CC=$(TARGET)-gcc
AR=$(TARGET)-ar

LIBS = libcwd.a
TARGETS = $(LIBS)

OBJECTS = cwd.o

all:  $(TARGETS)
libs: $(LIBS)

.c.o:
	$(CC) -c -o $@ $< $(ALLFLAGS)

cwd.o: cwd.c cwd.h Makefile
stat.o: stat.o cwd.h Makefile
open.o: open.o cwd.h Makefile
rename.o: rename.o cwd.h Makefile
stat.o: stat.o cwd.h Makefile
tempnam.o: tempnam.o cwd.h Makefile
unlink.o: unlink.o cwd.h Makefile

libcwd.a: $(OBJECTS) Makefile
	rm -f $@
	$(AR) r $@ $(OBJECTS)

install:

clean:
	rm -f $(LIBS) $(OBJECTS)

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

.PHONY: all install clean dist bindist srcdist
