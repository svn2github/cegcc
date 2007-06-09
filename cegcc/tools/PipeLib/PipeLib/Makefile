TARGET=arm-wince-mingw32ce

CXX=$(TARGET)-g++
AR=$(TARGET)-ar

INCLUDES=-I../PipeDev/

ALLFLAGS=$(INCLUDES) $(CXXFLAGS)

all: libPipeLib.a

PipeLib.o: PipeLib.cpp
	$(CXX) $< -c -o $@ $(ALLFLAGS)

libPipeLib.a: PipeLib.o
	rm -f $@
	$(AR) r $@ $<

clean:
	rm -f libPipeLib.a PipeLib.o

.PHONE: all clean
