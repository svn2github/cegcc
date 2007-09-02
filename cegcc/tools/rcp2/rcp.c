/*
 * Copyright (c) 1983, 1990, 1992, 1993, 2002
 *	The Regents of the University of California.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 4. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include <config.h>

#include <sys/param.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <winsock2.h>
#include <windows.h>
#include <ctype.h>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string.h>
#include <unistd.h>
#include <getopt.h>
#include <utime.h>

#include "extern.h"

#define	OPTIONS "dfprtV"

static const char *short_options = OPTIONS;
static struct option long_options[] =
{
  { "recursive", required_argument, 0, 'r' },
  { "preserve", no_argument, 0, 'p' },
  { "help", no_argument, 0, 'h' },
  { "version", no_argument, 0, 'V' },
  /* Server option.  */
  { "directory", required_argument, 0, 'd' },
  { "from", required_argument, 0, 'f' },
  { "to", required_argument, 0, 't' },
  { 0, 0, 0, 0 }
};

typedef unsigned short u_short;

struct passwd *pwd;
u_short	port;
int errs;
int pflag, iamremote, iamrecursive, targetshouldbedirectory;

#define	CMDNEEDS	64
char cmd[CMDNEEDS];		/* must hold "rcp -r -p -d\0" */

int	 response (void);
void	 rsource (char *, struct stat *);
void	 sink (int, char *[]);
void	 source (int, char *[]);
void	 tolocal (int, char *[]);
void	 toremote (char *, int, char *[]);
void	 usage (void);
void	 help (void);

char *program_name;

/* Print the program name and error message MESSAGE, which is a
   printf-style format string with optional args.  If ERRNUM is
   nonzero, print its corresponding system error message.  Exit with
   status STATUS if it is nonzero.  */
void
error (int status, int errnum, const char *message, ...)
{
  va_list args;

  fflush (stdout);
  fprintf (stderr, "%s: ", program_name);

  va_start (args, message);
  vfprintf (stderr, message, args);
  va_end (args);

  if (errnum)
    {
      char const *s = strerror (errnum);
      if (!s)
	s = "Unknown system error";

      fprintf (stderr, ": %s", s);
    }
  putc ('\n', stderr);
  fflush (stderr);
  if (status)
    exit (status);
}

//#define WAIT_DEBUGGER

#ifdef WAIT_DEBUGGER
volatile int cont = 0;
#endif

int
main (int argc, char *argv[])
{
#ifdef WAIT_DEBUGGER
  /* useful to attach a debugger.  */
  while (!cont)
    Sleep (100); /* put a break here, and then 'p cont = 1' */
  if (cont < 0)
    return 0;
#endif

  struct servent *sp;
  int ch, fflag, tflag;
  char *targ;
  const char *shell;

  program_name = argv[0];

  fflag = tflag = 0;
  while ((ch = getopt_long (argc, argv, short_options, long_options, 0))
	 != EOF)
    switch(ch)
      {			/* User-visible flags. */
      case 'K':
	break;

      case 'p':
	pflag = 1;
	break;

      case 'r':
	iamrecursive = 1;
	break;

	/* Server options. */
      case 'd':
	targetshouldbedirectory = 1;
	break;

      case 'f':			/* "from" */
	iamremote = 1;
	fflag = 1;
	break;

      case 't':			/* "to" */
	iamremote = 1;
	tflag = 1;
	break;

      case 'V':
	printf ("rcp (%s) %s\n", PACKAGE_NAME, PACKAGE_VERSION);
	exit (0);
	break;

      case 'h':
	help ();
	break;

      case '?':
      default:
	usage ();
      }
  argc -= optind;
  argv += optind;
#if 0
  sp = getservbyname (shell = "shell", "tcp");
  if (sp == NULL)
    error (1, 0, "%s/tcp: unknown service", shell);
  port = sp->s_port;
#endif

  if (fflag)
    {			/* Follow "protocol", send data. */
      response ();
      source (argc, argv);
      exit (errs);
    }

  if (tflag)
    {			/* Receive data. */
      sink (argc, argv);
      exit (errs);
    }

  if (argc < 2)
    usage ();
  if (argc > 2)
    targetshouldbedirectory = 1;

  /* Command to be executed on remote system using "rsh". */
  snprintf (cmd, sizeof cmd, "rcp%s%s%s",
	    iamrecursive ? " -r" : "", pflag ? " -p" : "",
	    targetshouldbedirectory ? " -d" : "");

  targ = colon (argv[argc - 1]);	/* Dest is remote host. */
  if (targ)	/* Dest is remote host. */
    toremote (targ, argc, argv);
  else
    {
      tolocal (argc, argv);		/* Dest is local host. */
      if (targetshouldbedirectory)
	verifydir(argv[argc - 1]);
    }
  exit (errs);
}

void
toremote (char *targ, int argc, char *argv[])
{
  int i;
  //  int len, tos;
  char *src, *tuser, *thost;

  //  char *bp, *host, *suser, ;

  *targ++ = 0;
  if (*targ == 0)
    targ = ".";

  thost = strchr (argv[argc - 1], '@');
  if (thost)
    {
      /* user@host */
      *thost++ = 0;
      tuser = argv[argc - 1];
      if (*tuser == '\0')
	tuser = NULL;
      else if (!okname(tuser))
	exit (1);
    }
  else
    {
      thost = argv[argc - 1];
      tuser = NULL;
    }

  for (i = 0; i < argc - 1; i++)
    {
      src = colon (argv[i]);
      if (src)
	{			/* remote to remote */
#if 0
/* Currently unsupported.  */
	  *src++ = 0;
	  if (*src == 0)
	    src = ".";
	  host = strchr (argv[i], '@');
	  len = strlen (PATH_RSH) + strlen (argv[i]) +
	    strlen (src) + (tuser ? strlen (tuser) : 0) +
	    strlen (thost) + strlen (targ) + CMDNEEDS + 20;
	  if (!(bp = malloc (len)))
	    err (1, NULL);
	  if (host)
	    {
	      *host++ = 0;
	      suser = argv[i];
	      if (*suser == '\0')
		suser = pwd->pw_name;
	      else if (!okname (suser))
		continue;
	      snprintf (bp, len,
			"%s %s -l %s -n %s %s '%s%s%s:%s'",
			PATH_RSH, host, suser, cmd, src,
			tuser ? tuser : "", tuser ? "@" : "",
			thost, targ);
	    }
	  else
	    snprintf (bp, len,
		      "exec %s %s -n %s %s '%s%s%s:%s'",
		      PATH_RSH, argv[i], cmd, src,
		      tuser ? tuser : "", tuser ? "@" : "",
		      thost, targ);
	  susystem (bp, userid);
	  free (bp);
#endif
	}
      else
	{			/* local to remote */
#if 0
	  if (rem == -1)
	    {
	      len = strlen (targ) + CMDNEEDS + 20;
	      if (!(bp = malloc (len)))
		err (1, NULL);
	      snprintf (bp, len, "%s -t %s", cmd, targ);
	      host = thost;
	      rem = rcmd (&host, port, pwd->pw_name,
			  tuser ? tuser : pwd->pw_name, bp, 0);
	      if (rem < 0)
		exit (1);
#if defined (IP_TOS) && defined (IPPROTO_IP) && defined (IPTOS_THROUGHPUT)
	      tos = IPTOS_THROUGHPUT;
	      if (setsockopt (rem, IPPROTO_IP, IP_TOS,
			      (char *) &tos, sizeof(int)) < 0)
		error (0, errno, "TOS (ignored)");
#endif
	      if (response () < 0)
		exit(1);
	      free(bp);
	      setuid(userid);
	    }
#endif
	  source(1, argv+i);
	}
    }
}

void
tolocal (int argc, char *argv[])
{
#if 0

  int i, len, tos;
  char *bp, *host, *src, *suser;

  for (i = 0; i < argc - 1; i++)
    {
      if (!(src = colon (argv[i])))
	{		/* Local to local. */
	  len = strlen (PATH_CP) + strlen (argv[i]) +
	    strlen (argv[argc - 1]) + 20;
	  if (!(bp = malloc (len)))
	    err (1, NULL);
	  snprintf (bp, len, "exec %s%s%s %s %s", PATH_CP,
		    iamrecursive ? " -r" : "", pflag ? " -p" : "",
		    argv[i], argv[argc - 1]);
	  if (susystem (bp, userid))
	    ++errs;
	  free (bp);
	  continue;
	}
      *src++ = 0;
      if (*src == 0)
	src = ".";
      if ((host = strchr(argv[i], '@')) == NULL)
	{
	  host = argv[i];
	  suser = pwd->pw_name;
	}
      else
	{
	  *host++ = 0;
	  suser = argv[i];
	  if (*suser == '\0')
	    suser = pwd->pw_name;
	  else if (!okname (suser))
	    continue;
	}
      len = strlen (src) + CMDNEEDS + 20;
      if ((bp = malloc (len)) == NULL)
	err (1, NULL);
      snprintf (bp, len, "%s -f %s", cmd, src);
      rem =
	rcmd (&host, port, pwd->pw_name, suser, bp, 0);
      free (bp);
      if (rem < 0)
	{
	  ++errs;
	  continue;
	}
      seteuid (userid);
#if defined (IP_TOS) && defined (IPPROTO_IP) && defined (IPTOS_THROUGHPUT)
      tos = IPTOS_THROUGHPUT;
      if (setsockopt (rem, IPPROTO_IP, IP_TOS, (char *) &tos, sizeof (int)) < 0)
	error (0, errno, "TOS (ignored)");
#endif
      sink (1, argv + argc - 1);
      seteuid (0);
      close (rem);
      rem = -1;
    }
#endif
}

static int
write_stat_time (int fd, struct stat *stat)
{
  char buf[4 * sizeof (long) * 3 + 2];
  time_t a_sec, m_sec;
  long a_usec = 0, m_usec = 0;

  a_sec = stat->st_atime;
  m_sec = stat->st_mtime;

  snprintf (buf, sizeof(buf), "T%ld %ld %ld %ld\n",
	    m_sec, m_usec, a_sec, a_usec);
  return write (fd, buf, strlen (buf));
}

void
source (int argc, char *argv[])
{
  struct stat stb;
  static BUF buffer;
  BUF *bp;
  off_t i;
  int amt, fd, haderr, indx, result;
  char *last, *name, buf[BUFSIZ];

  for (indx = 0; indx < argc; ++indx)
    {
      name = argv[indx];
#ifndef _WIN32
      if ((fd = open (name, O_RDONLY, 0)) == -1)
	goto syserr;
      if (fstat (fd, &stb))
#else
      fd = -1;
      if (stat (name, &stb))
#endif
	{
	syserr:
	  run_err ("%s: %s", name, strerror (errno));
	  goto next;
	}
      switch (stb.st_mode & S_IFMT)
	{
	case S_IFREG:
#ifdef _WIN32
	  if ((fd = open (name, O_RDONLY, 0)) == -1)
	    goto syserr;
#endif
	  break;

	case S_IFDIR:
	  if (iamrecursive)
	    {
	      rsource (name, &stb);
	      goto next;
	    }
	  /* FALLTHROUGH */
	default:
	  run_err ("%s: not a regular file", name);
	  goto next;
	}
      if ((last = strrchr (name, '/')) == NULL
	  && (last = strrchr (name, '\\')) == NULL)
	last = name;
      else
	++last;
      if (pflag)
	{
	  write_stat_time (STDOUT_FILENO, &stb);
	  if (response() < 0)
	    goto next;
	}
#define	RCP_MODEMASK	(S_IRWXU)
      snprintf(buf, sizeof buf,
	       (sizeof(stb.st_size) > sizeof(long)
		? "C%04o %qd %s\n"
		: "C%04o %ld %s\n"),
	       stb.st_mode & RCP_MODEMASK, stb.st_size, last);
      write (STDOUT_FILENO, buf, strlen (buf));
      if (response () < 0)
	goto next;
      if ((bp = allocbuf (&buffer, fd, BUFSIZ)) == NULL)
	{
	next:
	  close (fd);
	  continue;
	}

      /* Keep writing after an error so that we stay sync'd up. */
      for (haderr = i = 0; i < stb.st_size; i += bp->cnt)
	{
	  amt = bp->cnt;
	  if (i + amt > stb.st_size)
	    amt = stb.st_size - i;
	  if (!haderr)
	    {
	      result = read (fd, bp->buf, amt);
	      if (result != amt)
		haderr = result >= 0 ? EIO : errno;
	    }
	  if (haderr)
	    write (STDOUT_FILENO, bp->buf, amt);
	  else
	    {
	      result = write (STDOUT_FILENO, bp->buf, amt);
	      if (result != amt)
		haderr = result >= 0 ? EIO : errno;
	    }
	}
      if (close (fd) && !haderr)
	haderr = errno;
      if (!haderr)
	write (STDOUT_FILENO, "", 1);
      else
	run_err ("%s: %s", name, strerror(haderr));
      response ();
    }
}

void
rsource (char *name, struct stat *statp)
{
  DIR *dirp;
  struct dirent *dp;
  char *last, *vect[1];
  char *buf;
  int buf_len;

  if (!(dirp = opendir (name)))
    {
      run_err ("%s: %s", name, strerror(errno));
      return;
    }
  last = strrchr (name, '/');
  if (last == 0)
    last = strrchr (name, '\\');
  if (last == 0)
    last = name;
  else
    last++;

  if (pflag)
    {
      write_stat_time (STDOUT_FILENO, statp);
      if (response () < 0) {
	closedir (dirp);
	return;
      }
    }

  buf_len =
    1 + sizeof (int) * 3 + 1 + sizeof (int) * 3 + 1 + strlen (last) + 2;
  buf = malloc (buf_len);
  if (! buf)
    {
      run_err ("malloc failed for %d bytes", buf_len);
      closedir (dirp);
      return;
    }

  sprintf (buf, "D%04o %d %s\n", statp->st_mode & RCP_MODEMASK, 0, last);
  write (STDOUT_FILENO, buf, strlen (buf));
  free (buf);

  if (response () < 0)
    {
      closedir (dirp);
      return;
    }

  while ((dp = readdir(dirp)))
    {
      if (!strcmp (dp->d_name, ".") || !strcmp (dp->d_name, ".."))
	continue;

      buf_len = strlen (name) + 1 + strlen (dp->d_name) + 1;
      buf = malloc (buf_len);
      if (! buf)
	{
	  run_err ("malloc_failed for %d bytes", buf_len);
	  continue;
	}

      sprintf (buf, "%s/%s", name, dp->d_name);
      vect[0] = buf;
      source(1, vect);
      free (buf);
    }

  closedir (dirp);
  write (STDOUT_FILENO, "E\n", 2);
  response ();
}

void
sink (int argc, char *argv[])
{
  static BUF buffer;
  struct stat stb;
  struct timeval tv[2];
  enum { YES, NO, DISPLAYED } wrerr;
  BUF *bp;
  off_t i, j;
  int amt, count, exists, first, mode, ofd, omode;
  int setimes, size, targisdir, wrerrno;
  char ch, *cp, *np, *targ, *vect[1], buf[BUFSIZ];
  const char *why;

#define	atime	tv[0]
#define	mtime	tv[1]
#define	SCREWUP(str)	{ why = str; goto screwup; }

  setimes = targisdir = 0;
  if (argc != 1)
    {
      run_err ("ambiguous target");
      exit (1);
    }
  targ = *argv;
  if (targetshouldbedirectory)
    verifydir (targ);
  if (write (STDOUT_FILENO, "", 1) != 1)
    return;
  if (stat (targ, &stb) == 0 && S_ISDIR (stb.st_mode))
    targisdir = 1;
  for (first = 1;; first = 0)
    {
      cp = buf;
      if (read (STDIN_FILENO, cp, 1) <= 0)
	return;
      if (*cp++ == '\n')
	SCREWUP ("unexpected <newline>");
      do
	{
	  if (read (STDIN_FILENO, &ch, sizeof ch) != sizeof ch)
	    SCREWUP("lost connection");
	  *cp++ = ch;
	} while (cp < &buf[BUFSIZ - 1] && ch != '\n');
      *cp = 0;

      if (buf[0] == '\01' || buf[0] == '\02')
	{
	  if (iamremote == 0)
	    write (STDERR_FILENO, buf + 1, strlen(buf + 1));
	  if (buf[0] == '\02')
	    exit (1);
	  ++errs;
	  continue;
	}
      if (buf[0] == 'E')
	{
	  write (STDOUT_FILENO, "", 1);
	  return;
	}

      if (ch == '\n')
	*--cp = 0;

#define getnum(t) (t) = 0; while (isdigit(*cp)) (t) = (t) * 10 + (*cp++ - '0');
      cp = buf;
      if (*cp == 'T')
	{
	  setimes++;
	  cp++;
	  getnum(mtime.tv_sec);
	  if (*cp++ != ' ')
	    SCREWUP ("mtime.sec not delimited");
	  getnum (mtime.tv_usec);
	  if (*cp++ != ' ')
	    SCREWUP ("mtime.usec not delimited");
	  getnum (atime.tv_sec);
	  if (*cp++ != ' ')
	    SCREWUP ("atime.sec not delimited");
	  getnum (atime.tv_usec);
	  if (*cp++ != '\0')
	    SCREWUP ("atime.usec not delimited");
	  write (STDOUT_FILENO, "", 1);
	  continue;
	}
      if (*cp != 'C' && *cp != 'D')
	{
	  /*
	   * Check for the case "rcp remote:foo\* local:bar".
	   * In this case, the line "No match." can be returned
	   * by the shell before the rcp command on the remote is
	   * executed so the ^Aerror_message convention isn't
	   * followed.
	   */
	  if (first)
	    {
	      run_err ("%s", cp);
	      exit (1);
	    }
	  SCREWUP ("expected control record");
	}
      mode = 0;
      for (++cp; cp < buf + 5; cp++)
	{
	  if (*cp < '0' || *cp > '7')
	    SCREWUP ("bad mode");
	  mode = (mode << 3) | (*cp - '0');
	}
      if (*cp++ != ' ')
	SCREWUP ("mode not delimited");

      for (size = 0; isdigit(*cp);)
	size = size * 10 + (*cp++ - '0');
      if (*cp++ != ' ')
	SCREWUP ("size not delimited");
      if (targisdir)
	{
	  static char *namebuf;
	  static int cursize;
	  size_t need;

	  need = strlen (targ) + strlen (cp) + 250;
	  if (need > cursize)
	    {
	      if (!(namebuf = malloc (need)))
		run_err("%s", strerror (errno));
	    }
	  snprintf (namebuf, need, "%s%s%s", targ, *targ ? "/" : "", cp);
	  np = namebuf;
	}
      else
	np = targ;
      exists = stat (np, &stb) == 0;
      if (buf[0] == 'D')
	{
	  int mod_flag = pflag;
	  if (exists)
	    {
	      if (!S_ISDIR (stb.st_mode))
		{
		  __set_errno (ENOTDIR);
		  goto bad;
		}
	      if (pflag)
		chmod (np, mode);
	    }
	  else
	    {
	      /* Handle copying from a read-only directory */
	      mod_flag = 1;
	      if (mkdir (np) < 0)
		goto bad;
	    }
	  vect[0] = np;
	  sink (1, vect);
	  if (setimes)
	    {
	      struct utimbuf utbuf;
	      utbuf.actime = atime.tv_sec;
	      utbuf.modtime = mtime.tv_sec;
	      setimes = 0;
	      if (utime (np, &utbuf) < 0)
		run_err ("%s: set times: %s", np, strerror (errno));
	    }
	  if (mod_flag)
	    chmod (np, mode);
	  continue;
	}
      omode = mode;
      mode |= S_IWRITE;
      if ((ofd = open (np, O_WRONLY|O_CREAT, mode)) == -1)
	{
	bad:
	  run_err ("%s: %s", np, strerror(errno));
	  continue;
	}
      write (STDOUT_FILENO, "", 1);
      if ((bp = allocbuf (&buffer, ofd, BUFSIZ)) == NULL)
	{
	  close (ofd);
	  continue;
	}
      cp = bp->buf;
      wrerr = NO;
      for (count = i = 0; i < size; i += BUFSIZ)
	{
	  amt = BUFSIZ;
	  if (i + amt > size)
	    amt = size - i;
	  count += amt;
	  do
	    {
	      j = read (STDIN_FILENO, cp, amt);
	      if (j <= 0)
		{
		  run_err ("%s", j ? strerror(errno) : "dropped connection");
		  exit (1);
		}
	      amt -= j;
	      cp += j;
	    } while (amt > 0);
	  if (count == bp->cnt)
	    {
	      /* Keep reading so we stay sync'd up. */
	      if (wrerr == NO)
		{
		  j = write (ofd, bp->buf, count);
		  if (j != count)
		    {
		      wrerr = YES;
		      wrerrno = j >= 0 ? EIO : errno;
		    }
		}
	      count = 0;
	      cp = bp->buf;
	    }
	}
      if (count != 0 && wrerr == NO
	  && (j = write (ofd, bp->buf, count)) != count)
	{
	  wrerr = YES;
	  wrerrno = j >= 0 ? EIO : errno;
	}
      if (ftruncate (ofd, size))
	{
	  run_err ("%s: truncate: %s", np, strerror (errno));
	  wrerr = DISPLAYED;
	}
      if (pflag)
	{
	  if (exists || omode != mode)
	    if (chmod (np, omode))
	      run_err ("%s: set mode: %s", np, strerror (errno));
	}
      else
	{
	  if (!exists && omode != mode)
	    if (chmod (np, omode))
	      run_err ("%s: set mode: %s", np, strerror (errno));
	}
      close (ofd);
      response ();
      if (setimes && wrerr == NO)
	{
	  struct utimbuf utbuf;
	  utbuf.actime = atime.tv_sec;
	  utbuf.modtime = mtime.tv_sec;
	  setimes = 0;
	  if (utime (np, &utbuf) < 0)
	    {
	      run_err ("%s: set times: %s", np, strerror (errno));
	      wrerr = DISPLAYED;
	    }
	}
      switch (wrerr)
	{
	case YES:
	  run_err ("%s: %s", np, strerror(wrerrno));
	  break;

	case NO:
	  write (STDOUT_FILENO, "", 1);
	  break;

	case DISPLAYED:
	  break;
	}
    }
screwup:
      run_err ("protocol error: %s", why);
      exit (1);
}

int
response ()
{
  char ch, *cp, resp, rbuf[BUFSIZ];

  if (read (STDIN_FILENO, &resp, sizeof resp) != sizeof resp)
    lostconn (0);

  cp = rbuf;
  switch (resp)
    {
    case 0:				/* ok */
      return 0;

    default:
      *cp++ = resp;
      /* FALLTHROUGH */
    case 1:				/* error, followed by error msg */
    case 2:				/* fatal error, "" */
      do
	{
	  if (read (STDIN_FILENO, &ch, sizeof(ch)) != sizeof(ch))
	    lostconn(0);
	  *cp++ = ch;
	} while (cp < &rbuf[BUFSIZ] && ch != '\n');

      if (!iamremote)
	write (STDERR_FILENO, rbuf, cp - rbuf);
      ++errs;
      if (resp == 1)
	return -1;
      exit (1);
    }
  /* NOTREACHED */
}

void
usage ()
{
  fprintf (stderr,
	   "usage: rcp [-p] f1 f2; or: rcp [-pr] f1 ... fn directory\n");
  exit (1);
}

void
help ()
{
  puts ("rcp - remote file copy.");
  puts ("usage: rcp [-p] f1 f2; or: rcp [-pr] f1 ... fn directory\n");
  puts ("\
  -p, --preserve    attempt to preserve (duplicate) in its copies the\n\
                    modification times and modes of the source files");
  puts ("\
  -r, --recursive   If any of the source files are directories, copies\n\
                    each subtree rooted at that name; in this case the\n\
                    destination must be a directory");

  puts ("\
      --help        give this help list");
  puts ("\
  -V, --version     print program version");
  fprintf (stdout, "\nSubmit bug reports to %s.\n", PACKAGE_BUGREPORT);
  exit (0);
}

#include <stdarg.h>

void
warnerr(int doerrno, const char *fmt, va_list ap)
{
    int sverrno = errno;
    fprintf(stderr, "rcp");
    if(fmt != NULL || doerrno)
	fprintf(stderr, ": ");
    if (fmt != NULL){
	vfprintf(stderr, fmt, ap);
	if(doerrno)
	    fprintf(stderr, ": ");
    }
    if(doerrno)
	fprintf(stderr, "%s", strerror(sverrno));
    fprintf(stderr, "\n");
}

void
vwarnx(const char *fmt, va_list ap)
{
    warnerr(0, fmt, ap);
}

void
run_err (const char *fmt, ...)
{
  static FILE *fp;
  va_list ap;
  va_start (ap, fmt);
  ++errs;
  if (fp == NULL && !(fp = fdopen (STDOUT_FILENO, "w")))
    return;
  fprintf (fp, "%c", 0x01);
  fprintf (fp, "rcp: ");
  vfprintf (fp, fmt, ap);
  fprintf (fp, "\n");
  fflush (fp);

  if (!iamremote)
    vwarnx (fmt, ap);

  va_end (ap);
}
