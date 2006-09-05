# Pedro Alves <pedro_alves@portugalmail.pt> (C) 2006

PREFIX?=/usr/local

OUTFILE = cegccthrd

TARGET=arm-wince-pe

BUILD_DIR?=.
MY_DIR=${BUILD_DIR}/cegccthrd
TOP_SRCDIR?=../../..
SRCDIR=${TOP_SRCDIR}/src/cegcc/cegccthrd

AR=$(TARGET)-ar
CC=$(TARGET)-gcc
STRIP=$(TARGET)-strip
WINDRES=$(TARGET)-windres

CFLAGS = -g -mwin32
CFLAGS +=-DDEBUG
#CFLAGS +=-DDEBUG_PRINTF

LDFLAGS += -Wl,-Map,${MY_DIR}/${OUTFILE}.map
#LDFLAGS += -Wl,--verbose

CFILES = mthr.c mthr_init.c

OBJECTS = $(addprefix $(MY_DIR)/, $(addsuffix .o, $(basename $(CFILES)))) ${MY_DIR}/version.rc.o

all::
	mkdir -p ${MY_DIR}

all::	 ${MY_DIR}/stripped/${OUTFILE}.dll

install: all
	cp -vf ${MY_DIR}/${OUTFILE}.dll $(PREFIX)/$(TARGET)/lib
	cp -vf ${MY_DIR}/lib${OUTFILE}.dll.a $(PREFIX)/$(TARGET)/lib
	cp -vf ${MY_DIR}/stripped/${OUTFILE}.dll $(PREFIX)/$(TARGET)/lib/device

${MY_DIR}/version.rc.o: version.rc
	$(WINDRES) version.rc -o $@

installdev: all
	pput -v -v -f ${MY_DIR}/stripped/${OUTFILE}.dll \Windows

${MY_DIR}/stripped/${OUTFILE}.dll : ${MY_DIR}/${OUTFILE}.dll
	mkdir -p ${MY_DIR}/stripped
	cp -v ${MY_DIR}/${OUTFILE}.dll ${MY_DIR}/stripped/${OUTFILE}.dll
	$(STRIP) ${MY_DIR}/stripped/${OUTFILE}.dll

${MY_DIR}/${OUTFILE}.dll : $(OBJECTS) Makefile
	$(CC) -o $@ -shared $(LDFLAGS) \
	-Wl,--whole-archive $(OBJECTS) \
	-Wl,--no-whole-archive -lcegcc \
	-Wl,--enable-auto-import \
	-Wl,--out-implib=${MY_DIR}/lib${OUTFILE}.dll.a \
	-Wl,--output-def=${MY_DIR}/${OUTFILE}.def

clean: 
	rm -rfv ${MY_DIR}/${OUTFILE}.dll ${MY_DIR}/lib${OUTFILE}.dll.a ${MY_DIR}/${OUTFILE}.def \
		   ${MY_DIR}/stripped ${MY_DIR}/$(OBJECTS) ${MY_DIR}/${OUTFILE}.map \
		   ${MY_DIR}

${MY_DIR}/mthr_init.o:	mthr_init.c
	${CC} ${CFLAGS} -c $< -o $@

${MY_DIR}/mthr.o:	mthr.c
	${CC} ${CFLAGS} -c $< -o $@
