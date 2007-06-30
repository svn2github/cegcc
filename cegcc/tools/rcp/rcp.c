/*
Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions
are met:
1. The name of the author may not be used to endorse or promote products
   derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.  */

/* Follows the original Copyright and disclaimer statements of the
   original package.  */

//////////////////////////////////////////////////////////////////////////
//
//  rshd_rcp.cpp
//
//  This file contains code which will extend rshd to handle rcp
//  requests.
//
//  Author:  Gary L. Doss  NCR Corporation
//  Date:    December 16, 1996
//
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
//
// rsh daemon for Windows NT/95
// (c) 1996 Silviu Marghescu - Emaginet, Inc.
//
//
// This program is free software; you can redistribute and/or modify
//      it.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
//
//
// rshd - Remote Shell Daemon for Windows NT/95
// Author: Silviu C. Marghescu (http://www.cs.umd.edu/~silviu)
// Date:   May 16, 1996
//
//////////////////////////////////////////////////////////////////////////

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <io.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <dirent.h>
#include <stdarg.h>

#include <windows.h>

#include <cwd.h>

static const char *logfile = "rcp.log";

static int debugFlag = 0;

static void
vlog (const char *pre, const char *format, va_list ap)
{
  FILE *f = fopen (logfile, "a+");
  if (!f)
    return;
  fprintf (f, "%s", pre);
  vfprintf (f, format, ap);
  fclose (f);
}

static void
error (const char *format, ...)
{
  if (debugFlag)
    {
      va_list ap;
      va_start (ap, format);
      vlog ("error: ", format, ap);
      vlog ("", "\n", ap);
      va_end (ap);
    }
}

static void
debug (const char *format, ...)
{
  if (debugFlag)
    {
      va_list ap;
      va_start (ap, format);
      vlog ("debug: ", format, ap);
      va_end (ap);
    }
}

#ifndef MIN
#define MIN(A, B) ((A) < (B) ? (A) : (B))
#endif

static volatile DWORD totalread;

DWORD WINAPI
LogThread (void *arg)
{
  int count = 0;

  while (1)
    if (WAIT_TIMEOUT == (WaitForSingleObject (GetModuleHandle (NULL), 1000)))
      debug ("(%08d)\ttotalread = %lu\n", count++, totalread);
    else
      break;

  return 0;
}

/* This is the size of the buffer used with rcp */
#define RCP_BUFFER_SIZE 8192

/* Several of the messages sent in the rcp protocol are single
   byte ( 0x00 ) or are text messages terminated by a ('\n').
   This routine reads the message a single byte at a time and
   checks for the appropriate termination condition.
   BUFF is the buffer to hold the data.
   BLEN is the length of the buffer.
   returns -1 if recv fails or number of characters received on SUCCESS.  */
static int
RcpReceive (char *buff, int blen)
{
  int i;
  int rlen;
  char tchar;
  int fildes = fileno (stdin);

  i = 0;
  buff[0] = 0;
  do
    {
      rlen = read (fildes, &buff[i], 1);
      if (rlen == -1)
	{
	  error ("stdin closed.\n");
	  return -1;
	}

      if (rlen == 0)
	{
	  debug ("...got 0 chars.\n");
	  continue;
	}

      debug ("...got 1 char. [%c]\n", buff[i]);
      tchar = buff[i];
      i++;
      totalread++;

      if (i > blen)
	{
	  /* The buffer has overflowed.  */
	  debug ("buffer overflow (%d/%d)\n", i, blen);
	  SetLastError (WSAEMSGSIZE);
	  return -1;
	}
    }
  while (tchar != '\n' && tchar != '\0');

  return i;
}

/* ParseTarget is the first step in processing environment
   variables and wild card characters that may exists in
   the target specification of the rcp command.  All
   environment variables are expanded and a find is initiated
   to handle the wild card characters ( ? and * ).
   HFILE is a pointer to a file handle.  The file handle is used
   by the calling process to obtain more files associated with the target.
   TARGET is the target file/directory that needs to be expanded.
   BDIR is a flag will be set to TRUE if the TARGET is a directory and
   FALSE if it is a file.
   Return TRUE if there are possibly more files that match the target
   and FALSE if this is the only file that matches.

   The wildcard characters are only valid if used in the last item
   in the path specified by Target.

   See Also NextTarget and CloseTarget.  */
static BOOL
ParseTarget (HANDLE * hFile, char *Target, BOOL *bDir)
{
  char strPath[MAX_PATH];
#if 0
  long lLen;
#endif
  WIN32_FIND_DATAA wfdFileData;
  BOOL bMoreFiles = FALSE;
  char *strLastSlash;
  char strDirectory[MAX_PATH];
  struct stat statbuf;

  /* TARGET may contain:
     Environment Variables:  %name%
     Wild Card Characters: ? and *  */
#if 0
  lLen = ExpandEnvironmentStrings (Target, strPath, MAX_PATH);

  debug ("The expanded path is %d chars %d: %s\n", lLen,
	 GetLastError (), strPath);
#else
  strcpy (strPath, Target);
#endif
  /* Determine the directory name for the expanded target.  */
  strLastSlash = strchr (strPath, '/');
  while (strLastSlash != NULL)
    {
      *strLastSlash = '\\';
      strLastSlash++;
      strLastSlash = strrchr (strLastSlash, '/');
    }

  strLastSlash = strrchr (strPath, '\\');
  if ((strLastSlash == NULL) || (strLastSlash == strPath))
    strDirectory[0] = 0;
  else
    {
      strncpy (strDirectory, strPath, (long) (strLastSlash - strPath));
      strDirectory[(long) (strLastSlash - strPath)] = 0;
      strcat (strDirectory, "\\");
    }

  /* If the target has wildcards, process them.  */
  if ((strchr (strPath, '?') != NULL) || (strchr (strPath, '*') != NULL))
    {
      *hFile = FindFirstFileA (strPath, &wfdFileData);
      if (*hFile != INVALID_HANDLE_VALUE)
	{
	  bMoreFiles = TRUE;
	  if (wfdFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
	    {
	      *bDir = TRUE;
	      /* Ignore directories "." and ".."  */
	      while (!(strcmp (wfdFileData.cFileName, ".")) ||
		     !(strcmp (wfdFileData.cFileName, "..")))
		{
		  if (!FindNextFileA (*hFile, &wfdFileData))
		    {
		      /* Handle error.  */
		      Target[0] = 0;
		      *bDir = FALSE;
		      return FALSE;
		    }
		  *bDir =
		    (wfdFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
		    ? TRUE : FALSE;
		}
	    }
	  else
	    *bDir = FALSE;

	  sprintf (Target, "%s%s", strDirectory, wfdFileData.cFileName);
	}
      else
	{
	  Target[0] = 0;
	  *bDir = FALSE;
	  return FALSE;
	}
    }
  else
    {
      /* Check to see if Target is a file or a directory.  */
      strcpy (Target, strPath);
      if (stat (Target, &statbuf) != 0)
	return FALSE;
      else
	*bDir = (statbuf.st_mode & S_IFDIR) ? TRUE : FALSE;
    }
  return bMoreFiles;
}

/* This function gets the next available target that matches
   the specification passed to ParseTarget for the specified
   HANDLE.  HFILE is a HANDLE returned from call to ParseTarget.
   BDIR is a flag that will be set to TRUE if the TARGET is a
   directory and to FALSE if it is a regular file.
   Returns NULL if no more matches exist, of a pointer to target
   name if a match is found.

   Assumptions:
   The pointer returned by NextTarget should never be deleted.
   NextTarget is always called after ParseTarget.
   No target names will be larger than MAX_PATH.

   See Also:  ParseTarget, CloseTarget.  */
static char *
NextTarget (HANDLE hFile, BOOL * bDir)
{
  static char Target[MAX_PATH];
  WIN32_FIND_DATAA wfdFileData;

  /* Make sure the handle is not bad.  */
  if (hFile == INVALID_HANDLE_VALUE)
    {
      *bDir = FALSE;
      return NULL;
    }

  if (FindNextFileA (hFile, &wfdFileData))
    {
      /* A match was found.  */
      if (wfdFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
	{
	  *bDir = TRUE;
	  /* Ignore directories "." and "..".  */
	  while (!(strcmp (wfdFileData.cFileName, ".")) ||
		 !(strcmp (wfdFileData.cFileName, "..")))
	    {
	      if (!FindNextFileA (hFile, &wfdFileData))
		{
		  /* Handle error.  */
		  Target[0] = 0;
		  *bDir = FALSE;
		  return NULL;
		}
	      if (wfdFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
		*bDir = TRUE;
	      else
		*bDir = FALSE;
	    }
	}
      else
	*bDir = FALSE;

      sprintf (Target, "%s", wfdFileData.cFileName);
    }
  else
    {
      Target[0] = 0;
      *bDir = FALSE;
      return NULL;
    }
  return (Target);
}

/* Terminate the search for target matches that was initiated
   in the call to ParseTarget.  HFILE is a HANDLE returned from
   the call to ParseTarget.

   Assumptions:
   CloseTarget must always be used to close the find initiated by
   ParseTarget.
   There are no more matches to a target when NextTarget returns
   NULL.

   See Also:  ParseTarget, NextTarget.  */
void
CloseTarget (HANDLE hFile)
{
  if (hFile != INVALID_HANDLE_VALUE)
    FindClose (hFile);
  return;
}

/* This functions processes an rcp request to send files to
   a remote system. TARGET is the target specified in the rcp
   request.  BRECURSIVE specified that this request must recurse
   the sub-directories of the specfied target.

   Assumptions:
   All files sent are read as BINARY files.  This prevents
   the translation of CR-LF to LF and preserves the size of the file
   and the data contained in it.  */
void
RcpSvrSend (const char *Target, BOOL bRecursive)
{
  char buff[RCP_BUFFER_SIZE + 1];
  int blen = RCP_BUFFER_SIZE;
  int FileId;
  int nFileSize;
  int nBytesRead;
  int nBytesSent;
  int dwBytes;
  int nValue;
  BOOL bMoreFiles;
  HANDLE hFile = INVALID_HANDLE_VALUE;
  char expTarget[MAX_PATH];
  char *Target2;
  char *FileName;
  BOOL bDir;
  BOOL bTarget;
  BOOL bProcessing;
  struct _stat statbuf;

  debug ("sending %s\n", Target);

  /* Copy the target to a buffer we know will hold MAX_PATH.  */
  strcpy (expTarget, Target);
  /* Check the target for environment variables and wild cards.  */
  bMoreFiles = ParseTarget (&hFile, expTarget, &bDir);
  bTarget = bDir;

  if (!bRecursive & bDir)
    {
      /* Error condition.  */
      buff[0] = 1;
      sprintf (&buff[1], "rcp:  %s: Not a plain file\n", expTarget);
      if (write (fileno (stdout), buff, strlen (&buff[1]) + 1) < 1)
	error ("Error sending result status.");
      if (bMoreFiles)
	CloseTarget (hFile);
      return;
    }

  debug ("sending %s\n", expTarget);

  if (access (expTarget, 02) != 0)
    {
      debug ("error\n");
      /* Error condition */
      buff[0] = 1;
      if (errno == ENOENT)
	sprintf (&buff[1], "rcp: %s: No such file or directory\n", expTarget);
      else
	sprintf (&buff[1], "rcp: %s: Permission Denied\n", expTarget);
      if (write (fileno (stdout), buff, strlen (&buff[1]) + 1) < 1)
	error ("Error sending result status.");
      if (bMoreFiles)
	CloseTarget (hFile);
      return;
    }

  debug ("waiting for '\\0'\n");

  /* Receive data from the client expecting 0x00.  */
  if ((dwBytes = RcpReceive (buff, blen)) == -1)
    {
      error ("Cannot receive client data.");
      if (bMoreFiles)
	CloseTarget (hFile);
      return;
    }

  debug ("waiting for '\\0' : done\n");

  if (buff[0] != 0)
    {
      error ("Remote system failed.");
      if (bMoreFiles)
	CloseTarget (hFile);
      return;
    }

  bProcessing = TRUE;
  Target2 = expTarget;
  while (Target2 != NULL)
    {
      if (bDir)
	{
	  /* Notify remote system to create a directory.  */
	  FileName = strrchr (Target2, '\\');
	  if (FileName == NULL)
	    FileName = Target2;
	  else
	    FileName++;

	  sprintf (buff, "D0755 0 %s\n", FileName);

	  if (write (fileno (stdout), buff, strlen (buff)) < 1)
	    {
	      error ("Error sending directory status.");
	      if (bMoreFiles)
		CloseTarget (hFile);
	      return;
	    }

	  chdir (Target2);
	  RcpSvrSend ("*", bRecursive);
	  chdir ("..");
	}
      else
	{
	  FileName = strrchr (Target2, '\\');
	  if (FileName == NULL)
	    FileName = Target2;
	  else
	    {
	      *FileName = 0;
	      chdir (Target2);
	      FileName++;
	    }
	  /* Open the file for reading.  */
	  FileId = _open (FileName, O_RDONLY | O_BINARY, S_IWRITE);
	  if (FileId == -1)
	    {
	      /* Error condition.  */
	      buff[0] = 1;
	      sprintf (&buff[1], "rcp: %s: Cannot open file\n", FileName);
	      if (write (fileno (stdout), buff, strlen (&buff[1]) + 1) < 1)
		error ("Error sending result status.");
	      if (bMoreFiles)
		CloseTarget (hFile);
	      return;
	    }
	  else
	    {
	      /* Notify remote system to create a file.  */
	      nValue = _fstat (FileId, &statbuf);
	      nFileSize = statbuf.st_size;
	      sprintf (buff, "C0644 %d %s\n", nFileSize, FileName);
	      if (write (fileno (stdout), buff, strlen (buff)) < 1)
		{
		  error ("Error sending result status.");
		  close (FileId);
		  if (bMoreFiles)
		    CloseTarget (hFile);
		  return;
		}

	      /* Receive data from the client expecting 0x00.  */
	      if ((dwBytes = RcpReceive (buff, blen)) == -1)
		{
		  error ("Cannot receive client data.");
		  close (FileId);
		  if (bMoreFiles)
		    CloseTarget (hFile);
		  return;
		}
	      if (buff[0] != 0)
		{
		  error ("Remote system Failed.");
		  close (FileId);
		  if (bMoreFiles)
		    CloseTarget (hFile);
		  return;
		}

	      /* Process the contents of the file.  */
	      nBytesSent = 0;
	      while (nBytesSent < nFileSize)
		{
		  /* Read the file.  */
		  nBytesRead = read (FileId, buff, blen);
		  if (nBytesRead <= 0)
		    {
		      /* Error condition.  */
		      buff[0] = 1;
		      sprintf (&buff[1], "rcp: %s: Cannot read source\n",
			       FileName);
		      if (write (fileno (stdout), buff, strlen (&buff[1]) + 1)
			  < 1)
			error ("Error sending result status.");
		      close (FileId);
		      if (bMoreFiles)
			CloseTarget (hFile);
		      return;
		    }

		  totalread += nBytesRead;

		  nBytesSent += nBytesRead;
		  if (write (fileno (stdout), buff, nBytesRead) < 1)
		    {
		      error ("Error sending file.");
		      close (FileId);
		      if (bMoreFiles)
			CloseTarget (hFile);
		      return;
		    }
		}

	      close (FileId);

	      buff[0] = 0;
	      if (write (fileno (stdout), buff, 1) < 1)
		{
		  error ("Error sending file termination.");
		  if (bMoreFiles)
		    CloseTarget (hFile);
		  return;
		}

	    }

	  /* Receive data from the client expecting 0x00.  */
	  if ((dwBytes = RcpReceive (buff, blen)) == -1)
	    {
	      error ("Cannot receive client data.");
	      if (bMoreFiles)
		CloseTarget (hFile);
	      return;
	    }
	  if (buff[0] != 0)
	    {
	      error ("Remote system failed.");
	      if (bMoreFiles)
		CloseTarget (hFile);
	      return;
	    }

	}

      Target2 = NextTarget (hFile, &bDir);
      if (Target2 == NULL)
	CloseTarget (hFile);
    }

  if (bRecursive)
    {
      /* Recursive sends are closed by sending "E\n".  */
      sprintf (buff, "E\n");
      if (write (fileno (stdout), buff, strlen (buff)) < 1)
	{
	  error ("Error sending directory status.");
	  return;
	}

      /* Receive data from the client.  */
      if ((dwBytes = RcpReceive (buff, blen)) == -1)
	{
	  error ("Cannot receive client data.");
	  return;
	}
      if (buff[0] != 0)
	{
	  error ("Remote system Failed.");
	  return;
	}

    }

}

/* Process files being sent by a remote system.
   TARGET is the target specified in the rcp request.
   BRECURSIVE specifies if this request recurses sub-directories
   on the remote system.  Directories may need to be created.
   BTARGDIR specifies if the target specified MUST be a directory.

   Assumptions:
   All files are written as BINARY to preserve the file size and
   data contained in the files.  */
void
RcpSvrRecv (char *Target, BOOL bRecursive, BOOL bTargDir)
{
  char buff[RCP_BUFFER_SIZE + 1];
  DWORD blen = RCP_BUFFER_SIZE;
  int FileId;
  DWORD dwFileSize;
  DWORD dwBytesRecv;
  int dwBytes;
  int nValue;
  BOOL bMoreFiles;
  HANDLE hFile = INVALID_HANDLE_VALUE;
  char expTarget[MAX_PATH];
  char *Target2;
  char *NewLine;
  BOOL bDir;
  BOOL bTarget;
  BOOL bProcessing;

  debug ("RcpSvrRecv\n");

  strcpy (expTarget, Target);
  bDir = bTargDir;
  bMoreFiles = ParseTarget (&hFile, expTarget, &bDir);
  if (bMoreFiles)
    {
      Target2 = NextTarget (hFile, &bTarget);
      if (Target2 != NULL)
	{
	  /* Error condition:  more than one target.  */
	  buff[0] = 1;
	  sprintf (&buff[1], "rcp:  ambiguous target\n");
	  if (write (fileno (stdout), buff, strlen (&buff[1]) + 1) < 1)
	    {
	      error ("Error sending result status.");
	    }
	  CloseTarget (hFile);
	  return;
	}
    }
  CloseTarget (hFile);
  bTarget = bDir;

  if (bTargDir & !bDir)
    {
      /* Error condition:  Directory required but file specified.  */
      buff[0] = 1;
      sprintf (&buff[1], "rcp:  %s: Not a directory\n", expTarget);
      if (write (fileno (stdout), buff, strlen (&buff[1]) + 1) < 1)
	{
	  error ("Error sending result status.");
	}
      return;
    }

  if (access (expTarget, 02) != 0)
    {
      if (bDir || (!bDir && (errno != ENOENT)))
	{
	  /* Error condition:  Can't access the target.  */
	  buff[0] = 1;
	  if (bDir && (errno == ENOENT))
	    sprintf (&buff[1], "rcp: %s: No such file or directory\n",
		     expTarget);
	  else
	    sprintf (&buff[1], "rcp: %s: Permission Denied\n", expTarget);
	  if (write (fileno (stdout), buff, strlen (&buff[1]) + 1) < 1)
	    error ("Error sending result status.");
	  return;
	}
    }

  bProcessing = TRUE;
  Target2 = expTarget;

  /* Process files/directories from the remote system.  */
  while (bProcessing)
    {
      debug ("Sending null byte ...\n");
      buff[0] = 0;
      if (write (fileno (stdout), buff, 1) < 1)
	{
	  error ("Error sending result status.");
	  return;
	}
      debug ("Sent.\n");

      if (bDir)
	{
	  nValue = chdir (Target2);
	  if (nValue == -1)
	    {
	      /* Error condition.  */
	      buff[0] = 1;
	      sprintf (&buff[1], "rcp: %s: No such file or directory\n",
		       expTarget);
	      if (write (fileno (stdout), buff, strlen (&buff[1]) + 1) < 1)
		error ("Error sending result status.");
	      return;
	    }
	}

      debug ("waiting for data.\n");

      /* Receive data from the client
      	 File/dir  specification ends in a '\n', so read byte by byte
	 until one is reached or a '\0' is received.  */
      if ((dwBytes = RcpReceive (buff, blen)) == -1)
	{
	  error ("Cannot receive client data.");;
	  return;
	}

      debug ("got %d bytes.\n", dwBytes);
      debug ("got: %d (%c)\n", buff[0], buff[0]);

      /* Process the file or directory specification.  */
      switch (buff[0])
	{
	case 0:
	case 1:
	  /* Finished processing.  */
	  return;

	case 'E':
	  /* Finished with current directory.  Backup to the
	     parent directory.  */

	  Target2 = "..";
	  bDir = TRUE;
	  continue;

	case 'T':
	  /* This is permissions data related to the -p option.
	     Just ignore it.  */
	  continue;

	case 'D':
	  /* A directory is being identified.  */

	  bDir = TRUE;
	  Target2 = strtok (buff, " ");
	  Target2 = strtok (NULL, " ");
	  Target2 = strtok (NULL, " ");
	  NewLine = strchr (Target2, 0x0a);
	  *NewLine = 0;
	  strcpy (expTarget, Target2);
	  Target2 = expTarget;

	  if (access (Target2, 02) != 0)
	    {
	      if (errno != ENOENT)
		{
		  /* Error condition:  Can't access directory.  */
		  buff[0] = 1;
		  sprintf (&buff[1], "rcp: %s: Directory access failure %d\n",
			   expTarget, errno);
		  if (write (fileno (stdout), buff, strlen (&buff[1]) + 1) < 1)
		    error ("Error sending result status.");
		  return;
		}
	      /* Create directory.  */
	      nValue = mkdir (Target2);
	      if (nValue == -1)
		{
		  /* Error condition:  Can't create directory.  */
		  buff[0] = 1;
		  sprintf (&buff[1], "rcp: %s: Directory creation failed\n",
			   expTarget);
		  if (write (fileno (stdout), buff, strlen (&buff[1]) + 1) < 1)
		    error ("Error sending result status.");
		  return;
		}
	    }
	  continue;

	case 'C':
	  /* A file  is being identified.  */
	  if (bTarget)
	    {
	      Target2 = strtok (buff, " ");
	      Target2 = strtok (NULL, " ");
	      Target2 = strtok (NULL, " ");
	      NewLine = strchr (Target2, 0x0a);
	      *NewLine = 0;
	      strcpy (expTarget, Target2);
	      Target2 = expTarget;
	    }

	  bDir = FALSE;
	  /* Open the file for writing.  */
	  FileId =
	    _open (Target2, _O_WRONLY | _O_TRUNC | _O_CREAT | _O_BINARY,
		   _S_IWRITE);
	  if (FileId == -1)
	    {
	      /* Error condition.  */
	      buff[0] = 1;
	      sprintf (&buff[1], "rcp: %s :Cannot open file\n", Target2);
	      if (write (fileno (stdout), buff, strlen (&buff[1]) + 1) < 1)
		error ("Error sending result status.");
	      return;
	    }
	  break;

	default:
	  return;
	}

      dwFileSize = atol (&buff[6]);
      debug ("Receiving file %s of size %lu.\n", Target2, dwFileSize);

      buff[0] = 0;
      if (write (fileno (stdout), buff, 1) < 1)
	{
	  error ("Error sending result status.");
	  close (FileId);
	  return;
	}

      /* Process the file being transferred.  */
      dwBytesRecv = 0;
      /* If file size=0, expect 1 byte 0x00.  */
      if (dwFileSize == 0)
	{
	  if ((dwBytes = RcpReceive (buff, blen)) == -1)
	    {
	      error ("Cannot receive client data.");
	      close (FileId);
	      return;
	    }
	  else if (buff[0] != 0)
	    error ("Received data for zero length file");
	}
      else
	{
	  while (dwBytesRecv < dwFileSize)
	    {
	      /* Receive data from the client.  */
	      int toread = MIN (dwFileSize - dwBytesRecv, blen);
	      if ((dwBytes = read (fileno (stdin), buff, toread)) == -1)
		{
		  error ("Cannot receive client data.");
		  close (FileId);
		  return;
		}

	      totalread += dwBytes;

	      dwBytesRecv += dwBytes;

	      debug ("Got %lu (%lu/%lu).\n", dwBytes, dwBytesRecv, dwFileSize);

	      /* Write the data to the file.  */
	      nValue = write (FileId, buff, dwBytes);

	      if (nValue != dwBytes)
		{
		  /* Error condition:  write failure.  */
		  buff[0] = 1;
		  sprintf (&buff[1], "rcp: %s :Cannot write to file\n",
			   Target2);
		  if (write (fileno (stdout), buff, strlen (&buff[1]) + 1) < 1)
		    error ("Error writing error status.");
		  close (FileId);
		  return;
		}
	    }
	}

      close (FileId);
    }

  return;
}

/* Determines what type of rcp is being
   requested and processes it accordingly.

   Valid rcp requests are in the form:
   rcp -t [-d] [-r] [-p] target
   rcp -f [r] [-p] target

   NOTE:  The -p option is being ignored since there is not a good
   correlation between UNIX and NT when it comes to file
   permissions and ownership.  */
int
rcp_main (int argc, char **argv)
{
  int i;
  char *HomeDir;

  BOOL bTargDir = FALSE;
  BOOL bSvrRecv = FALSE;
  BOOL bSvrSend = FALSE;
  BOOL bRecursive = FALSE;

  /* Get the current working directory.  */
  HomeDir = getcwd (NULL, MAX_PATH);

  i = 1;
  for (; argv[i] && argv[i][0] == '-'; i++)
    {
      switch (argv[i][1])
	{
	case 'd':
	  /* Target must be directory.  */
	  bTargDir = TRUE;
	  break;

	case 't':
	  /* rcp is receiving files.  */
	  bSvrRecv = TRUE;
	  break;

	case 'f':
	  /* rcp is sending files.  */
	  bSvrSend = TRUE;
	  break;

	case 'p':
	  /* Preserve Permissions.  This option is ignored for now.  */
	  break;

	case 'r':
	  /* Recursive send/recv.  */
	  bRecursive = TRUE;
	  break;

	default:
	  fprintf (stderr, "wrong args\n");
	  exit (1);
	}
    }

  if (!bSvrRecv && !bSvrSend)
    {
      error ("Invalid args");
      exit (1);
    }

  debug ("starting up\n");

  if (debugFlag)
    CloseHandle (CreateThread (NULL, 0, LogThread, NULL, 0, NULL));

  if (bSvrRecv)
    {
      if (bRecursive)
	bTargDir = TRUE;

      RcpSvrRecv (argv[i], bRecursive, bTargDir);
    }
  else
    RcpSvrSend (argv[i], bRecursive);

  debug ("closing down\n");

  /* Make sure we end up where we started.  */
  chdir (HomeDir);
  free (HomeDir);

  debug ("closing down (2)\n");

  return 0;
}

int
main (int argc, char **argv)
{
  return rcp_main (argc, argv);
}
