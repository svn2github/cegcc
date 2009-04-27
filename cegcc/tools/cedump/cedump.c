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

typedef void (*StreamHandler)(FILE *, int, int);

void HandleSystemInfo(FILE *, int, int);
void HandleElementList(FILE *, int, int);
void HandleException(FILE *, int, int);

struct {
	enum _MINIDUMP_STREAM_TYPE	stream_type;
	char				*name;
	StreamHandler			handler;
} StreamTypes[] = {
	{ UnusedStream,			"Unused Stream",	NULL },
	{ ceStreamNull,			"Stream Null",		NULL },
	{ ceStreamSystemInfo,		"System info",		HandleSystemInfo },
	{ ceStreamException,		"Exception",		HandleException },
	{ ceStreamModuleList,		"Module",		HandleElementList },
	{ ceStreamProcessList,		"Process",		HandleElementList },
	{ ceStreamThreadList,		"Thread",		HandleElementList },
	{ ceStreamThreadContextList,	"Thread Context",	HandleElementList },
	{ ceStreamThreadCallStackList,	"Thread Callstack",	NULL },
	{ ceStreamMemoryVirtualList,	"Memory virtual list",	NULL },
	{ ceStreamMemoryPhysicalList,	"Memory physical list",	NULL },
	{ ceStreamBucketParameters,	"Bucket parameters",	NULL },
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
					d[i].Location.DataSize);
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

void HandleSystemInfo(FILE *f, int off, int len)
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

void HandleElementList(FILE *f, int off, int len)
{
	CEDUMP_ELEMENT_LIST	el;
	CEDUMP_FIELD_INFO	*field;
	char			*label, *format;
	int			i, j, sz;
	int			pos;
	char			buf[256];

	fseek(f, off, SEEK_SET);
	fread(&el, sizeof(el), 1, f);
//	field = calloc(el.NumberOfFieldInfo, sizeof(CEDUMP_FIELD_INFO));
	field = malloc(len);
	fread(field, sizeof(CEDUMP_FIELD_INFO), el.NumberOfFieldInfo, f);

	fprintf(stderr, "Element list : header size %d, field info sz %d\n"
			"\t# field info %d, #elements %d\n"
			"\tElements at 0x%X..0x%X\n",
			el.SizeOfHeader,
			el.SizeOfFieldInfo,
			el.NumberOfFieldInfo,
			el.NumberOfElements,
			el.Elements, el.Elements + len);

	/* Count sizes */
	for (sz=i=0; i<el.NumberOfFieldInfo; i++) {
		sz += field[i].FieldSize;
	}
	fprintf(stderr, "Field sizes add up to %d, so elements from 0x%X .. 0x%X\n",
			sz,
			el.Elements,
			el.Elements + sz * el.NumberOfElements);

	/* Start looking from el.Elements */
	pos = el.Elements;
	for (j=0; j < el.NumberOfElements; j++) {
		for (i=0; i<el.NumberOfFieldInfo; i++) {
			fseek(f, pos, SEEK_SET);
			fread(buf, sizeof(char), field[i].FieldSize, f);

			label = ReadString(f, field[i].FieldLabel);
			format = ReadString(f, field[i].FieldFormat);

#if 1
			fprintf(stderr, "  Field %d : %s ", i, label);
#else
			fprintf(stderr, "  Field %d : %s (id %d, off %X, fmt %s, len %d) ",
					i, label,
					field[i].FieldId, pos,
					format, field[i].FieldSize);
#endif
			pos += field[i].FieldSize;

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

#if 0
	fseek(f, pos, SEEK_SET);
	fread(buf, 1, pos - el.Elements, f);

	for (i=0; i<pos-el.Elements; i++)
		if (i == 0)
			fprintf(stderr, "\t%02X", 255 & buf[i]);
		else if ((i % 16) == 0)
			fprintf(stderr, "\n\t%02X", 255 & buf[i]);
		else
			fprintf(stderr, " %02X", 255 & buf[i]);
	if ((pos - el.Elements) % 16 != 0)
		fprintf(stderr, "\n");

	for (i=0; i<pos-el.Elements; i++)
		if (i == 0)
			fprintf(stderr, "\t  %c",
					isprint(buf[i]) ? 255 & buf[i] : '.');
		else if ((i % 16) == 0)
			fprintf(stderr, "\n\t  %c",
					isprint(buf[i]) ? 255 & buf[i] : '.');
		else
			fprintf(stderr, "   %c",
					isprint(buf[i]) ? 255 & buf[i] : '.');
	if ((pos - el.Elements) % 16 != 0)
		fprintf(stderr, "\n");
#endif
}

void HandleException(FILE *f, int off, int len)
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
