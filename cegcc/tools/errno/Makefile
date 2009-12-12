TARGET=arm-mingw32ce

CC=$(TARGET)-gcc
AR=$(TARGET)-ar

INCLUDES=-I.
ALLFLAGS=$(CFLAGS) $(INCLUDES)

GENFILES=errno.h errno_tab.c errno_str.c
OBJECTS=errno.o

all: liberrno.a

errno_tab.c: errno.tab errno_tab.awk Makefile
	awk '{ print $$2 " " $$1 }' < errno.tab | sort | awk -f errno_tab.awk > $@

errno_str.c: errno.tab errno_str.awk Makefile
	awk -f errno_str.awk < errno.tab > $@

errno.h: errno.tab errno_h.awk Makefile
	awk -f errno_h.awk < errno.tab > $@

errno.o: errno.c errno_tab.c errno_str.c errno.h Makefile
	$(CC) -c -o $@ $< $(ALLFLAGS)

liberrno.a: errno.o Makefile
	rm -f $@
	$(AR) r $@ $<

clean:
	rm -f liberrno.a $(OBJECTS)

distclean: clean
	rm -f $(GENFILES)

regen: distclean $(GENFILES)

.PHONY: all clean regen
