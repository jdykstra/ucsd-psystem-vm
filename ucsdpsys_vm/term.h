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

#ifndef UCSDPSYS_VM_TERM_H
#define UCSDPSYS_VM_TERM_H

void TermOpen(int UseXTerm, int BatchFd);
void TermClose(void);
void TermWrite(char ch, word Mode);
char TermRead(void);
int TermStat(void);

/**
  * The term_width function may be used to obtain the width of the
  * terminal in character columns.  This assumes an old-time CRT
  * terminal, or an emulation such as xterm or GnomeTerminal.
  */
int term_width(void);

/**
  * The term_height function may be used to obtain the height of the
  * terminal in text lines.  This assumes an old-time CRT terminal, or
  * an emulation such as xterm or GnomeTerminal.
  */
int term_height(void);

/**
  * The term_is_batch_mode function may be used to determine whether or
  * not the terminal is in batch mode (true) or "curses" mode (false).
  */
int term_is_batch_mode(void);

#endif /* UCSDPSYS_VM_TERM_H */
