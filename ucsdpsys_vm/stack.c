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
#include <assert.h>

#include <lib/psystem.h>
#include <lib/memory.h>

#include <ucsdpsys_vm/stack.h>


extern word Sp;


word
Pop(void)
{
    word ret = MemRd(Sp);
    Sp = WordIndexed(Sp, 1);
    if (Sp > SP_TOP)
        panic("Stack underflow");
    return (ret);
}


Integer
PopInteger(void)
{
    word w = Pop();

    if (w & 0x8000)
        return (w - 0x10000);
    else
        return (w);
}


typedef union float_pun float_pun;
union float_pun
{
    float f;
    word w[2];
};


float
PopReal(void)
{
    float_pun p;
    assert(sizeof(float) == 2 * sizeof(word));
    p.w[0] = Pop();
    p.w[1] = Pop();
    return p.f;
}


void
Push(word Value)
{
    Sp = WordIndexed(Sp, -1);
    MemWr(Sp, Value);
}


void
PushReal(float Value)
{
    float_pun p;
    assert(sizeof(float) == 2 * sizeof(word));
    p.f = Value;
    Push(p.w[1]);
    Push(p.w[0]);
}
