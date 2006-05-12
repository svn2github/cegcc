#ifndef _WCEMEMORY_H_
#define _WCEMEMORY_H_

#include <sys/wcetypes.h>
#include <sys/wcebase.h>

#define PAGE_NOACCESS            (0x01)
#define PAGE_READONLY            (0x02)
#define PAGE_READWRITE           (0x04)
#define PAGE_WRITECOPY           (0x08)
#define PAGE_EXECUTE             (0x10)
#define PAGE_EXECUTE_READ        (0x20)
#define PAGE_EXECUTE_READWRITE   (0x40)
#define PAGE_EXECUTE_WRITECOPY   (0x80)
#define PAGE_GUARD               (0x100)
#define PAGE_NOCACHE             (0x200)
#define PAGE_PHYSICAL		          (0x400)
#define PAGE_WRITECOMBINE        (0x400)
#define MEM_COMMIT               (0x1000)
#define MEM_RESERVE              (0x2000)
#define MEM_DECOMMIT             (0x4000)
#define MEM_RELEASE              (0x8000)
#define MEM_FREE                 (0x10000)
#define MEM_PRIVATE              (0x20000)
#define MEM_MAPPED               (0x40000)
#define MEM_RESET                (0x80000)
#define MEM_TOP_DOWN             (0x100000)
#define MEM_AUTO_COMMIT          (0x200000)
#define MEM_4MB_PAGES            (0x80000000)
#define SEC_FILE                 (0x800000)
#define SEC_IMAGE                (0x1000000)
#define SEC_VLM                  (0x2000000)
#define SEC_RESERVE              (0x4000000)
#define SEC_COMMIT               (0x8000000)
#define SEC_NOCACHE              (0x10000000)
#define MEM_IMAGE                (SEC_IMAGE)

#define SECTION_QUERY       (0x0001)
#define SECTION_MAP_WRITE   (0x0002)
#define SECTION_MAP_READ    (0x0004)
#define SECTION_MAP_EXECUTE (0x0008)
#define SECTION_EXTEND_SIZE (0x0010)

#define SECTION_ALL_ACCESS (STANDARD_RIGHTS_REQUIRED|SECTION_QUERY|\
                            SECTION_MAP_WRITE |      \
                            SECTION_MAP_READ |       \
                            SECTION_MAP_EXECUTE |    \
                            SECTION_EXTEND_SIZE)

#define FILE_MAP_WRITE      SECTION_MAP_WRITE
#define FILE_MAP_READ       SECTION_MAP_READ
#define FILE_MAP_ALL_ACCESS SECTION_ALL_ACCESS

typedef struct _MEMORYSTATUS {
  DWORD dwLength;
  DWORD dwMemoryLoad;
  DWORD dwTotalPhys;
  DWORD dwAvailPhys;
  DWORD dwTotalPageFile;
  DWORD dwAvailPageFile;
  DWORD dwTotalVirtual;
  DWORD dwAvailVirtual;
} MEMORYSTATUS, *LPMEMORYSTATUS;

typedef struct _MEMORY_BASIC_INFORMATION {
  PVOID BaseAddress;
  PVOID AllocationBase;
  DWORD AllocationProtect;
  DWORD RegionSize;
  DWORD State;
  DWORD Protect;
  DWORD Type;
} MEMORY_BASIC_INFORMATION, *PMEMORY_BASIC_INFORMATION;

#ifdef __cplusplus
extern "C" {
#endif

HANDLE CreateFileForMappingW(LPCWSTR, DWORD, DWORD, LPSECURITY_ATTRIBUTES, DWORD, DWORD,	HANDLE);
HANDLE CreateFileMappingW(HANDLE,  LPSECURITY_ATTRIBUTES, DWORD, DWORD, DWORD, LPCWSTR);
BOOL   FlushViewOfFile(LPCVOID, DWORD);
VOID   GlobalMemoryStatus(LPMEMORYSTATUS);
LPVOID MapViewOfFile(HANDLE, DWORD, DWORD, DWORD, DWORD);
BOOL   UnmapViewOfFile(LPCVOID);
LPVOID VirtualAlloc(LPVOID, DWORD, DWORD, DWORD);
BOOL   VirtualFree(LPVOID, DWORD, DWORD);
DWORD  VirtualQuery(LPCVOID, PMEMORY_BASIC_INFORMATION, DWORD);

#ifdef __cplusplus
}
#endif
#endif  /* _WCEMEMORY_H_ */
