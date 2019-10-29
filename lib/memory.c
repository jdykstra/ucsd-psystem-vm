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

#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>

#include <lib/psystem.h>
#include <lib/pcode.h>
#include <lib/memory.h>

#ifdef XXX
#define MEM_INVALID     1
#define MEM_RD_ONLY     2
#define MEM_WARN        4
#endif
#define MEM_EMULATE     8

byte_sex_t byte_sex = little_endian;

typedef struct memory_t memory_t;
struct memory_t
{
#ifdef WORD_MEMORY
    word value;
#else
    byte value;
#endif
    byte flags;
};

static
#ifdef __SASC
__far
#endif
    memory_t Mem[0x10000];


typedef struct emulate_memory_t emulate_memory_t;
struct emulate_memory_t
{
    word from;
    word to;
    memory_emulate_read_func read_func;
    memory_emulate_write_func write_func;
};

static size_t emulator_length;
static size_t emulator_maximum;
static emulate_memory_t *emulator_list;


void
memory_emulate_setw(word addr, memory_emulate_read_func read_func,
    memory_emulate_write_func write_func)
{
#ifdef WORD_MEMORY
    memory_emulate_set(addr, addr, read_func, write_func);
#else
    memory_emulate_set(addr, addr + 1, read_func, write_func);
#endif
}


void
memory_emulate_set(word from, word to, memory_emulate_read_func read_func,
    memory_emulate_write_func write_func)
{
#ifdef MEM_EMULATE
    emulate_memory_t *p;
    memory_t        *mp;
    memory_t        *mp_end;

    /*
     * Let the memory know it is being emulated.
     */
    mp_end = Mem + to;
    for (mp = Mem + from; mp <= mp_end; ++mp)
    {
#ifdef MEM_INVALID
        mp->flags &= ~MEM_INVALID;
#endif
        mp->flags |= MEM_EMULATE;
    }

    /*
     * Add the emulation functions to the list of known emulations.
     */
    if (emulator_length >= emulator_maximum)
    {
        size_t          new_max;
        emulate_memory_t *new_list;

        new_max = emulator_maximum * 2 + 4;
        new_list = malloc(new_max * sizeof(emulate_memory_t));
        memcpy
        (
            new_list,
            emulator_list,
            emulator_length * sizeof(emulate_memory_t)
        );
        if (emulator_list)
            free(emulator_list);
        emulator_list = new_list;
        emulator_maximum = new_max;
    }
    p = emulator_list + emulator_length++;
    p->from = from;
    p->to = to;
    p->read_func = read_func;
    p->write_func = write_func;
#else
    (void)from;
    (void)to;
    (void)read_func;
    (void)write_func;
#endif
}


static word
mem_emulate_read(word addr)
{
    size_t          j;

    // FIXME: binary chop, see below
    for (j = 0; j < emulator_length; ++j)
    {
        emulate_memory_t *p = emulator_list + j;
        if (p->from <= addr && addr <= p->to)
            return p->read_func(addr);
    }
    return 0;
}


static void
mem_emulate_write(word addr, word value)
{
    size_t          j;

    // FIXME: binary chop
    // at the moment we only have one emulator, so this isn't too slow.
    // when we have > 3, switch to binary chop
    // when we have > 16, switch to linear dynamic hash (addr as key)
    for (j = 0; j < emulator_length; ++j)
    {
        emulate_memory_t *p = emulator_list + j;
        if (p->from <= addr && addr <= p->to)
        {
            if (p->write_func)
                p->write_func(addr, value);
            return;
        }
    }
}


#ifdef WORD_MEMORY


word
MemRd(word Addr)
{
    PointerCheck(Addr);
    memory_t *p = Mem + Addr;
    if (p->flags)
    {
        /*
         * We are trying to avoid N tests for N bits, if none of the
         * bits are set.  By using the above test to bracket this whole
         * section, if there is nothing special about this piece of
         * memory, less performance is lost.
         */
#ifdef MEM_INVALID
        if (p->flags & MEM_INVALID)
            warning("MemRdByte: Reading uninitialized memory 0x%04x", Addr);
#endif
#ifdef MEM_WARN
        if (p->flags & MEM_WARN)
            warning("MemRdByte: Access to 0x%04x", Addr);
#endif
#ifdef MEM_EMULATE
        if (p->flags & MEM_EMULATE)
            return mem_emulate_read(Addr);
#endif
    }
    return (p->value);
}


void
MemWr(word Addr, word value)
{
    PointerCheck(Addr);
    memory_t *p = Mem + Addr;
    if (p->flags)
    {
        /*
         * We are trying to avoid N tests for N bits, if none of the
         * bits are set.  By using the above test to bracket this whole
         * section, if there is nothing special about this piece of
         * memory, less performance is lost.
         */
#ifdef MEM_RD_ONLY
        if (p->flags & MEM_RD_ONLY)
        {
            warning("MemWrByte: 0x%04x is read only", Addr);
            XeqError(XBADMEM);
        }
#endif
#ifdef MEM_WARN
        if (p->flags & MEM_WARN)
            warning("MemWrByte: Access to 0x%04x", Addr);
#endif
#ifdef MEM_INVALID
        p->flags &= ~MEM_INVALID;
#endif
#ifdef MEM_EMULATE
        if (p->flags & MEM_EMULATE)
            mem_emulate_write(Addr, value);
#endif
    }
    p->value = value;
}


byte
MemRdByte(word Addr, Integer Offset)
{
    PointerCheck(Addr);
    Addr += (Offset & ~1) / 2;
    Offset &= 1;
    //
    // +--------+----------+--------+
    // | offset | byte sex | branch |
    // +--------+----------+--------+
    // |   0    |  little  |  else  |
    // |   1    |  little  |  then  |
    // |   0    |   big    |  then  |
    // |   1    |   big    |  else  |
    // +--------+----------+--------+
    //
    if (!Offset == (byte_sex == big_endian))
        return (MemRd(Addr) >> 8);
    else
        return (MemRd(Addr) & 0xff);
}


void
MemWrByte(word Addr, Integer Offset, byte value)
{
    word w;
    PointerCheck(Addr);
    Addr += (Offset & ~1) / 2;
    Offset &= 1;
    w = MemRd(Addr);
    if (!Offset == (byte_sex == big_endian))
        w = (w & 0x00ff) | (value << 8);
    else
        w = (w & 0xff00) | (value & 0xff);
    MemWr(Addr, w);
}


#else /* !WORD_MEMORY */


byte
MemRdByte(word Addr, Integer Offset)
{
    memory_t        *p;

    PointerCheck(Addr);
    Addr += Offset;
    p = Mem + Addr;
    if (p->flags)
    {
        /*
         * We are trying to avoid N tests for N bits, if none of the
         * bits are set.  By using the above test to bracket this whole
         * section, if there is nothing special about this piece of
         * memory, less performance is lost.
         */
#ifdef MEM_INVALID
        if (p->flags & MEM_INVALID)
            warning("MemRdByte: Reading uninitialized memory 0x%04x", Addr);
#endif
#ifdef MEM_WARN
        if (p->flags & MEM_WARN)
            warning("MemRdByte: Access to 0x%04x", Addr);
#endif
#ifdef MEM_EMULATE
        if (p->flags & MEM_EMULATE)
        {
            return mem_emulate_read(Addr);
        }
#endif
    }
    return (p->value);
}


void
MemWrByte(word Addr, Integer Offset, byte value)
{
    memory_t        *p;

    PointerCheck(Addr);
    Addr += Offset;
    p = Mem + Addr;
    if (p->flags)
    {
        /*
         * We are trying to avoid N tests for N bits, if none of the
         * bits are set.  By using the above test to bracket this whole
         * section, if there is nothing special about this piece of
         * memory, less performance is lost.
         */
#ifdef MEM_RD_ONLY
        if (p->flags & MEM_RD_ONLY)
        {
            warning("MemWrByte: 0x%04x is read only", Addr);
            XeqError(XBADMEM);
        }
#endif
#ifdef MEM_WARN
        if (p->flags & MEM_WARN)
            warning("MemWrByte: Access to 0x%04x", Addr);
#endif
#ifdef MEM_EMULATE
        if (p->flags & MEM_EMULATE)
        {
            return mem_emulate_write(Addr, value);
        }
#endif
#ifdef MEM_INVALID
        p->flags &= ~MEM_INVALID;
#endif
    }
    Mem[Addr].value = value;
}


word
MemRd(word Addr)
{
    PointerCheck(Addr);
    if (byte_sex == little_endian)
    {
        return (MemRdByte(Addr, 0) + (MemRdByte(Addr, 1) << 8));
    }
    else
    {
        return (MemRdByte(Addr, 1) + (MemRdByte(Addr, 0) << 8));
    }
}


void
MemWr(word Addr, word value)
{
    PointerCheck(Addr);
    if (byte_sex == little_endian)
    {
        MemWrByte(Addr, 0, value & 0xff);
        MemWrByte(Addr, 1, value >> 8);
    }
    else
    {
        MemWrByte(Addr, 1, value & 0xff);
        MemWrByte(Addr, 0, value >> 8);
    }
}


#endif


void
MemReadOnly(word from, word to, int RO)
{
#ifdef MEM_RD_ONLY
    while (from <= to)
    {
        if (RO)
            Mem[from].flags |= MEM_RD_ONLY;
        else
            Mem[from].flags &= ~MEM_RD_ONLY;
        from++;
    }
#else
    (void)from;
    (void)to;
    (void)RO;
#endif
}


void
MemDump(FILE *f, word Start, word End)
{
    int             w;
    int             i;
    int             Count;
#ifdef WORD_MEMORY
    word            value;
    char            b[6];
    byte            ch1;
#else
    byte            value;
    char            b[4];
#endif
    byte            ch;
    char            Buffer[80];
    char            OldBuffer[80];

    Count = 0;
    OldBuffer[0] = '\0';
    for (w = Start; w <= End;)
    {
#ifdef WORD_MEMORY
        w = w & 0xfff8;
#else
        w = w & 0xfff0;
#endif
        sprintf(Buffer, "%04x: %48s %16s\n", w, "", "");
#ifdef WORD_MEMORY
        for (i = 0; i < 8; i++)
#else
        for (i = 0; i < 16; i++)
#endif
        {
#ifdef MEM_INVALID
            if (!(Mem[w].flags & MEM_INVALID))
            {
#endif /* MEM_INVALID */
                value = Mem[w].value;
#ifdef WORD_MEMORY
                sprintf(b, "%04x", value);
#else
                sprintf(b, "%02x", value);
#endif
                ch = value & 0xff;
                if (!isprint(ch))
                    ch = '.';
#ifdef WORD_MEMORY
                ch1 = value >> 8;
                if (!isprint(ch1))
                    ch1 = '.';
#endif
#ifdef MEM_INVALID
            }
            else
            {
#ifdef WORD_MEMORY
                strcpy(b, "....");
                ch1 = ' ';
#else
                strcpy(b, "..");
#endif
                ch = ' ';
            }
#endif /* MEM_INVALID */
#ifdef WORD_MEMORY
            Buffer[6 + 5 * i] = b[0];
            Buffer[6 + 5 * i + 1] = b[1];
            Buffer[6 + 5 * i + 2] = b[2];
            Buffer[6 + 5 * i + 3] = b[3];
            Buffer[6 + 5 * 8 + 2 * i] = ch;
            Buffer[6 + 5 * 8 + 2 * i + 1] = ch1;
#else
            Buffer[6 + 3 * i] = b[0];
            Buffer[6 + 3 * i + 1] = b[1];
            Buffer[6 + 3 * 16 + i] = ch;
#endif
            w++;
        }
        if (strcmp(Buffer + 4, OldBuffer + 4) != 0)
        {
            if (Count > 1)
                fprintf(f, ".... %d line%s omitted\n", Count - 1,
                    Count == 2 ? "" : "s");
            if (Count > 0)
                fputs(OldBuffer, f);
            Count = 0;
            fputs(Buffer, f);
        }
        else
            Count++;
        strcpy(OldBuffer, Buffer);
    }
}


void
MemInit(void)
{
    memory_t        *p;

    for (p = Mem; p < ENDOF(Mem); ++p)
    {
        p->value = 0;
#ifdef MEM_INVALID
        p->flags = MEM_INVALID;
#else
        p->flags = 0;
#endif
    }
}
