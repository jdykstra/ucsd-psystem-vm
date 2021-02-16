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

#include <ctype.h>
#include <getopt.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <lib/diskio.h>
#include <lib/memory.h>
#include <lib/progname.h>
#include <lib/psystem.h>
#include <lib/version.h>


word Syscom;
word IoResult = 0;

static const char *Month[16] =
{
    "???", "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul",
    "Aug", "Sep", "Oct", "Nov", "Dec", "???", "???", "???"
};

static const char *FileTypes[8] =
{
    "Svol", "Bad ", "Code", "Text",
    "Info", "Data", "Graf", "Foto"
};


void
warning(const char *Msg, ...)
{
    va_list         ap;
    char            Buffer[512];

    va_start(ap, Msg);
    vsnprintf(Buffer, sizeof(Buffer), Msg, ap);
    va_end(ap);
    fprintf(stderr, "warning: %s\n", Buffer);
}


void
XeqError(word err)
{
    fprintf(stderr, "XeqError: %d\n", err);
    exit(1);
}


void
IoError(word err)
{
    IoResult = err;
}


static void
DoList(word unit, int ExtList)
{
    int             i;
    int             len;
    word            Entry;
    word            w;
    word            NumBlocks;
    word            NumFiles;
    word            FreeBlocks;
    word            MaxFree;
    word            LastBlock;
    word            Free;

    DiskRead(unit, 0x100, 0, 2048, 2);
    if (IoResult)
    {
        fprintf(stderr, "DoList: Directory read error\n");
        exit(1);
    }

    if (MemRd(WordIndexed(0x100, 8)) & 0xff00)
        byte_sex = byte_sex_other(byte_sex);

    Entry = 0x100;
    NumBlocks = MemRd(WordIndexed(Entry, 7));
    LastBlock = MemRd(WordIndexed(Entry, 1));
    NumFiles = MemRd(WordIndexed(Entry, 8));
    FreeBlocks = 0;
    MaxFree = 0;

    if (ExtList)
    {
        Entry = 0x100;
        for (len = 0; len < MemRdByte(WordIndexed(Entry, 3), 0); len++)
        {
            char ch;

            ch = MemRdByte(WordIndexed(Entry, 3), 1 + len);
            if (isupper(ch))
                ch = tolower(ch);
            putchar(ch);
        }
        putchar(':');
        putchar('\n');
    }

    for (i = 0; i < NumFiles; i++)
    {
        Entry = WordIndexed(0x100, 13 + 13 * i);

        if (ExtList)
        {
            if (MemRd(WordIndexed(Entry, 0)) > LastBlock)
            {
                Free = MemRd(WordIndexed(Entry, 0)) - LastBlock;
                printf("< unused >     %5d           %5d\n", Free, LastBlock);
                FreeBlocks += Free;
                if (Free > MaxFree)
                    MaxFree = Free;
            }
        }
        LastBlock = MemRd(WordIndexed(Entry, 1));

        for (len = 0; len < MemRdByte(WordIndexed(Entry, 3), 0); len++)
        {
            char            ch;

            ch = MemRdByte(WordIndexed(Entry, 3), 1 + len);
            if (isupper(ch))
                ch = tolower(ch);
            putchar(ch);
        }

        if (ExtList)
        {
            for (; len < 15; len++)
                putchar(' ');

            w = MemRd(WordIndexed(Entry, 12));
            printf
            (
                "%5d %2d-%3s-%02d %5d %5d %4sfile",
                MemRd(WordIndexed(Entry, 1)) - MemRd(WordIndexed(Entry, 0)),
                (w >> 4) & 0x1f,
                Month[w & 0x0f], w >> 9,
                MemRd(WordIndexed(Entry, 0)),
                MemRd(WordIndexed(Entry, 11)),
                FileTypes[MemRd(WordIndexed(Entry, 2)) & 0x07]
            );
        }
        putchar('\n');
    }

    if (ExtList)
    {
        if (NumBlocks > LastBlock)
        {
            Free = NumBlocks - LastBlock;
            printf("< unused >     %5d           %5d\n", Free, LastBlock);
            FreeBlocks += Free;
            if (Free > MaxFree)
                MaxFree = Free;
        }
        printf
        (
            "%d files, %d blocks used, %d unused, %d in largest\n",
            NumFiles,
            NumBlocks - FreeBlocks,
            FreeBlocks,
            MaxFree
        );
    }
}


static word
LookupFile(word unit, const char *Name, word *Offset, word *Len, word *Last)
{
    int             i;

    DiskRead(unit, 0x100, 0, 2048, 2);
    if (IoResult)
        return (0);

    if (MemRd(WordIndexed(0x100, 8)) & 0xff00)
        byte_sex = byte_sex_other(byte_sex);

    for (i = 0; i < MemRd(WordIndexed(0x100, 8)); i++)
    {
        word            Entry;
        int             len;

        Entry = WordIndexed(0x100, 13 + 13 * i);
        for (len = 0; len < MemRdByte(WordIndexed(Entry, 3), 0); len++)
        {
            if (toupper(MemRdByte(WordIndexed(Entry, 3), 1 + len)) !=
                toupper(Name[len]))
            {
                goto next;
            }
        }
        if (Name[len])
            continue;

        *Offset = MemRd(WordIndexed(Entry, 0));
        *Len = MemRd(WordIndexed(Entry, 1));
        *Last = MemRd(WordIndexed(Entry, 11));

        return (1);
    next:
        ;
    }
    return (0);
}


static void
DoExtractFile(int unit, char *name, int asText, FILE *f)
{
    word            Offset;
    word            End;
    word            Last;

    if (!LookupFile(unit, name, &Offset, &End, &Last))
    {
        fprintf(stderr, "%s: File not found\n", name);
        exit(1);
    }

    if (asText)
        Offset += 2;

    while (Offset < End)
    {
        int             Size;
        int             i;

        Size = (End - Offset - 1) * 512 + Last;
        if (Size > 1024)
            Size = 1024;
        DiskRead(4, 0x100, 0, Size, Offset);
        for (i = 0; i < Size; i++)
        {
            char            ch;

            ch = MemRdByte(0x100, i);
            if (asText)
            {
                if (ch == 0x10)
                {
                    i++;
                    for (ch = MemRdByte(0x100, i); ch > 0x20; ch--)
                    {
                        fputc(' ', f);
                    }
                }
                else if (ch == 0)
                    break;
                else
                {
                    if (ch == '\r')
                        ch = '\n';
                    fputc(ch, f);
                }
            }
            else
                fputc(ch, f);
        }
        Offset += 2;
    }
}


static void
DoExtractAll(word unit)
{
    int             i;
    int             len;
    word            Entry;
    word            NumBlocks;
    word            NumFiles;
    word            FreeBlocks;
    word            MaxFree;
    word            LastBlock;
    int             type;
    int             asText;
    char            name[17];
    FILE            *f;

    DiskRead(unit, 0x100, 0, 2048, 2);
    if (IoResult)
    {
        fprintf(stderr, "DoExtractAll: Directory read error\n");
        exit(1);
    }

    if (MemRd(WordIndexed(0x100, 8)) & 0xff00)
        byte_sex = byte_sex_other(byte_sex);

    Entry = 0x100;
    NumBlocks = MemRd(WordIndexed(Entry, 7));
    LastBlock = MemRd(WordIndexed(Entry, 1));
    NumFiles = MemRd(WordIndexed(Entry, 8));
    FreeBlocks = 0;
    MaxFree = 0;

    for (i = 0; i < NumFiles; ++i)
    {
        DiskRead(unit, 0x100, 0, 2048, 2);
        if (IoResult)
        {
            fprintf(stderr, "DoExtractAll: Directory read error\n");
            exit(1);
        }

        Entry = WordIndexed(0x100, 13 + 13 * i);

        LastBlock = MemRd(WordIndexed(Entry, 1));

        for (len = 0; len < MemRdByte(WordIndexed(Entry, 3), 0); len++)
        {
            char            ch;

            ch = MemRdByte(WordIndexed(Entry, 3), 1 + len);
            if (isupper(ch))
                ch = tolower(ch);
            name[len] = ch;
        }
        name[len] = '\0';

        type = MemRd(WordIndexed(Entry, 2)) & 0x07;
        asText = (type == 3);

        printf("extracting %s (%s)", name, FileTypes[type]);
        if (asText)
            printf(" as text");
        else
            printf(" as binary");

        f = fopen(name, asText ? "w" : "wb");
        DoExtractFile(unit, name, asText, f);
        fclose(f);

        putchar('\n');
    }
}


static void
usage(void)
{
    const char      *prog;

    prog = progname_get();
    fprintf(stderr, "Usage: %s -l <volume-file>\n", prog);
    fprintf(stderr, "       %s -e <volume-file>\n", prog);
    fprintf(stderr, "       %s -x <volume-file>\n", prog);
    fprintf(stderr, "       %s [ -t ] <volume-file> <filename>\n", prog);
    fprintf(stderr, "       %s -V\n", prog);
    exit(1);
}


static const struct option options[] =
{
    { "extended", 0, 0, 'e' },
    { "list", 0, 0, 'l' },
    { "text", 0, 0, 't' },
    { "extract-all", 0, 0, 'x' },
    { "version", 0, 0, 'V' },
    { 0, 0, 0, 0 }
};


int
main(int argc, char **argv)
{
    char            *name;
    int             List = 0;
    int             ExtList = 0;
    int             Text = 0;
    int             ExtractAll = 0;
    int             unit = 4;

    progname_set(argv[0]);
    Syscom = WordIndexed(0xfffe, -SYSCOM_SIZE);

    for (;;)
    {
        int             c;

        c = getopt_long(argc, argv, "eltVx", options, 0);
        if (c < 0)
            break;
        switch (c)
        {
        case 'e':
            ExtList = 1;
            List = 1;
            break;

        case 'l':
            List = 1;
            break;

        case 't':
            Text = 1;
            break;

        case 'V':
            version_print();
            return 0;

        case 'x':
            ExtractAll = 1;
            break;

        default:
            usage();
        }
    }

    if (optind >= argc)
    {
        fprintf(stderr, "%s: Needs <volume-name> argument\n", progname_get());
        usage();
    }
    DiskMount(unit, argv[optind++], ReadOnly);

    if (List)
    {
        if (optind < argc)
            usage();
        DoList(unit, ExtList);
        exit(0);
    }
    if (ExtractAll)
    {
        if (optind < argc)
            usage();
        DoExtractAll(unit);
    }

    if (optind >= argc)
    {
        fprintf(stderr, "%s: Needs <filename> argument\n", progname_get());
        usage();
    }

    name = argv[optind++];
    DoExtractFile(unit, name, Text, stdout);
    return 0;
}
