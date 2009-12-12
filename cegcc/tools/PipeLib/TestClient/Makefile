TARGET=arm-mingw32ce

CXX=$(TARGET)-g++
AR=$(TARGET)-ar

WARNFLAGS=-Wall -Wextra

INCLUDES=
LIBS=

ALLFLAGS=$(INCLUDES) $(CXXFLAGS) $(WARNFLAGS)

all: TestClient.exe

TestClient.exe: TestClient.cpp Makefile
	$(CXX) $< -o $@ $(ALLFLAGS) $(LIBS) $(LDFLAGS)

clean:
	rm -f TestClient.exe

.PHONE: all clean
