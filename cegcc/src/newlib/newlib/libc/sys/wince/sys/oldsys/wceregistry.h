#ifndef _WCEREGISTRY_H_
#define _WCEREGISTRY_H_

#include <_ansi.h>

#include "sys/wcetypes.h"
#include "sys/wcebase.h"
#include "sys/wcetime.h"

#define HKEY_CLASSES_ROOT    (( HKEY ) (ULONG *)0x80000000 )
#define HKEY_CURRENT_USER    (( HKEY ) (ULONG *)0x80000001 )
#define HKEY_LOCAL_MACHINE   (( HKEY ) (ULONG *)0x80000002 )
#define HKEY_USERS           (( HKEY ) (ULONG *)0x80000003 )
#define KEY_QUERY_VALUE      (0x0001)

#define REG_NONE                (0)     /* No value type */
#define REG_SZ                  (1)     /* Unicode null terminated string */
#define REG_EXPAND_SZ           (2)     /* Unicode null terminated string */
#define REG_BINARY              (3)     /* Free form binary */
#define REG_DWORD               (4)     /* 32-bit number */
#define REG_DWORD_LITTLE_ENDIAN (4)     /* 32-bit number (same as REG_DWORD) */
#define REG_DWORD_BIG_ENDIAN    (5)     /* 32-bit number */
#define REG_LINK                (6)     /* Symbolic Link (unicode) */
#define REG_MULTI_SZ            (7)     /* Multiple Unicode strings */
#define REG_RESOURCE_LIST       (8)     /* Resource list in the resource map */
#define REG_FULL_RESOURCE_DESCRIPTOR (9)  /* Resource list in the hardware description */
#define REG_RESOURCE_REQUIREMENTS_LIST (10)

#define KEY_QUERY_VALUE         (0x0001)
#define KEY_SET_VALUE           (0x0002)
#define KEY_CREATE_SUB_KEY      (0x0004)
#define KEY_ENUMERATE_SUB_KEYS  (0x0008)
#define KEY_NOTIFY              (0x0010)
#define KEY_CREATE_LINK         (0x0020)

#define KEY_READ                ((STANDARD_RIGHTS_READ| KEY_QUERY_VALUE | \
	KEY_ENUMERATE_SUB_KEYS |KEY_NOTIFY)  \
	& (~SYNCHRONIZE))


#define KEY_WRITE               ((STANDARD_RIGHTS_WRITE | KEY_SET_VALUE | KEY_CREATE_SUB_KEY) \
	& (~SYNCHRONIZE))

#define KEY_EXECUTE             ((KEY_READ) & (~SYNCHRONIZE))

#define KEY_ALL_ACCESS          ((STANDARD_RIGHTS_ALL | KEY_QUERY_VALUE | KEY_SET_VALUE |\
	KEY_CREATE_SUB_KEY | KEY_ENUMERATE_SUB_KEYS | KEY_NOTIFY | KEY_CREATE_LINK) \
	& (~SYNCHRONIZE))


typedef ACCESS_MASK REGSAM;

#ifdef __cplusplus
extern "C" {
#endif

LONG  RegCloseKey(HKEY);
LONG  RegCreateKeyExW(HKEY, LPCWSTR, DWORD, LPWSTR, DWORD, REGSAM, LPSECURITY_ATTRIBUTES, PHKEY, LPDWORD);
LONG  RegDeleteKeyW(HKEY, LPCWSTR);
LONG  RegDeleteValueW(HKEY, LPCWSTR);
LONG  RegEnumKeyExW(HKEY, DWORD, LPWSTR, LPDWORD, LPDWORD, LPWSTR, LPDWORD, PFILETIME);
LONG  RegEnumValueW(HKEY, DWORD, LPWSTR, LPDWORD, LPDWORD, LPDWORD, LPBYTE, LPDWORD);
LONG  RegFlushKey(HKEY);
LONG  RegOpenKeyExW(HKEY, LPCWSTR, DWORD, REGSAM, PHKEY);
LONG  RegQueryInfoKeyW(HKEY, LPWSTR, LPDWORD, LPDWORD, LPDWORD, LPDWORD, LPDWORD, LPDWORD, LPDWORD, LPDWORD, LPDWORD, PFILETIME);
LONG  RegQueryValueExW(HKEY, LPCWSTR, LPDWORD, LPDWORD, LPBYTE, LPDWORD);
LONG  RegSetValueExW(HKEY, LPCWSTR, DWORD, DWORD, CONST BYTE*, DWORD);

#ifdef __cplusplus
}
#endif
#endif  /* _WCEREGISTRY_H_ */
