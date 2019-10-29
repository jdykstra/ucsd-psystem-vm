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

#ifndef UCSDPSYS_VM_NATIVE6502_H
#define UCSDPSYS_VM_NATIVE6502_H

/**
  * The ProcessNative function is used to call a (simulation of) a
  * native assembler code function or procedure.
  *
  * @param jtab
  *     The address of the jump table (procedure attributes) of the
  *     native code procedure or function to be called.
  */
void ProcessNative(word jtab);

#endif /* UCSDPSYS_VM_NATIVE6502_H */
