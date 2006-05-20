#ifndef __KFUNCS_H_FIXED__
# define __KFUNCS_H_FIXED__

#include "fixincl.h"
#include_next "kfuncs.h"

#undef DebugBreak()
#define DebugBreak() asm( ".word 0xE6000010" )

#endif
