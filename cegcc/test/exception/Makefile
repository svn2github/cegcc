.SUFFIXES:	.exe .c .h .rc .rsc .C .od .o

DIR="/storage card/e"

ARCH=		arm-mingw32ce
#
# Note - exception handling doesn't work with cegcc yet,
# only with mingw32ce
#
# ARCH=		arm-cegcc

CC=		${ARCH}-gcc ${CFLAGS}
WINDRES=	${ARCH}-windres

CFLAGS=		-g -D_WIN32_IE=0x0400 -D_WIN32_WCE=0x0400 \
		-DDATAPATH='"'$(DIR)'"'

.rc.rsc:
		${ARCH}-windres $? $@

.o.exe:
	${ARCH}-gcc ${CFLAGS} -o $@ $?

.c.exe:
	${ARCH}-gcc ${CFLAGS} -o $@ $?

.C.exe:
	${ARCH}-g++ ${CFLAGS} -o $@ $?

.exe.od:
	${ARCH}-objdump -p $? >$@

CSRC=	dialog1.c exrecord.c stdio.c twohandlers.c file.c
# CSRC=	ex1.c ex2.c ex3.c syntax.c stdio.c ex8.c ex9.c ex8b.c
CPPSRC=	# hello.C

CEXE=	${CSRC:.c=.exe}
COBJ=	${CSRC:.c=.o}
COBJDUMP=	${CSRC:.c=.od}

CPPEXE=	${CPPSRC:.C=.exe}
CPPOBJ=	${CPPSRC:.C=.o}
CPPOBJDUMP=	${CPPSRC:.C=.od}

all::	${CEXE} ${CPPEXE}

# all::	${COBJDUMP} ${CPPOBJDUMP}

clean::
	-rm -f ${CEXE} ${CPPEXE}
	-rm -f ${COBJ} ${CPPOBJ}
	-rm -f ${COBJDUMP} ${CPPOBJDUMP}

dist:	${CEXE} ${CPPEXE}
	for i in ${CEXE} ${CPPEXE} ; \
		do \
		pcp $$i :${DIR}/$$i ; \
		done

clean::
	-rm -f *.exe
	-rm -f *.od
