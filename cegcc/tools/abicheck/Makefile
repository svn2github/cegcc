TARGET=arm-mingw32ce
CXX=$(TARGET)-g++
STRIP=$(TARGET)-strip
CXXFLAGS=-O3

OBJECTS=driver.o

all: driver_gcc.exe

msvc:
	makemsvc.sh

all-msvc: msvc all

msvc.obj:

.cpp.o:
	$(CXX) $< -o $@ $(CXXFLAGS) -c

driver_gcc_unstripped.exe: $(OBJECTS) msvc.obj
	$(CXX) $(OBJECTS) msvc.obj -o $@ $(CXXFLAGS)

driver_gcc.exe: driver_gcc_unstripped.exe
	$(STRIP) $< -o $@

clean:
	rm -f $(OBJECTS) driver_gcc.exe driver_gcc_unstripped.exe *~

clean-msvc: clean
	rm -f msvc.obj

.PHONY: all clean
