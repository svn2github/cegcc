#include <winsock2.h>


/* rexec.c --- rexec for winsock
 *
 * Time-stamp: <22/02/01 07:23:38 keuchel@w2k>
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdarg.h>

#define __THROW

#define bcopy(from, to, len) memcpy(to, from, len)

/*
 * sdk doesn't have these, what to do?
 */
#define WSAEAGAIN 23
#define SENDFLAG 0
#define RECVFLAG 0

#define WSAGetLastError() GetLastError()

int rexec (char **__restrict ahost, int rport,
						   __const char *__restrict user,
						   __const char *__restrict pass,
						   __const char *__restrict cmd, int *__restrict fd2p)
						   __THROW
{
  int s, timo = 1;
  struct sockaddr_in sin, from;
  char c;
  int lport = IPPORT_RESERVED - 1;
  struct hostent *hp = NULL;
  char username[16+1] = "";
  char password[16+1] = "";
  struct in_addr ina;

  if(user == NULL || pass == NULL)
    {
      return SOCKET_ERROR;
    }

  // first check numeric address...
  if(ascii2addr(AF_INET, *ahost, &ina) < 0)
    {
      hp = gethostbyname (*ahost);

      if (hp == NULL)
        {
          wserror("rexec:gethostbyname failed");
          return (INVALID_SOCKET);
        }

      *ahost = hp->h_name;
    }
  else
    {
    }

  for (;;)
    {
      s = rresvport(&lport);
      if (s == INVALID_SOCKET)
	{
	  if (WSAGetLastError() == WSAEAGAIN)
	    wserror ("rexec:socket:all ports in use");
	  else
	    wserror ("rexec:socket failed");
	  return (INVALID_SOCKET);
	}

      if(hp)
        {
          sin.sin_family = hp->h_addrtype;
          bcopy (hp->h_addr_list[0], &sin.sin_addr, hp->h_length);
        }
      else
        {
          sin.sin_family = AF_INET;
          bcopy(&ina, &sin.sin_addr, 4);
        }

      sin.sin_port = rport;

      /* when connected, exit loop */

      if (connect(s, (struct sockaddr *) &sin, sizeof (sin)) >= 0)
	break;

      int wserrno = WSAGetLastError();

      closesocket (s);  

      if (wserrno == WSAEADDRINUSE)
	{
	  lport--;
	  continue;
	}

      if (wserrno == WSAECONNREFUSED && timo <= 16)
	{
	  Sleep(timo * 1000);
	  timo *= 2;
	  continue;
	}

      if (hp && hp->h_addr_list[1] != NULL)
	{
	  wserror("connect to address %s: ", inet_ntoa (sin.sin_addr));
	  hp->h_addr_list++;
	  bcopy (hp->h_addr_list[0], &sin.sin_addr, hp->h_length);
	  wserror("trying %s...\n", inet_ntoa (sin.sin_addr));
	  continue;
	}
      return (INVALID_SOCKET);
    }

  /* we are connected */

  lport--;
  if (fd2p == 0)
    {
      send (s, "", 1, SENDFLAG);
      lport = 0;
    }
  else
    {
      char num[8];
      wchar_t wnum[8];

      int s2 = rresvport (&lport), s3;
      size_t len = sizeof (from);

      if (s2 == INVALID_SOCKET)
	goto bad;
      listen (s2, 1);

      sprintf(num, "%d", lport);

      if (send (s, num, strlen (num) + 1, SENDFLAG) != (int) strlen (num) + 1)
	{
	  wserror (("rexec:write: setting up stderr"));
	  closesocket (s2);
	  goto bad;
	}
      s3 = accept (s2, (struct sockaddr *) &from, &len);
      closesocket (s2);
      if (s3 == INVALID_SOCKET)
	{
	  wserror (("rexec:accept failed"));
	  lport = 0;
	  goto bad;
	}
      *fd2p = s3;
      from.sin_port = ntohs ((u_short) from.sin_port);
      if (from.sin_family != AF_INET 
#if 1
          // SCO does not obeye this...
          || from.sin_port >= IPPORT_RESERVED
#endif
          )
	{
	  wserror(("socket: protocol failure in circuit setup."));
	  goto bad2;
	}
    }
  /* these are null terminated */
  int cc;
  cc = send (s, user, strlen (user) + 1, SENDFLAG);
  if(cc == SOCKET_ERROR || cc == 0)
  {
	  wserror("rcmd: send error");
	  goto bad2;
  }

  cc = send (s, pass, strlen (pass) + 1, SENDFLAG);
  if(cc == SOCKET_ERROR || cc == 0)
  {
	  wserror("rcmd: send error");
	  goto bad2;
  }

  cc = send (s, cmd, strlen (cmd) + 1, SENDFLAG);
  if(cc == SOCKET_ERROR || cc == 0)
  {
	  wserror("rcmd: send error");
	  goto bad2;
  }

  if (recv (s, &c, 1, RECVFLAG) != 1)
    {
		if(cc == 0)
			wserror ("Connection closed by foreign host\n");
		else
			wserror (("rexec:cannot read from socket"));
      goto bad2;
    }
  /* get one line from server: "Permission denied" */
  if (c != 0)
    {
      char buf[126];
      int i = 0;

      while (recv (s, &c, 1, RECVFLAG) == 1)
	{
		if (i < sizeof(buf))
          buf[i++] = c;
	  if (c == '\n')
	    break;
	}
      buf[i] = 0;

	  wserror("got permission denied? buf: %s", buf);

      goto bad2;
    }
  /* return socket to caller */
  return (s);

bad2:
  if (lport)
    closesocket (*fd2p);
bad:
  closesocket (s);
  return (INVALID_SOCKET);
}

int rresvport (int *alport) __THROW
{
  struct sockaddr_in sin;
  int s;

  sin.sin_family = AF_INET;
  sin.sin_addr.s_addr = INADDR_ANY;
  s = socket (AF_INET, SOCK_STREAM, 0);
  if (s == INVALID_SOCKET)
    {
//      wserrno = WSAGetLastError ();
      return (INVALID_SOCKET);
    }
  for (;;)
    {
      sin.sin_port = htons ((u_short) * alport);
      if (bind (s, (struct sockaddr *) &sin, sizeof (sin)) == 0)
	return (s);
//      wserrno = WSAGetLastError ();
      if (WSAGetLastError () != WSAEADDRINUSE)
	{
	  closesocket (s);
	  return (INVALID_SOCKET);
	}
      (*alport)--;
      if (*alport == IPPORT_RESERVED / 2)
	{
	  closesocket (s);
//	  wserrno = WSAEAGAIN;
	  return (INVALID_SOCKET);
	}
    }
}
