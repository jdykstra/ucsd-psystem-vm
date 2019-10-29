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

#ifndef UCSDPSYS_VM_ARRAY_H
#define UCSDPSYS_VM_ARRAY_H

/**
  * The StrCmp function is used to compare two Pascal strings.
  *
  * @param s1
  *     The left hand side of the comparison (memory word address)
  * @param s2
  *     The right hand side of the comparison (memory word address).
  * @returns
  *     minus one (-1) if s1 < s2;
  *     zero (0) if s1 == s2; or,
  *     plus one (1) if s1 > s2.
  */
int StrCmp(word s1, word s2);

/**
  * The ByteCmp function is used to compare byte arrays.
  *
  * @param ba1
  *     The left hand side of the comparison (memory word address)
  * @param ba2
  *     The right hand side of the comparison (memory word address).
  * @param len
  *     The number of bytes to be compared
  * @returns
  *     minus one (-1) if ba1 < ba2;
  *     zero (0) if ba1 == ba2; or,
  *     plus one (1) if ba1 > ba2.
  */
int ByteCmp(word ba1, word ba2, word len);

/**
  * The WordCmp function i sused to compare word arrays.
  *
  * @param wa1
  *     The left hand side of the comparison (memory word address)
  * @param wa2
  *     The right hand side of the comparison (memory word address).
  * @param len
  *     The number of words to be compared
  * @returns
  *     zero (0) if wa1 == wa2; or, one (1) if wa1 != wa2
  */
int WordCmp(word wa1, word wa2, word len);

#endif /* UCSDPSYS_VM_ARRAY_H */
