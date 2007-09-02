/*-
 * Copyright (c) 1992, 1993
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
 *
 *	@(#)extern.h	8.1 (Berkeley) 5/31/93
 */

typedef struct {
	int cnt;
	char *buf;
} BUF;

extern int iamremote;

BUF	*allocbuf (BUF *, int, int);
char	*colon (char *);
void	 lostconn (int);
void	 nospace (void);
int	 okname (char *);
void	 run_err (const char *, ...);
int	 susystem (char *, int);
void	 verifydir (char *);

/* Print the program name and error message MESSAGE, which is a
   printf-style format string with optional args.  If ERRNUM is
   nonzero, print its corresponding system error message.  Exit with
   status STATUS if it is nonzero.  */
void     error (int status, int errnum, const char *message, ...);

struct rcp_stat
{
  _dev_t  st_dev;         /* Equivalent to drive number 0=A 1=B ... */
  _ino_t  st_ino;         /* Always zero ? */
  _mode_t st_mode;        /* See above constants */
  short   st_nlink;       /* Number of links. */
  short   st_uid;         /* User: Maybe significant on NT ? */
  short   st_gid;         /* Group: Ditto */
  _dev_t  st_rdev;        /* Seems useless (not even filled in) */
  _off_t  st_size;        /* File size in bytes */
  time_t  st_atime;       /* Accessed date (always 00:00 hrs local
                                 * on FAT) */
  time_t  st_mtime;       /* Modified time */
  time_t  st_ctime;       /* Creation time */
};

#include <cwd.h>
#include <dirent.h>

int rcp_open (const char *path, int flag, int mode);
int rcp_mkdir (const char *path);
int rcp_stat (const char *path, struct rcp_stat *statbuf);
int rcp_fstat (int fd, struct rcp_stat *statbuf);
DIR * rcp_opendir (const char *path);


#define open rcp_open
#define mkdir rcp_mkdir
#define stat rcp_stat
#define fstat rcp_fstat
#define opendir rcp_opendir
