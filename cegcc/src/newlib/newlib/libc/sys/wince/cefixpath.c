// fixpath.c
//
// Time-stamp: <17/04/01 21:44:57 keuchel@w2k>

#include "sys/wcebase.h"
#include "sys/sysconf.h"
#include <wchar.h>

char *XCEFixPathA(const char *pathin, char *pathout);
wchar_t *XCEFixPathW(const wchar_t *wpathin, wchar_t *wpathout);
wchar_t *XCECanonicalizePathW(wchar_t *target);

char *
XCEFixPathA(const char *pathin, char *pathout)
{
  wchar_t wpathin[MAX_PATH];
  wchar_t wpathout[MAX_PATH];

  //printf("FixPathA: %s\n", pathin);

  MultiByteToWideChar(CP_ACP, 0, pathin, -1, wpathin, MAX_PATH);

  XCEFixPathW(wpathin, wpathout);

  WideCharToMultiByte(CP_ACP, 0, 
		      wpathout, -1, 
		      pathout, MAX_PATH, 
		      NULL, NULL);

  return pathout;
}

static void append_slash_if_needed(wchar_t* wbuf)
{
	int len = wcslen (wbuf);
	if (len >0 && wbuf[len-1] != '\\' && wbuf[len-1] != '/')
		wcscat(wbuf, L"\\");
}

wchar_t *
XCEFixPathW(const wchar_t *wpathin, wchar_t *wpathout)
{
  wchar_t wroot[MAX_PATH+1];
  wchar_t wdir[MAX_PATH+1];
  wchar_t *p;

  //wprintf(L"FixPathW: %s\n", wpathin);

  wpathout[0] = 0;

  XCEGetCurrentRootDirectoryW(sizeof(wroot), wroot);
  wcscpy(wpathout, wroot);
  append_slash_if_needed(wpathout);

  if(wpathin[0] != '\\' && wpathin[0] != '/')
  {
	  XCEGetCurrentDirectoryW(sizeof(wdir), wdir);

	  wcscat(wpathout, wdir);
	  append_slash_if_needed(wpathout);

//	  if(!wcscmp(wdir, L"\\"))
//		  wcscat(wpathout, L"\\");
  }

  wcscat(wpathout, wpathin);

  for(p = wpathout; *p; p++)
    {
      if(*p == '/')
	*p = '\\';
    }

  // don't allow slash at end of directory name...
  if(p[-1] == '\\' && p != wpathout + 1)
    p[-1] = 0;

  // now remove . and ..
  XCECanonicalizePathW(wpathout);

  //XCETRACEW((L"NewPath: %s\n", wpathout));

  return wpathout;
}

#define IS_DIRECTORY_SEP(X) (X == '\\')
#define IS_ANY_SEP(X) (X == '\\')

// this is from emacs...
wchar_t *
XCECanonicalizePathW(wchar_t *target)
{
  wchar_t *p = target;
  wchar_t *o = target;

  //wprintf(L"Can1: %s\n", target);

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
	       && o != target
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

  //wprintf(L"Can2: %s\n", target);

  return target;
}
