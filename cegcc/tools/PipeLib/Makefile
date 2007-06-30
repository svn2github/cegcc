TARGET=arm-wince-mingw32ce

CXX=$(TARGET)-g++
AR=$(TARGET)-ar
STRIP=$(TARGET)-strip

WARNFLAGS=-Wall -Wextra

INCLUDES=

ALLFLAGS=$(INCLUDES) $(CXXFLAGS) $(WARNFLAGS)

all: PipeDev.dll libPipeLib.a

PipeDev_unstripped.dll: PipeDev.cpp PipeDev.h Makefile
	$(CXX) $< -shared -o $@ $(ALLFLAGS)

PipeDev.dll: PipeDev_unstripped.dll
	$(STRIP) $< -o $@

PipeLib.o: PipeLib.cpp
	$(CXX) $< -c -o $@ $(ALLFLAGS)

libPipeLib.a: PipeLib.o
	rm -f $@
	$(AR) r $@ $<

download: all
	pcp PipeDev.dll ":/Windows/PipeDev.dll"

clean-dev:
	rm -f PipeDev.dll PipeDev_unstripped.dll

clean-lib:
	rm -f libPipeLib.a PipeLib.o

clean: clean-dev clean-lib

.PHONY: all clean clean-dev clean-lib download
