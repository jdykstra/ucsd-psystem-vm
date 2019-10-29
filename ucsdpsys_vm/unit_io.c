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

#include <lib/psystem.h>
#include <lib/memory.h>
#include <lib/diskio.h>

#include <ucsdpsys_vm/printer.h>
#include <ucsdpsys_vm/turtlegr.h>
#include <ucsdpsys_vm/term.h>


void
IoError(word Result)
{
    MemWr(IORSLT, Result);
}


void
UnitRead(word Unit, word Addr, Integer AddrOffset, word Len, word Block,
    word Mode)
{
    (void)Mode;
    IoError(0);
    switch (Unit)
    {
    case 1:
    case 2:
        while (Len--)
        {
            char ch = TermRead();
            if (Unit == 1)
                TermWrite(ch, 0);
            MemWrByte(Addr, AddrOffset++, ch);
        }
        break;

    case 4:
    case 5:
    case 9:
    case 10:
    case 11:
    case 12:
        DiskRead(Unit, Addr, AddrOffset, Len, Block);
        break;

    default:
        IoError(2);
        break;
    }
}


void
UnitWrite(word Unit, word Addr, Integer AddrOffset, Integer Len, word Block,
    word Mode)
{
    IoError(0);
    switch (Unit)
    {
    case 1:
    case 2:
        while (Len > 0)
        {
            --Len;
            TermWrite(MemRdByte(Addr, AddrOffset), Mode);
            ++AddrOffset;
        }
        break;

    case 4:
    case 5:
    case 9:
    case 10:
    case 11:
    case 12:
        DiskWrite(Unit, Addr, AddrOffset, Len, Block);
        break;

    case 6:
        while (Len > 0)
        {
            --Len;
            PrinterWrite(MemRdByte(Addr, AddrOffset), Mode);
            ++AddrOffset;
        }
        break;

    default:
        IoError(2);
        break;
    }
}


void
UnitClear(word Unit)
{
    IoError(0);
    switch (Unit)
    {
    case 1:
    case 2:
        break;

    case 4:
    case 5:
    case 11:
    case 12:
        DiskClear(Unit);
        break;

    case 3:
        PrinterClear();
#ifdef TURTLEGRAPHICS
        GraphicsClear();
#endif
        break;

    case 6:
        PrinterClear();
        break;

    default:
        IoError(9);
        break;
    }
}


void
UnitStat(word Unit, word Addr, Integer Offset, word Dummy)
{
    (void)Offset;
    (void)Dummy;
    IoError(0);
    switch (Unit)
    {
    case 1:
    case 2:
        MemWr(Addr, TermStat());
        break;

    case 4:
    case 5:
    case 11:
    case 12:
        DiskStat(Unit);
        break;

    default:
        IoError(9);
        break;
    }
}


word
UnitBusy(word Unit)
{
    (void)Unit;
    IoError(0);
    return (0);
}


void
UnitWait(word Unit)
{
    (void)Unit;
    IoError(0);
}
