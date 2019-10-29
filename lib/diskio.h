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

#ifndef LIB_DISKIO_H
#define LIB_DISKIO_H

#include <lib/byte_sex.h>
#include <lib/psystem.h>

#define MAX_UNIT 20

enum DiskMode
{
    ReadOnly,
    Forget,
    ReadWrite
};

void DiskRead(word Unit, word Addr, Integer AddrOffset, word Len, word BlockNo);
void DiskWrite(word Unit, word Addr, Integer AddrOffset, word Len,
    word BlockNo);
void DiskClear(word Unit);
void DiskStat(word Unit);

int DiskMount(word Unit, const char *FileName, enum DiskMode Mode);
void DiskUmount(word Unit);
void DiskInit(void);

/**
  * The disk_get_byte_sex function is used to obtain the byte sex of the
  * disk image mounted on the given unit.
  *
  * @param unit_number
  *     The number of the disk unit to be examined.
  */
byte_sex_t disk_get_byte_sex(word unit_number);

#endif /* LIB_DISKIO_H */
