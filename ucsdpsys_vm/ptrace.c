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
#include <string.h>

#include <lib/memory.h>

#include <ucsdpsys_vm/ptrace.h>

static const char *Instructions[256] =
{
    "sldc 0",
    "sldc 1",
    "sldc 2",
    "sldc 3",
    "sldc 4",
    "sldc 5",
    "sldc 6",
    "sldc 7",
    "sldc 8",
    "sldc 9",
    "sldc 10",
    "sldc 11",
    "sldc 12",
    "sldc 13",
    "sldc 14",
    "sldc 15",
    "sldc 16",
    "sldc 17",
    "sldc 18",
    "sldc 19",
    "sldc 20",
    "sldc 21",
    "sldc 22",
    "sldc 23",
    "sldc 24",
    "sldc 25",
    "sldc 26",
    "sldc 27",
    "sldc 28",
    "sldc 29",
    "sldc 30",
    "sldc 31",
    "sldc 32",
    "sldc 33",
    "sldc 34",
    "sldc 35",
    "sldc 36",
    "sldc 37",
    "sldc 38",
    "sldc 39",
    "sldc 40",
    "sldc 41",
    "sldc 42",
    "sldc 43",
    "sldc 44",
    "sldc 45",
    "sldc 46",
    "sldc 47",
    "sldc 48",
    "sldc 49",
    "sldc 50",
    "sldc 51",
    "sldc 52",
    "sldc 53",
    "sldc 54",
    "sldc 55",
    "sldc 56",
    "sldc 57",
    "sldc 58",
    "sldc 59",
    "sldc 60",
    "sldc 61",
    "sldc 62",
    "sldc 63",
    "sldc 64",
    "sldc 65",
    "sldc 66",
    "sldc 67",
    "sldc 68",
    "sldc 69",
    "sldc 70",
    "sldc 71",
    "sldc 72",
    "sldc 73",
    "sldc 74",
    "sldc 75",
    "sldc 76",
    "sldc 77",
    "sldc 78",
    "sldc 79",
    "sldc 80",
    "sldc 81",
    "sldc 82",
    "sldc 83",
    "sldc 84",
    "sldc 85",
    "sldc 86",
    "sldc 87",
    "sldc 88",
    "sldc 89",
    "sldc 90",
    "sldc 91",
    "sldc 92",
    "sldc 93",
    "sldc 94",
    "sldc 95",
    "sldc 96",
    "sldc 97",
    "sldc 98",
    "sldc 99",
    "sldc 100",
    "sldc 101",
    "sldc 102",
    "sldc 103",
    "sldc 104",
    "sldc 105",
    "sldc 106",
    "sldc 107",
    "sldc 108",
    "sldc 109",
    "sldc 110",
    "sldc 111",
    "sldc 112",
    "sldc 113",
    "sldc 114",
    "sldc 115",
    "sldc 116",
    "sldc 117",
    "sldc 118",
    "sldc 119",
    "sldc 120",
    "sldc 121",
    "sldc 122",
    "sldc 123",
    "sldc 124",
    "sldc 125",
    "sldc 126",
    "sldc 127",
    "abi          (absolute value integer)",
    "abr          (absolute value real)",
    "adi          (add integer)",
    "adr          (add real)",
    "land         (logical and)",
    "dif          (set difference)",
    "dvi          (divide integer)",
    "dvr          (divide real)",
    "chk          (check)",
    "flo          (float tos-1)",
    "flt          (float tos)",
    "inn          (set memberschip)",
    "int          (set intersection)",
    "lor          (logical or)",
    "modi         (modulo integer)",
    "mpi          (multiply integer)",
    "mpr          (multiply real)",
    "ngi          (negate integer)",
    "ngr          (negate real)",
    "lnot         (logical not)",
    "srs          (build subrange set)",
    "sbi          (substract integer)",
    "sbr          (substract real)",
    "sgs          (build singleton set)",
    "sqi          (square integer)",
    "sqr          (square real)",
    "sto          (store indirect word)",
    "ixs          (index string array)",
    "uni          (set union)",
    "lde  U,B     (load extended word)",
    "csp  Q       (call standard procedure)",
    "ldcn         (load constant nil)",
    "adj  U       (adjust set)",
    "fjp  J       (false jump)",
    "inc  B       (increment field pointer)",
    "ind  B       (load indirect word)",
    "ixa  B       (index array)",
    "lao  B       (load global address)",
    "lsa  Y       (load string address)",
    "lae  U,B     (load extended address)",
    "mov  B       (move words)",
    "ldo  B       (load global word)",
    "sas  U P     (string assign)",
    "sro  B       (store global word)",
    "xjp  R       (case jump)",
    "rnp  D       (return non-base procedure)",
    "cip  C       (call intermediate procedure)",
    "equ  T       (equal)",
    "geq  T       (greater or equal)",
    "grt  T       (greater)",
    "lda  D,B     (load intermediate address)",
    "ldc  X       (load multiple word constant)",
    "leq  T       (less or equal)",
    "les  T       (less than)",
    "lod  D,B     (load intermediate word)",
    "neq  T       (not equal)",
    "str  D,B     (store intermediate word)",
    "ujp  J       (unconditional jump)",
    "ldp          (load packed field)",
    "stp          (store into packed field)",
    "ldm  U       (load multiple words)",
    "stm  U       (store multiple words)",
    "ldb          (load byte)",
    "stb          (store byte)",
    "ixp  U,U     (index packed array)",
    "rbp  D       (return base procedure)",
    "cbp  C       (call base procedure)",
    "equi         (equal integer)",
    "geqi         (greater or equal integer)",
    "grti         (greater iteger)",
    "lla  B       (load local address)",
    "ldci W       (load constant integer)",
    "leqi         (less or equal integer)",
    "lesi         (less than integer)",
    "ldl  B       (load local word)",
    "neqi         (not equal integer)",
    "stl  B       (store local word)",
    "cxp  A       (call external procedure)",
    "clp  C       (call local procedure)",
    "cgp  C       (call global procedure)",
    "lpa  Z       (load packed array)",
    "ste  U,B     (store extended word)",
    ".db  210",
    "efj  J       (equal false jump)",
    "nfj  J       (not equal false jump)",
    "bpt  B       (breakpoint)",
    "xit          (exit operating system)",
    "nop          (no operation)",
    "sldl 1       (short load local word)",
    "sldl 2       (short load local word)",
    "sldl 3       (short load local word)",
    "sldl 4       (short load local word)",
    "sldl 5       (short load local word)",
    "sldl 6       (short load local word)",
    "sldl 7       (short load local word)",
    "sldl 8       (short load local word)",
    "sldl 9       (short load local word)",
    "sldl 10      (short load local word)",
    "sldl 11      (short load local word)",
    "sldl 12      (short load local word)",
    "sldl 13      (short load local word)",
    "sldl 14      (short load local word)",
    "sldl 15      (short load local word)",
    "sldl 16      (short load local word)",
    "sldo 1       (short load global word)",
    "sldo 2       (short load global word)",
    "sldo 3       (short load global word)",
    "sldo 4       (short load global word)",
    "sldo 5       (short load global word)",
    "sldo 6       (short load global word)",
    "sldo 7       (short load global word)",
    "sldo 8       (short load global word)",
    "sldo 9       (short load global word)",
    "sldo 10      (short load global word)",
    "sldo 11      (short load global word)",
    "sldo 12      (short load global word)",
    "sldo 13      (short load global word)",
    "sldo 14      (short load global word)",
    "sldo 15      (short load global word)",
    "sldo 16      (short load global word)",
    "sind 0       (short load indirect word)",
    "sind 1       (short load indirect word)",
    "sind 2       (short load indirect word)",
    "sind 3       (short load indirect word)",
    "sind 4       (short load indirect word)",
    "sind 5       (short load indirect word)",
    "sind 6       (short load indirect word)",
    "sind 7       (short load indirect word)"
};


static void
PString(char *Buffer, word Addr)
{
    int Len = MemRdByte(Addr, 0);
    int i;
    *Buffer++ = '\'';
    for (i = 1; i <= Len; i++)
        *Buffer++ = MemRdByte(Addr, i);
    *Buffer++ = '\'';
    *Buffer++ = '\0';
}


static word
ReadStack(word Sp, int TosOffset)
{
    return (MemRdByte(Sp, 2 * TosOffset) +
        (MemRdByte(Sp, 2 * TosOffset + 1) << 8));
}


static void
ProcName(char *d, int Seg, int Proc, word Sp)
{
    if (Seg == 0)
    {
        switch (Proc)
        {
        case 5:
            sprintf(d, "Rewrite");
            if (Sp)
            {
                d += strlen(d);
                *d++ = '(';
                PString(d, ReadStack(Sp, 2));
                d += strlen(d);
                *d++ = ')';
                *d = '\0';
            }
            break;
        case 6:
            sprintf(d, "Close");
            break;
        case 7:
            sprintf(d, "Get");
            break;
        case 8:
            sprintf(d, "Put");
            break;
        case 10:
            sprintf(d, "Eof");
            break;
        case 11:
            sprintf(d, "Eoln");
            break;
        case 12:
            sprintf(d, "ReadInteger");
            break;
        case 13:
            sprintf(d, "WriteInteger");
            break;
        case 16:
            sprintf(d, "ReadChar");
            break;
        case 17:
            sprintf(d, "WriteChar");
            break;
        case 18:
            sprintf(d, "ReadString");
            break;
        case 19:
            sprintf(d, "WriteString");
            if (Sp)
            {
                d += strlen(d);
                *d++ = '(';
                PString(d, ReadStack(Sp, 1));
                d += strlen(d);
                *d++ = ')';
                *d = '\0';
            }
            break;
        case 21:
            sprintf(d, "ReadLn");
            break;
        case 22:
            sprintf(d, "WriteLn");
            break;
        case 23:
            sprintf(d, "Concat");
            if (Sp)
            {
                d += strlen(d);
                *d++ = '(';
                PString(d, ReadStack(Sp, 2));
                d += strlen(d);
                *d++ = ',';
                PString(d, ReadStack(Sp, 1));
                d += strlen(d);
                *d++ = ')';
                *d = '\0';
            }
            break;
        case 24:
            sprintf(d, "Insert");
            if (Sp)
            {
                d += strlen(d);
                *d++ = '(';
                PString(d, ReadStack(Sp, 3));
                d += strlen(d);
                *d++ = ',';
                PString(d, ReadStack(Sp, 2));
                d += strlen(d);
                sprintf(d, ", %d)", ReadStack(Sp, 0));
            }
            break;
        case 25:
            sprintf(d, "Copy");
            if (Sp)
            {
                d += strlen(d);
                *d++ = '(';
                PString(d, ReadStack(Sp, 3));
                d += strlen(d);
                sprintf(d, ", %d, %d)", ReadStack(Sp, 1), ReadStack(Sp, 0));
            }
            break;
        case 26:
            sprintf(d, "Delete");
            if (Sp)
            {
                d += strlen(d);
                *d++ = '(';
                PString(d, ReadStack(Sp, 2));
                d += strlen(d);
                sprintf(d, ", %d, %d)", ReadStack(Sp, 1), ReadStack(Sp, 0));
            }
            break;
        case 27:
            sprintf(d, "Pos");
            if (Sp)
            {
                d += strlen(d);
                *d++ = '(';
                PString(d, ReadStack(Sp, 3));
                d += strlen(d);
                *d++ = ',';
                PString(d, ReadStack(Sp, 2));
                d += strlen(d);
                *d++ = ')';
                *d = '\0';
            }
            break;
        case 28:
            sprintf(d, "BlockRead/BlockWrite");
            break;
        case 29:
            sprintf(d, "GotoXY");
            if (Sp)
            {
                d += strlen(d);
                sprintf(d, "( %d, %d)", ReadStack(Sp, 1), ReadStack(Sp, 0));
            }
            break;
        }
    }
}


/*  ?? Unsafe - No bounds check on provided Buffer.  */
word
DisasmP(char *Buffer, word SegNo, word IpcBase, word Ipc, word JTab, word Sp)
{
    unsigned char OpCode = MemRdByte(IpcBase, Ipc++);
    const char *s = Instructions[OpCode];
    char *d = Buffer;
    char ch;
    int Val;

    while ((ch = *s++))
    {
        switch (ch)
        {
        case 'A':
            /* CXP Arguments */
            {
                int Seg = MemRdByte(IpcBase, Ipc++);
                int Proc = MemRdByte(IpcBase, Ipc++);
                sprintf(d, "%d,%d ", Seg, Proc);
                d += strlen(d);
                ProcName(d, Seg, Proc, Sp);
                d += strlen(d);
            }
            break;

        case 'B':
            Val = MemRdByte(IpcBase, Ipc++);
            if (Val & 0x80)
                Val = ((Val & 0x7f) << 8) + MemRdByte(IpcBase, Ipc++);
            sprintf(d, "%d", Val);
            d += strlen(d);
            break;

        case 'C':
            Val = MemRdByte(IpcBase, Ipc++);
            sprintf(d, "%d ", Val);
            d += strlen(d);
            ProcName(d, SegNo, Val, Sp);
            d += strlen(d);
            break;

        case 'D':
        case 'U':
            Val = MemRdByte(IpcBase, Ipc++);
            sprintf(d, "%d", Val);
            d += strlen(d);
            break;

        case 'P':
            if (Sp)
            {
                PString(d, ReadStack(Sp, 0));
                d += strlen(d);
            }
            break;

        case 'S':
            Val = MemRdByte(IpcBase, Ipc++);
            if (Val & 0x80)
                Val = -(0x100 - Val);
            sprintf(d, "%d", Val);
            d += strlen(d);
            break;

        case 'T':
            Val = MemRdByte(IpcBase, Ipc++);
            switch (Val)
            {
            case 2:
                strcpy(d, "real");
                break;

            case 4:
                strcpy(d, "string");
                break;

            case 6:
                strcpy(d, "boolean");
                break;

            case 8:
                strcpy(d, "set");
                break;

            case 10:
                Val = MemRdByte(IpcBase, Ipc++);
                if (Val & 0x80)
                    Val = ((Val & 0x7f) << 8) + MemRdByte(IpcBase, Ipc++);
                sprintf(d, "byte array, %d bytes", Val);
                break;

            case 12:
                Val = MemRdByte(IpcBase, Ipc++);
                if (Val & 0x80)
                    Val = ((Val & 0x7f) << 8) + MemRdByte(IpcBase, Ipc++);
                sprintf(d, "%d words", Val);
                break;

            default:
                sprintf(d, "%d", Val);
                break;
            }
            d += strlen(d);
            break;

        case 'W':
            Val = MemRdByte(IpcBase, Ipc++);
            Val |= (MemRdByte(IpcBase, Ipc++)) << 8;
            sprintf(d, "%d", Val);
            d += strlen(d);
            break;

        case 'R': /* case arguments */
            {
                int Min;
                int Max;
                int Default;
                Ipc = (Ipc + 1) & (~1);
                Min = MemRdByte(IpcBase, Ipc++);
                Min |= (MemRdByte(IpcBase, Ipc++)) << 8;
                Max = MemRdByte(IpcBase, Ipc++);
                Max |= (MemRdByte(IpcBase, Ipc++)) << 8;
                Ipc++;
                Default = MemRdByte(IpcBase, Ipc++);
                if (Default & 0x80) /* less than zero? */
                {
                    Default = -(0x100 - Default);
                    Default = -Default;
                    Default = (int)MemRdByte(JTab, -2) +
                        (MemRdByte(JTab, -1) << 8) + 2 -
                        ((int)MemRdByte(JTab, -Default) +
                        (MemRdByte(JTab, -Default + 1) << 8) + Default);
                }
                else
                    Default = Ipc + Default;
                sprintf(d, "%d,%d,%d  ", Min, Max, Default);
                d += strlen(d);
                while (Min < Max + 1)
                {
                    Val = MemRdByte(IpcBase, Ipc++);
                    Val |= (MemRdByte(IpcBase, Ipc++)) << 8;
                    sprintf(d, ",%d", Ipc - 2 - Val);
                    d += strlen(d);
                    Min++;
                }
            }
            break;

        case 'Q':
            Val = MemRdByte(IpcBase, Ipc++);
            switch (Val)
            {
            case 1:
                strcpy(d, "new");
                break;

            case 2:
                strcpy(d, "Moveleft");
                break;

            case 3:
                strcpy(d, "Moveright");
                break;

            case 4:
                strcpy(d, "exit");
                break;

            case 5:
                strcpy(d, "unitread");
                break;

            case 6:
                strcpy(d, "unitwrite");
                break;

            case 7:
                strcpy(d, "idsearch");
                break;

            case 8:
                strcpy(d, "treesearch");
                break;

            case 9:
                strcpy(d, "time");
                break;

            case 10:
                strcpy(d, "fillchar");
                break;

            case 11:
                sprintf(d, "scan");
                break;

            case 12:
                strcpy(d, "unitstat");
                break;

            case 21:
                strcpy(d, "load_segment");
                break;

            case 22:
                strcpy(d, "unload_segment");
                break;

            case 32:
                strcpy(d, "mark");
                break;

            case 33:
                strcpy(d, "release");
                break;

            case 34:
                strcpy(d, "ioresult");
                break;

            case 35:
                strcpy(d, "unitbusy");
                break;

            case 37:
                strcpy(d, "unitwait");
                break;

            case 38:
                strcpy(d, "unitclear");
                break;

            case 39:
                strcpy(d, "halt");
                break;

            case 40:
                strcpy(d, "memavail");
                break;

            default:
                sprintf(d, "%d", Val);
                break;
            }
            d += strlen(d);
            break;

        case 'J':
            Val = MemRdByte(IpcBase, Ipc++);
            if (Val & 0x80) /* less than zero? */
            {
                Val = -(0x100 - Val);
                Val = -Val;

                Val = (int)MemRdByte(JTab, -2) +
                    (MemRdByte(JTab, -1) << 8) + 2 -
                    ((int)MemRdByte(JTab, -Val) +
                    (MemRdByte(JTab, -Val + 1) << 8) + Val);
            }
            else
            {
                Val = Ipc + Val;
            }
            sprintf(d, "%d", Val);
            d += strlen(d);
            break;

        case 'V':
            *d = '\0';
            d += strlen(d);
            break;

        case 'X':
            Val = MemRdByte(IpcBase, Ipc++);
            sprintf(d, "%d", Val);
            d += strlen(d);
            Ipc = (Ipc + 1) & ~1;
            while (Val--)
            {
                word w;
                w = MemRdByte(IpcBase, Ipc) + (MemRdByte(IpcBase,
                        Ipc + 1) << 8);
                Ipc += 2;
                sprintf(d, ",%04x", w);
                d += strlen(d);
            }
            break;

        case 'Y':
            Val = MemRdByte(IpcBase, Ipc++);
            sprintf(d, "%d,'", Val);
            d += strlen(d);
            while (Val--)
                *d++ = MemRdByte(IpcBase, Ipc++);
            *d++ = '\'';
            break;

        case 'Z':
            Val = MemRdByte(IpcBase, Ipc++);
            sprintf(d, "%d", Val);
            d += strlen(d);
            Ipc = (Ipc + 1) & ~1;
            while (Val--)
            {
                sprintf(d, ",%02x", MemRdByte(IpcBase, Ipc));
                Ipc++;
                d += strlen(d);
            }
            break;

        default:
            *d++ = ch;
        }
    }
    *d++ = '\0';
    return (Ipc);
}


void
PrintStack(char *Buffer, word Sp)
{
    char *p = Buffer;
    *p = '\0';
    while (Sp < 0x200)
    {
        sprintf(p, " %04x", MemRd(Sp));
        p += strlen(p);
        Sp += 2;
    }
}


void
PrintStaticChain(char *Buffer, word Mp)
{
    char *p = Buffer;
    *p = '\0';
    for (;;)
    {
        word NextMp;
        sprintf(p, " %04x", Mp);
        p += strlen(p);
        NextMp = MemRd(Mp);
        if (!NextMp || (Mp == NextMp))
            break;
        Mp = NextMp;
    }
}
