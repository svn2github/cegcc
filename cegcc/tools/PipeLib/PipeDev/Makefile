TARGET=arm-wince-mingw32ce

CXX=$(TARGET)-g++
AR=$(TARGET)-ar
STRIP=$(TARGET)-strip

WARNFLAGS=-Wall -Wextra

INCLUDES=

ALLFLAGS=$(INCLUDES) $(CXXFLAGS) $(WARNFLAGS)

all: PipeDev.dll

PipeDev_unstripped.dll: PipeDevice.cpp Makefile
	$(CXX) $< -shared -o $@ $(ALLFLAGS)

PipeDev.dll: PipeDev_unstripped.dll
	$(STRIP) $< -o $@

download: PipeDev.dll
	pcp PipeDev.dll ":/Windows/PipeDev.dll"

clean:
	rm -f PipeDev.dll

.PHONY: all clean download
