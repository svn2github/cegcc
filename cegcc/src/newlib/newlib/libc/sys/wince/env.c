
// based on celib's code from keuchel@netwave.de

#include "sys/shared.h"

#include <windows.h>

#define ERROR_NO_MORE_ITEMS              259L

void
_initenv_from_reg()
{
  int res;
  char valname[126];
  char data[1024];
  char buf[1024];
  HKEY hKey;
  DWORD dwValSize = sizeof(valname);
  DWORD dwDataSize = sizeof(data);
  DWORD dwType;
  int idx = 0;
  char *p;

  static int initted = 0;
  if (initted)
	  return;
  initted = 1;

  if((res = XCERegOpenKeyExA(HKEY_LOCAL_MACHINE, "Environment", 0,
	  KEY_READ, &hKey)) != 0)
  {
	  // use defaults
	  putenv("UNIXROOTDIR=/");
	  putenv("PATH=.:/:/Windows");
	  putenv("HOME=/My Documents");
	  putenv("TMP=/Temp");
	  return;
  }

  while(1)
    {
      dwValSize = sizeof(valname);
      dwDataSize = sizeof(data);

      res = XCERegEnumValueA(hKey, idx++, valname, &dwValSize,
			     NULL, &dwType, data, &dwDataSize);

      if(res != 0)
	{
	  if(res != ERROR_NO_MORE_ITEMS)
	  {
	    XCEShowMessageA("RegEnumValue: %d", res);
	  }
	  break;
	}

      if(dwType != REG_SZ)
	continue;

      sprintf(buf, "%s=%s", valname, data);
      putenv(buf);
    }

  RegCloseKey(hKey);
}

void
_initenv_from_envblock(char *buf)
{
  char *s;

  for(s = buf; *s;)
    {
      putenv(s);
      s += strlen(s) + 1;
    }
}

void _initenv(_SHMBLK shmblk)
{
  if (shmblk)
  {
	  char buf[MAX_ENVIRONBLK];

          /* We are being initialized from a cegcc app.
             Kill environ set from the registry.  */
	  environ[0] = 0;
	  _shared_getenvblk(shmblk, buf);
	  if(buf[0] != 0)
		  _initenv_from_envblock(buf);
	  else
		  _initenv_from_reg();
	  _shared_setenvblk(shmblk, "");
  }
  else
  {
	  _initenv_from_reg();
  }
}
