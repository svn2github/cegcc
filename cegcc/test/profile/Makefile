MCC=	arm-wince-mingw32ce-gcc ${CFLAGS}
CEGCC=	arm-wince-cegcc-gcc ${CFLAGS}
CFLAGS= -pg

all:	m1.gmon.out m1.exe m1.explain

m1.exe:	m1.c a.c b.c
	${MCC} -o m1.exe m1.c a.c b.c

m1.gmon.out:	m1.exe
	pcp m1.exe ":/storage card/devel/m1.exe"
	prun "/storage card/devel/m1.exe"
	sleep 10
	pcp ":/gmon.out" m1.gmon.out
	prm ":/gmon.out"

m1.explain:	m1.gmon.out
	arm-wince-mingw32ce-gprof m1.exe m1.gmon.out >m1.explain

clean:
	-rm -f m1.exe m1.gmon.out m1.explain
	-rm -f m2.exe m2.gmon.out m2.explain

all:	m2.gmon.out m2.exe m2.explain

m2.exe:	m2.c a.c b.c
	${CEGCC} -o m2.exe m2.c a.c b.c

m2.gmon.out:	m2.exe
	pcp m2.exe ":/storage card/devel/m2.exe"
	prun "/storage card/devel/m2.exe"
	sleep 10
	pcp ":/gmon.out" m2.gmon.out
	prm ":/gmon.out"

m2.explain:	m2.gmon.out
	arm-wince-cegcc-gprof m2.exe m2.gmon.out >m2.explain
