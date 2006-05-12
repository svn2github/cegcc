#ifndef _WCEFILE_H_
#define _WCEFILE_H_

#include <sys/wcetypes.h>
#include <sys/wcebase.h>
#include <sys/wcetime.h>

#define FILE_FLAG_WRITE_THROUGH         0x80000000
#define FILE_FLAG_OVERLAPPED            0x40000000
#define FILE_FLAG_NO_BUFFERING          0x20000000
#define FILE_FLAG_RANDOM_ACCESS         0x10000000
#define FILE_FLAG_SEQUENTIAL_SCAN       0x08000000
#define FILE_FLAG_DELETE_ON_CLOSE       0x04000000
#define FILE_FLAG_BACKUP_SEMANTICS      0x02000000
#define FILE_FLAG_POSIX_SEMANTICS       0x01000000

#define FILE_SHARE_READ                 0x00000001  
#define FILE_SHARE_WRITE                0x00000002  
#define FILE_SHARE_DELETE               0x00000004  
#define FILE_ATTRIBUTE_READONLY         0x00000001  
#define FILE_ATTRIBUTE_HIDDEN           0x00000002  
#define FILE_ATTRIBUTE_SYSTEM           0x00000004  
#define FILE_ATTRIBUTE_DIRECTORY        0x00000010  
#define FILE_ATTRIBUTE_ARCHIVE          0x00000020  
#define FILE_ATTRIBUTE_INROM	           0x00000040
#define FILE_ATTRIBUTE_ENCRYPTED        0x00000040  
#define FILE_ATTRIBUTE_NORMAL           0x00000080  
#define FILE_ATTRIBUTE_TEMPORARY        0x00000100  
#define FILE_ATTRIBUTE_SPARSE_FILE      0x00000200  
#define FILE_ATTRIBUTE_REPARSE_POINT    0x00000400  
#define FILE_ATTRIBUTE_COMPRESSED       0x00000800  
#define FILE_ATTRIBUTE_OFFLINE          0x00001000  
#define FILE_ATTRIBUTE_ROMSTATICREF			  0x00001000
#define FILE_ATTRIBUTE_NOT_CONTENT_INDEXED  0x00002000  
#define FILE_ATTRIBUTE_ROMMODULE			     0x00002000
#define FILE_NOTIFY_CHANGE_FILE_NAME    0x00000001   
#define FILE_NOTIFY_CHANGE_DIR_NAME     0x00000002   
#define FILE_NOTIFY_CHANGE_ATTRIBUTES   0x00000004   
#define FILE_NOTIFY_CHANGE_SIZE         0x00000008   
#define FILE_NOTIFY_CHANGE_LAST_WRITE   0x00000010   
#define FILE_NOTIFY_CHANGE_LAST_ACCESS  0x00000020   
#define FILE_NOTIFY_CHANGE_CREATION     0x00000040   
#define FILE_NOTIFY_CHANGE_SECURITY     0x00000100   
#define FILE_ACTION_ADDED               0x00000001   
#define FILE_ACTION_REMOVED             0x00000002   
#define FILE_ACTION_MODIFIED            0x00000003   
#define FILE_ACTION_RENAMED_OLD_NAME    0x00000004   
#define FILE_ACTION_RENAMED_NEW_NAME    0x00000005   
#define MAILSLOT_NO_MESSAGE             ((DWORD)-1) 
#define MAILSLOT_WAIT_FOREVER           ((DWORD)-1) 
#define FILE_CASE_SENSITIVE_SEARCH      0x00000001  
#define FILE_CASE_PRESERVED_NAMES       0x00000002  
#define FILE_UNICODE_ON_DISK            0x00000004  
#define FILE_PERSISTENT_ACLS            0x00000008  
#define FILE_FILE_COMPRESSION           0x00000010  
#define FILE_VOLUME_QUOTAS              0x00000020  
#define FILE_SUPPORTS_SPARSE_FILES      0x00000040  
#define FILE_SUPPORTS_REPARSE_POINTS    0x00000080  
#define FILE_SUPPORTS_REMOTE_STORAGE    0x00000100  
#define FILE_VOLUME_IS_COMPRESSED       0x00008000  
#define FILE_SUPPORTS_OBJECT_IDS        0x00010000  
#define FILE_SUPPORTS_ENCRYPTION        0x00020000  

#define FILE_TYPE_UNKNOWN          1
#define FILE_TYPE_DISK	           2
#define FILE_TYPE_CHAR	           3
#define FILE_TYPE_PIPE             4

#define CREATE_NEW          1
#define CREATE_ALWAYS       2
#define OPEN_EXISTING       3
#define OPEN_ALWAYS         4
#define TRUNCATE_EXISTING   5
#define OPEN_FOR_LOADER	    6

#define INVALID_HANDLE_VALUE (HANDLE)-1
#define INVALID_FILE_SIZE (DWORD)0xFFFFFFFF

#define FILE_BEGIN           0
#define FILE_CURRENT         1
#define FILE_END             2

/* File Structures */
typedef struct _OVERLAPPED {
  DWORD   Internal;
  DWORD   InternalHigh;
  DWORD   Offset;
  DWORD   OffsetHigh;
  HANDLE  hEvent;
} OVERLAPPED, *LPOVERLAPPED;

typedef struct _BY_HANDLE_FILE_INFORMATION {
  DWORD dwFileAttributes;
  FILETIME ftCreationTime;
  FILETIME ftLastAccessTime;
  FILETIME ftLastWriteTime;
  DWORD dwVolumeSerialNumber;
  DWORD nFileSizeHigh;
  DWORD nFileSizeLow;
  DWORD nNumberOfLinks;
  DWORD nFileIndexHigh;
  DWORD nFileIndexLow;
	 DWORD dwOID;
} BY_HANDLE_FILE_INFORMATION, *PBY_HANDLE_FILE_INFORMATION, *LPBY_HANDLE_FILE_INFORMATION;

typedef struct _WIN32_FILE_ATTRIBUTE_DATA {
  DWORD dwFileAttributes;
  FILETIME ftCreationTime;
  FILETIME ftLastAccessTime;
  FILETIME ftLastWriteTime;
  DWORD nFileSizeHigh;
  DWORD nFileSizeLow;
} WIN32_FILE_ATTRIBUTE_DATA, *LPWIN32_FILE_ATTRIBUTE_DATA;

#define MAX_PATH  (260)

typedef struct _WIN32_FIND_DATAW {
  DWORD dwFileAttributes;
  FILETIME ftCreationTime;
  FILETIME ftLastAccessTime;
  FILETIME ftLastWriteTime;
  DWORD nFileSizeHigh;
  DWORD nFileSizeLow;
  DWORD dwOID;
  WCHAR cFileName[MAX_PATH];
} WIN32_FIND_DATAW, *PWIN32_FIND_DATAW, *LPWIN32_FIND_DATAW;

#ifdef __cplusplus
extern "C" {
#endif

extern void *_fileno(void *);
extern void *_getstdfilex(int fd);

BOOL   CloseHandle(HANDLE);
LONG   CompareFileTime(const FILETIME *, const FILETIME *);
BOOL   CreateDirectoryW(LPCWSTR, LPSECURITY_ATTRIBUTES);
HANDLE CreateFile(LPCSTR, DWORD, DWORD, LPSECURITY_ATTRIBUTES, DWORD, DWORD, HANDLE);
HANDLE CreateFileW(LPCWSTR, DWORD, DWORD, LPSECURITY_ATTRIBUTES, DWORD, DWORD, HANDLE);
 BOOL   DeleteFileW(LPCWSTR);
BOOL   FileTimeToLocalFileTime(const FILETIME *, LPFILETIME);
BOOL   FileTimeToSystemTime(const FILETIME *, LPSYSTEMTIME);
BOOL   FindClose(HANDLE);
HANDLE FindFirstFlashCard(LPWIN32_FIND_DATAW);
BOOL   FindNextFlashCard(HANDLE, LPWIN32_FIND_DATAW);
HANDLE FindFirstFileW(LPCWSTR, LPWIN32_FIND_DATAW);
BOOL   FindNextFileW(HANDLE, LPWIN32_FIND_DATAW);
BOOL   FlushFileBuffers(HANDLE);
BOOL   GetDiskFreeSpaceEx(LPCWSTR, PQWORD, PQWORD, PQWORD);
BOOL   GetFileInformationByHandle(HANDLE, LPBY_HANDLE_FILE_INFORMATION);
BOOL   GetFileTime(HANDLE, LPFILETIME, LPFILETIME, LPFILETIME);
BOOL   LocalFileTimeToFileTime(const FILETIME *, LPFILETIME);
BOOL   MoveFileW(LPCWSTR, LPCWSTR);
BOOL   ReadFile(HANDLE, LPVOID, DWORD, LPDWORD, LPOVERLAPPED);
BOOL   RemoveDirectoryW(LPCWSTR);
BOOL   SetEndOfFile(HANDLE);
DWORD  SetFilePointer(HANDLE, LONG, PLONG, DWORD);
BOOL   SetFileTime (HANDLE, CONST FILETIME *, CONST FILETIME *, CONST FILETIME *);
BOOL   SystemTimeToFileTime(const SYSTEMTIME *, LPFILETIME);
BOOL   WriteFile(HANDLE, LPCVOID, DWORD, LPDWORD, LPOVERLAPPED);
BOOL   DeviceIoControl(HANDLE, DWORD, LPVOID, DWORD, LPVOID, DWORD, LPDWORD, LPOVERLAPPED);

BOOL   XCECloseHandle(HANDLE);
LONG   XCECompareFileTime(const FILETIME *, const FILETIME *);
BOOL   XCECreateDirectoryW(LPCWSTR, LPSECURITY_ATTRIBUTES);
HANDLE XCECreateFileW(LPCWSTR, DWORD, DWORD, LPSECURITY_ATTRIBUTES, DWORD, DWORD, HANDLE);
BOOL   XCEDeleteFileW(LPCWSTR);
BOOL   XCEMoveFileW(LPCWSTR, LPCWSTR);
BOOL   XCEReadFile(HANDLE, LPVOID, DWORD, LPDWORD, LPOVERLAPPED);
BOOL   XCERemoveDirectoryW(LPCWSTR);
BOOL   XCEWriteFile(HANDLE, LPCVOID, DWORD, LPDWORD, LPOVERLAPPED);


typedef int (* CONSOLE_READ_FUNC)(int, unsigned char *, int);
typedef int (* CONSOLE_WRITE_FUNC)(int, const unsigned char *, int);
typedef int (* CONSOLE_IOCTL_FUNC)(int, int, void *);

#ifdef __cplusplus
}
#endif
#endif  /* _WCEFILE_H_ */
