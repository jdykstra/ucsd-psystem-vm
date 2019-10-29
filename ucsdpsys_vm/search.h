/*
 * UCSD p-System virtual machine
 * Copyright (C) 2010 Peter Miller
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or (at
 * you option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program. If not, see <http://www.gnu.org/licenses/>
 */

#ifndef UCSDPSYS_VM_SEARCH_H
#define UCSDPSYS_VM_SEARCH_H

#include <lib/psystem.h>

/**
  * The CspIdSearch function is used to implement the CSP IDSEARCH
  * opcode.
  *
  * @param bufptr
  *     The text buffer to be scanned
  * @param arg2ptr
  *     The record containing the pointer into the buffer, and also to
  *     accept the results.
  */
void CspIdSearch(word bufptr, word arg2ptr);

/**
  * The CspTreeSearch function is used to implement the CSP TREESEARCH
  * opcode.
  *
  * @param token_buf
  *     The value to search for (PACKED ARRAY [0..7] OF CHAR)
  * @param result_ptr
  *     Where to store the resulting node pointer.
  * @param node_ptr
  *     The root node of the tree to be searched.
  */
word CspTreeSearch(word token_buf, word result_ptr, word node_ptr);

#endif /* UCSDPSYS_VM_SEARCH_H */
