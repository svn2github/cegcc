# generate cegcc.dll from libc.a, libc.a, libm.a, libgcc.a
# based on script from Mamaich's version of the toolchain <http://mamaich.cjb.net/>
#
# Pedro Alves <pedro_alves@portugalmail.pt> (C) 2006, 2007

# FIXME: override -j > 1, because this makefile doesn't
# support parallel builds.
MAKEFLAGS:=$(MAKEFLAGS) -j 1

PREFIX?=/opt/cegcc

# BUILD_DIR=$TOP_SRCDIR/build/$MY_HOST_ARCH/$TGT_ARCH
BUILD_DIR?=.
MY_DIR=${BUILD_DIR}/cegccdll
TOP_SRCDIR?=../../..
SRCDIR=${TOP_SRCDIR}/src/cegcc/cegccdll

TARGET=arm-wince-cegcc
DLLNAME=cegcc.dll
IMPLIB=lib$(DLLNAME).a

GCCVERSION=$(shell $(TARGET)-gcc -dumpversion)

AR=$(TARGET)-ar
CC=$(TARGET)-gcc
STRIP=$(TARGET)-strip
WINDRES=$(TARGET)-windres

LIB_PATH=$(PREFIX)/$(TARGET)/lib

# link to local ${IMPLIB} instead of preinstalled one
LDFLAGS += -L ${MY_DIR} -Wl,-Map,${MY_DIR}/$(DLLNAME).map

RMFILES=
RMFILES+=callocr.o mallocr.o reallocr.o malignr.o mallinfor.o malloptr.o
RMFILES+=cfreer.o freer.o msizer.o realloc.o pvallocr.o vallocr.o
RMFILES+=signalr.o 
RMFILES+=_addsub_df.o _addsub_sf.o _compare_df.o _compare_sf.o _eq_df.o
RMFILES+=_eq_sf.o _df_to_si.o _ge_df.o _ge_sf.o _gt_df.o _gt_sf.o _le_df.o
RMFILES+=_le_sf.o _lt_df.o _lt_sf.o _mul_df.o _div_df.o _mul_sf.o _div_sf.o
RMFILES+=_ne_df.o _ne_sf.o _negate_df.o _negate_sf.o _sf_to_df.o _sf_to_si.o
RMFILES+=_si_to_df.o _si_to_sf.o _df_to_sf.o _unord_df.o _unord_sf.o _usi_to_df.o
RMFILES+=_usi_to_sf.o

all: clean mktmp mktmp_static version.rc.o extract delfiles ${MY_DIR}/$(DLLNAME) ${MY_DIR}/$(DLLNAME).def ${MY_DIR}/$(IMPLIB) ${MY_DIR}/stripped/$(DLLNAME)

install:
	@echo "installing files for gcc $(GCCVERSION)"
	mkdir -p $(LIB_PATH) $(LIB_PATH)/device
	cp -vf ${MY_DIR}/$(DLLNAME) $(LIB_PATH)
	cp -vf ${MY_DIR}/stripped/$(DLLNAME) $(LIB_PATH)/device/
	cp -vf ${MY_DIR}/$(IMPLIB) $(LIB_PATH)
# force the usage of all symbols from the dll
# by making empty stubs. when -static is used the real static versions are found instead.
	rm -f $(LIB_PATH)/libc.dll.a
	rm -f $(LIB_PATH)/libm.dll.a
	rm -f $(PREFIX)/lib/gcc/$(TARGET)/$(GCCVERSION)/libgcc.dll.a
	$(AR) q $(LIB_PATH)/libc.dll.a
	$(AR) q $(LIB_PATH)/libm.dll.a
	$(AR) q $(PREFIX)/lib/gcc/$(TARGET)/$(GCCVERSION)/libgcc.dll.a

delfiles:
	@list='$(RMFILES)'; \
		for file in $$list; \
			do rm -f ${MY_DIR}/_tmp/$$file \
		done; \
	done
	mkdir -p ${MY_DIR}/_tmp_static
# for c++ contructors to run, the call to __gccmain must be from static code
	mv ${MY_DIR}/_tmp/__main.o ${MY_DIR}/_tmp_static
	mv ${MY_DIR}/_tmp/dllmaincrtstartup_t.o ${MY_DIR}/_tmp_static
	mv ${MY_DIR}/_tmp/mthr_thunk.o ${MY_DIR}/_tmp_static
# must be static to enable the overriding
	mv ${MY_DIR}/_tmp/dllmain.o ${MY_DIR}/_tmp_static
	mv ${MY_DIR}/_tmp/main.o ${MY_DIR}/_tmp_static
# functions called from crt0
	mv ${MY_DIR}/_tmp/startup_thunk.o	${MY_DIR}/_tmp_static
# --enable-runtime-pseudo-reloc support
	mv ${MY_DIR}/_tmp/pseudo-reloc_t.o ${MY_DIR}/_tmp_static

extract:
	@echo "extracting files for gcc $(GCCVERSION)"
	cd ${MY_DIR}/_tmp && $(AR) -x $(LIB_PATH)/libc.a
#	cd ${MY_DIR}/_tmp && $(AR) -x $(LIB_PATH)/libg.a
	cd ${MY_DIR}/_tmp && $(AR) -x $(LIB_PATH)/libm.a
	cd ${MY_DIR}/_tmp && $(AR) -x $(LIB_PATH)/../../lib/gcc/$(TARGET)/$(GCCVERSION)/libgcc.a

${MY_DIR}/stripped/$(DLLNAME):
	rm -f ${MY_DIR}/stripped/$(DLLNAME)
	@cp -vf ${MY_DIR}/$(DLLNAME) ${MY_DIR}/$(DLLNAME).stripped.dll
	$(STRIP) ${MY_DIR}/$(DLLNAME).stripped.dll
	@mkdir -p ${MY_DIR}/stripped
	@mv -vf ${MY_DIR}/$(DLLNAME).stripped.dll ${MY_DIR}/stripped/$(DLLNAME)

# create and link to a new import lib
# add all the static symbols to it, and then link to it, using 
# -nostdlib to avoid linking to an installed libcegcc.dll.a, 
# no need to use exclude-libs, as ld already excludes libcegcc* by default
${MY_DIR}/$(DLLNAME):
	rm -f ${MY_DIR}/$(IMPLIB)
	rm -f ${MY_DIR}/$(IMPLIB).tmp
	$(AR) q ${MY_DIR}/$(IMPLIB) ${MY_DIR}/_tmp_static/*.o
	rm -f ${MY_DIR}/_tmp/_eprintf.o
	$(CC) -o $@ $(LDFLAGS) -shared -nostdlib \
	-Wl,--whole-archive ${MY_DIR}/_tmp/*.o ${LIB_PATH}/dllcrt1.o \
	-Wl,--no-whole-archive -lcegcc -lcoredll -liphlpapi \
	-Wl,--output-def=${MY_DIR}/$(DLLNAME).def \
	-Wl,--out-implib,${MY_DIR}/$(IMPLIB).tmp
	mv -f ${MY_DIR}/libcegcc.dll.a.tmp ${MY_DIR}/libcegcc.dll.a
	$(AR) q ${MY_DIR}/$(IMPLIB) ${MY_DIR}/_tmp_static/*.o

#	-Wl,--verbose \
#	-Wl,--enable-extra-pe-debug \
#	-Wl,--enable-auto-import \
#	-Wl,--enable-runtime-pseudo-reloc \

installdll: ${MY_DIR}/stripped/$(DLLNAME)
	pput.exe -v -v -f ${MY_DIR}/stripped/$(DLLNAME) "\Windows\$(DLLNAME)"

clean:
	rm -rf ${MY_DIR}/_tmp
	rm -rf ${MY_DIR}/_tmp_static
	rm -rf ${MY_DIR}/stripped
	rm -f ${MY_DIR}/$(DLLNAME) ${MY_DIR}/$(IMPLIB).tmp ${MY_DIR}/$(IMPLIB) ${MY_DIR}/$(DLLNAME).def ${MY_DIR}/$(DLLNAME).map
	rm -rf ${MY_DIR}

mktmp:
	rm -rf ${MY_DIR}/_tmp
	mkdir -p ${MY_DIR}/_tmp

mktmp_static:
	rm -rf ${MY_DIR}/_tmp_static
	mkdir -p ${MY_DIR}/_tmp_static

version.rc.o: version.rc
	$(WINDRES) version.rc -o $@
	mv $@ ${MY_DIR}/_tmp

.PHONY: all extract clean delfiles mktmp mktmp_static strip
