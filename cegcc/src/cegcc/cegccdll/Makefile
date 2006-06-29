# generate cegcc.dll from libc.a, libc.a, libm.a, libgcc.a
# based on script from Mamaich's version of the toolchain <http://mamaich.cjb.net/>
#
# Pedro Alves <pedro_alves@portugalmail.pt> (C) 2006

PREFIX=/usr/local
DLLNAME=cegcc.dll
IMPLIB=lib$(DLLNAME).a

GCCVERSION=$(shell ./gccversion.sh)

AR=arm-wince-pe-ar -x
LIB_PATH=$(PREFIX)/arm-wince-pe/lib

# link to local ${IMPLIB} instead of preinstalled one
LDFLAGS += -L . -Wl,-Map,$(DLLNAME).map

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

all: clean mktmp mktmp_static version.rc.o extract delfiles $(DLLNAME) $(DLLNAME).def $(IMPLIB) stripped/$(DLLNAME)

install:
	@echo "installing files for gcc $(GCCVERSION)"
	cp -vf $(DLLNAME) $(PREFIX)/arm-wince-pe/bin
	cp -vf $(IMPLIB) $(PREFIX)/arm-wince-pe/lib
# force the usage of all symbols from the dll
# by making empty stubs. when -static is used the real static versions are found instead.
	rm -f $(PREFIX)/arm-wince-pe/lib/libc.dll.a
	rm -f $(PREFIX)/arm-wince-pe/lib/libm.dll.a
	rm -f $(PREFIX)/lib/gcc/arm-wince-pe/$(GCCVERSION)/libgcc.dll.a
	arm-wince-pe-ar q $(PREFIX)/arm-wince-pe/lib/libc.dll.a
	arm-wince-pe-ar q $(PREFIX)/arm-wince-pe/lib/libm.dll.a
	arm-wince-pe-ar q $(PREFIX)/lib/gcc/arm-wince-pe/$(GCCVERSION)/libgcc.dll.a
	
delfiles:
	@list='$(RMFILES)'; \
		for file in $$list; \
			do rm -f _tmp/$$file \
		done; \
	done
	mkdir -p _tmp_static
# for c++ contructors to run, the call to __gccmain must be from static code
	mv _tmp/__main.o _tmp_static
	mv _tmp/dllmaincrtstartup_t.o _tmp_static
	mv _tmp/mthr_thunk.o _tmp_static
# must be static to enable the overriding
	mv _tmp/dllmain.o _tmp_static
	mv _tmp/main.o _tmp_static
# functions called from crt0
	mv _tmp/startup_thunk.o	_tmp_static
# --enable-runtime-pseudo-reloc support
	mv _tmp/pseudo-reloc_t.o _tmp_static

extract:
	@echo "extracting files for gcc $(GCCVERSION)"
	$(AR) $(LIB_PATH)/libc.a
#	$(AR) $(LIB_PATH)/libg.a
	$(AR) $(LIB_PATH)/libm.a
	$(AR) $(LIB_PATH)/../../lib/gcc/arm-wince-pe/$(GCCVERSION)/libgcc.a
	mv *.o _tmp

stripped/$(DLLNAME):
	rm -f stripped/$(DLLNAME)
	@cp -vf $(DLLNAME) $(DLLNAME).stripped.dll
	arm-wince-pe-strip $(DLLNAME).stripped.dll
	@mkdir -p stripped
	@mv -vf $(DLLNAME).stripped.dll stripped/$(DLLNAME)

# create and link to a new import lib
# add all the static symbols to it, and then link to it, using 
# -nostdlib to avoid linking to an installed libcegcc.dll.a, 
# no need to use exclude-libs, as ld already excludes libcegcc* by default
$(DLLNAME):
	rm -f $(IMPLIB)
	rm -f $(IMPLIB).tmp
	arm-wince-pe-ar q $(IMPLIB) _tmp_static/*.o
	arm-wince-pe-gcc -o $@ $(LDFLAGS) -shared -nostdlib \
	-Wl,--whole-archive _tmp/*.o ${LIB_PATH}/dllcrt1.o \
	-Wl,--no-whole-archive -lcegcc -lcoredll -liphlpapi \
	-Wl,--output-def=$(DLLNAME).def \
	-Wl,--out-implib,$(IMPLIB).tmp
	mv -f libcegcc.dll.a.tmp libcegcc.dll.a
	arm-wince-pe-ar q $(IMPLIB) _tmp_static/*.o

#	-Wl,--verbose \
#	-Wl,--enable-extra-pe-debug \
#	-Wl,--enable-auto-import \
#	-Wl,--enable-runtime-pseudo-reloc \

installdll: stripped/$(DLLNAME)
	pput.exe -v -v -f stripped\$(DLLNAME) \Windows\$(DLLNAME)
	
clean:
	rm -rf _tmp
	rm -rf _tmp_static
	rm -rf stripped
	rm -f $(DLLNAME) $(IMPLIB).tmp $(IMPLIB) $(DLLNAME).def $(DLLNAME).map

mktmp:
	rm -rf _tmp
	mkdir _tmp

mktmp_static:
	rm -rf _tmp_static
	mkdir _tmp_static
	
version.rc.o: version.rc
	arm-wince-pe-windres version.rc -o $@
	mv $@ _tmp

.PHONY: all extract clean delfiles mktmp mktmp_static strip
