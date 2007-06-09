TARGET=arm-wince-mingw32ce

CXX=$(TARGET)-g++
AR=$(TARGET)-ar

WARNFLAGS=-Wall -Wextra

INCLUDES=

ALLFLAGS=$(INCLUDES) $(CXXFLAGS) $(WARNFLAGS)

all: PipeDev.dll

PipeDev.dll: PipeDevice.cpp
	$(CXX) $< -shared -o $@ $(ALLFLAGS)

clean:
	rm -f PipeDev.dll

.PHONE: all clean
