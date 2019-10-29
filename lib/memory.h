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

#ifndef LIB_MEMORY_H
#define LIB_MEMORY_H

#include <lib/byte_sex.h>
#include <lib/psystem.h>

#define WORD_MEMORY

extern byte_sex_t byte_sex;

/**
  * The MemRdByte function is used to read a byte of memory, from the
  * simulated memory.
  *
  * @param addr
  *     The base word address.
  * @param offset
  *     The byte offset to be added to the word address.
  * @returns
  *     the value 0..255 of the byte in the simulated memory.
  */
byte MemRdByte(word addr, Integer offset);

/**
  * The MemWrByte function is used to write a byte of memory, to the
  * similated memory.  An execution error will occur if a read-only byte
  * is attempted to be written.
  *
  * @param addr
  *     The base word address.
  * @param offset
  *     The byte offset to be added to the word address.
  * @param value
  *     the desired value 0..255 of the byte in the simulated memory.
  */
void MemWrByte(word addr, Integer offset, byte value);

/**
  * The MemRd function is used to read the value of a word of simulated
  * memory.
  *
  * @param addr
  *     The base word address.
  * @returns
  *     The value of a word of memory (unsigned).
  */
word MemRd(word addr);

/**
  * The MemWr function is used to write to the value of a word of
  * similated memory.  An execution error will occur if a read-only word
  * is attempted to be written.
  *
  * @param addr
  *     The base word address.
  * @param value
  *     The desired value of a word of memory.
  */
void MemWr(word addr, word value);

/**
  * The MemReadOnly function is used to set the given range of words of
  * memory to be read-only, or not read-only.
  *
  * @param from
  *     The lower bound word address (inclusive).
  * @param to
  *     The upper bound word address (inclusive).
  * @param ro
  *     zero (0) to clear any read-only indication, or
  *     non-zero (1) to set the words to read-only.
  */
void MemReadOnly(word from, word to, int ro);

/**
  * The MemDump function is used to print a representation of memory to
  * the given file.
  *
  * @param f
  *     The file to write the memory dump to.
  * @param start
  *     The lower bound word address (inclusive).
  * @param end
  *     The upper bound word address (inclusive).
  */
void MemDump(FILE *f, word start, word end);

/**
  * The MemInit function is used to initialize the simulated memory.
  * All values are zero, all word addresses are writable.
  */
void MemInit(void);

#define NIL             0

#define PointerCheck(Pointer) if ((Pointer) == NIL) XeqError(7)

#ifdef WORD_MEMORY
#define WordIndexed(Pointer, Offset) ((Pointer) + (Offset))
#else
#define WordIndexed(Pointer, Offset) ((Pointer) + 2*(Offset))
#endif

/**
  * The memory_emulate_read_func type is used to point to memory
  * emulation read functions.
  * (See WORD_MEMORY for whether we are in word mode or byte mode).
  *
  * @param addr
  *     The address being read
  * @returns
  *     The synthesized value of the word.
  */
typedef word (*memory_emulate_read_func)(word addr);

/**
  * The memory_emulate_write_func type is used to point to memory
  * emulation write functions.
  * (See WORD_MEMORY for whether we are in word mode or byte mode).
  *
  * @param addr
  *     The address being written
  * @param value
  *     The value being written.
  */
typedef void (*memory_emulate_write_func)(word addr, word value);

/**
  * The memory_emulate_set function is used to request memory emulation
  * at the specified range of addresses.
  *
  * If overlapping memory ranges are specified, the results are undefined.
  *
  * @param from
  *     The lower address bound, inclusive.
  * @param to
  *     The upper address bound, inclusive.
  * @param read_func
  *     The function to be called when at attempt is made to read from
  *     an address in the given range.  May not be NULL.
  * @param write_func
  *     The function to be called when at attempt is made to write to
  *     an address in the given range.  If NULL, value will be discarded.
  */
void memory_emulate_set(word from, word to, memory_emulate_read_func read_func,
    memory_emulate_write_func write_func);

/**
  * The memory_emulate_setw function is used to request memory emulation
  * at the specified word.  It takes care of figuring out the ramgs
  * differently on word and bye memory styles.
  *
  * It is implemented via the #memory_emulate_set function.
  *
  * @param addr
  *     The address of the word of interest.
  * @param read_func
  *     The function to be called when at attempt is made to read from
  *     an address in the given range.  May not be NULL.
  * @param write_func
  *     The function to be called when at attempt is made to write to
  *     an address in the given range.  If NULL, value will be discarded.
  */
void memory_emulate_setw(word addr, memory_emulate_read_func read_func,
    memory_emulate_write_func write_func);

#endif /* LIB_MEMORY_H */
