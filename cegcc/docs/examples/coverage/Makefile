.SUFFIXES:	.exe .c .h .rc .rsc .C

ARCH=		arm-wince-cegcc
CC=		${ARCH}-gcc ${CFLAGS}

EXECS=		fibo.exe m1.exe
CFLAGS=		-g -D_WIN32_IE=0x0400 --coverage

.o.exe:
	${ARCH}-gcc ${CFLAGS} -o $@ $?

.c.exe:
	${ARCH}-gcc ${CFLAGS} -o $@ $?

all::	${EXECS}

clean:
		-rm -f *.o *.exe *.rsc *.gcno *.gcda *.gcov

dist:
	for i in $(EXECS) ; \
	do \
		pcp $$i ":/storage card/devel/"$$i ; \
	done

m1.exe:	m1.o a.o b.o

fibo.gcda:	fibo.exe
	pcp fibo.exe ":/storage card/devel/fibo.exe"
	-prm "/storage card/devel/fibo.gcda"
	prun "/storage card/devel/fibo.exe"
	sleep 4
	pcp ":/storage card/devel/fibo.gcda" fibo.gcda
	-prm "/storage card/devel/fibo.gcda"

fibo.c.gcov: fibo.gcda
	${ARCH}-gcov -a fibo.c
