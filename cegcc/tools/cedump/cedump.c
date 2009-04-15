#include <stdio.h>
#include <stdlib.h>

#include "DwCeDump.h"

main(int argc, char *argv[])
{
	int	i;
	FILE	*f = fopen(argv[1], "rb");
	MINIDUMP_HEADER	buf;
	int	r;
	char	s[128];

	if (!f) {
		fprintf(stderr, "Could not open %s", argv[1]);
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

	fclose(f);
}
