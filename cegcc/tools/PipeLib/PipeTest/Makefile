TARGET=arm-mingw32ce

CXX=$(TARGET)-g++
AR=$(TARGET)-ar

WARNFLAGS=-Wall -Wextra

INCLUDES=-I..
LIBS=-L.. -lPipeLib

ALLFLAGS=$(INCLUDES) $(CXXFLAGS) $(WARNFLAGS)

all: PipeTest.exe

PipeTest.exe: PipeTest.cpp Makefile
	$(CXX) $< -o $@ $(ALLFLAGS) $(LIBS) $(LDFLAGS)

clean:
	rm -f PipeTest.exe

.PHONE: all clean
