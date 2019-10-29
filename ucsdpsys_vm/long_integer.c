/*
 * UCSD p-System virtual machine
 * Copyright (C) 2000 Mario Klebsch
 * Copyright (C) 2010 Peter Miller
 *
 * Parts of this file are taken from GNU bc-1.04.
 * Copyright (C) 1991, 1992, 1993, 1994, 1997 Free Software Foundation, Inc.
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

#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>

#include <lib/memory.h>
#include <lib/pcode.h>
#include <lib/psystem.h>

#include <ucsdpsys_vm/long_integer.h>
#include <ucsdpsys_vm/stack.h>

typedef enum { PLUS, MINUS } sign;

typedef struct bc_struct bc_struct;
struct bc_struct
{
    sign n_sign;
    int n_len;                  /* The number of digits */
    char n_value[1];            /* The storage. Not zero char
                                   terminated. It is allocated with
                                   all other fields.  */
};

typedef bc_struct *bc_num;

#undef DEBUG

#define MAX(a,b)        (((a)>(b))?(a):(b))
#define MIN(a,b)        (((a)<(b))?(a):(b))

/* Storage used for special numbers. */


/*
 * "Frees" a bc_num NUM.  Actually decreases reference count and only
 * frees the storage if reference count is zero.
 */

static void
free_num(bc_num *num)
{
    if (*num == NULL)
        return;
    free(*num);
    *num = NULL;
}


/*
 * new_num allocates a number and sets fields to known values.
 */

static bc_num
new_num(int length)
{
    bc_num temp;

    temp = (bc_num)malloc(sizeof(bc_struct) + length);
    assert(temp);
    temp->n_sign = PLUS;
    temp->n_len = length;
    temp->n_value[0] = 0;
    return temp;
}

/*
 * Compare two bc numbers.  Return value is 0 if equal, -1 if N1 is less
 * than N2 and +1 if N1 is greater than N2.  If USE_SIGN is false, just
 * compare the magnitudes.
 */

static int
_do_compare(bc_num n1, bc_num n2, int use_sign)
{
    char *n1ptr,
    *n2ptr;
    int count;

    /* First, compare signs. */
    if (use_sign && n1->n_sign != n2->n_sign)
    {
        if (n1->n_sign == PLUS)
            return (1); /* Positive N1 > Negative N2 */
        else
            return (-1); /* Negative N1 < Positive N1 */
    }

    /* Now compare the magnitude. */
    if (n1->n_len != n2->n_len)
    {
        if (n1->n_len > n2->n_len)
        {
            /* Magnitude of n1 > n2. */
            if (!use_sign || n1->n_sign == PLUS)
                return (1);
            else
                return (-1);
        }
        else
        {
            /* Magnitude of n1 < n2. */
            if (!use_sign || n1->n_sign == PLUS)
                return (-1);
            else
                return (1);
        }
    }

    /*
     * If we get here, they have the same number of integer digits.
     * check the integer part and the equal length part of the fraction.
     */
    count = n1->n_len;
    n1ptr = n1->n_value;
    n2ptr = n2->n_value;

    while ((count > 0) && (*n1ptr == *n2ptr))
    {
        n1ptr++;
        n2ptr++;
        count--;
    }
    if (count != 0)
    {
        if (*n1ptr > *n2ptr)
        {
            /* Magnitude of n1 > n2. */
            if (!use_sign || n1->n_sign == PLUS)
                return (1);
            else
                return (-1);
        }
        else
        {
            /* Magnitude of n1 < n2. */
            if (!use_sign || n1->n_sign == PLUS)
                return (-1);
            else
                return (1);
        }
    }

    /* They must be equal! */
    return (0);
}


/*
 * This is the "user callable" routine to compare numbers N1 and N2.
 */

static int
bc_compare(bc_num n1, bc_num n2)
{
    return _do_compare(n1, n2, 1);
}


/*
 * In some places we need to check if the number NUM is zero.
 */

static int
is_zero(bc_num num)
{
    int count;
    char *nptr;

    /* Initialize */
    count = num->n_len;
    nptr = num->n_value;

    /* The check */
    while ((count > 0) && (*nptr++ == 0))
        count--;

    if (count != 0)
        return 0;
    else
        return 1;
}


#if 0

/*
 * In some places we need to check if the number is negative.
 */

static int
is_neg(bc_num num)
{
    return num->n_sign == MINUS;
}

#endif


/*
 * For many things, we may have leading zeros in a number NUM.
 * _rm_leading_zeros just moves the data to the correct
 * place and adjusts the length.
 */

static void
_rm_leading_zeros(bc_num num)
{
    int bytes;
    char *dst;
    char *src;

    /* Do a quick check to see if we need to do it. */
    if (*num->n_value != 0)
        return;

    /* The first "digit" is 0, find the first non-zero digit in the second
       or greater "digit" to the left of the decimal place. */
    bytes = num->n_len;
    src = num->n_value;
    while (bytes > 1 && *src == 0)
        src++, bytes--;
    num->n_len = bytes;
    dst = num->n_value;
    while (bytes-- > 0)
        *dst++ = *src++;
}


/*
 * Perform addition: N1 is added to N2 and the value is
 * returned.  The signs of N1 and N2 are ignored.
 * SCALE_MIN is to set the minimum scale of the result.
 */

static bc_num
_do_add(bc_num n1, bc_num n2)
{
    int n1bytes = n1->n_len;
    int n2bytes = n2->n_len;
    bc_num sum = new_num(MAX(n1bytes, n2bytes) + 1);
    char *n1ptr = (char *)(n1->n_value + n1bytes - 1);
    char *n2ptr = (char *)(n2->n_value + n2bytes - 1);
    char *sumptr = (char *)(sum->n_value + sum->n_len - 1);
    int carry = 0;

    while ((n1bytes > 0) && (n2bytes > 0))
    {
        *sumptr = *n1ptr-- + *n2ptr-- + carry;
        if (*sumptr >= 10)
        {
            carry = 1;
            *sumptr -= 10;
        }
        else
            carry = 0;
        sumptr--;
        n1bytes--;
        n2bytes--;
    }

    /* Now add carry the longer integer part. */
    if (n1bytes == 0)
    {
        n1bytes = n2bytes;
        n1ptr = n2ptr;
    }
    while (n1bytes-- > 0)
    {
        *sumptr = *n1ptr-- + carry;
        if (*sumptr >= 10)
        {
            carry = 1;
            *sumptr -= 10;
        }
        else
            carry = 0;
        sumptr--;
    }

    /* Set final carry. */
    if (carry)
        *sumptr += 1;

    /* Adjust sum and return. */
    _rm_leading_zeros(sum);
    return sum;
}


/*
 * Perform subtraction: N2 is subtracted from N1 and the value is
 * returned.  The signs of N1 and N2 are ignored.  Also, N1 is
 * assumed to be larger than N2.  SCALE_MIN is the minimum scale
 * of the result.
 */

static bc_num
_do_sub(bc_num n1, bc_num n2)
{
    bc_num diff;
    int diff_len;
    int min_len;
    char *n1ptr;
    char *n2ptr;
    char *diffptr;
    int borrow;
    int count;
    int val;

    /* Allocate temporary storage. */
    diff_len = MAX(n1->n_len, n2->n_len);
    min_len = MIN(n1->n_len, n2->n_len);
    diff = new_num(diff_len);

    /* Initialize the subtract. */
    n1ptr = (char *)(n1->n_value + n1->n_len - 1);
    n2ptr = (char *)(n2->n_value + n2->n_len - 1);
    diffptr = (char *)(diff->n_value + diff_len - 1);

    /* Subtract the numbers. */
    borrow = 0;

    /* Now do the equal length scale and integer parts. */

    for (count = 0; count < min_len + 0; count++)
    {
        val = *n1ptr-- - *n2ptr-- - borrow;
        if (val < 0)
        {
            val += 10;
            borrow = 1;
        }
        else
            borrow = 0;
        *diffptr-- = val;
    }

    /* If n1 has more digits then n2, we now do that subtract. */
    if (diff_len != min_len)
    {
        for (count = diff_len - min_len; count > 0; count--)
        {
            val = *n1ptr-- - borrow;
            if (val < 0)
            {
                val += 10;
                borrow = 1;
            }
            else
                borrow = 0;
            *diffptr-- = val;
        }
    }

    /* Clean up and return. */
    _rm_leading_zeros(diff);
    return diff;
}


/*
 * Here is the full add routine that takes care of negative numbers.
 * N1 is added to N2 and the result placed into RESULT.  SCALE_MIN
 * is the minimum scale for the result.
 */

static void
bc_add(bc_num n1, bc_num n2, bc_num *result)
{
    bc_num sum = 0;

    if (n1->n_sign == n2->n_sign)
    {
        sum = _do_add(n1, n2);
        sum->n_sign = n1->n_sign;
    }
    else
    {
        /* subtraction must be done. */
        switch (_do_compare(n1, n2, 0)) /* Compare magnitudes. */
        {
        case -1:
            /* n1 is less than n2, subtract n1 from n2. */
            sum = _do_sub(n2, n1);
            sum->n_sign = n2->n_sign;
            break;

        case 0:
            /* They are equal! return zero with the correct scale! */
            sum = new_num(1);
            memset(sum->n_value, 0, 1);
            break;

        case 1:
            /* n2 is less than n1, subtract n2 from n1. */
            sum = _do_sub(n1, n2);
            sum->n_sign = n1->n_sign;
            break;
        }
    }

    /* Clean up and return. */
    free_num(result);
    *result = sum;
}


/*
 * Here is the full subtract routine that takes care of negative numbers.
 * N2 is subtracted from N1 and the result placed in RESULT.  SCALE_MIN
 * is the minimum scale for the result.
 */

static void
bc_sub(bc_num n1, bc_num n2, bc_num *result)
{
    bc_num diff = 0;

    if (n1->n_sign != n2->n_sign)
    {
        diff = _do_add(n1, n2);
        diff->n_sign = n1->n_sign;
    }
    else
    { /* subtraction must be done. */
        switch (_do_compare(n1, n2, 0)) /* Compare magnitudes. */
        {
        case -1:
            /* n1 is less than n2, subtract n1 from n2. */
            diff = _do_sub(n2, n1);
            diff->n_sign = (n2->n_sign == PLUS ? MINUS : PLUS);
            break;

        case 0:
            /* They are equal! return zero! */
            diff = new_num(1);
            memset(diff->n_value, 0, 1);
            break;

        case 1:
            /* n2 is less than n1, subtract n2 from n1. */
            diff = _do_sub(n1, n2);
            diff->n_sign = n1->n_sign;
            break;
        }
    }

    /* Clean up and return. */
    free_num(result);
    *result = diff;
}


/*
 * The multiply routine.  N2 time N1 is put int PROD
 */

static void
bc_multiply(bc_num n1, bc_num n2, bc_num *prod)
{
    int len1 = n1->n_len;
    int len2 = n2->n_len;
    int total_digits = len1 + len2;
    bc_num pval = new_num(total_digits); /* For the working storage. */
    char *n1end = (char *)(n1->n_value + len1 - 1); /* To the end of n1 */
    char *n2end = (char *)(n2->n_value + len2 - 1); /*   and n2. */
    char *pvptr = (char *)(pval->n_value + total_digits - 1);
    long sum = 0;

    int indx;

    pval->n_sign = (n1->n_sign == n2->n_sign ? PLUS : MINUS);

    for (indx = 0; indx < total_digits - 1; indx++)
    {
        char *n1ptr = (char *)(n1end - MAX(0, indx - len2 + 1));
        char *n2ptr = (char *)(n2end - MIN(indx, len2 - 1));
        while ((n1ptr >= n1->n_value) && (n2ptr <= n2end))
            sum += *n1ptr-- * *n2ptr++;
        *pvptr-- = sum % 10;
        sum = sum / 10;
    }
    *pvptr-- = sum;

    /* Assign to prod and clean up the number. */
    free_num(prod);
    *prod = pval;
    _rm_leading_zeros(*prod);
    if (is_zero(*prod))
        (*prod)->n_sign = PLUS;
}


/*
 * Some utility routines for the divide:  First a one digit multiply.
 * NUM (with SIZE digits) is multiplied by DIGIT and the result is
 * placed into RESULT.  It is written so that NUM and RESULT can be
 * the same pointers.
 */

static void
_one_mult(unsigned char *num, int size, int digit, unsigned char *result)
{
    if (digit == 0)
        memset(result, 0, size);
    else if (digit == 1)
        memcpy(result, num, size);
    else
    {
        unsigned char *nptr = (unsigned char *)(num + size - 1);
        unsigned char *rptr = (unsigned char *)(result + size - 1);
        int carry = 0;
        int value;

        while (size-- > 0)
        {
            value = *nptr-- * digit + carry;
            *rptr-- = value % 10;
            carry = value / 10;
        }

        if (carry != 0)
            *rptr = carry;
    }
}


/*
 * The full division routine. This computes N1 / N2.  It returns
 * 0 if the division is ok and the result is in QUOT.  The number of
 * digits after the decimal point is SCALE. It returns -1 if division
 * by zero is tried.  The algorithm is found in Knuth Vol 2. p237.
 */

static int
bc_divide(bc_num n1, bc_num n2, bc_num *quot)
{
    bc_num qval;
    unsigned char *num1;
    unsigned char *num2;
    unsigned char *ptr1;
    unsigned char *ptr2;
    unsigned char *n2ptr;
    unsigned char *qptr;
    int val;
    unsigned int len1;
    unsigned int len2;
    unsigned int qdigits;
    unsigned int count;
    unsigned int qdig;
    unsigned int qguess;
    unsigned int borrow;
    unsigned int carry;
    unsigned char *mval;
    char zero;

    /* Test for divide by zero. */
    if (is_zero(n2))
        return -1;

    /* Test for divide by 1.  If it is we must truncate. */
    if (n2->n_len == 1 && *n2->n_value == 1)
    {
        qval = new_num(n1->n_len);
        qval->n_sign = (n1->n_sign == n2->n_sign ? PLUS : MINUS);
        memcpy(qval->n_value, n1->n_value, n1->n_len);
        free_num(quot);
        *quot = qval;
    }

    /* Set up the divide.  Move the decimal point on n1 by n2's scale.
       Remember, zeros on the end of num2 are wasted effort for dividing. */
    n2ptr = (unsigned char *)n2->n_value + n2->n_len - 1;

    len1 = n1->n_len;
    num1 = (unsigned char *)malloc(n1->n_len + 2);
    assert(num1);
    memset(num1, 0, n1->n_len + 2);
    memcpy(num1 + 1, n1->n_value, n1->n_len);

    len2 = n2->n_len;
    num2 = (unsigned char *)malloc(len2 + 1);
    assert(num2);
    memcpy(num2, n2->n_value, len2);
    *(num2 + len2) = 0;
    n2ptr = num2;
    while (*n2ptr == 0)
    {
        n2ptr++;
        len2--;
    }

    /* Calculate the number of quotient digits. */
    if (len2 > len1)
    {
        qdigits = 1;
        zero = 1;
    }
    else
    {
        zero = 0;
        if (len2 > len1)
            qdigits = 1; /* One for the zero integer part. */
        else
            qdigits = len1 - len2 + 1;
    }

    /* Allocate and zero the storage for the quotient. */
    qval = new_num(qdigits);
    memset(qval->n_value, 0, qdigits);

    /* Allocate storage for the temporary storage mval. */
    mval = (unsigned char *)malloc(len2 + 1);
    assert(mval);

    /* Now for the full divide algorithm. */
    if (!zero)
    {
        unsigned int norm = 10 / ((int)*n2ptr + 1); /* Normalize */
        if (norm != 1)
        {
            _one_mult(num1, len1 + 1, norm, num1);
            _one_mult(n2ptr, len2, norm, n2ptr);
        }

        /* Initialize divide loop. */
        qdig = 0;
        if (len2 > len1)
            qptr = (unsigned char *)qval->n_value + len2 - len1;
        else
            qptr = (unsigned char *)qval->n_value;

        /* Loop */
        while (qdig <= len1 - len2)
        {
            /* Calculate the quotient digit guess. */
            if (*n2ptr == num1[qdig])
                qguess = 9;
            else
                qguess = (num1[qdig] * 10 + num1[qdig + 1]) / *n2ptr;

            /* Test qguess. */
            if (n2ptr[1] * qguess >
                (num1[qdig] * 10 + num1[qdig + 1] - *n2ptr * qguess) * 10
                + num1[qdig + 2])
            {
                qguess--;
                /* And again. */
                if (n2ptr[1] * qguess >
                    (num1[qdig] * 10 + num1[qdig + 1] - *n2ptr * qguess) * 10
                    + num1[qdig + 2])
                    qguess--;
            }

            /* Multiply and subtract. */
            borrow = 0;
            if (qguess != 0)
            {
                *mval = 0;
                _one_mult(n2ptr, len2, qguess, mval + 1);
                ptr1 = (unsigned char *)num1 + qdig + len2;
                ptr2 = (unsigned char *)mval + len2;
                for (count = 0; count < len2 + 1; count++)
                {
                    val = (int)*ptr1 - (int)*ptr2-- - borrow;
                    if (val < 0)
                    {
                        val += 10;
                        borrow = 1;
                    }
                    else
                        borrow = 0;
                    *ptr1-- = val;
                }
            }

            /* Test for negative result. */
            if (borrow == 1)
            {
                qguess--;
                ptr1 = (unsigned char *)num1 + qdig + len2;
                ptr2 = (unsigned char *)n2ptr + len2 - 1;
                carry = 0;
                for (count = 0; count < len2; count++)
                {
                    val = (int)*ptr1 + (int)*ptr2-- + carry;
                    if (val > 9)
                    {
                        val -= 10;
                        carry = 1;
                    }
                    else
                        carry = 0;
                    *ptr1-- = val;
                }
                if (carry == 1)
                    *ptr1 = (*ptr1 + 1) % 10;
            }

            /* We now know the quotient digit. */
            *qptr++ = qguess;
            qdig++;
        }
    }

    /* Clean up and return the number. */
    qval->n_sign = (n1->n_sign == n2->n_sign ? PLUS : MINUS);
    if (is_zero(qval))
        qval->n_sign = PLUS;
    _rm_leading_zeros(qval);
    free_num(quot);
    *quot = qval;

    /* Clean up temporary storage. */
    free(mval);
    free(num1);
    free(num2);

    return 0; /* Everything is OK. */
}


#ifdef DEBUG

/**
  * Output of a bcd number.  NUM is written in base O_BASE using OUT_CHAR
  * as the routine to do the actual output of the characters.
  */
static void
out_num(bc_num num)
{
    /* The negative sign if needed. */
    if (num->n_sign == MINUS)
        putchar('-');

    /* Output the number. */
    if (is_zero(num))
        putchar('0');
    else
    {
        char *nptr = num->n_value;
        int index;

        if (num->n_len > 1 || *nptr != 0)
        {
            for (index = num->n_len; index > 0; index--)
                putchar((*nptr++) + '0');
        }
        else
            nptr++;
    }
}

#endif


static void
PopLongInt(bc_num *num)
{
    int Len;
    char *nptr;

    Len = (Pop() & 0xff) - 1;
    *num = new_num(4 * Len);

    (*num)->n_sign = Pop()? MINUS : PLUS;

    nptr = (*num)->n_value;
    while (Len--)
    {
        word w = Pop();
        nptr[0] = (w & 0xf);
        w >>= 4;
        nptr[1] = (w & 0xf);
        w >>= 4;
        nptr[2] = (w & 0xf);
        w >>= 4;
        nptr[3] = (w & 0xf);
        nptr += 4;
    }
    _rm_leading_zeros(*num);
}


static void
PushLongInt(bc_num num)
{
    int i;
    int Len = 0;
    char *data = (char *)(num->n_value + num->n_len - 1);

    while (data >= &num->n_value[0]) /* as long as data available */
    {
        word w = 0;
        for (i = 0; i < 4; i++)
        {
            w <<= 4;
            if (data >= &num->n_value[0])
                w += *data--;
        }
        Push(w);
        Len++;
    }

    Push((num->n_sign == MINUS) ? 0xff : 0);
    Push(Len + 1);
}


/*
 * Convert to boolean.
 */
static inline word
Boolean(word i)
{
    return (i ? 1 : 0);
}


void
LongInt(word EntryPoint)
{
    word Op;

    (void)EntryPoint;
    Op = Pop();
    switch (Op)
    {
    case 0:
        /* normalize LongInt to store using stm */
        {
            word NewLen = Pop();
            word Len = Pop() & 0x0ff;
            word Sign = Pop();
            if (Len < NewLen)
            {
                while (Len++ < NewLen)
                    Push(0);
            }
            else if (Len > NewLen)
            {
                while (Len-- > NewLen)
                    Pop();
            }
            Push(Sign);
        }
        break;

    case 2:
        /* arg1 + arg2 */
        {
            bc_num Arg1;
            bc_num Arg2;
            bc_num Result = 0;

            PopLongInt(&Arg2);
            PopLongInt(&Arg1);

            bc_add(Arg1, Arg2, &Result);
            PushLongInt(Result);

#ifdef DEBUG
            out_num(Arg1);
            printf(" + ");
            out_num(Arg2);
            printf(" = ");
            out_num(Result);
            printf("\n");
#endif

            free_num(&Arg1);
            free_num(&Arg2);
            free_num(&Result);
        }
        break;

    case 4:
        /* arg1 - arg2 */
        {
            bc_num Arg1;
            bc_num Arg2;
            bc_num Result = 0;

            PopLongInt(&Arg2);
            PopLongInt(&Arg1);

            bc_sub(Arg1, Arg2, &Result);
            PushLongInt(Result);

#ifdef DEBUG
            out_num(Arg1);
            printf(" - ");
            out_num(Arg2);
            printf(" = ");
            out_num(Result);
            printf("\n");
#endif

            free_num(&Arg1);
            free_num(&Arg2);
            free_num(&Result);
        }
        break;

    case 6:
        /* - arg1 */
        {
            int Len = Pop();
            int Sign = Pop()? 1 : 0;
            Sign = !Sign;
            Push(Sign ? 0xff : 0);
            Push(Len);
        }
        break;

    case 8:
        /* arg1 * arg2 */
        {
            bc_num Arg1;
            bc_num Arg2;
            bc_num Result = 0;

            PopLongInt(&Arg2);
            PopLongInt(&Arg1);

            bc_multiply(Arg1, Arg2, &Result);
            PushLongInt(Result);

#ifdef DEBUG
            out_num(Arg1);
            printf(" * ");
            out_num(Arg2);
            printf(" = ");
            out_num(Result);
            printf("\n");
#endif

            free_num(&Arg1);
            free_num(&Arg2);
            free_num(&Result);
        }
        break;

    case 10:
        /* arg1 / arg2 */
        {
            bc_num Arg1;
            bc_num Arg2;
            bc_num Result = 0;

            PopLongInt(&Arg2);
            PopLongInt(&Arg1);
            if (bc_divide(Arg1, Arg2, &Result) < 0)
            {
                free_num(&Arg1);
                free_num(&Arg2);
                free_num(&Result);
                XeqError(XDIVZER);
            }
            PushLongInt(Result);

#ifdef DEBUG
            out_num(Arg1);
            printf(" / ");
            out_num(Arg2);
            printf(" = ");
            out_num(Result);
            printf("\n");
#endif

            free_num(&Arg1);
            free_num(&Arg2);
            free_num(&Result);
        }
        break;

    case 12:
        /* Convert LongInt to String */
        {
            word StrAddr;
            word IntLen;
            word Sign;
            int Idx = 0;
            int Supress = 1;

            Pop();
            StrAddr = Pop();
            IntLen = Pop() - 1; /* -1 for sign */
            Sign = Pop();

            if (Sign)
                MemWrByte(StrAddr, ++Idx, '-');

            while (IntLen--)
            {
                word w = Pop();
                int i;
                for (i = 0; i < 4; i++)
                {
                    byte Digit = w >> (4 * i) & 0x0f;
                    if (!Supress || Digit > 0)
                    {
                        MemWrByte(StrAddr, ++Idx, Digit + '0');
                        Supress = 0;
                    }
                }
            }
            if (Supress)
                MemWrByte(StrAddr, ++Idx, '0');
            MemWrByte(StrAddr, 0, Idx);
        }
        break;

    case 16:
        /* compare LongInts */
        {
            bc_num Arg1;
            bc_num Arg2;
            int Cmp = Pop();
            int Result;

            PopLongInt(&Arg2);
            PopLongInt(&Arg1);
            Result = bc_compare(Arg1, Arg2);
            switch (Cmp)
            {
            case 8:
                /* arg1 < arg2 */
                Push(Boolean(Result < 0));
                break;

            case 9:
                /* arg1 <= arg2 */
                Push(Boolean(Result <= 0));
                break;

            case 10:
                /* arg1 >= arg2 */
                Push(Boolean(Result >= 0));
                break;

            case 11:
                /* arg1 > arg2 */
                Push(Boolean(Result > 0));
                break;

            case 12:
                /* arg1 != arg2 */
                Push(Boolean(Result != 0));
                break;

            case 13:
                /* arg1 = arg2 ? */
                Push(Boolean(Result == 0));
                break;

            default:
                XeqError(XNOTIMP);
            }
        }
        break;

    case 18:
        /* convert Integer to LongInt */
        {
            Integer Val;
            word Sign;
            int i;
            int j;

            Val = PopInteger();

            if (!(Sign = (Val < 0)))
                Val = -Val;

            for (i = 0; i < 2; i++)
            {
                word w = 0;
                for (j = 0; j < 4; j++)
                {
                    w = (w << 4) - Val % 10;
                    Val = Val / 10;
                }
                Push(w);
            }
            Push(Sign ? 0xff : 0);
            Push(3);
        }
        break;

    case 20:
        /* trunc */
        {
            bc_num Arg1;
            int i;
            Integer Result = 0;

            PopLongInt(&Arg1);

            for (i = 0; i < Arg1->n_len; i++)
                Result = Result * 10 - Arg1->n_value[i];

            if (Arg1->n_sign == MINUS)
                Push(Result);
            else
                Push(-Result);

            free_num(&Arg1);
        }
        break;

    case 14:
    default:
#ifdef XXX
        panic("LongInt: unsupported Operation %d", Op);
#else
        XeqError(XNOTIMP);
#endif
    }
}
