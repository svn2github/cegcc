# Pedro Alves <pedro_alves@portugalmail.pt> (C) 2006

PREFIX=/usr/local

SOURCEDIR = $(shell pwd)/..

VPATH = $(SOURCEDIR)

OUTFILE = cegccthrd

CC = arm-wince-pe-gcc

CFLAGS = -g -mwin32
CFLAGS +=-DDEBUG
#CFLAGS +=-DDEBUG_PRINTF

LDFLAGS += -Wl,-Map,${OUTFILE}.map
#LDFLAGS += -Wl,--verbose

CFILES = mthr.c mthr_init.c

OBJECTS = $(addsuffix .o, $(basename $(CFILES))) version.rc.o

all: stripped/${OUTFILE}.dll

install: all
	cp -vf ${OUTFILE}.dll $(PREFIX)/arm-wince-pe/bin
	cp -vf lib${OUTFILE}.dll.a $(PREFIX)/arm-wince-pe/lib

version.rc.o: version.rc
	arm-wince-pe-windres version.rc -o $@

installdev: all
	pput -v -v -f stripped/${OUTFILE}.dll \Windows

stripped/${OUTFILE}.dll : ${OUTFILE}.dll
	mkdir -p stripped
	cp -v ${OUTFILE}.dll stripped/${OUTFILE}.dll
	arm-wince-pe-strip.exe stripped/${OUTFILE}.dll

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
