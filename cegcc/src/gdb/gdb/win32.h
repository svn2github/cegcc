#ifndef WIN32_H
#define WIN32_H

#include <windows.h>
#include "frame.h"		/* required by inferior.h */

/* Thread information structure used to track information that is
not available in gdb's thread structure. */
typedef struct thread_info_struct
{
  struct thread_info_struct *next;
  DWORD id;
  HANDLE h;
  char *name;
  int suspend_count;
  int reload_context;
  CONTEXT context;
#if defined(MIPS) || defined(ARM) || defined(SH4)
  int stepped;		/* True if stepped.  */
  CORE_ADDR step_pc;
  unsigned long step_prev;
#else
  STACKFRAME sf;
#endif
}
thread_info;

typedef struct win32_target_iface_t
{
  BOOL WINAPI (*CloseHandle) ( HANDLE hObject );
  BOOL WINAPI (*ContinueDebugEvent) ( DWORD dwProcessId, DWORD dwThreadId, DWORD dwContinueStatus );
  BOOL WINAPI (*CreateProcessA) (LPCSTR, LPSTR, LPSECURITY_ATTRIBUTES, LPSECURITY_ATTRIBUTES,
                            BOOL, DWORD, PVOID,LPCSTR, LPSTARTUPINFOA, LPPROCESS_INFORMATION);
  BOOL WINAPI (*DebugActiveProcess) ( DWORD dwProcessId );
  BOOL WINAPI (*GetThreadContext) ( HANDLE hThread, LPCONTEXT lpContext );
  BOOL WINAPI (*FlushInstructionCache) ( HANDLE hProcess, LPCVOID lpBaseAddress, DWORD dwSize );
  HANDLE WINAPI (*OpenProcess) ( DWORD fdwAccess, BOOL fInherit, DWORD IDProcess );
  BOOL WINAPI (*ReadProcessMemory) ( HANDLE hProcess, LPCVOID lpBaseAddress, 
                              LPVOID lpBuffer, DWORD nSize, LPDWORD lpNumberOfBytesRead );
  DWORD WINAPI (*ResumeThread) ( HANDLE hThread );
  BOOL WINAPI (*SetThreadContext) ( HANDLE hThread, CONST CONTEXT *lpContext );
  DWORD WINAPI (*SuspendThread) ( HANDLE hThread );
  BOOL WINAPI (*TerminateProcess) ( HANDLE hProcess, UINT uExitCode );
  BOOL WINAPI (*WaitForDebugEvent) ( LPDEBUG_EVENT lpDebugEvent, DWORD dwMilliseconds );
  DWORD WINAPI (*WaitForSingleObject) ( HANDLE hHandle, DWORD dwMilliseconds );
  BOOL WINAPI (*WriteProcessMemory) (HANDLE, LPVOID, LPCVOID, SIZE_T, SIZE_T*);
} win32_target_iface_t;

extern win32_target_iface_t win32_target_iface;

#define DEBUG_CloseHandle win32_target_iface.CloseHandle
#define DEBUG_ContinueDebugEvent win32_target_iface.ContinueDebugEvent
#define DEBUG_CreateProcessA win32_target_iface.CreateProcessA
#define DEBUG_DebugActiveProcess win32_target_iface.DebugActiveProcess
#define DEBUG_GetThreadContext win32_target_iface.GetThreadContext
#define DEBUG_FlushInstructionCache win32_target_iface.FlushInstructionCache
#define DEBUG_OpenProcess win32_target_iface.OpenProcess
#define DEBUG_ReadProcessMemory win32_target_iface.ReadProcessMemory
#define DEBUG_ResumeThread win32_target_iface.ResumeThread
#define DEBUG_SetThreadContext win32_target_iface.SetThreadContext
#define DEBUG_SuspendThread win32_target_iface.SuspendThread
#define DEBUG_TerminateProcess win32_target_iface.TerminateProcess
#define DEBUG_WaitForDebugEvent win32_target_iface.WaitForDebugEvent
#define DEBUG_WaitForSingleObject win32_target_iface.WaitForSingleObject
#define DEBUG_WriteProcessMemory win32_target_iface.WriteProcessMemory

/* From win32-nat.c. Should be renamed, put in struct, whatever is needed
   to hide them from the global namespace. */
extern const int mappings[];
extern HANDLE current_process_handle;	/* Currently executing process */
extern thread_info *current_thread;	/* Info on currently selected thread */
extern struct so_list solib_start, *solib_end;
extern int max_dll_name_len;

#define check_for_step(a, x) (x)

#ifdef ARM
#undef check_for_step
enum target_signal check_for_step (DEBUG_EVENT *ev, enum target_signal x);
#endif

#ifndef _WIN32_WCE
# define HAVE_CREATE_NEW_PROCESS_GROUP
#endif

thread_info *thread_rec (DWORD id, int get_context);
void win32_load (char *file, int from_tty);
void win32_stop_stub_comm( void );

#ifdef _WIN32_WCE
void _initialize_wince (void);
void wince_initialize (void);
void wince_insert_breakpoint (thread_info *th, CORE_ADDR where);

char *upload_to_device (const char *to, const char *from);
void _init_win32_wce_iface (void);
#endif

#endif
