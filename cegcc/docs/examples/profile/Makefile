.SUFFIXES:	.exe .c .h .rc .rsc .C

ARCH=		arm-wince-cegcc
CC=		${ARCH}-gcc ${CFLAGS}

CFLAGS=		-g -D_WIN32_IE=0x0400 -pg

.o.exe:
	${ARCH}-gcc ${CFLAGS} -o $@ $?

.c.exe:
	${ARCH}-gcc ${CFLAGS} -o $@ $?

.C.exe:
	${ARCH}-g++ ${CFLAGS} -o $@ $?

all::	fibo.exe fibo.explain

clean:
	-rm -f *.o *.exe *.rsc gmon.out *.explain *.gmon

dist:
	pcp fibo.exe ":/storage card/devel/fibo.exe"

fibo.explain:	fibo.gmon fibo.exe
	${ARCH}-gprof -q -x -l fibo.exe fibo.gmon > fibo.explain

fibo.gmon:	fibo.exe
	pcp fibo.exe ":/storage card/devel/fibo.exe"
	prun "/storage card/devel/fibo.exe"
	sleep 5
	pcp ":/storage card/devel/gmon.out" fibo.gmon
	prm "/storage card/devel/gmon.out"
