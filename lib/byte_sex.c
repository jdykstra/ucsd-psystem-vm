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

#include <stdio.h>

#include <lib/byte_sex.h>


const char *
byte_sex_name(byte_sex_t value)
{
    static char     buf[20];

    switch (value)
    {
    case little_endian:
        return "little-endian";

    case big_endian:
        return "big-endian";
    }
    snprintf(buf, sizeof(buf), "%d", value);
    return buf;
}


unsigned
byte_sex_get_word_le(const unsigned char *data)
{
    return (data[0] | (data[1] << 8));
}


unsigned
byte_sex_get_word_be(const unsigned char *data)
{
    return ((data[0] << 8) | data[1]);
}


unsigned
byte_sex_get_word(byte_sex_t bs, const unsigned char *data)
{
    if (bs == little_endian)
        return byte_sex_get_word_le(data);
    else
        return byte_sex_get_word_be(data);
}


void
byte_sex_put_word_le(unsigned char *data, unsigned value)
{
    data[0] = value;
    data[1] = value >> 8;
}


void
byte_sex_put_word_be(unsigned char *data, unsigned value)
{
    data[0] = value >> 8;
    data[1] = value;
}


void
byte_sex_put_word(byte_sex_t bs, unsigned char *data, unsigned value)
{
    if (bs == little_endian)
        byte_sex_put_word_le(data, value);
    else
        byte_sex_put_word_be(data, value);
}


byte_sex_t
byte_sex_other(byte_sex_t bs)
{
    return (byte_sex_t)((int)little_endian + (int)big_endian - (int)bs);
}
