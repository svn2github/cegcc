#include "fixincl.h"
#include_next "windef.h"
#undef max
#undef LOWORD
#define LOWORD(l)           ((WORD)(DWORD)(l))
