#ifndef _WCEPROCESS_H_
#define _WCEPROCESS_H_

#include "sys/wcetypes.h"
#include "sys/wcetime.h"
#include "sys/wcemachine.h"

#include <stdint.h>

__IMPORT int ExecuteProcessA(const char *commandline, BOOL bWait, LPDWORD lpdwProcId);

#endif  /* _WCETHREAD_H_ */
