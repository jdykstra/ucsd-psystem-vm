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

#ifndef UCSDPSYS_VM_LONG_INTEGER_H
#define UCSDPSYS_VM_LONG_INTEGER_H

#include <lib/psystem.h>

/**
  * The LongInt function is used to perform long integer arithmetic.
  *
  * @param selector
  *     The function selector.
  *     DECOPS_ADJUST = 0,
  *     DECOPS_ADD = 2,
  *     DECOPS_SUBTRACT = 4,
  *     DECOPS_NEGATE = 6,
  *     DECOPS_MULTIPLY = 8,
  *     DECOPS_DIVIDE = 10,
  *     DECOPS_STR = 12,
  *     DECOPS_CONVERT_TOS_M1 = 14,
  *     DECOPS_COMPARE = 16,
  *     DECOPS_CONVERT_TOS = 18,
  *     DECOPS_TRUNC = 20
  */
void LongInt(word selector);

#endif /* UCSDPSYS_VM_LONG_INTEGER_H */
