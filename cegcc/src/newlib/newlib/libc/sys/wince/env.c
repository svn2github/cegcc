
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
	    XCEShowMessageA("RegEnumValue: %d", res);
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
      char *buf;
      int no;

      /* We are being initialized from a cegcc app.  Only use the
	 registry environment if there are no environment variables in
	 the shared block.  */

      no = _shared_getenvironblk(shmblk, &buf);

      if (no)
	{
	  _initenv_from_envblock(buf);
	  free(buf);
	}
      else
	_initenv_from_reg();
  }
  else
    _initenv_from_reg();
}
