/*
 * UCSD p-System virtual machine
 * Copyright (C) 2000 Mario Klebsch
 * Copyright (C) 2010 Peter Miller
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

#include <assert.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>

#include <lib/progname.h>
#include <lib/version.h>


static struct option options[] =
{
    { "version", 0, 0, 'V' },
    { 0, 0, 0, 0 }
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


typedef union float_pun float_pun;
union float_pun
{
    float           f;
    unsigned short  s[2];
    unsigned char   c[4];
};


int
main(int argc, char **argv)
{
    progname_set(argv[0]);
    for (;;)
    {
        int                     c;

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
        freopen(argv[optind + 1], "w", stdout);
        /* fall through... */

    case 1:
        freopen(argv[optind], "r", stdin);
        /* fall through... */

    case 0:
        break;

    default:
        fprintf(stderr, "Usage: %s [ <infile> [ <outfile> ]]\n", argv[0]);
        fprintf(stderr, "       %s -V\n", argv[0]);
        return 1;
    }

    for (;;)
    {
        float_pun       p;
        int             i;
        char            Buffer[256];

        if (!fgets(Buffer, sizeof(Buffer), stdin))
            break;
        assert(sizeof(float) == 2 * sizeof(unsigned short));
        assert(sizeof(float) == 4);
        sscanf(Buffer, "%f", &p.f);
        for (i = 0; i < 4; i++)
        {
            if (i)
                putchar(' ');
            printf("%02X", p.c[i]);
        }
        printf("\n");
        for (i = 0; i < 2; i++)
        {
            if (i)
                putchar(' ');
            printf("%04X", p.s[i]);
        }
        printf("\n");
    }
    fflush(stdout);
    return 0;
}
