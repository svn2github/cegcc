// registry.c
//
// Time-stamp: <15/02/02 20:50:46 keuchel@netwave.de>

#include "sys/wceregistry.h"
#include "sys/wcebase.h"

#include <stdlib.h>

LONG __IMPORT
XCERegCreateKeyExA(HKEY hKey,         
				const char *subkey,  
				DWORD dwRes,
				LPSTR lpszClass,
				DWORD ulOptions,   
				REGSAM samDesired,
				LPSECURITY_ATTRIBUTES sec_att,
				PHKEY phkResult,
				DWORD *lpdwDisp
				)
{
	long res;
	wchar_t subkeyw[256];

	MultiByteToWideChar(CP_ACP, 0, subkey, -1, subkeyw, COUNTOF(subkeyw));

	res = RegCreateKeyExW(hKey, subkeyw, dwRes, NULL, ulOptions, 
		samDesired, NULL, phkResult, lpdwDisp);

	return res;
}

LONG __IMPORT
XCERegOpenKeyExA(HKEY hKey,         
			  const char *subkey,  
			  DWORD ulOptions,   
			  REGSAM samDesired,
			  PHKEY phkResult
			  )
{
	long res;
	wchar_t subkeyw[256];

	MultiByteToWideChar(CP_ACP, 0, subkey, -1, subkeyw, COUNTOF(subkeyw));

	res = RegOpenKeyExW(hKey, subkeyw, ulOptions, samDesired, phkResult);

	return res;
}

LONG __IMPORT
XCERegQueryValueExA(  
				 HKEY hKey,           
				 const char *valname,  
				 LPDWORD lpReserved,  
				 LPDWORD lpType,      
				 LPBYTE lpData,       
				 LPDWORD lpcbData     
				 )
{
	wchar_t valnamew[256];
	LONG res;
	LPBYTE lpDataNew = NULL;
	DWORD dwDataSize;
	DWORD dword_spare = 0;
	LPDWORD lpType_spare = &dword_spare;

	if(lpData != NULL)
	{
		dwDataSize = *lpcbData * 2;
		lpDataNew = malloc(dwDataSize);
	}

	// suggested by W. Garland
	if (lpType == NULL)
		lpType = lpType_spare;

	MultiByteToWideChar(CP_ACP, 0, valname, -1, valnamew, COUNTOF(valnamew));

	res = RegQueryValueExW(hKey, valnamew, lpReserved, lpType, lpDataNew,
		&dwDataSize);

	if(res != 0)
	{
		free(lpDataNew);
		return res;
	}

	if(lpData)
	{
		if(*lpType == REG_SZ)
		{
			WideCharToMultiByte(CP_ACP, 0, 
				(wchar_t *)lpDataNew, *lpcbData, 
				(char *) lpData, *lpcbData, 
				NULL, NULL);
			*lpcbData = dwDataSize/2;
		}
		else
		{
			*lpcbData = dwDataSize;
			memcpy(lpData, lpDataNew, *lpcbData);
		}
	}
	else // query only...
	{
		if(lpcbData)
			*lpcbData = dwDataSize;
	}

	free(lpDataNew);

	return 0;
}

LONG __IMPORT
XCERegSetValueExA(  
			   HKEY hKey,           
			   const char *valname,  
			   DWORD dwReserved,  
			   DWORD dwType,      
			   LPBYTE lpData,       
			   DWORD dwSize
			   )
{
	wchar_t valnamew[256];
	LONG res;
	// suggested by W. Garland
	LPBYTE lpDataNew = lpData;
	DWORD dwDataSize = dwSize;

	MultiByteToWideChar(CP_ACP, 0, valname, -1, valnamew, COUNTOF(valnamew));

	if(dwType == REG_SZ || dwType == REG_EXPAND_SZ)
	{
		dwDataSize = dwSize * 2;
		lpDataNew = malloc(dwDataSize);

		MultiByteToWideChar(CP_ACP, 0, lpData, -1, (wchar_t *)lpDataNew, 
			dwDataSize);
	}

	res = RegSetValueExW(hKey, valnamew, dwReserved, dwType, (BYTE*)lpDataNew,
		dwDataSize);

	// doesn't this possibly free lpData?
	free(lpDataNew);

	return res;
}

LONG __IMPORT
XCERegEnumValueA(
			  HKEY hKey,              
			  DWORD dwIndex,          
			  char *lpValueName,     
			  LPDWORD lpcbValueName,  
			  LPDWORD lpReserved,     
			  LPDWORD lpType,         
			  LPBYTE lpData,          
			  LPDWORD lpcbData        
			  )
{
	long res;
	DWORD dwValueSize = 0;
	DWORD dwDataSize = 0;
	wchar_t *lpValueNameW = NULL;
	LPBYTE lpDataNew = NULL;

	if(lpcbValueName) // size in characters
		dwValueSize = *lpcbValueName;
	if(lpcbData)
		dwDataSize = *lpcbData;

	if(lpValueName != NULL)
		lpValueNameW = malloc(dwValueSize*2);

	if(lpData != NULL)
		lpDataNew = malloc(dwDataSize*2);

	res = RegEnumValueW(hKey, dwIndex, lpValueNameW, &dwValueSize,
		lpReserved, lpType, lpDataNew, &dwDataSize);

	if(res == 0)
	{
		if(lpValueName)
		{
			WideCharToMultiByte(CP_ACP, 0, 
				(wchar_t *)lpValueNameW, *lpcbValueName, 
				(char *) lpValueName, *lpcbValueName, 
				NULL, NULL);
		}

		if(lpcbValueName)
			*lpcbValueName = dwValueSize;

		if(lpData)
		{
			if(*lpType == REG_SZ)
			{
				WideCharToMultiByte(CP_ACP, 0, 
					(wchar_t *)lpDataNew, *lpcbData, 
					(char *) lpData, *lpcbData, 
					NULL, NULL);
			}
			else
			{
				*lpcbData = dwDataSize;
				memcpy(lpData, lpDataNew, *lpcbData);
			}
		}

		if(lpcbData)
			*lpcbData = dwDataSize;
	}

	free(lpValueNameW);
	free(lpDataNew);

	return res;
}

typedef char* LPCTSTR;

LONG __IMPORT
XCERegDeleteKeyA(
			  HKEY  hKey,
			  LPCTSTR  lpszSubKey 
			  )
{
	WCHAR *lpszSubKeyW;
	int len;

	len = strlen(lpszSubKey);
	lpszSubKeyW = alloca(2*(len+1));

	MultiByteToWideChar(CP_ACP, 0, lpszSubKey, -1, lpszSubKeyW, len+1);

	return RegDeleteKeyW(hKey, lpszSubKeyW);
}

LONG __IMPORT 
XCERegEnumKeyExA(
			  HKEY  hKey,	
			  DWORD  iSubkey,
			  LPSTR  lpszName,
			  LPDWORD  lpcchName,
			  LPDWORD  lpdwReserved,
			  LPSTR  lpszClass,
			  LPDWORD  lpcchClass,
			  PFILETIME  lpftLastWrite
			  )
{
	WCHAR lpszNameW[256];
	WCHAR lpszClassW[256];
	int res;
	int old_name_size = 0;
	int old_class_size = 0;

	old_name_size = *lpcchName;

	if(lpszClass)
		old_class_size = *lpcchClass;

	res = RegEnumKeyExW(hKey, iSubkey, lpszNameW, lpcchName, lpdwReserved,
		lpszClassW, lpcchClass, lpftLastWrite);

	if(res == 0)
	{
		WideCharToMultiByte(CP_ACP, 0, lpszNameW, -1, 
			lpszName, old_name_size, 0, 0);
		if(lpszClass)
		{
			WideCharToMultiByte(CP_ACP, 0, lpszClassW, -1, 
				lpszClass, old_class_size, 0, 0);
		}
	}

	return res;
}

//////////////////////////////////////////////////////////////////////

LONG __IMPORT
XCERegCreateKeyA(
			  HKEY  hKey,
			  LPCTSTR  lpszSubKey,
			  PHKEY  phkResult
			  )
{
	DWORD dwDisp;

	return XCERegCreateKeyExA(hKey, lpszSubKey, 0, NULL, 0, 0, 0, phkResult,
		&dwDisp);
}

LONG __IMPORT
XCERegOpenKeyA(
			HKEY  hKey,
			LPCTSTR  lpszSubKey,
			PHKEY  phkResult
			)
{
	DWORD dwDisp;

	return XCERegOpenKeyExA(hKey, lpszSubKey, 0, 0, phkResult);
}

LONG __IMPORT
XCERegCloseKey(HKEY hKey)
{
	// just for tracing...
	return RegCloseKey(hKey);
}


LONG __IMPORT 
XCERegEnumKeyA(
			HKEY  hKey,	
			DWORD  iSubKey,
			LPSTR  lpszName,
			DWORD  cchName
			)
{
	int res;
	char lpszClass[126];
	DWORD cchClass = 126;
	FILETIME ft;

	res = XCERegEnumKeyExA(hKey, iSubKey, lpszName, &cchName, NULL, lpszClass,
		&cchClass, &ft);

	return res;
}

LONG __IMPORT
XCERegDeleteValueA(
				HKEY hKey,	
				LPSTR lpszValue 	
				)
{
	LONG res;
	int len;
	WCHAR *lpszValueW;

	len = strlen(lpszValue);
	lpszValueW = alloca((len+1) * 2);
	MultiByteToWideChar(CP_ACP, 0, lpszValue, -1, lpszValueW, len+1);
	res = RegDeleteValueW(hKey, lpszValueW);

	return res;
}
