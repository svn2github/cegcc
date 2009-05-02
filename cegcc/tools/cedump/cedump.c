/*
 * cedump
 *
 * Portable application that inspects a CE dump file and reports about
 * its contents. This is type type of information that CE offers to send
 * to Microsoft free of charge to you.
 *
 * LICENSE:
 *
 * Copyright (c) 2009, Danny Backx.
 *
 * This file is part of CeGCC.
 *
 * This is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This file is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this file; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

#include "DwCeDump.h"

/*
 * Use this list to make the HandleElementList less verbose
 */
typedef struct {
	char	*arg;
} Arguments;
typedef void (*StreamHandler)(FILE *, int, int, Arguments *);

void HandleSystemInfo(FILE *, int, int, Arguments *);
void HandleException(FILE *, int, int, Arguments *);
void HandleThreadCallStack(FILE *, int, int, Arguments *);
void HandleMemoryList(FILE *, int, int, Arguments *);
void HandleBucketParameters(FILE *, int, int, Arguments *);

void HandleElementList(FILE *, int, int, Arguments *);

/*
 * The list of fields to report about for Modules
 *
 *   Name : NK.EXE
 *   Base Ptr : 0x80001000
 *   Size : 442368
 *   RW Data Start : 0x802D6000
 *   RW Data End : 0x80368FB3
 *   Timestamp : 0x349ED8B6
 *   PDB Format : RSDS
 *   PDB Guid : %U
 *   PDB Age : 1
 *   hDll : 0x00000000
 *   In Use : 0x00000001
 *   Flags :
 *   Trust Level : 2
 *   RefCount :
 *   Pointer : 0x%08kX
 *   FileVersionMS : 0x00050002
 *   FileVersionLS : 0x00000000
 *   ProductVersionMS : 0x00050002
 *   ProductVersionLS : 0x00000000
 *   PDB Name : kernkitl.pdb
 */
Arguments ArgML[] = {
	"Name",
	"Base Ptr",
	NULL
};

/*
 * The list of fields to report about for Processes
 * Pick your labels from this list :
 *   ProcSlot# : 15
 *   Name : wroadmap.exe
 *   VMBase : 0x20000000
 *   AccessKey : 0x00008000
 *   TrustLevel : 
 *   hProcess : 0xA0D26B72
 *   BasePtr : 0x00010000
 *   TlsUseL32b : 0x000000FF
 *   TlsUseH32b : 0x00000000
 *   CurZoneMask : 0x00000000
 *   pProcess : 0x8034E070
 *   CmdLine : 
 */
Arguments ArgPL[] = {
	"ProcSlot#",
	"Name",
	"VMBase",
	"pProcess",
	"BasePtr",
	NULL
};

/*
 * The list of fields to report about for Threads
 * Here's an example that you can pick labels from.
 *  pThread : 0x851FA7C0
 *  RunState : Awak,RunBlkd
 *  InfoStatus : UMode,UsrBlkd
 *  hThread : 0xE5E91CDA
 *  WaitState : 
 *  AccessKey : 0x00008011
 *  hCurProcIn : 0xA718C392
 *  hOwnerProc : 0xA0D26B72
 *  CurPrio : 2698144763
 *  BasePrio : 2698144763
 *  KernelTime : 0
 *  UserTime : 381
 *  Quantum : 100
 *  QuantuLeft : 100
 *  SleepCount : 29489432
 *  SuspendCount : 29489408
 *  TlsPtr : 0x2064FF00
 *  LastError : 0x00000000
 *  StackBase : 0x20550000
 *  StkLowBnd : 0x2064F000
 *  CreatTimeH : 0x01C9BDA3
 *  CreatTimeL : 0xC3DB0700
 *  PC : 0x80016F08
 *  NcrPtr : 0x00000000
 *  StkRetAddr : 0x03F63AB0
 *
 */
Arguments ArgTL[] = {
	"pThread",
	"RunState",
	"hThread",
	"LastError",
	"PC",
	"StkRetAddr",
	NULL
};

/*
 * The list of fields to report about for Thread Contexts
 *
 *   ThreadID : 0xa17a19a2
 *   Arm Integer Context : 
 *
 */
Arguments ArgTCL[] = {
	"ThreadID",
	NULL
};

struct {
	enum _MINIDUMP_STREAM_TYPE	stream_type;
	char				*name;
	StreamHandler			handler;
	Arguments			*arguments;
} StreamTypes[] = {
	{ UnusedStream,			"Unused Stream",	NULL,			NULL },
	{ ceStreamNull,			"Stream Null",		NULL,			NULL },
	{ ceStreamSystemInfo,		"System info",		HandleSystemInfo,	NULL },
	{ ceStreamException,		"Exception",		HandleException,	NULL },
	{ ceStreamModuleList,		"Module",		HandleElementList,	ArgML },
	{ ceStreamProcessList,		"Process",		HandleElementList,	ArgPL },
	{ ceStreamThreadList,		"Thread",		HandleElementList,	ArgTL },
	{ ceStreamThreadContextList,	"Thread Context",	HandleElementList,	ArgTCL },
	{ ceStreamThreadCallStackList,	"Thread Callstack",	HandleThreadCallStack,	NULL },
	{ ceStreamMemoryVirtualList,	"Memory virtual list",	HandleMemoryList,	NULL },
	{ ceStreamMemoryPhysicalList,	"Memory physical list",	HandleMemoryList,	NULL },
	{ ceStreamBucketParameters,	"Bucket parameters",	HandleBucketParameters,	NULL },
	/* The end */
	{ LastReservedStream, NULL, NULL }
};

main(int argc, char *argv[])
{
	int	i, j;
	FILE	*f = fopen(argv[1], "rb");
	MINIDUMP_HEADER	buf;
	int	r;
	char	s[128];
	MINIDUMP_DIRECTORY	*d;

	if (argc != 2) {
		fprintf(stderr, "Usage: %s dumpfile\n", argv[0]);
		fprintf(stderr,
			"Usual place : "
			"/windows/system/dumpfiles/ceMMDDYY-NN/ceMMDDYY-NN.kdmp\n");
		exit(1);
	}
	if (!f) {
		fprintf(stderr, "Could not open %s\n", argv[1]);
		perror("fopen");
		exit(1);
	}

	r = fread(&buf, sizeof(MINIDUMP_HEADER), 1, f);
	if (r <= 0) {
		perror("fread");
		fclose(f);
		exit(0);
	}
//		wcstombs(s, buf.pwzQueryString, 128);
	fprintf(stderr, "Number of streams %d\n", buf.NumberOfStreams);

	/* Allocate directory buffers */
	d = (MINIDUMP_DIRECTORY *) calloc(buf.NumberOfStreams,
			sizeof (MINIDUMP_DIRECTORY));

	/* Spool to directory */
	for (i=0; i<buf.NumberOfStreams; i++) {
		fseek(f, buf.StreamDirectoryRva
			+ i * sizeof(MINIDUMP_DIRECTORY), SEEK_SET);
		r = fread(&d[i], sizeof(MINIDUMP_DIRECTORY), 1, f);
		if (r != 1) {
			fprintf(stderr, "Invalid dir size, %d\n", r);
			exit (1);
		}
		for (j=0; StreamTypes[j].stream_type != LastReservedStream; j++)
			if (StreamTypes[j].stream_type == d[i].StreamType)
				break;
		if (StreamTypes[j].stream_type != d[i].StreamType) {
			fprintf(stderr, "Invalid stream type %4x\n",
					d[i].StreamType);
			continue;
		}
		fprintf(stderr, "stream %d, type %4x (%s), size %ld\n",
				i,
				d[i].StreamType,
				StreamTypes[j].name,
				d[i].Location.DataSize);
		if (StreamTypes[j].handler) {
			(StreamTypes[j].handler)(f,
					d[i].Location.Rva,
					d[i].Location.DataSize,
					StreamTypes[j].arguments);
		}
	}

	fclose(f);
}

/*
 * Caller must free result.
 * Also convert to char *
 */
char *ReadString(FILE *f, int off)
{
	char	*oem, *woem;
	ULONG32	sl;
	int	i;

	fseek(f, off, SEEK_SET);
	fread(&sl, sizeof(ULONG32), 1, f);
	oem = malloc(sl+1);
	woem = malloc(2 * sl + 2);
	fread(woem, 2, sl, f);
	for (i=0; i<sl; i++)
		oem[i] = woem[2*i];
	oem[sl] = 0;
	free(woem);
	return oem;
}

void HandleSystemInfo(FILE *f, int off, int len, Arguments *arg)
{
	int			l;
	CEDUMP_SYSTEM_INFO	si;
	char			*oem;

	fseek(f, off, SEEK_SET);
	fread(&si, sizeof(si), 1, f);
	oem = ReadString(f, si.OEMStringRva);
	fprintf(stderr, "System info :\n\tarch %d, %d processors\n"
			"\tProc type %d level %d rev %d\n"
			"\tProc family %d Platform %d\n"
			"\tLocale id %d %4X\n"
			"\tOEM {%s}\n",
			si.ProcessorArchitecture,
			si.NumberOfProcessors,
			si.ProcessorType,
			si.ProcessorLevel,
			si.ProcessorRevision,
			si.ProcessorFamily,
			si.PlatformId,
			si.LCID, si.LCID,
			oem);
	free(oem);
}

void PrintBitfield(char *buf, char *format)
{
	char		*p, *q;
	char		txt[64];
	int		bit;
	unsigned int	fld;
	int		set;
	int		first = 1;

	fld = *(unsigned int *)buf;

//	fprintf(stderr, "BitField(%u, %s) ", fld, format);

	for (p = format+3; p && *p; p = q ? q+1 : 0) {
		q = strchr(p, ',');
		if (!q)
			q = strchr(p, '}');

		sscanf(p, "%d=%[^,}]", &bit, &txt);

		if (bit < 32) {
			set = fld & (1 << bit);
			if (set) {
				if (first)
					fprintf(stderr, "%s", txt);
				else
					fprintf(stderr, ",%s", txt);
				first = 0;
			}
		} else {
			set = fld & (1 << (bit-32));
			if (! set) {
				if (first)
					fprintf(stderr, "%s", txt);
				else
					fprintf(stderr, ",%s", txt);
				first = 0;
			}
		}
	}
}

void PrintEnumeration(char *buf, char *format)
{
	char		*p, *q;
	char		txt[64];
	int		bit;
	unsigned int	fld;
	int		set;

	fld = *(unsigned int *)buf;

	for (p = format+2; p && *p; p = q ? q+1 : 0) {
		q = strchr(p, ',');
		sscanf(p, "%d=%[^,}]", &bit, &txt);

		if (bit == fld) {
			fprintf(stderr, "%s", txt);
//			fprintf(stderr, "Enumeration (%s)", txt);
			return;
		}
	}
}

void HandleElementList(FILE *f, int off, int len, Arguments *arg)
{
	CEDUMP_ELEMENT_LIST	el;
	CEDUMP_FIELD_INFO	*field;
	char			*label, *format;
	int			i, j, sz, k, pos;
	int			bufsize = 0;
	char			*buf = NULL;

	fseek(f, off, SEEK_SET);
	fread(&el, sizeof(el), 1, f);
//	field = calloc(el.NumberOfFieldInfo, sizeof(CEDUMP_FIELD_INFO));
	field = malloc(len);
	fread(field, sizeof(CEDUMP_FIELD_INFO), el.NumberOfFieldInfo, f);
#if 0
	fprintf(stderr, "Element list : header size %d, field info sz %d\n"
			"\t# field info %d, #elements %d\n"
			"\tElements at 0x%X..0x%X\n",
			el.SizeOfHeader,
			el.SizeOfFieldInfo,
			el.NumberOfFieldInfo,
			el.NumberOfElements,
			el.Elements, el.Elements + len);
#endif
	/* Count sizes */
	for (sz=i=0; i<el.NumberOfFieldInfo; i++) {
		sz += field[i].FieldSize;
	}
#if 0
	fprintf(stderr, "Field sizes add up to %d, so elements from 0x%X .. 0x%X\n",
			sz,
			el.Elements,
			el.Elements + sz * el.NumberOfElements);
#endif
	/* Start looking from el.Elements */
	pos = el.Elements;
	for (j=0; j < el.NumberOfElements; j++) {
		fprintf(stderr, "  Element %d :\n", j);
		for (i=0; i<el.NumberOfFieldInfo; i++) {
			int	thispos;

			/* Figure out whether we need to collect this */
			label = ReadString(f, field[i].FieldLabel);

			thispos = pos;
			pos += field[i].FieldSize;
#if 1
			/* Make this program less verbose */
			if (arg) {
				int req = 0;
				for (k=0; arg[k].arg && !req; k++)
					if (strcmp(arg[k].arg, label) == 0)
						req++;
				if (req == 0)
					continue;
			}
#endif

			fseek(f, thispos, SEEK_SET);
			if (bufsize < field[i].FieldSize) {
				if (buf)
					free(buf);
				bufsize = field[i].FieldSize;
				buf = malloc(bufsize);
			}
			memset(buf, 0, bufsize);
			fread(buf, sizeof(char), field[i].FieldSize, f);

			format = ReadString(f, field[i].FieldFormat);
#if 0
			if (strcmp(label, "ProcSlot#") == 0) {
				int i;
				for (i=0; i<4; i++)
					fprintf(stderr, "%d ", 255 & buf[i]);
				fprintf(stderr, "\n");
			}
#endif
#if 0
			fprintf(stderr, "  Field %d : %s (id %d, off %X, fmt %s, len %d) ",
					i, label,
					field[i].FieldId, pos,
					format, field[i].FieldSize);
#endif
#if 0
			fprintf(stderr, "  Field %d : %s ", i, label);
#else
			fprintf(stderr, "\t%s : ", label);
#endif

			if (strncmp(format, "%N", 2) == 0) {
				PrintEnumeration(buf, format);
			} else if (strncmp(format, "%T", 2) == 0) {
				PrintBitfield(buf, format);
			} else if (strcmp(format, "0x%08lX") == 0) {
				fprintf(stderr, "0x%08X", *(int *)buf);
			} else if (strcmp(format, "%s") == 0) {
				fprintf(stderr, format, buf);
			} else
				fprintf(stderr, format, *(int *)buf);
			fprintf(stderr, "\n");

			free(label);
			free(format);
		}
	}

}

void HandleException(FILE *f, int off, int len, Arguments *arg)
{
	CEDUMP_EXCEPTION_STREAM	exs;
	CEDUMP_EXCEPTION	ex;
	ULONG32			*param;
	int			i;

	fseek(f, off, SEEK_SET);
	fread(&exs, sizeof(exs), 1, f);
	fread(&ex, sizeof(ex), 1, f);

	fprintf(stderr, "Exception : code %X, flags %x, address %X\n",
			ex.ExceptionCode,
			ex.ExceptionFlags,
			ex.ExceptionAddress);
	param = calloc(ex.NumberParameters, sizeof(ULONG32));
	fread(param, sizeof(ULONG32), ex.NumberParameters, f);
	for (i=0; i<ex.NumberParameters; i++)
		fprintf(stderr, "Parameter %d : [%X]\n",
				i, param[i]);
}

void HandleThreadCallStack(FILE *f, int off, int len, Arguments *arg)
{
	CEDUMP_THREAD_CALL_STACK_LIST	tcsl;
	CEDUMP_THREAD_CALL_STACK	*tcs;
	CEDUMP_THREAD_CALL_STACK_FRAME	frame;
	char			*label, *format;
	int			i, j, sz, k, pos;
	int			bufsize = 0;
	char			*buf = NULL;

	fseek(f, off, SEEK_SET);
	fread(&tcsl, sizeof(tcsl), 1, f);

	tcs = calloc(tcsl.NumberOfEntries, sizeof(CEDUMP_THREAD_CALL_STACK));
	fread(tcs, sizeof(CEDUMP_THREAD_CALL_STACK), tcsl.NumberOfEntries, f);

	fprintf(stderr, "Thread Call Stack List : %d entries\n", tcsl.NumberOfEntries);
	for (i=0; i<tcsl.NumberOfEntries; i++) {
		fprintf(stderr, "  Process %08X thread %08X, %d frames\n",
				tcs[i].ProcessId,
				tcs[i].ThreadId,
				tcs[i].NumberOfFrames);

		for (j=0; j<tcs[i].NumberOfFrames; j++) {
			fseek(f, tcs[i].StackFrames, SEEK_SET);
			fread(&frame, sizeof(frame), 1, f);

			fprintf(stderr, "    Frame %d : RetAddr 0x%08X FP 0x%08X\n",
					j,
					frame.ReturnAddr,
					frame.FramePtr);
		}
	}

	free(tcs);
#if 0
	frame = calloc(tcs.NumberOfFrames, sizeof(CEDUMP_THREAD_CALL_STACK_FRAME));
	fread(frame, sizeof(CEDUMP_THREAD_CALL_STACK_FRAME), tcs.NumberOfFrames, f);

	for (i=0; i<tcs.NumberOfFrames; i++) {
		fprintf(stderr, "  Frame %d : ", i);
	}
#endif
}

void HandleMemoryList(FILE *f, int off, int len, Arguments *arg)
{
	CEDUMP_MEMORY_LIST		ml;
	MINIDUMP_MEMORY_DESCRIPTOR	*md;
	int				i, j;

	fseek(f, off, SEEK_SET);
	fread(&ml, sizeof(CEDUMP_MEMORY_LIST), 1, f);

	fprintf(stderr, "  Memory list %d entries\n", ml.NumberOfEntries);

	md = calloc(ml.NumberOfEntries, sizeof(MINIDUMP_MEMORY_DESCRIPTOR));
	fread(md, sizeof(MINIDUMP_MEMORY_DESCRIPTOR), ml.NumberOfEntries, f);

	for (i=0; i<ml.NumberOfEntries; i++) {
		unsigned long	somr = md[i].StartOfMemoryRange;
		unsigned long	len = md[i].Memory.DataSize;
		unsigned long	rva = md[i].Memory.Rva;
		char		*buf;

		/*
		 * Using variables here to avoid printf getting screwed up
		 * with alignment of the long long field.
		 */
		fprintf(stderr, "    Entry %d start 0x%08X len %x Rva %04X\n",
				i,
				somr, len, rva);

		buf = malloc(len);
		fseek(f, rva, SEEK_SET);
		fread(buf, len, 1, f);

		int start = somr & 0xFFFFFFF0;
		int the_end = (somr + len - 1) | 0x0F;
		int p;
		for (p=start; p<=the_end; p++) {
			if (p < somr && ((p & 0x0F) == 0))
				fprintf(stderr, "%08X    ", p);
			else if (p < somr)
				fprintf(stderr, "   ");
			else if (p >= somr + len && ((p & 0x0F) == 0x0F))
				fprintf(stderr, "\n");
			else if (p >= somr + len)
				fprintf(stderr, "   ");
			else if ((p & 0x0F) == 0)
				fprintf(stderr, "%08X  %02X", p,
						255 & buf[p-somr]);
			else if ((p & 0x0F) == 0x0F)
				fprintf(stderr, " %02X\n", 255 & buf[p-somr]);
			else
				fprintf(stderr, " %02X", 255 & buf[p-somr]);
		}
		free(buf);
	}
}

void HandleBucketParameters(FILE *f, int off, int len, Arguments *arg)
{
	CEDUMP_BUCKET_PARAMETERS	bp;
	char				*app, *owner, *mod;

	fseek(f, off, SEEK_SET);
	fread(&bp, sizeof(CEDUMP_BUCKET_PARAMETERS), 1, f);

	app = ReadString(f, bp.AppName);
	owner = ReadString(f, bp.OwnerName);
	mod = ReadString(f, bp.ModName);

	fprintf(stderr, "  Bucket parameters :\n"
			"\tAppName [%s]\n\tOwnerName [%s]\n\tModule [%s]\n",
			app, owner, mod);
}
