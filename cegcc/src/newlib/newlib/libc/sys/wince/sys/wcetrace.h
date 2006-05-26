#ifndef _WCETRACE_H_
#define _WCETRACE_H_

#include <sys/config.h>

/* Define CE_NOTRACE to disable function tracing */

#define WCE_IO        0x0008
#define WCE_NETWORK   0x0010
#define WCE_SIGNALS   0x0020
#define WCE_FIFOS     0x0040
#define WCE_TIME      0x0080
#define WCE_SYNCH     0x0100
#define WCE_MALLOC    0x0200
#define WCE_VM        0x0400

/* user application reserved */
#define WCE_APP       0x8000

#define WCE_ALL \
    (WCE_IO | \
     WCE_NETWORK | \
     WCE_SIGNALS | \
     WCE_FIFOS | \
     WCE_TIME | \
     WCE_SYNCH | \
     WCE_MALLOC | \
     WCE_VM)

#ifdef __cplusplus
extern "C" {
#endif

#ifndef CE_NOTRACE
void WCETRACEGETENV(void);
void WCETRACESET(int trace);
int  WCETRACEGET(void);
void WCETRACE(int level, const char *fmt, ...);
void WCETRACE_DEBUGGER_SET(int trace);
int  WCETRACE_DEBUGGER_GET(void);
void WCETRACECLOSE(void);
void __WCETraceError(int level, unsigned long werr, const char* funct);
#define WCETRACE_ERROR(T, ERR) __WCETraceError(T, ERR, __FUNCTION__)
#else
#define WCETRACEGETENV() do {} while (0)
#define WCETRACESET(trace) do {} while (0)
#define WCETRACEGET() do {} while (0)
#define WCETRACE(...) do {} while (0)
#define WCETRACE_DEBUGGER_SET(trace) do {} while (0)
#define WCETRACE_DEBUGGER_GET() do {} while (0)
#define WCETRACECLOSE() do {} while (0)
#define WCETRACE_ERROR(T, ERR) do {} while (0)
#endif

#ifdef __cplusplus
}
#endif

#endif  /* _WCETRACE_H_ */
