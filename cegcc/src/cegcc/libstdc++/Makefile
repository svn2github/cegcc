#Pedro Alves <pedro_alves@portugalmail.pt> (C) 2006

PREFIX?=/usr/local
DLLNAME=libstdc++.dll
IMPLIB=$(DLLNAME).a

AR=arm-wince-pe-ar -x
LIB_PATH=$(PREFIX)/arm-wince-pe/lib

LDFLAGS += -Wl,-Map,$(DLLNAME).map
#LDFLAGS += -Wl,--verbose -Wl,--enable-extra-pe-debug

RMFILES=

# this should be solved with defines passed to libstdc++. see libmath
#RMFILES+=wf_hypot.o 

all: clean mktmp version.rc.o extract delfiles $(DLLNAME) $(DLLNAME).def $(IMPLIB) stripped/$(DLLNAME)

install:
	cp -vf $(DLLNAME) $(PREFIX)/arm-wince-pe/lib
	cp -vf $(IMPLIB) $(PREFIX)/arm-wince-pe/lib
	cp -vf stripped/$(DLLNAME) $(PREFIX)/arm-wince-pe/lib/device

delfiles:
	@list='$(RMFILES)'; \
		for file in $$list; \
			do rm -f _tmp/$$file \
		done; \
	done

extract:
	$(AR) $(LIB_PATH)/libsupc++.a
	$(AR) $(LIB_PATH)/libstdc++.a
	mv *.o _tmp

stripped/$(DLLNAME):
	@mkdir -p stripped
	rm -f stripped/$(DLLNAME)
	@cp -vf $(DLLNAME) $(DLLNAME)-stripped.dll
	arm-wince-pe-strip $(DLLNAME)-stripped.dll
	@mv -vf $(DLLNAME)-stripped.dll stripped/$(DLLNAME)

$(DLLNAME).a \
$(DLLNAME).def \
$(DLLNAME):
	arm-wince-pe-gcc $(LDFLAGS) -shared \
		-Wl,--enable-auto-import \
		-Wl,--enable-runtime-pseudo-reloc \
		-Wl,--export-all-symbols \
		-o $(DLLNAME) \
		-Wl,--out-implib,$(DLLNAME).a \
		-Wl,--output-def,$(DLLNAME).def \
		-Wl,--whole-archive _tmp/*.o \
		-Wl,-no-whole-archive -lcegcc -lcoredll


installdll: stripped/$(DLLNAME)
	pput.exe -v -v -f stripped/$(DLLNAME) "\Windows\$(DLLNAME)"

clean:
	rm -rf _tmp
	rm -rf stripped
	rm -f version.aps
	rm -f $(DLLNAME) $(IMPLIB) $(DLLNAME).def $(DLLNAME).map

mktmp:
	rm -rf _tmp
	mkdir _tmp

version.rc.o: version.rc
	arm-wince-pe-windres version.rc -o $@
	mv $@ _tmp

.PHONY: all extract clean delfiles mktmp strip
