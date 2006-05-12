#ifndef _WCEMACHINE_H_
#define _WCEMACHINE_H_

#include <sys/wcetypes.h>
#include <sys/wcebase.h>

#if defined(SARM) || defined(__arm__)

/* These flags control the contents of the CONTEXT structure */
#define CONTEXT_ARM      (0x0000040)
#define CONTEXT_CONTROL  (CONTEXT_ARM | 0x00000001L)
#define CONTEXT_INTEGER  (CONTEXT_ARM | 0x00000002L)

#define CONTEXT_FULL (CONTEXT_CONTROL | CONTEXT_INTEGER)

/*
 * Context Frame:
 * This frame is used to store a limited processor context into the
 * Thread structure for CPUs which have no floating point support.
 */

typedef struct _CONTEXT {
/* The flags values within this flag control the contents of
 * a CONTEXT record.
 *
 * If the context record is used as an input parameter, then
 * for each portion of the context record controlled by a flag
 * whose value is set, it is assumed that that portion of the
 * context record contains valid context. If the context record
 * is being used to modify a thread's context, then only that
 * portion of the threads context will be modified.
 *
 * If the context record is used as an IN OUT parameter to capture
 * the context of a thread, then only those portions of the thread's
 * context corresponding to set flags will be returned.
 *
 * The context record is never used as an OUT only parameter.
 */

  ULONG ContextFlags;

  
  /* This section is specified/returned if the ContextFlags word contains
   * the flag CONTEXT_INTEGER
   */
  ULONG R0;
  ULONG R1;
  ULONG R2;
  ULONG R3;
  ULONG R4;
  ULONG R5;
  ULONG R6;
  ULONG R7;
  ULONG R8;
  ULONG R9;
  ULONG R10;
  ULONG R11;
  ULONG R12;

  /*
   * This section is specified/returned if the ContextFlags word contains
   * the flag CONTEXT_CONTROL.
   */
  ULONG Sp;
  ULONG Lr;
  ULONG Pc;
  ULONG Psr;
} CONTEXT;

typedef CONTEXT *PCONTEXT, *LPCONTEXT;
#endif  /* SARM */

#ifdef __cplusplus
extern "C" {
#endif


#ifdef __cplusplus
}
#endif
#endif  /* _WCEMACHINE_H_ */
