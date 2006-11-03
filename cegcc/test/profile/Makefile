MCC=	arm-wince-mingw32ce-gcc ${CFLAGS}
CEGCC=	arm-wince-cegcc-gcc ${CFLAGS}
CFLAGS= -pg

.SUFFIXES:	.c .mexe .cexe .co .mo

.c.mexe:
	${MCC} -o $@ $?

.c.cexe:
	${CEGCC} -o $@ $?

.c.mo:
	${MCC} -c $? -o $@

.c.co:
	${CEGCC} -c $? -o $@

all::	execs explains

execs::	m1.cexe m1.mexe m2.cexe m2.mexe

m1.cexe:	m1.c a.co b.co

m1.mexe:	m1.c a.mo b.mo

m2.cexe:	m2.c a.co b.co

m2.mexe:	m2.c a.mo b.mo

clean:
	-rm -f *.co *.mo *.mexe *.cexe *.explain *.gmon.out

m1c.explain:	m1c.gmon.out
	arm-wince-cegcc-gprof m1.cexe m1c.gmon.out >m1c.explain

m2c.explain:	m2c.gmon.out
	arm-wince-cegcc-gprof m2.cexe m2c.gmon.out >m2c.explain

m1m.explain:	m1m.gmon.out
	arm-wince-mingw32ce-gprof m1.mexe m1m.gmon.out >m1m.explain

m2m.explain:	m2m.gmon.out
	arm-wince-mingw32ce-gprof m2.mexe m2m.gmon.out >m2m.explain

explains::	m1c.explain m2c.explain m1m.explain m2m.explain

m1c.gmon.out:	m1.cexe
	pcp m1.cexe ":/storage card/devel/m1c.exe"
	prun "/storage card/devel/m1c.exe"
	sleep 10
	pcp ":/storage card/devel/m1c.gmo" m1c.gmon.out
	prm ":/storage card/devel/m1c.gmo"

m1m.gmon.out:	m1.mexe
	pcp m1.mexe ":/storage card/devel/m1m.exe"
	prun "/storage card/devel/m1m.exe"
	sleep 10
	pcp ":/storage card/devel/m1m.gmo" m1m.gmon.out
	prm ":/storage card/devel/m1m.gmo"

m2c.gmon.out:	m2.cexe
	pcp m2.cexe ":/storage card/devel/m2c.exe"
	prun "/storage card/devel/m2c.exe"
	sleep 10
	pcp ":/storage card/devel/m2c.gmo" m2c.gmon.out
	prm ":/storage card/devel/m2c.gmo"

m2m.gmon.out:	m2.mexe
	pcp m2.mexe ":/storage card/devel/m2m.exe"
	prun "/storage card/devel/m2m.exe"
	sleep 10
	pcp ":/storage card/devel/m2m.gmo" m2m.gmon.out
	prm ":/storage card/devel/m2m.gmo"
