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

#include <lib/psystem.h>
#include <ucsdpsys_vm/sets.h>


void
SetPop(Set_t *Set)
{
    int             i;

    Set->Size = Pop();
    for (i = 0; i < Set->Size; i++)
        Set->Data[i] = Pop();
}


void
SetAdj(Set_t *Set, word Size)
{
    int             i;

    for (i = Set->Size; i < Size; i++)
        Set->Data[i] = 0;
    Set->Size = Size;
}


void
SetPush(Set_t *Set)
{
    int             i;

    for (i = Set->Size; i > 0; i--)
        Push(Set->Data[i - 1]);
    Push(Set->Size);
}


int
SetNeq(Set_t *Set1, Set_t *Set2)
{
    int             Size;
    int             i;

    Size = (Set1->Size > Set2->Size) ? Set1->Size : Set2->Size;
    if (Set1->Size < Size)
        SetAdj(Set1, Size);
    if (Set2->Size < Size)
        SetAdj(Set2, Size);

    for (i = 0; i < Size; i++)
        if (Set1->Data[i] != Set2->Data[i])
            return 1;
    return 0;
}


int
set_is_improper_subset(const Set_t *haystack, const Set_t *needle)
{
    int             Size;
    int             i;

    Size = needle->Size;
    while (Size > 0 && needle->Data[Size - 1] == 0)
        --Size;
    if (haystack->Size < Size)
        return 0;
    for (i = 0; i < Size; i++)
        if ((haystack->Data[i] & needle->Data[i]) != needle->Data[i])
            return 0;
    return 1;
}


int
set_is_proper_subset(Set_t *haystack, Set_t *needle)
{
    return set_is_improper_subset(haystack, needle) && SetNeq(haystack, needle);
}
