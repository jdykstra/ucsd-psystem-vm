/*
 * UCSD p-System virtual machine
 * Copyright (C) 2000 Mario Klebsch
 * Copyright (C) 2006, 2010 Peter Miller
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program. If not, see <http://www.gnu.org/licenses/>
 */

#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <lib/progname.h>
#include <lib/version.h>

#define SEC_SIZE        256
#define SEC_PER_TRACK   16
static const int table[SEC_PER_TRACK] =
{
    0, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 15
};


static const struct option options[] =
{
    { "version", 0, 0, 'V' },
    { 0, 0, 0, 0}
};


static void
usage(void)
{
    const char      *prog;

    prog = progname_get();
    fprintf(stderr, "Usage: %s [ <infile> [ <outfile> ]]\n", prog);
    fprintf(stderr, "       %s --version\n", prog);
    exit(1);
}


int
main(int argc, char **argv)
{
    progname_set(argv[0]);
    for (;;)
    {
        int             c;

        c = getopt_long(argc, argv, "V", options, 0);
        if (c < 0)
            break;
        switch (c)
        {
        case 'V':
            version_print();
            return 0;

        default:
            usage();
        }
    }
    switch (argc - optind)
    {
    case 2:
        freopen(argv[optind + 1], "wb", stdout);
        /* Fall through... */

    case 1:
        freopen(argv[optind], "rb", stdin);
        /* Fall through... */

    case 0:
        break;

    default:
        usage();
    }

    for (;;)
    {
        int             i;
        unsigned char   Buffer[SEC_SIZE * SEC_PER_TRACK];

        if (fread(Buffer, sizeof(Buffer), 1, stdin) <= 0)
            break;
        for (i = 0; i < SEC_PER_TRACK; i++)
        {
            fwrite
            (
                Buffer + table[i] * SEC_SIZE,
                SEC_SIZE,
                1,
                stdout
            );
        }
    }
    fflush(stdout);
    return 0;
}
