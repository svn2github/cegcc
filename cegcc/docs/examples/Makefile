.SUFFIXES:	.exe .c .h .rc .rsc .C

ARCH=		arm-wince-pe
CC=		${ARCH}-gcc ${CFLAGS}
WINDRES=	${ARCH}-windres
CFLAGS=		-g

.rc.rsc:
		${ARCH}-windres $? $@

.o.exe:
	${ARCH}-gcc ${CFLAGS} -o $@ $?

.c.exe:
	${ARCH}-gcc ${CFLAGS} -o $@ $?

all::	f.exe fibo.exe hello-dialog.exe menu-resource.exe \
	hello-small.exe hello-small-g.exe hello-small-s.exe

menu-resource.exe:	menu-resource.o menu-resource.rsc
		${CC} -o $@ menu-resource.o menu-resource.rsc -lcommctrl

menu-resource.o:	menu-resource.c
		${CC} -c $?

menu-resource.rsc:	menu-resource.rc menu-resource.h
		${WINDRES} menu-resource.rc menu-resource.rsc

clean:
		-rm -f *.o *.exe *.rsc

hello-small-g.exe:	hello-dialog.c
	arm-wince-mingw32-gcc -g -o $@ $?

hello-small-s.exe:	hello-dialog.c
	arm-wince-mingw32-gcc -s -o $@ $?

hello-small.exe:	hello-dialog.c
	arm-wince-mingw32-gcc -o $@ $?

menu-resource.tar.gz:	Makefile menu-resource.c menu-resource.rc \
			menu-resource.h menu-resource.exe
	tar cfz menu-resource.tar.gz \
		Makefile menu-resource.c menu-resource.rc \
		menu-resource.h menu-resource.exe

f.exe:	fibo.c
	${ARCH}-gcc -o $@ $?
