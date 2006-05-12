#include <reent.h>

__IMPORT int
_DEFUN_VOID (vfork)
{
  return _vfork_r (_REENT);
}
