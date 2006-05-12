#include <unistd.h>
#include <pwd.h>
#include <grp.h>
#include <sys/types.h>

static struct group gr;
static char namebuf[32];
static char pwbuf[32];
static char members[8][10];

/* The deal here is that WinCE does not have any group identification
 * in its file attributes.  It seems like we just have to spoof -
 * Our view of things is thus: one group ("voxware"), root is its only member
 */

/*pedro: originally, the groups was being set to voxware :), I removed it 
because it wasn't making sense to users.
It is now "root" too.*/

struct group *
getgrgid(gid_t gid)
{
  gr.gr_name = namebuf;
  gr.gr_passwd = pwbuf;
  gr.gr_gid = 0;
  gr.gr_mem = (char **)members;
  sprintf(namebuf, "root");
  sprintf(pwbuf, "*");
  sprintf(members[0], "root");
  return(&gr);
}
  
struct group *
getgrnam(const char *name)
{
  gr.gr_name = namebuf;
  gr.gr_passwd = pwbuf;
  gr.gr_gid = 0;
  gr.gr_mem = (char **)members;
  sprintf(namebuf, "root");
  sprintf(pwbuf, "*");
  sprintf(members[0], "root");
  return(&gr);
}

int
setgid(gid_t id)
{
  return 0;
}

int
setegid(gid_t id)
{
  return 0;
}

gid_t
getgid()
{
  return 0;
}

gid_t
getegid()
{
  return 0;
}

int 
initgroups(const char *groupname, gid_t id)
{
  return 0;
}
