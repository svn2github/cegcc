#ifndef _WCEBASE_H_
#define _WCEBASE_H_

#pragma GCC system_header

#include <sys/config.h>
#include <sys/wcetypes.h>

#undef _T
#define _T_(X) L##X
#define _T(X) _T_(X)

#define COUNTOF(X) (sizeof(X)/sizeof(X[0]))

#define CP_ACP 0 
#define MB_OK 0x00000000L

/* Cache manipulation constants */
#define CACHE_SYNC_DISCARD       (0x001)
#define CACHE_SYNC_INSTRUCTIONS  (0x002)
#define CACHE_SYNC_WRITEBACK	    (0x004)

#define STD_INPUT_HANDLE  0
#define STD_OUTPUT_HANDLE 1
#define STD_ERROR_HANDLE  2

/* Kernel Constants from M$ kfuncs.h */
#if defined(SARM) || defined(__arm__)
#define PUserKData ((LPBYTE)0xFFFFC800)
#else
#define PUserKData ((LPBYTE)0x00005800)
#endif
#define SYSHANDLE_OFFSET 0x004

#define NUM_SYS_HANDLES  32

#define SYS_HANDLE_BASE	 64
#define SH_WIN32                0
#define SH_CURTHREAD            1
#define SH_CURPROC              2

/* Access Types */
#define GENERIC_READ                     (0x80000000L)
#define GENERIC_WRITE                    (0x40000000L)
#define GENERIC_EXECUTE                  (0x20000000L)
#define GENERIC_ALL                      (0x10000000L)

#define DELETE                           (0x00010000L)
#define READ_CONTROL                     (0x00020000L)
#define WRITE_DAC                        (0x00040000L)
#define WRITE_OWNER                      (0x00080000L)
#define SYNCHRONIZE                      (0x00100000L)

#define STANDARD_RIGHTS_REQUIRED         (0x000F0000L)

#define STANDARD_RIGHTS_READ             (READ_CONTROL)
#define STANDARD_RIGHTS_WRITE            (READ_CONTROL)
#define STANDARD_RIGHTS_EXECUTE          (READ_CONTROL)

#define STANDARD_RIGHTS_ALL              (0x001F0000L)
#define SPECIFIC_RIGHTS_ALL              (0x0000FFFFL)

/* Constants for SystemParametersInfo */
#define SPI_GETPLATFORMTYPE              (257)
#define SPI_GETOEMINFO                   (258)

/* Toolhelp32 Constants */
#define MS_MAX_PATH                      (260)
#define TH32CS_GETALLMODS	               (0x80000000)
#define TH32CS_SNAPHEAPLIST              (0x00000001)
#define TH32CS_SNAPPROCESS               (0x00000002)
#define TH32CS_SNAPTHREAD                (0x00000004)
#define TH32CS_SNAPMODULE                (0x00000008)
#define TH32CS_SNAPALL	(TH32CS_SNAPHEAPLIST | TH32CS_SNAPPROCESS | TH32CS_SNAPTHREAD | TH32CS_SNAPMODULE)

#define HF32_DEFAULT		1

#define LF32_FIXED                       (0x00000001)
#define LF32_FREE                        (0x00000002)
#define LF32_MOVEABLE                    (0x00000004)
#define LF32_DECOMMIT                    (0x00000008)
#define LF32_BIGBLOCK                    (0x00000010)

/* DLL Constants */
#define DLL_PROCESS_DETACH 0    
#define DLL_PROCESS_ATTACH 1    
#define DLL_THREAD_ATTACH  2    
#define DLL_THREAD_DETACH  3    
#define DLL_PROCESS_EXITING 4
#define DLL_SYSTEM_STARTED 5
#define DLL_MEMORY_LOW 6

#ifndef _WINBASE_

typedef struct _SECURITY_ATTRIBUTES {
  DWORD nLength;
  LPVOID lpSecurityDescriptor;
  BOOL bInheritHandle;
} SECURITY_ATTRIBUTES, *PSECURITY_ATTRIBUTES, *LPSECURITY_ATTRIBUTES;

typedef struct _PROCESS_INFORMATION {
  HANDLE  hProcess;
  HANDLE  hThread;
  DWORD   dwProcessId;
  DWORD   dwThreadId;
} PROCESS_INFORMATION, *LPPROCESS_INFORMATION;

typedef struct _STARTUPINFOW {
  DWORD   cb;
  LPWSTR  lpReserved;
  LPWSTR  lpDesktop;
  LPWSTR  lpTitle;
  DWORD   dwX;
  DWORD   dwY;
  DWORD   dwXSize;
  DWORD   dwYSize;
  DWORD   dwXCountChars;
  DWORD   dwYCountChars;
  DWORD   dwFillAttribute;
  DWORD   dwFlags;
  WORD    wShowWindow;
  WORD    cbReserved2;
  LPBYTE  lpReserved2;
  HANDLE  hStdInput;
  HANDLE  hStdOutput;
  HANDLE  hStdError;
} STARTUPINFOW, *LPSTARTUPINFOW;

typedef struct _SYSTEM_INFO {
  union {
    DWORD dwOemId;          /* Obsolete field...do not use */
    struct {
      WORD wProcessorArchitecture;
      WORD wReserved;
    };
  };
  DWORD dwPageSize;
  LPVOID lpMinimumApplicationAddress;
  LPVOID lpMaximumApplicationAddress;
  DWORD dwActiveProcessorMask;
  DWORD dwNumberOfProcessors;
  DWORD dwProcessorType;
  DWORD dwAllocationGranularity;
  WORD wProcessorLevel;
  WORD wProcessorRevision;
} SYSTEM_INFO, *LPSYSTEM_INFO;

typedef struct _OSVERSIONINFOW {
  DWORD dwOSVersionInfoSize;
  DWORD dwMajorVersion;
  DWORD dwMinorVersion;
  DWORD dwBuildNumber;
  DWORD dwPlatformId;
  WCHAR  szCSDVersion[128];
} OSVERSIONINFOW, *POSVERSIONINFOW, *LPOSVERSIONINFOW;

typedef struct _SYSTEM_POWER_STATUS_EX2 {
  BYTE ACLineStatus;
  BYTE BatteryFlag;
  BYTE BatteryLifePercent;
  BYTE Reserved1;
  DWORD BatteryLifeTime;
  DWORD BatteryFullLifeTime;
  BYTE Reserved2;
  BYTE BackupBatteryFlag;
  BYTE BackupBatteryLifePercent;
  BYTE Reserved3;
  DWORD BackupBatteryLifeTime;
  DWORD BackupBatteryFullLifeTime;
  DWORD BatteryVoltage;
  DWORD BatteryCurrent;
  DWORD BatteryAverageCurrent;
  DWORD BatteryAverageInterval;
  DWORD BatterymAHourConsumed;
  DWORD BatteryTemperature;
  DWORD BackupBatteryVoltage;
  BYTE  BatteryChemistry;
} SYSTEM_POWER_STATUS_EX2, *PSYSTEM_POWER_STATUS_EX2, *LPSYSTEM_POWER_STATUS_EX2;

/* Toolhelp 32 Stuff */
typedef struct _HEAPLIST32 {
  DWORD dwSize;
  DWORD th32ProcessID;
  DWORD th32HeapID;
  DWORD dwFlags;
} HEAPLIST32, *PHEAPLIST32, *LPHEAPLIST32;

typedef struct _HEAPENTRY32 {
  DWORD dwSize;
  HANDLE hHandle;
  DWORD dwAddress;
  DWORD dwBlockSize;
  DWORD dwFlags;
  DWORD dwLockCount;
  DWORD dwResvd;
  DWORD th32ProcessID;
  DWORD th32HeapID;
} HEAPENTRY32, *PHEAPENTRY32, *LPHEAPENTRY32;

typedef struct _PROCESSENTRY32 {
  DWORD dwSize;
  DWORD cntUsage;
  DWORD th32ProcessID;
  DWORD th32DefaultHeapID;
  DWORD th32ModuleID;
  DWORD cntThreads;
  DWORD th32ParentProcessID;
  LONG  pcPriClassBase;
  DWORD dwFlags;
  TCHAR szExeFile[MS_MAX_PATH];
  DWORD	th32MemoryBase;
  DWORD	th32AccessKey;
} PROCESSENTRY32, *PPROCESSENTRY32, *LPPROCESSENTRY32;

typedef struct _THREADENTRY32 {
  DWORD dwSize;
  DWORD cntUsage;
  DWORD th32ThreadID;
  DWORD th32OwnerProcessID;
  LONG  tpBasePri;
  LONG  tpDeltaPri;
  DWORD dwFlags;
  DWORD	th32AccessKey;
  DWORD	th32CurrentProcessID;
} THREADENTRY32, *PTHREADENTRY32, *LPTHREADENTRY32;

typedef struct _MODULEENTRY32 {
  DWORD dwSize;
  DWORD th32ModuleID;
  DWORD th32ProcessID;
  DWORD GlblcntUsage;
  DWORD ProccntUsage;
  BYTE *modBaseAddr;
  DWORD modBaseSize;
  HMODULE hModule;
  TCHAR szModule[MS_MAX_PATH];
  TCHAR szExePath[MS_MAX_PATH];
  DWORD	dwFlags;
} MODULEENTRY32, *PMODULEENTRY32, *LPMODULEENTRY32;

#endif

#ifndef _WINDEF_
typedef struct _POINT
{
  LONG x;
  LONG y;
} POINT;

typedef struct _MSG
{
  HWND hwnd;
  UINT message;
  WPARAM wParam;
  LPARAM lParam;
  DWORD time;
  POINT pt;
} MSG;

#endif

#ifdef __cplusplus
extern "C" {
#endif

DWORD  CacheSync(DWORD);
BOOL   CreateProcessW(LPCWSTR, LPCWSTR, LPSECURITY_ATTRIBUTES, LPSECURITY_ATTRIBUTES, BOOL, DWORD, LPVOID, LPWSTR, LPSTARTUPINFOW, LPPROCESS_INFORMATION);

//VOID   DebugBreak();
#ifndef DebugBreak
#define DebugBreak() asm( ".word 0xE6000010" )
#endif

int    ExtEscape(HDC, int, int, LPCSTR, int, LPSTR);
BOOL   FlushFileBuffers(HANDLE);
BOOL   FlushInstructionCache(HANDLE, LPCVOID, DWORD);
LPWSTR GetCommandLineW();
HDC    GetDC(HWND);
BOOL   GetExitCodeProcess(HANDLE, DWORD*);
DWORD  GetLastError();
DWORD  GetModuleFileNameW(HINSTANCE, LPWSTR, DWORD);
HINSTANCE GetModuleHandleW(LPCWSTR);

typedef int (*FARPROC)();

FARPROC GetProcAddressA(HINSTANCE, LPCSTR);
FARPROC GetProcAddressW(HINSTANCE, LPCWSTR);
VOID   GetSystemInfo(LPSYSTEM_INFO);
DWORD  GetSystemPowerStatusEx2(PSYSTEM_POWER_STATUS_EX2, DWORD, BOOL);
DWORD  GetTickCount();
LONG   InterlockedIncrement(LPLONG);
LONG   InterlockedDecrement(LPLONG);
LONG   InterlockedExchange(LPLONG, LONG);
#define InterlockedTestExchange(TGT, oldval, newval) \
  InterlockedCompareExchange((TGT), (newval), (oldval))
//LONG   InterlockedTestExchange(LPLONG, LONG, LONG);
HINSTANCE LoadLibraryW(LPCWSTR);
HINSTANCE LoadLibraryExW(LPCWSTR, HANDLE, DWORD);
HANDLE OpenProcess(DWORD, BOOL, DWORD);
BOOL   ReadProcessMemory(HANDLE, LPCVOID, LPVOID, DWORD, LPDWORD);
int    ReleaseDC(HWND, HDC);
BOOL   WriteProcessMemory(HANDLE, LPVOID, LPVOID, DWORD, LPDWORD);
VOID   SetLastError(DWORD);
VOID   Sleep(DWORD);
BOOL   SystemParametersInfoW(UINT, UINT, PVOID, UINT);
BOOL   TerminateProcess(HANDLE, DWORD);

/* Toolhelp Methods */
HANDLE CreateToolhelp32Snapshot(DWORD, DWORD);
BOOL   CloseToolhelp32Snapshot(HANDLE);

#ifndef _WINBASE_
BOOL   Process32First(HANDLE, LPPROCESSENTRY32);
BOOL   Process32Next(HANDLE, LPPROCESSENTRY32);
BOOL   Thread32First(HANDLE, LPTHREADENTRY32);
BOOL   Thread32Next(HANDLE, LPTHREADENTRY32);
BOOL   Module32First(HANDLE, LPMODULEENTRY32);
BOOL   Module32Next(HANDLE, LPMODULEENTRY32);
BOOL   Heap32ListFirst(HANDLE, LPHEAPLIST32);
BOOL   Heap32ListNext(HANDLE, LPHEAPLIST32);
#endif

#ifdef __cplusplus
}
#endif


#if 1
/* it is exported in coredll */
LONG InterlockedCompareExchange(LPLONG Destination, LONG Exchange, LONG Comperand);
#else

#define InterlockedCompareExchange(ptr, newval, oldval) \
	((PVOID)InterlockedTestExchange((LPLONG)ptr, (LONG)oldval, (LONG)newval))

#endif

/* Process/Thread ID Methods */
#define GetCurrentThread() ((HANDLE)(SH_CURTHREAD+SYS_HANDLE_BASE))
#define GetCurrentProcess() ((HANDLE)(SH_CURPROC+SYS_HANDLE_BASE))
#define GetCurrentThreadId() ((DWORD)(((HANDLE *)(PUserKData+SYSHANDLE_OFFSET))[SH_CURTHREAD]))
#define GetCurrentProcessId() ((DWORD)(((HANDLE *)(PUserKData+SYSHANDLE_OFFSET))[SH_CURPROC]))

#endif  /* _WCEBASE_H_ */
