#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include <sys/wcebase.h>
#include <sys/wcetime.h>
#include <sys/wceregistry.h>
#include <sys/wcetrace.h>
#include <sys/findpath.h>

#if 0
#define RSTRLEN  (128)

int
rgetenv(char *name, char *value, int valuelen)
{
  int   retval, rval = 0;
  HKEY  hkey;
  DWORD size;
  DWORD type;
  char  data[1024];
  char  pathbuf[RSTRLEN];
  wchar_t keyw[RSTRLEN], namew[RSTRLEN];

  if (name == 0 || strlen(name) == 0) {
    errno = EINVAL;
    return(-1);
  }

  /* Look up "name" in /HKLM/Environment */
  mbstowcs(keyw, "Environment", RSTRLEN);
  retval = RegOpenKeyExW(HKEY_LOCAL_MACHINE, keyw, 0, 0, &hkey);
  WCETRACE(WCE_IO, "rgetenv: RegOpenKeyExW returns %d", retval);
  if (retval == 0) {
    mbstowcs(namew, name, RSTRLEN);
    size = 1024;
    retval = RegQueryValueExW(hkey, namew, NULL, &type, data, &size);
    WCETRACE(WCE_IO, "rgetenv: RegQueryValueExW returns %d", retval);
    value[0] = '\0';
    if (retval == 0 && type == REG_SZ) {
      if (valuelen > wcslen((wchar_t *)data) + 1) {
        wcstombs(value, (wchar_t *) data, valuelen);
        WCETRACE(WCE_IO, "rgetenv: \"%s\" is \"%s\"", name, value);
      } else {
        WCETRACE(WCE_IO, "rgetenv: ERROR buffer too small");
        errno = EINVAL;
        rval = -1;
      }
    } else
    {
      errno = ENOENT;
      rval = -1;
    }
    RegCloseKey(hkey);
  } else {
    errno = ENOENT;
    rval = -1;
  }

  return(rval);
}
#endif

int
_findexec(char *exename, char *pathbuf, int buflen)
{
  char *pathseq;
  char  path[MAXPATHLEN];
  char  buf[MAXPATHLEN];
  int   retval;
  char *p;

  WCETRACE(WCE_IO, "_findexec: CALLED exename \"%s\"", exename);

  /* If there is a / in exename, do not check PATH */
  if (strchr(exename, '/') != NULL && exename[0]!='.' && exename[0]!='/') {
    if (access(exename, X_OK) == 0)	{
      strcpy(pathbuf, exename);
      return 0;
    } else {
      errno = ENOENT;
      return(-1);
    }
  }

  /* Path hierarchy: inherited from environment */
  p = getenv("PATH");
  if (p == NULL || p[0]==0) {
    p = "/windows:/bin:/:.";
  }

  pathseq = strdup(p);

  WCETRACE(WCE_IO, "_findexec: search path is \"%s\"", pathseq);

  for (p = strtok(pathseq, ":"); p; p = strtok(NULL, ":\r\n")) {
    strcpy(path, p);

    /* Handle "." as a special case here */
    if (strcmp(path, ".") == 0) {
      if (getcwd(buf, MAXPATHLEN) != NULL) {
        strcpy(path, buf);
      }
    }

    if (strlen(path) > 0)
      strcat(path, "/");

    strcat(path, exename);

    retval = access(path, X_OK);

    WCETRACE(WCE_IO, "_findexec: access returns %d for \"%s\"", retval, path);

    if(retval==-1)
    {
	    strcat(path, ".exe");
	    retval = access(path, X_OK);
	    WCETRACE(WCE_IO, "_findexec: access returns %d for \"%s\"", retval, path);
    }

    if (retval == 0) {
      free(pathseq);

      if (strlen(path) > buflen) {
        errno = EINVAL;
        return(-1);
      }

      strcpy(pathbuf, path);
      return(0);
    }
  }

  free(pathseq);

  errno = ENOENT;
  return(-1);
}
  
