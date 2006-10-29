TARGET=		arm-wince-cegcc
CC=		${TARGET}-gcc
CCC=		${TARGET}-g++
WINDRES=	${TARGET}-windres
OBJDUMP=	${TARGET}-objdump
DEBUG=		-g
CFLAGS=		-D_WIN32_IE=0x0420 -D_WIN32_WCE=0x0400 ${DEBUG} -D_WINSOCKAPI_
CXXFLAGS=	-D_WIN32_IE=0x0420 -D_WIN32_WCE=0x0400 ${DEBUG} -D_WINSOCKAPI_
LDFLAGS=	-lcommctrl ${CE_VERSION}

CE_VERSION=	-Wl,--major-subsystem-version,2,--minor-subsystem-version,0

SRCS=		t.cpp draw.cpp
RCS=		draw.rc

EXE=		${SRCS:.cpp=.exe}
OFILES=		${SRCS:.cpp=.o}
DLLINFO=	${SRCS:.cpp=.dllinfo}
DUMPS=		${SRCS:.cpp=.objdump}
RSCS=		${RCS:.rc=.rsc}

.SUFFIXES:	.cpp .C .exe .dllinfo .objdump .rsc .rc

.cpp.o:
	${CCC} ${CXXFLAGS} -c -o $@ $?

.cpp.exe:
	${CCC} ${CXXFLAGS} -o $@ $?

.exe.objdump:
	${OBJDUMP} -x $? >$@

.objdump.dllinfo:
	fgrep "DLL Name: " $? >$@

.rc.rsc:
	${WINDRES} $? $@

all:	${EXE}

dist:	rolo.exe
	pcp rolo.exe ":/storage card/devel/rolo.exe"
	pcp draw.exe ":/storage card/devel/draw.exe"

clean:
	-rm -f ${EXE} ${DLLINFO} ${DUMPS} ${OFILES} ${RSCS}

draw.exe:	draw.o draw.rsc
	${CCC} ${CXXFLAGS} -o $@ draw.o draw.rsc ${LDFLAGS}
