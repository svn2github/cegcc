# Pedro Alves <pedro_alves@portugalmail.pt> (C) 2006

PREFIX?=/usr/local

OUTFILE = cegccthrd

TARGET=arm-wince-pe

AR=$(TARGET)-ar
CC=$(TARGET)-gcc
STRIP=$(TARGET)-strip
WINDRES=$(TARGET)-windres

CFLAGS = -g -mwin32
CFLAGS +=-DDEBUG
#CFLAGS +=-DDEBUG_PRINTF

LDFLAGS += -Wl,-Map,${OUTFILE}.map
#LDFLAGS += -Wl,--verbose

CFILES = mthr.c mthr_init.c

OBJECTS = $(addsuffix .o, $(basename $(CFILES))) version.rc.o

all: stripped/${OUTFILE}.dll

install: all
	cp -vf ${OUTFILE}.dll $(PREFIX)/$(TARGET)/lib
	cp -vf lib${OUTFILE}.dll.a $(PREFIX)/$(TARGET)/lib
	cp -vf stripped/${OUTFILE}.dll $(PREFIX)/$(TARGET)/lib/device

version.rc.o: version.rc
	$(WINDRES) version.rc -o $@

installdev: all
	pput -v -v -f stripped/${OUTFILE}.dll \Windows

stripped/${OUTFILE}.dll : ${OUTFILE}.dll
	mkdir -p stripped
	cp -v ${OUTFILE}.dll stripped/${OUTFILE}.dll
	$(STRIP) stripped/${OUTFILE}.dll

${OUTFILE}.dll : $(OBJECTS) Makefile
	$(CC) -o $@ -shared $(LDFLAGS) \
	-Wl,--whole-archive $(OBJECTS) \
	-Wl,--no-whole-archive -lcegcc \
	-Wl,--enable-auto-import \
	-Wl,--out-implib=lib${OUTFILE}.dll.a \
	-Wl,--output-def=${OUTFILE}.def

clean: 
	rm -rfv ${OUTFILE}.dll lib${OUTFILE}.dll.a ${OUTFILE}.def \
		   stripped $(OBJECTS) ${OUTFILE}.map
