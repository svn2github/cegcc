// makeunixpath.c
//
// Time-stamp: <18/02/01 14:02:30 keuchel@w2k>

#include <stdlib.h>
#include <stdio.h>
#include <wchar.h>
#include <string.h>
#include <setjmp.h>
#include <signal.h>

#include "sys/sysconf.h"


static char _unixdir[MAX_PATH+1] = "";

#define MIN(a,b) ((a)<(b)?(a):(b))

char *
XCEGetUnixPath(const char *path)
{
  static char newpath[MAX_PATH+1];

  if(_unixdir[0] == 0)
  {
    char* env = getenv("UNIXROOTDIR");
    if(env) 
    {
      int l = strlen(env);
      strncpy(_unixdir, env, MIN(l, MAX_PATH));
    }
    else
    {
      XCEShowMessageA("Warning: UNIXROOTDIR not set in environment, defaulting to \"/\"");
      strcpy(_unixdir, "/");
    }
  }

  XCEToUnixPath(_unixdir, -1);

  // we expect that path is absolute...
  if(!strcmp(_unixdir, "/"))
    strcpy(newpath, path);
  else
  {
    strcpy(newpath, _unixdir);
    strcat(newpath, path);
  }

  return newpath;
}
