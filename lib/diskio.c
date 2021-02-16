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
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#ifdef USE_MMAP
#include <sys/mman.h>
#endif

#include <lib/psystem.h>
#include <lib/memory.h>
#include <lib/diskio.h>

static int DskTable[16] =
    { 0, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 15 };

typedef struct unit_t unit_t;
struct unit_t
{
    int             fd;
    byte            *Data;
    size_t          Size;
    int             RdOnly;
    int             *Translate;
};
static unit_t unit_table[MAX_UNIT];


void
DiskRead(word Unit, word addr, Integer AddrOffset, word Len, word BlockNo)
{
    int             i;
    size_t          Offset;
    int             Track;
    int             Sector;
    unit_t          *u;

    Offset = BlockNo * 512;
    Track = BlockNo / 8;
    Sector = (BlockNo & 7) * 2;
    assert(Unit < MAX_UNIT);
    u = &unit_table[Unit];
    if (u->fd < 0 && !u->Data)
    {
        IoError(9);
        return;
    }
    if (Offset + Len > u->Size)
    {
        IoError(64);
        return;
    }

    while (Len)
    {
        int             Size;
        int             Sec;

        Size = 256;
        Sec = Sector;
        if (Len < Size)
            Size = Len;
        if (u->Translate)
            Sec = u->Translate[Sector];
        if (u->Data)
        {
            for (i = 0; i < Size; i++)
            {
                MemWrByte
                (
                    addr,
                    AddrOffset + i,
                    u->Data[(Track * 16 + Sec) * 256 + i]
                );
            }
        }
        else
        {
            char            Buf[256];

            if
            (
                lseek(u->fd, (Track * 16 + Sec) * 256, SEEK_SET) < 0
            ||
                read(u->fd, Buf, Size) != Size
            )
            {
                IoError(64);
                return;
            }
            for (i = 0; i < Size; i++)
                MemWrByte(addr, AddrOffset + i, Buf[i]);
        }
        AddrOffset += Size;
        Len -= Size;
        Sector++;
        if (Sector >= 16)
        {
            Track++;
            Sector = 0;
        }
    }
    IoError(0);
}


void
DiskWrite(word Unit, word addr, Integer AddrOffset, word Len, word BlockNo)
{
    int             i;
    size_t          Offset;
    int             Track;
    int             Sector;
    unit_t          *u;

    Offset = BlockNo * 512;
    Track = BlockNo / 8;
    Sector = (BlockNo & 7) * 2;
    assert(Unit < MAX_UNIT);
    u = &unit_table[Unit];
    if (u->fd < 0 && !u->Data)
    {
        IoError(9);
        return;
    }
    else if (u->RdOnly)
    {
        IoError(16);
        return;
    }
    else if (Offset + Len > u->Size)
    {
        IoError(64);
        return;
    }

    while (Len)
    {
        int             Size;
        int             Sec;

        Size = 256;
        Sec = Sector;
        if (Len < Size)
            Size = Len;
        if (u->Translate)
            Sec = u->Translate[Sector];
        if (u->Data)
        {
            for (i = 0; i < Size; i++)
            {
                u->Data[(Track * 16 + Sec) * 256 + i] =
                    MemRdByte(addr, AddrOffset + i);
            }
        }
        else
        {
            char            Buf[256];

            for (i = 0; i < Size; i++)
                Buf[i] = MemRdByte(addr, AddrOffset + i);
            if
            (
                lseek(u->fd, (Track * 16 + Sec) * 256, SEEK_SET) < 0
            ||
                write(u->fd, Buf, Size) != Size
            )
            {
                IoError(64);
                return;
            }
        }
        AddrOffset += Size;
        Len -= Size;
        Sector++;
        if (Sector >= 16)
        {
            Track++;
            Sector = 0;
        }
    }
    IoError(0);
}


void
DiskClear(word Unit)
{
    assert(Unit < MAX_UNIT);
    IoError(0);
}


void
DiskStat(word Unit)
{
    unit_t          *u;

    assert(Unit < MAX_UNIT);
    u = &unit_table[Unit];
    if (u->fd < 0 && !u->Data)
        IoError(9);
    else
        IoError(0);
}


void
DiskUmount(word Unit)
{
    unit_t          *u;

    assert(Unit < MAX_UNIT);
    u = &unit_table[Unit];
    if (u->Data)
    {
#ifdef USE_MMAP
        munmap(u->Data, u->Size);
#else
        free(u->Data);
#endif
        u->Data = NULL;
    }
    if (u->fd >= 0)
    {
        close(u->fd);
        u->fd = -1;
    }
}


int
DiskMount(word Unit, const char *FileName, enum DiskMode Mode)
{
    int             fd;
    struct stat     buf;
    unit_t          *u;

    assert(Unit < MAX_UNIT);
    u = &unit_table[Unit];
    fd =
        open
        (
            FileName,
            (Mode == ReadWrite) ? O_RDWR : O_RDONLY,
            0666
        );
    if (fstat(fd, &buf) < 0)
    {
        close(fd);
        return (-1);
    }
    DiskUmount(Unit);

    u->fd = fd;
    u->Size = buf.st_size;
    u->RdOnly = (Mode == ReadOnly);
    u->Translate = NULL;

    if
    (
        strlen(FileName) > 4
    &&
        strcmp(FileName + strlen(FileName) - 4, ".dsk") == 0
    )
        u->Translate = DskTable;
#ifdef USE_MMAP
    u->Data =
        mmap
        (
            NULL,
            u->Size,
            (Mode == ReadOnly) ? PROT_READ : PROT_READ | PROT_WRITE,
            (Mode == Forget) ? MAP_PRIVATE : MAP_SHARED,
            u->fd,
            0
        );
    close(Unit[Unit].fd);
    Unit[Unit].fd = -1;
#else
    if (Mode == Forget)
    {
        u->Data = malloc(u->Size);
        assert(u->Data);
        if (read(u->fd, u->Data, u->Size) != (ssize_t) u->Size)
        {
            free(u->Data);
            u->Data = NULL;
            close(u->fd);
            u->fd = -1;
            return (-1);
        }
        close(u->fd);
        u->fd = -1;
    }
#endif
    return (0);
}


void
DiskInit(void)
{
    int             i;

    for (i = 0; i < MAX_UNIT; i++)
    {
        unit_table[i].Data = NULL;
        unit_table[i].fd = -1;
    }
}


byte_sex_t
disk_get_byte_sex(word unit_number)
{
    unit_t          *u;

    //
    // Figure out if the volume is little-endian or big-endian.
    //
    // We do this by looking at the "dlastblock" field, which is 16 bits, and
    // assume the value will be less than 256 (it should be 6 or 10).
    //
    assert(unit_number < MAX_UNIT);
    u = &unit_table[unit_number];
    if (u->Data)
    {
        return (u->Data[0x402] ? little_endian : big_endian);
    }

    // If we don't have mmap,
    // we will have to read the data from disk.
    if (u->fd >= 0)
    {
        char            buf[128];

        if
        (
            lseek(u->fd, 0x400, SEEK_SET) >= 0
        &&
            read(u->fd, buf, sizeof(buf)) == sizeof(buf)
        )
        {
            return (buf[2] ? little_endian : big_endian);
        }
    }

    //
    // What the heck?  Nothing worked, default to little-endian.
    // That is what the Apple's 6502 was, anyway.
    //
    return little_endian;
}
