// wsdb.c - missing netdb stuff for windows ce
//
// Time-stamp: <09/08/01 21:47:18 keuchel@w2k>

#include <errno.h>
#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

char* XCEGetUnixPath(const char *path);

struct servent {
	char* s_name;
	char** s_aliases;
	short s_port;
	char* s_proto;
};

struct protoent {
	char * p_name;
	char ** p_aliases;
	short p_proto;
};

struct servent *
getservbyname(const char *sname, const char *sproto)
{
  static struct servent serv;
  static char name[64];
  static char proto[64];

  FILE *f;
  char path[MAX_PATH];
  int port;
  char buf[512];

  if(sproto == NULL)
    sproto = "tcp";

  strcpy(path, XCEGetUnixPath("/etc/services"));

  if((f = fopen(path, "r")) == NULL)
    {
      XCEShowMessageA("Cannot open %s", path);
      return NULL;
    }

  while(fgets(buf, sizeof(buf), f))
    {
      if(buf[0] == '#' || buf[0] == '\n')
	continue;

      if(sscanf(buf, "%s %d/%s", name, &port, proto) == 3)
	{
	  if(strcmp(sname, name) == 0 && strcmp(sproto, proto) == 0)
	    {
	      serv.s_name = name;
	      serv.s_aliases = NULL;
	      serv.s_port = htons((unsigned short) port);
	      serv.s_proto = proto;

	      fclose(f);
	      return &serv;
	    }
	}
    }

  XCEShowMessageA("Cannot find service %s", sname);
  errno = ENOENT;

  return NULL;
}

struct servent *
getservbyport(int aport, const char *sproto)
{
  static struct servent serv;
  static char name[64];
  static char proto[64];

  FILE *f;
  char path[MAX_PATH];
  int port;
  char buf[512];

  strcpy(path, XCEGetUnixPath("/etc/services"));

  if((f = fopen(path, "r")) == NULL)
    {
      XCEShowMessageA("Cannot open %s", path);
      return NULL;
    }

  while(fgets(buf, sizeof(buf), f))
    {
      if(buf[0] == '#' || buf[0] == '\n')
	continue;

      if(sscanf(buf, "%s %d/%s", name, &port, proto) == 3)
	{
	  if(aport == port && strcmp(sproto, proto) == 0)
	    {
	      serv.s_name = name;
	      serv.s_aliases = NULL;
	      serv.s_port = htons((unsigned short) port);
	      serv.s_proto = proto;

	      fclose(f);
	      return &serv;
	    }
	}
    }

  XCEShowMessageA("Cannot find service for port %d", aport);
  errno = ENOENT;

  return NULL;
}

struct protoent *
getprotobyname(const char *name)
{
  static struct protoent pent;

  memset(&pent, 0, sizeof(pent));

  if(strcmp(name, "tcp"))
    {
      pent.p_name = "tcp";
      pent.p_proto = PF_UNSPEC;
    }
  else if(strcmp(name, "udp"))
    {
      pent.p_name = "udp";
      pent.p_proto = PF_UNSPEC;
    }
  else
    {
      errno = ENOENT;
      return NULL;
    }

  return &pent;
}

struct protoent *
getprotobynumber(int number)
{
  static struct protoent pent;

  memset(&pent, 0, sizeof(pent));

  if(number == IPPROTO_TCP)
    {
      pent.p_name = "tcp";
      pent.p_proto = PF_UNSPEC;
    }
  else if(number == IPPROTO_UDP)
    {
      pent.p_name = "udp";
      pent.p_proto = PF_UNSPEC;
    }
  else
    {
      errno = ENOENT;
      return NULL;
    }

  return &pent;
}
 
