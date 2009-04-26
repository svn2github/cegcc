TARGET=	arm-mingw32ce
CFLAGS=	-g
WCFLAGS=	-I../errno -g

all:	cedump cedump.exe

cedump:	cedump.c DwCeDump.h
	$(CC) ${CFLAGS} -o $@ $<

cedump.exe:	cedump.c DwCeDump.h
	$(TARGET)-gcc ${WCFLAGS} -o $@ $< ../errno/liberrno.a

