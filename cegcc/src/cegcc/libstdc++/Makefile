#Pedro Alves <pedro_alves@portugalmail.pt> (C) 2006

PREFIX?=/usr/local

BUILD_DIR?=.
MY_DIR=${BUILD_DIR}/libstdc++
TOP_SRCDIR?=../../..
SRCDIR=${TOP_SRCDIR}/src/cegcc/libstdc++

DLLNAME=libstdc++.dll
IMPLIB=$(DLLNAME).a
TARGET=arm-wince-pe

AR=$(TARGET)-ar
CC=$(TARGET)-gcc
STRIP=$(TARGET)-strip
WINDRES=$(TARGET)-windres

LIB_PATH=$(PREFIX)/$(TARGET)/lib

LDFLAGS += -Wl,-Map,${MY_DIR}/$(DLLNAME).map
#LDFLAGS += -Wl,--verbose -Wl,--enable-extra-pe-debug

RMFILES=

# this should be solved with defines passed to libstdc++. see libmath
#RMFILES+=wf_hypot.o 

all::
	mkdir -p ${MY_DIR}

all:: clean mktmp ${MY_DIR}/version.rc.o extract delfiles ${MY_DIR}/$(DLLNAME) ${MY_DIR}/$(DLLNAME).def ${MY_DIR}/$(IMPLIB) ${MY_DIR}/stripped/$(DLLNAME)

install:
	cp -vf ${MY_DIR}/$(DLLNAME) $(LIB_PATH)
	cp -vf ${MY_DIR}/$(IMPLIB) $(LIB_PATH)
	cp -vf ${MY_DIR}/stripped/$(DLLNAME) $(LIB_PATH)/device

delfiles:
	@list='$(RMFILES)'; \
		for file in $$list; \
			do rm -f ${MY_DIR}/_tmp/$$file \
		done; \
	done

extract:
	cd ${MY_DIR}/_tmp; $(AR) -x $(LIB_PATH)/libsupc++.a
	cd ${MY_DIR}/_tmp; $(AR) -x $(LIB_PATH)/libstdc++.a
#	mv ${MY_DIR}/*.o ${MY_DIR}/_tmp

${MY_DIR}/stripped/$(DLLNAME):
	@mkdir -p ${MY_DIR}/stripped
	rm -f ${MY_DIR}/stripped/$(DLLNAME)
	@cp -vf ${MY_DIR}/$(DLLNAME) ${MY_DIR}/$(DLLNAME)-stripped.dll
	$(STRIP) ${MY_DIR}/$(DLLNAME)-stripped.dll
	@mv -vf ${MY_DIR}/$(DLLNAME)-stripped.dll ${MY_DIR}/stripped/$(DLLNAME)

${MY_DIR}/$(DLLNAME).a \
${MY_DIR}/$(DLLNAME).def \
${MY_DIR}/$(DLLNAME):
	$(CC) $(LDFLAGS) -shared \
		-Wl,--enable-auto-import \
		-Wl,--enable-runtime-pseudo-reloc \
		-Wl,--export-all-symbols \
		-o ${MY_DIR}/$(DLLNAME) \
		-Wl,--out-implib,${MY_DIR}/$(DLLNAME).a \
		-Wl,--output-def,${MY_DIR}/$(DLLNAME).def \
		-Wl,--whole-archive ${MY_DIR}/_tmp/*.o \
		-Wl,-no-whole-archive -lcegcc -lcoredll


installdll: ${MY_DIR}/stripped/$(DLLNAME)
	pput.exe -v -v -f ${MY_DIR}/stripped/$(DLLNAME) "\Windows\$(DLLNAME)"

clean:
	rm -rf ${MY_DIR}/_tmp
	rm -rf ${MY_DIR}/stripped
	rm -f ${MY_DIR}/version.aps
	rm -f ${MY_DIR}/$(DLLNAME) ${MY_DIR}/$(IMPLIB) ${MY_DIR}/$(DLLNAME).def ${MY_DIR}/$(DLLNAME).map
	rm -rf ${MY_DIR}

mktmp:
	rm -rf ${MY_DIR}/_tmp
	mkdir -p ${MY_DIR}/_tmp

${MY_DIR}/version.rc.o: version.rc
	$(WINDRES) version.rc -o $@
	mv $@ ${MY_DIR}/_tmp

.PHONY: all extract clean delfiles mktmp strip
