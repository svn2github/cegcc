TARGET=	arm-mingw32ce
CFLAGS=	-g
WCFLAGS=	-I../errno -g

all:	cedump cedump.exe copydump.exe

cedump:	cedump.c DwCeDump.h
	$(CC) ${CFLAGS} -o $@ $<

cedump.exe:	cedump.c DwCeDump.h
	$(TARGET)-gcc ${WCFLAGS} -o $@ $< ../errno/liberrno.a

copydump.exe:	copydump.c
	$(TARGET)-gcc ${WCFLAGS} -o $@ $< ../errno/liberrno.a

clean::
	-rm -f *.o core.*
	-rm -f cedump cedump.exe copydump.exe
	-rm -f *~
