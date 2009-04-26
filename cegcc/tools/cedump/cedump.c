#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

#include "DwCeDump.h"

typedef void (*StreamHandler)(FILE *, int, int);

void HandleSystemInfo(FILE *, int, int);

struct {
	enum _MINIDUMP_STREAM_TYPE	stream_type;
	char				*name;
	StreamHandler			handler;
} StreamTypes[] = {
	{ UnusedStream, "Unused Stream", NULL },
	{ ceStreamNull,	"Stream Null",	NULL },
	{ ceStreamSystemInfo,	"System info",	HandleSystemInfo },
	{ ceStreamException,	"Exception",	NULL },
	{ ceStreamModuleList,	"Module",	NULL },
	{ ceStreamProcessList,	"Process",	NULL },
	{ ceStreamThreadList,	"Thread",	NULL },
	{ ceStreamThreadContextList,	"Thread Context", NULL },
	{ ceStreamThreadCallStackList,	"Thread Callstack", NULL },
	{ ceStreamMemoryVirtualList,	"Memory virtual list", NULL },
	{ ceStreamMemoryPhysicalList,	"Memory physical list", NULL },
	{ ceStreamBucketParameters,	"Bucket parameters", NULL },
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
		fprintf(stderr, "Usual place : /windows/system/dumpfiles/ceMMDDYY-NN/ceMMDDYY-NN.kdmp\n");
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

