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

#ifndef LIB_GCC_ATTRIBUTES_H
#define LIB_GCC_ATTRIBUTES_H

#ifdef __GNUC__
#define FORMAT_PRINTF(x, y) __attribute__((__format__(printf, x, y)))
#define NORETURN __attribute__((__noreturn__))
#define DEPRECATED __attribute__((__deprecated__))
#else
#define FORMAT_PRINTF(x, y)
#define DEPRECATED
#define NORETURN
#endif

#endif /* LIB_GCC_ATTRIBUTES_H */
