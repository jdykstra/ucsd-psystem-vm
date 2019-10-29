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

#ifndef UCSDPSYS_VM_PTRACE_H
#define UCSDPSYS_VM_PTRACE_H

#include <lib/psystem.h>

/**
  * The DisasmP function is used to disassemble a p-code opcode from
  * memory, into the given buffer.
  *
  * @param buffer
  *     The buffer to print the opcode into.
  * @param segnum
  *     The segment number the code is within
  * @param ipc_base
  *     The procedure base address (enter_ic).
  * @param ipc
  *     The instruction counter (address within segment) of the
  *     instruction to disassemble.
  * @param jtab
  *     The jump table (procedure attributes) of the procedure being
  *     executed, or disassembled.
  * @param sp
  *     Stack pointer (?)
  */
word DisasmP(char *buffer, word segnum, word ipc_base, word ipc, word jtab,
    word sp);

/**
  * The PrintStack function is used to print a representation of the
  * value stack into the buffer provided.
  *
  * @param buffer
  *     The buffer to print into.
  * @param sp
  *     The top-of-stack pointer.
  */
void PrintStack(char *buffer, word sp);

/**
  * The PrintStaticChain function is used to print a representation of
  * the proceedure call chain into the buffer provided.
  *
  * @param buffer
  *     The buffer to print into.
  * @param mp
  *     The value of the MP register, the top of the call stack.
  */
void PrintStaticChain(char *buffer, word mp);

#endif /* UCSDPSYS_VM_PTRACE_H */
