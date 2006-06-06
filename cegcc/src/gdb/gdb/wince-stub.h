/* wince-stub.h -- Definitions for commnicating with the WinCE stub.

   Copyright 1999, 2000 Free Software Foundation, Inc.
   Contributed by Cygnus Solutions, A Red Hat Company.

   This file is part of GDB.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without eve nthe implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
 */

/* by Christopher Faylor (cgf@cygnus.com) */

enum win_func
  {
    GDB_FIRST = 42,

    GDB_CLOSEHANDLE = GDB_FIRST,
    GDB_CONTINUEDEBUGEVENT,
    GDB_CREATEPROCESS,
    GDB_DEBUGACTIVEPROCESS,
    GDB_FLUSHINSTRUCTIONCACHE,
    GDB_GETTHREADCONTEXT,
		GDB_OPENPROCESS,
    GDB_READPROCESSMEMORY,
    GDB_RESUMETHREAD,
    GDB_SETTHREADCONTEXT,
    GDB_SINGLESTEP,
    GDB_STOPSTUB,
    GDB_SUSPENDTHREAD,
    GDB_TERMINATEPROCESS,
    GDB_WAITFORDEBUGEVENT,
    GDB_WAITFORSINGLEOBJECT,
    GDB_WRITEPROCESSMEMORY,

    GDB_INVALID
  };

typedef unsigned char gdb_wince_id;
typedef unsigned long gdb_wince_len;
typedef unsigned long gdb_wince_result;

/* Convenience define for outputting a "gdb_wince_len" type. */

#define putlen putdword
#define getlen getdword
