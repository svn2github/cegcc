# Target: Acorn RISC machine (ARM) with simulator
TDEPFILES= arm-tdep.o win32-nat.o wince.o corelow.o
DEPRECATED_TM_FILE= tm-wince.h
MT_CFLAGS=-D__arm__ -DARM \
		  -U_X86_ -U_M_IX86 -U__i386__ -U__i486__ -U__i586__ -U__i686__ \
		  -DUNICODE -DUNDER_CE -D_WIN32_WCE -DWINCE_STUB='"${target_alias}-stub.exe"'
TM_CLIBS=-lrapi
