/*
 * Copy a dump file from /windows/system/dumpfiles to /temp
 *
 * The reason to write this is that the regular CE File Explorer refuses
 * to copy such files.
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

main(int argc, char *argv[])
{
	int	i, j;
	FILE	*f, *df;
	int	r;
	char	dst[128];
	char	*buf, *p;

	if (argc != 2) {
		fprintf(stderr, "Usage: %s dumpfile\n", argv[0]);
		fprintf(stderr, "Usual place : /windows/system/dumpfiles/ceMMDDYY-NN/ceMMDDYY-NN.kdmp\n");
		exit(1);
	}
	p = strrchr(argv[1], '/');
	if (!p)
		p = strrchr(argv[1], '\\');
	if (!p) {
		fprintf(stderr, "No path ?\n");
		exit(1);
	}
	sprintf(dst, "/temp/%s", p+1);

	f = fopen(argv[1], "rb");
	if (!f) {
		perror("fopen");
		fprintf(stderr, "Could not open %s\n", argv[1]);
		exit(1);
	}
	df = fopen(dst, "wb");
	if (!df) {
		perror("fopen");
		fprintf(stderr, "Could not create %s\n", dst);
		exit(1);
	}

	buf = malloc(8192);
	while (1) {
		r = fread(buf, 1, 8192, f);
		if (r <= 0) {
			if (feof(f))
				break;

			perror("fread");
			fclose(f);
			fclose(df);
			exit(1);
		}
		fwrite(buf, 1, r, df);
	}

	fclose(f);
	fclose(df);
	exit(0);
}

