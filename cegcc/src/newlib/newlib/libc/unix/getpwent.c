#include <stdio.h>
#include <sys/types.h>
#include <pwd.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

static struct passwd pw_passwd;	/* password structure */
static FILE *passwd_fp;

static char logname[8];
static char password[1024];
static char gecos[1024];
static char dir[1024];
static char shell[1024];

#ifdef UNDER_CE
#include <sys/sysconf.h>
/* returns local var, so don't cache the result, only use in functions calls */
static char* get_passwd_file()
{
	char path[MAX_PATH];
	path[0] = 0;
	XCEGetEnvironmentVariableFromRegA ("UNIXROOTDIR", path, sizeof(path));
	if(!strcmp (path, "\\") == 0 || strcmp (path, "/"))
 		path[0] = 0;
	strcat(path, "\\etc\\passwd");
	return path;
}
#define _PASSWDFILE_ get_passwd_file()
#else
# define _PASSWDFILE_ "/etc/passwd"
#endif

struct passwd *
getpwnam (name)
     const char *name;
{
  FILE *fp;
  char buf[1024];

  if ((fp = fopen (_PASSWDFILE_, "r")) == NULL)
    {
      return NULL;
    }

  while (fgets (buf, sizeof (buf), fp))
    {
      sscanf (buf, "%[^:]:%[^:]:%d:%d:%[^:]:%[^:]:%s\n",
	      logname, password, &pw_passwd.pw_uid,
             &pw_passwd.pw_gid, gecos,
	      dir, shell);
      pw_passwd.pw_name = logname;
      pw_passwd.pw_passwd = password;
      pw_passwd.pw_comment = "";
      pw_passwd.pw_gecos = gecos;
      pw_passwd.pw_dir = dir;
      pw_passwd.pw_shell = shell;

      if (!strcmp (logname, name))
	{
	  fclose (fp);
	  return &pw_passwd;
	}
    }
  fclose (fp);
  return NULL;
}

struct passwd *
getpwuid (uid_t uid)
{
  FILE *fp;
  char buf[1024];

  if ((fp = fopen ("/etc/passwd", "r")) == NULL)
    {
      return NULL;
    }

  while (fgets (buf, sizeof (buf), fp))
    {
      sscanf (buf, "%[^:]:%[^:]:%d:%d:%[^:]:%[^:]:%s\n",
	      logname, password, &pw_passwd.pw_uid,
             &pw_passwd.pw_gid, gecos,
	      dir, shell);
      pw_passwd.pw_name = logname;
      pw_passwd.pw_passwd = password;
      pw_passwd.pw_comment = "";
      pw_passwd.pw_gecos = gecos;
      pw_passwd.pw_dir = dir;
      pw_passwd.pw_shell = shell;

      if (uid == pw_passwd.pw_uid)
	{
	  fclose (fp);
	  return &pw_passwd;
	}
    }
  fclose (fp);
  return NULL;
}

struct passwd *
getpwent ()
{
  char buf[1024];

  if (passwd_fp == NULL)
    return NULL;

  if (fgets (buf, sizeof (buf), passwd_fp) == NULL)
    return NULL;

  sscanf (buf, "%[^:]:%[^:]:%d:%d:%[^:]:%[^:]:%s\n",
	  logname, password, &pw_passwd.pw_uid,
         &pw_passwd.pw_gid, gecos,
	  dir, shell);
  pw_passwd.pw_name = logname;
  pw_passwd.pw_passwd = password;
  pw_passwd.pw_comment = "";
  pw_passwd.pw_gecos = gecos;
  pw_passwd.pw_dir = dir;
  pw_passwd.pw_shell = shell;

  return &pw_passwd;
}

void
setpwent ()
{
  if (passwd_fp != NULL)
    fclose (passwd_fp);

  passwd_fp = fopen (_PASSWDFILE_, "r");
}

void
endpwent ()
{
  if (passwd_fp != NULL)
    fclose (passwd_fp);
}
