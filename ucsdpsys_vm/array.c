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

#include <ucsdpsys_vm/array.h>


int
StrCmp(word s1, word s2)
{
    byte Len1 = MemRdByte(s1, 0);
    byte Len2 = MemRdByte(s2, 0);
    byte Len = Len1 < Len2 ? Len1 : Len2; /* Length to compare */
    byte i;

    for (i = 1; i <= Len; i++)
    {
        byte Ch1 = MemRdByte(s1, i); /* Get a char from both strings */
        byte Ch2 = MemRdByte(s2, i);
        if (Ch1 < Ch2)
            return (-1); /* S1 < S2 */
        else if (Ch1 > Ch2)
            return (1); /* S1 > S2 */
    }
    /* All chars in range of common length are equal. */
    if (Len1 < Len2)
    {
        /* S1 ist shorter?  If so S1 < S2 */
        return (-1);
    }
    else if (Len1 > Len2)
    {
        /* S2 is shorter? if so S1 > S2 */
        return (1);
    }
    else
    {
        /* both strings have the same length, so they are equal. */
        return (0);
    }
}


int
ByteCmp(word ba1, word ba2, word Len)
{
    word i;

    for (i = 0; i < Len; i++)
    {
        byte Ch1 = MemRdByte(ba1, i); /* Get a char from both strings */
        byte Ch2 = MemRdByte(ba2, i);
        if (Ch1 < Ch2)
            return (-1); /* BA1 < BA2 */
        else if (Ch1 > Ch2)
            return (1); /* BA1 > BA2 */
    }
    return (0);
}


int
WordCmp(word wa1, word wa2, word Len)
{
    word i;
    for (i = 0; i < Len; i++)
    {
        if (MemRd(WordIndexed(wa1, i)) != MemRd(WordIndexed(wa2, i)))
            return (1);
    }
    return (0);
}
