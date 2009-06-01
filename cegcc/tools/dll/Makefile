ARCH=	i386-mingw32ce

CC=	${ARCH}-gcc
SRC=	../../src
CDEF=	${SRC}/mingw/coredll.def
ADEF=	${SRC}/w32api/libce/aygshell.def
CFLAGS=	-D_WIN32_WCE=0x0600 -D_WIN32_IE=0x0600

all::	dllverify.exe

dllverify.exe:	dllverify.c
	${CC} -o $@ $<

clean:
	-rm -f *.o dllverify.exe
