#include "fixincl.h"
#include_next "kfuncs.h"

#undef DebugBreak
#define DebugBreak() asm( ".word 0xE6000010" )
