#include <stdlib.h>
#include <string.h>

#include <sys/wcetrace.h>
#include <sys/wcebase.h>
#include <sys/wcefile.h>
#include <sys/fixpath.h>

#include <unistd.h>

static char *_canonicalizepath(char *target);

#define IS_DIRECTORY_SEP(X) (X == '/' || X == '\\')
#define IS_ANY_SEP(X) IS_DIRECTORY_SEP(X)

char *
fixpath(const char *pathin_, char *pathout)
{
  char buf[MAX_PATH];

  int  len, idx;
  char *cp;
  char* pip;
  
  if (pathin_ == NULL || pathout == NULL) {
    return NULL;
  }
   
  char* pathin = alloca(strlen(pathin_)+1);
  strcpy(pathin, pathin_);

  extern char __currootdir[];
  strcpy(pathout, __currootdir);

  /* Strip off leading/trailing quotes */
  pip = pathin;
  idx = strlen(pip);
  if (idx > 0 && pip[0] == '"') {
    pip++;
    --idx;
    if (idx > 0 && pip[idx] == '"')
	{
      --idx;
      pip[idx] = '\0';
	}
  }

  int was_drive = 0;
  /* Preprocessing: strip off drive letter if any */
  if (idx >= 2 && (!IS_DIRECTORY_SEP(pip[0]) && pip[1] == ':')) {
    pip += 2;
	idx-=2;
	// we might get an empty path now, but it isn't a relative one
	was_drive = 1;
    WCETRACE(WCE_IO, "fixpath: drive letter stripped, pip \"%s\"", pip);
  }

  /* Path is not absolute - insert cwd */
  WCETRACE(WCE_IO, "fixpath: pip \"%s\"", pip);
  if (*pip && !IS_DIRECTORY_SEP(pip[0]) || pip[0] == '.') {
    if ((strncmp(pip, ".\\", 2) == 0) || (strncmp(pip, "./", 2) == 0)) {
      pip += 2;
	  idx -= 2;
    }
    strcat(pathout, getcwd(buf, MAX_PATH));

    /* If cwd is not the root directory (/) append "/" */
    if (!(IS_DIRECTORY_SEP(buf[0]) && buf[1] == '\0'))
      strcat(pathout, "\\");
    strcat(pathout, pip);
  } else {
	  if (!*pip && was_drive)
		  strcat(pathout, "\\");
	  else
		strcat(pathout, pip);
  }

  WCETRACE(WCE_IO, "fixpath(1): pathout \"%s\"", pathout);
  for (cp = pathout; *cp; cp++) {
    if (*cp == '/') {
      *cp = '\\';
    }
  }

  WCETRACE(WCE_IO, "fixpath(2): pathout \"%s\"", pathout);

  /* disallow slash at end of directory name */
  if (((cp-pathout)>0) && (cp[-1] == '\\' && cp != pathout + 1))
    cp[-1] = 0;

  WCETRACE(WCE_IO, "fixpath(3): pathout \"%s\"", pathout);

  /* now remove . and .. */
  _canonicalizepath(pathout);

  WCETRACE(WCE_IO, "fixpath(4): pathout \"%s\"", pathout);
  return pathout;
}

/* This is from emacs... */
static char *
_canonicalizepath(char *target)
{
  char *p = target;
  char *o = target;

  while (*p) {
    if (!IS_DIRECTORY_SEP(*p)) {
      *o++ = *p++;
    } else if (IS_DIRECTORY_SEP(p[0]) && p[1] == '.'
               && (IS_DIRECTORY_SEP(p[2]) || p[2] == 0)) {
      /* If "/." is the entire filename, keep the "/".  Otherwise just delete the whole "/.".  */
      if (o == target && p[2] == '\0')
        *o++ = *p;
      p += 2;
    } else if (IS_DIRECTORY_SEP(p[0]) && p[1] == '.' && p[2] == '.'
               /* `/../' is the "superroot" on certain file systems.  */
               && o != target && (IS_DIRECTORY_SEP(p[3]) || p[3] == 0)) {
      while (o != target && (--o) && !IS_DIRECTORY_SEP(*o));

      /* Keep initial / only if this is the whole name.  */
      if (o == target && IS_ANY_SEP(*o) && p[3] == 0)
        ++o;
      p += 3;
    } else {
      *o++ = *p++;
    }
  }

  *o = 0;

  return target;
}
