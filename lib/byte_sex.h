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

#ifndef LIB_BYTE_SEX_H
#define LIB_BYTE_SEX_H

/**
  * The byte_sex_t type is used to remember the byte ordering in 16-bit
  * words.  The name is historical, it is the term used in the UCSD
  * system sources.  (It also pre-dates "political correctness".)
  *
  * For the origins of the little-endian and big-endian names, see
  * http://en.wikipedia.org/wiki/Gulliver%27s_Travels
  * http://en.wikipedia.org/wiki/Endianness
  */
enum byte_sex_t
{
    /**
      * The litte-endian byte sex indicates that the least significant 8 bits
      * are found in the first (lower address) byte, and the most significant
      * 8 bits are found in the second (higher address) byte.  For historical
      * reasons this is the default (the first 3 host architectures were
      * little-endian).
      */
    little_endian,

    /**
      * The big-endian byte sex indicates that the most significant 8 bits are
      * found in the first (lower address) byte, and the least significant 8
      * bits are found in the second (higher address) byte.
      */
    big_endian
};

typedef enum byte_sex_t byte_sex_t;

/**
  * The byte_sex_name function may be used to obtain a human-readable
  * string describing a byte sex.
  *
  * @param value
  *     The byte sex to be translated.
  * @returns
  *     A string containg the description, or a number if an invalid
  *     value is given.
  */
const char *byte_sex_name(byte_sex_t value);

/**
  * The byte_sex_get_word function is used to decode a 16-bit word value
  * from the given data.
  *
  * @param bs
  *     The byte sex to be used to decode the data.
  * @param data
  *     The base address of two bytes to be decoded into a 16-bit value.
  * @returns
  *     The reconstructed 16-bit word value, range 0..65535.
  */
unsigned byte_sex_get_word(byte_sex_t bs, const unsigned char *data);

/**
  * The byte_sex_get_word_le function is used to decode a 16-bit word value
  * from the given data, using little-endian ordering.
  *
  * @param data
  *     The base address of two bytes to be decoded into a 16-bit value.
  * @returns
  *     The reconstructed 16-bit word value, range 0..65535.
  */
unsigned byte_sex_get_word_le(const unsigned char *data);

/**
  * The byte_sex_get_word_be function is used to decode a 16-bit word value
  * from the given data, using big-endian ordering.
  *
  * @param data
  *     The base address of two bytes to be decoded into a 16-bit value.
  * @returns
  *     The reconstructed 16-bit word value, range 0..65535.
  */
unsigned byte_sex_get_word_be(const unsigned char *data);

/**
  * The byte_sex_put_word function is used to encode a 16-bit word value
  * into two bytes of data.
  *
  * @param bs
  *     The byte sex to be used to encode the data.
  * @param data
  *     Where to place the two bytes of output.
  * @param value
  *     The value to be encoded.  Only the lower 16 bits are used, higher bits
  *     are ignored.
  */
void byte_sex_put_word(byte_sex_t bs, unsigned char *data, unsigned value);

/**
  * The byte_sex_put_word_le function is used to encode a 16-bit word value
  * into two bytes of data, using little-endian ordering.
  *
  * @param data
  *     Where to place the two bytes of output.
  * @param value
  *     The value to be encoded.  Only the lower 16 bits are used, higher bits
  *     are ignored.
  */
void byte_sex_put_word_le(unsigned char *data, unsigned value);

/**
  * The byte_sex_put_word_be function is used to encode a 16-bit word value
  * into two bytes of data, using big-endian ordering.
  *
  * @param data
  *     Where to place the two bytes of output.
  * @param value
  *     The value to be encoded.  Only the lower 16 bits are used, higher bits
  *     are ignored.
  */
void byte_sex_put_word_be(unsigned char *data, unsigned value);

/**
  * The byte_sex_other function may be use to obtain the opposite byte
  * sex, given one byte set.
  *
  * @param value
  *     The value to be inverted
  * @returns
  *     The opposite endian-ness.
  * @note
  *     The results are undefined if an invalid value is given.
  */
byte_sex_t byte_sex_other(byte_sex_t value);

#endif /* LIB_BYTE_SEX_H */
