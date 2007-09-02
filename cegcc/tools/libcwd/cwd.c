#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <wchar.h>

#include <windows.h>

#include "cwd.h"

static wchar_t *XCEFixPathW (const wchar_t *wpathin, wchar_t *wpathout);
static wchar_t *XCECanonicalizePathW (wchar_t *target);
static DWORD XCEGetCurrentDirectoryW (DWORD dwSize, wchar_t *buf);

#define COUNTOF(X) (sizeof (X) / sizeof ((X)[0]))
#define MIN(A, B) ((A) < (B) ? (A) : (B))

#define IS_DIRECTORY_SEP(X) (X == '\\')
#define IS_ANY_SEP(X) (X == '\\')

/* Per process as on WinNT
   ### TODO, we need a mutex here if we ever go multithread.  */

static wchar_t _current_dirw[MAX_PATH+1] = L"\\";
#define _CURRENTDIRW _current_dirw

static DWORD
XCEGetFileAttributesW (const wchar_t *wfname)
{
  wchar_t wpath[MAX_PATH];
  DWORD dwRes;

  XCEFixPathW (wfname, wpath);
  dwRes = GetFileAttributesW (wpath);

  return dwRes;
}

static char *
XCEToUnixPath (char *s, int len)
{
  char *p;

  if (len < 0)
    {
      for(p = s; *p; p++)
	if(*p == '\\')
	  *p = '/';
    }
  else
    {
      int i;
      for(i = 0; i < len ; i++)
	if(s[i] == '\\')
	  s[i] = '/';
    }

  return s;
}

static void
append_slash_if_needed (wchar_t* wbuf)
{
  int len = wcslen (wbuf);
  if (len > 0
      && wbuf[len - 1] != '\\'
      && wbuf[len - 1] != '/')
    wcscat (wbuf, L"\\");
}

static wchar_t *
XCEFixPathW (const wchar_t *wpathin, wchar_t *wpathout)
{
  wchar_t wdir[MAX_PATH+1];
  wchar_t *p;

  wpathout[0] = 0;
  append_slash_if_needed (wpathout);

  if(wpathin[0] != '\\' && wpathin[0] != '/')
    {
      XCEGetCurrentDirectoryW (sizeof(wdir), wdir);
      wcscat (wpathout, wdir);
      append_slash_if_needed (wpathout);
    }

  wcscat (wpathout, wpathin);

  for(p = wpathout; *p; p++)
    {
      if(*p == '/')
	*p = '\\';
    }

  /* don't allow slash at end of directory name... */
  if(p[-1] == '\\' && p != wpathout + 1)
    p[-1] = 0;

  /* now remove . and .. */
  XCECanonicalizePathW (wpathout);

  return wpathout;
}

static wchar_t *
XCECanonicalizePathW (wchar_t *target)
{
  wchar_t *p = target;
  wchar_t *o = target;

  while (*p)
    {
      if (!IS_DIRECTORY_SEP (*p))
	{
	  *o++ = *p++;
	}
      else if (IS_DIRECTORY_SEP (p[0])
	       && p[1] == '.'
	       && (IS_DIRECTORY_SEP (p[2])
		   || p[2] == 0))
	{
	  /* If "/." is the entire filename, keep the "/".  Otherwise,
	     just delete the whole "/.".  */
	  if (o == target && p[2] == '\0')
	    *o++ = *p;
	  p += 2;
	}
      else if (IS_DIRECTORY_SEP (p[0]) && p[1] == '.' && p[2] == '.'
	       /* `/../' is the "superroot" on certain file systems.  */
	       /* disabled this test, as it wasn't filtering '/..' because of it.
		  cegcc should also have the same problem.  */
	       /* && o != target */
	       && (IS_DIRECTORY_SEP (p[3]) || p[3] == 0))
	{
	  while (o != target && (--o) && !IS_DIRECTORY_SEP (*o))
	    ;
	  /* Keep initial / only if this is the whole name.  */
	  if (o == target && IS_ANY_SEP (*o) && p[3] == 0)
	    ++o;
	  p += 3;
	}
      else
	{
	  *o++ = *p++;
	}
    }

  *o = 0;

  return target;
}

static DWORD
XCEGetCurrentDirectoryW (DWORD dwSize, wchar_t *buf)
{
  size_t len = wcslen(_CURRENTDIRW);
  if (dwSize == 0 && buf == 0)
    return len+1;
  wcsncpy (buf, _CURRENTDIRW, dwSize - 1);
  if (dwSize > len)
    return len;
  else
    return len+1;
}

static DWORD
XCEGetCurrentDirectoryA (DWORD dwSize, char *buf)
{
  DWORD dwLen;
  wchar_t wbuf[MAX_PATH+1];

  if (dwSize == 0 && buf == 0)
    return dwSize;
  dwLen = XCEGetCurrentDirectoryW (dwSize, wbuf);
  WideCharToMultiByte (CP_ACP, 0, wbuf, -1,
		       buf, MIN(dwLen, dwSize), NULL, NULL);
  buf[MIN(dwLen, dwSize)] = 0;
  return dwLen;
}

static BOOL
XCESetCurrentDirectoryW (const wchar_t *wdir)
{
  DWORD dwAttr;
  wchar_t wtmp[MAX_PATH];

  int wlen = wcslen (wdir);
  if (wlen > (MAX_PATH - 1))
    return 0;
  else if (wlen > 0
	   && !(wdir[wlen-1] == '\\' || wdir[wlen-1] == '/')
	   && wlen > (MAX_PATH - 2))
    return 0;

  /* Oh great... There is some code generation bug in mingw32ce.  Inlining
     the call hides the problem.  */
  //  XCEFixPathW (wdir, wtmp);
  const wchar_t *wpathin = wdir;
  wchar_t *wpathout = wtmp;
  {
    wchar_t wdir[MAX_PATH+1];
    wchar_t *p;

    wpathout[0] = 0;

    if(wpathin[0] != '\\' && wpathin[0] != '/')
      {
	XCEGetCurrentDirectoryW (sizeof(wdir), wdir);
	wcscat (wpathout, wdir);
	append_slash_if_needed (wpathout);
      }

    wcscat (wpathout, wpathin);

    for(p = wpathout; *p; p++)
      {
	if(*p == '/')
	  *p = '\\';
      }

    /* Don't allow slash at end of directory name... */
    if(p[-1] == '\\' && p != wpathout + 1)
      p[-1] = 0;

    /* Now remove . and ..  */
    XCECanonicalizePathW (wpathout);
  }

  if((dwAttr = XCEGetFileAttributesW (wtmp)) == 0xFFFFFFFF)
    return FALSE;

  if((dwAttr & FILE_ATTRIBUTE_DIRECTORY) == 0)
    {
//      errno = ENOTDIR;
      return FALSE;
    }

  wcscpy (_CURRENTDIRW, wtmp);
  return TRUE;
}

static BOOL
XCESetCurrentDirectoryA (const char *dir)
{
  wchar_t wdir[MAX_PATH];
  int len = strlen(dir);
  if (len > (MAX_PATH - 1))
    return 0;
  else if (len>0
	   && !(dir[len-1] == '\\' || dir[len-1] == '/')
	   && len > (MAX_PATH - 2))
    return 0;

  MultiByteToWideChar (CP_ACP, 0, dir, -1, wdir, COUNTOF (wdir));
  return XCESetCurrentDirectoryW (wdir);
}

int
_chdir (const char *path)
{
  char fpath[MAX_PATH+1];

  if (path == NULL || strlen (path) == 0)
    {
      /* errno? */
      return -1;
    }

  if (!XCESetCurrentDirectoryA (path))
    return -1;

  XCEGetCurrentDirectoryA (MAX_PATH + 1, fpath);
  XCEToUnixPath (fpath, -1);
  return 0;
}

char *
_getcwd (char *buf, int size)
{
  int len;
  char *alloced = NULL;

  if (buf == NULL || size == 0)
    {
      size = MAX_PATH + 1;
      buf = alloced = malloc (size);
      if (!buf)
	return NULL;
    }

  len = XCEGetCurrentDirectoryA ((DWORD) size, buf);

  if (len > size)
    {
      XCEToUnixPath (buf, size - 1);
      return NULL;
    }

  if (len == 0)
    {
      if (alloced != NULL)
	free (alloced);
      return NULL;
    }

  XCEToUnixPath (buf, -1);
  return buf;
}

int
chdir (const char *path)
{
  return _chdir (path);
}

char *
getcwd (char *buf, int size)
{
  return _getcwd (buf, size);
}

char*
libcwd_fixpath (char* out, const char *in)
{
  wchar_t wout[MAX_PATH];
  wchar_t win[MAX_PATH];

  mbstowcs (win, in, MAX_PATH);
//  XCEFixPathW (win, wout);
  const wchar_t *wpathin = win;
  wchar_t *wpathout = wout;
  {
    wchar_t wdir[MAX_PATH+1];
    wchar_t *p;

    wpathout[0] = 0;

    if(wpathin[0] != '\\' && wpathin[0] != '/')
      {
	XCEGetCurrentDirectoryW (sizeof(wdir), wdir);
	wcscat (wpathout, wdir);
	append_slash_if_needed (wpathout);
      }

    wcscat (wpathout, wpathin);

    for(p = wpathout; *p; p++)
      {
	if(*p == '/')
	  *p = '\\';
      }

    /* Don't allow slash at end of directory name... */
    if(p[-1] == '\\' && p != wpathout + 1)
      p[-1] = 0;

    /* Now remove . and ..  */
    XCECanonicalizePathW (wpathout);
  }

  wcstombs (out, wout, MAX_PATH);
  return out;
}
