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

#ifndef UCSDPSYS_VM_SETS_H
#define UCSDPSYS_VM_SETS_H

#define SET_SIZE        256

typedef struct set
{
  word          Size;
  word          Data[SET_SIZE];
} Set_t;


/**
  * The SetPop function is used to pop a set from the value stack.
  * The top-of-stack is the number of words in the set (always <=
  * 255).  Subsequent values are the words of the set from leats bit to
  * greatest bit.
  *
  * @param Set
  *     Where to put the result.
  */
void SetPop(Set_t *Set);

/**
  * The AdjSet function is used to adjust the size of a set.
  * This is often needed for set comparisons.
  *
  * @param Set
  *     The set to be adjusted.
  * @param Size
  *     The desired size of the set.
  */
void SetAdj(Set_t *Set, word Size);

/**
  * The SetPush function is used to push a set onto the value stack.
  * When completed, the top-of-stack is the number of words in the set
  * (always <= 255).  Subsequent values are the words of the set from
  * leats bit to greatest bit.
  *
  * @param Set
  *     The set to push onto the stack.
  */
void SetPush(Set_t *Set);

/**
  * The SetNeq function is used to compare sets for inequality.
  * (The sets may be adjusted to be the same length as each other.)
  *
  * @param Set1
  *     The first of the two sets to be compared.
  * @param Set2
  *     The second of the two sets to be compared.
  * @returns
  *     "bool": 1 if the sets are NOT equal, true if they are equal.
  */
int SetNeq(Set_t *Set1, Set_t *Set2);

/**
  * The set_is_proper_subset function is used to compare sets for
  * improper (<=) subset or superset.
  *
  * @param haystack
  *     The alleged super set.
  * @param needle
  *     The alleged sub set.
  * @returns
  *     "bool": 1 if all of needle was found to be in haystack, 0 otherwise.
  */
int set_is_improper_subset(const Set_t *haystack, const Set_t *needle);

/**
  * The set_is_proper_subset function is used to compare sets for proper
  * (<) subset or superset.
  *
  * @param haystack
  *     The alleged super set.
  * @param needle
  *     The alleged sub set.
  * @returns
  *     "bool":  1 if all of needle was found to be in haystack but
  *     needle <> haystack, 0 otherwise.
  */
int set_is_proper_subset(Set_t *haystack, Set_t *needle);

#endif /* UCSDPSYS_VM_SETS_H */
