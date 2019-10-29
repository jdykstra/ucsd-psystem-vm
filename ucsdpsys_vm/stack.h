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

#ifndef UCSDPSYS_VM_STACK_H
#define UCSDPSYS_VM_STACK_H

#include <lib/psystem.h>

#define SP_TOP          0x200

/**
  * The Pop function is used to pop a single unsigned word value from
  * the value stack.  The stack pointer is adjusted.
  *
  * @returns
  *     the word on the top of the stack.
  */
word Pop(void);

/**
  * The PopInteger function is used to pop a single signed word value
  * from the value stack.  The stack pointer is adjusted.
  *
  * @returns
  *     the integer on the top of the stack.
  */
Integer PopInteger(void);

/**
  * The PopReal function is used to pop a single 32-bit floating point
  * value from the value stack.  The stack pointer is adjusted.
  *
  * @returns
  *     the real on the top of the stack.
  */
float PopReal(void);

/**
  * The Push function is used push a single (signed or unsigned) word
  * value onto the top of the value stack.  The stack pointer is
  * adjusted.
  *
  * @param value
  *     the value to be pushed onto the top of the stack.
  */
void Push(word value);

/**
  * The PushReal function is used push a 32-bit floating point value
  * onto the top of the value stack.  The stack pointer is adjusted.
  *
  * @param value
  *     the value to be pushed onto the top of the stack.
  */
void PushReal(float value);

#endif /* UCSDPSYS_VM_STACK_H */
