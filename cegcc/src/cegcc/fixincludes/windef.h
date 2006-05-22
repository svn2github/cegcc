#ifndef _WINDEF_H_FIXED_H_
#define _WINDEF_H_FIXED_H_

#include "fixincl.h"
#include_next "windef.h"
#undef max
#undef LOWORD
#define LOWORD(l)           ((WORD)(DWORD)(l))

#endif
