T=		arm-wince-pe
CC=		${T}-gcc
DLLTOOL=	${T}-dlltool

SRC=	demodll.c demodll.def Makefile testdll.c usedemo.c
EXE=	usedemo.exe libdemodll.a demodll.dll

all:	demodll.dll libdemodll.a usedemo.exe

demodll.dll:	demodll.c
	${CC} --shared -o demodll.dll demodll.c

libdemodll.a:	demodll.def
	${DLLTOOL} -d demodll.def -l libdemodll.a

usedemo.exe:	usedemo.c libdemodll.a
	${CC} -o usedemo.exe usedemo.c -L. -ldemodll

dist:	usedemo.exe demodll.dll
	pcp usedemo.exe ":/storage card/devel/usedemo.exe"
	pcp demodll.dll ":/storage card/devel/demodll.dll"

clean:
	-rm -f *.exe *.dll *~ *.a test-dll.tar.gz

pkg:	test-dll.tar.gz

test-dll.tar.gz:	${SRC} ${EXE}
	tar cfvz test-dll.tar.gz ${SRC} ${EXE}
