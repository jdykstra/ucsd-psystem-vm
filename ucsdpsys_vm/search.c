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

#include <stdio.h>
#include <string.h>

#include <lib/psystem.h>
#include <lib/memory.h>

#include <ucsdpsys_vm/array.h>
#include <ucsdpsys_vm/search.h>

#undef DEBUG

/*
 * IdSearch()
 *
 * Details about IdSearch can be found in
 * http://www.gno.org/pub/apple2/doc/apple/technotes/pasc/tn.pasc.014
 */

#define SYM_IDENT       0
#define SYM_COMMA       1
#define SYM_COLON       2
#define SYM_SEMICOLON   3
#define SYM_LPARENT     4
#define SYM_RPARENT     5
#define SYM_DO          6
#define SYM_TO          7
#define SYM_DOWNTO      8
#define SYM_END         9
#define SYM_UNTIL       10
#define SYM_OF          11
#define SYM_THEN        12
#define SYM_ELSE        13
#define SYM_BECOMES     14
#define SYM_LBRACK      15
#define SYM_RBRACK      16
#define SYM_ARROW       17
#define SYM_PERIOD      18
#define SYM_BEGIN       19
#define SYM_IF          20
#define SYM_CASE        21
#define SYM_REPEAT      22
#define SYM_WHILE       23
#define SYM_FOR         24
#define SYM_WITH        25
#define SYM_GOTO        26
#define SYM_LABEL       27
#define SYM_CONST       28
#define SYM_TYPE        29
#define SYM_VAR         30
#define SYM_PROC        31
#define SYM_FUNC        32
#define SYM_PROG        33
#define SYM_FORWARD     34
#define SYM_INTCONST    35
#define SYM_REALCONST   36
#define SYM_STRINGCONST 37
#define SYM_NOT         38
#define SYM_MULOP       39
#define SYM_ADDOP       40
#define SYM_RELOP       41
#define SYM_SET         42
#define SYM_PACKED      43
#define SYM_ARRAY       44
#define SYM_RECORD      45
#define SYM_FILE        46
#define SYM_OTHER       47
#define SYM_LONGCONST   48
#define SYM_USES        49
#define SYM_UNIT        50
#define SYM_INTER       51
#define SYM_IMPLE       52
#define SYM_EXTERNL     53
#define SYM_SEPARATE    54
#define SYM_OTHERWISE   55

#define OP_MUL          0
#define OP_RDIV         1
#define OP_ANDOP        2
#define OP_IDIV         3
#define OP_IMOD         4
#define OP_PLUS         5
#define OP_MINUS        6
#define OP_OROP         7
#define OP_LTOP         8
#define OP_LEOP         9
#define OP_GEOP         10
#define OP_GTOP         11
#define OP_NEOP         12
#define OP_EQOP         13
#define OP_INOP         14
#define OP_NOOP         15

typedef struct IdTable_t IdTable_t;
struct IdTable_t
{
    byte Token[8];
    byte Sym;
    byte Op;
};

static const IdTable_t IdTable[] =
{
    { "AND     ", SYM_MULOP, OP_ANDOP },
    { "ARRAY   ", SYM_ARRAY, OP_NOOP },
    { "BEGIN   ", SYM_BEGIN, OP_NOOP },
    { "CASE    ", SYM_CASE, OP_NOOP },
    { "CONST   ", SYM_CONST, OP_NOOP },
    { "DIV     ", SYM_MULOP, OP_IDIV },
    { "DO      ", SYM_DO, OP_NOOP },
    { "DOWNTO  ", SYM_DOWNTO, OP_NOOP },
    { "ELSE    ", SYM_ELSE, OP_NOOP },
    { "END     ", SYM_END, OP_NOOP },
    { "EXTERNAL", SYM_EXTERNL, OP_NOOP },
    { "FILE    ", SYM_FILE, OP_NOOP },
    { "FOR     ", SYM_FOR, OP_NOOP },
    { "FORWARD ", SYM_FORWARD, OP_NOOP },
    { "FUNCTION", SYM_FUNC, OP_NOOP },
    { "GOTO    ", SYM_GOTO, OP_NOOP },
    { "IF      ", SYM_IF, OP_NOOP },
    { "IMPLEMEN", SYM_IMPLE, OP_NOOP },
    { "IN      ", SYM_RELOP, OP_INOP },
    { "INTERFAC", SYM_INTER, OP_NOOP },
    { "LABEL   ", SYM_LABEL, OP_NOOP },
    { "MOD     ", SYM_MULOP, OP_IMOD },
    { "NOT     ", SYM_NOT, OP_NOOP },
    { "OF      ", SYM_OF, OP_NOOP },
    { "OR      ", SYM_ADDOP, OP_OROP },
    { "OTHERWIS", SYM_OTHERWISE, OP_NOOP },
    { "PACKED  ", SYM_PACKED, OP_NOOP },
    { "PROCEDUR", SYM_PROC, OP_NOOP },
    { "PROGRAM ", SYM_PROG, OP_NOOP },
    { "RECORD  ", SYM_RECORD, OP_NOOP },
    { "REPEAT  ", SYM_REPEAT, OP_NOOP },
    { "SEGMENT ", SYM_PROG, OP_NOOP },
    { "SEPARATE", SYM_SEPARATE, OP_NOOP },
    { "SET     ", SYM_SET, OP_NOOP },
    { "THEN    ", SYM_THEN, OP_NOOP },
    { "TO      ", SYM_TO, OP_NOOP },
    { "TYPE    ", SYM_TYPE, OP_NOOP },
    { "UNIT    ", SYM_UNIT, OP_NOOP },
    { "UNTIL   ", SYM_UNTIL, OP_NOOP },
    { "USES    ", SYM_USES, OP_NOOP },
    { "VAR     ", SYM_VAR, OP_NOOP },
    { "WHILE   ", SYM_WHILE, OP_NOOP },
    { "WITH    ", SYM_WITH, OP_NOOP }
};


void
CspIdSearch(word BufPtr, word Arg2Ptr)
{
    word BufOffset = MemRd(Arg2Ptr);
    byte TokenBuf[8];
    byte Ch;
    size_t Idx;
    const IdTable_t *idp;

    memset(TokenBuf, ' ', sizeof(TokenBuf));

    Idx = 0;

    for (;;)
    {
        Ch = MemRdByte(BufPtr, BufOffset);
        if (Ch != '_')
        {
            if ((Ch >= 'a') && (Ch <= 'z'))
                Ch -= 0x20;
            else if (((Ch < 'A') || (Ch > 'Z')) && ((Ch < '0') || (Ch > '9')))
                break;
            if (Idx < sizeof(TokenBuf))
                TokenBuf[Idx++] = Ch;
        }
        BufOffset++;
    }

    /* Offset Correct, this team book failed the test. */
    BufOffset--;

    MemWr(Arg2Ptr, BufOffset);
#ifdef DEBUG
    for (Idx = 0; Idx < sizeof(TokenBuf); Idx++)
        putchar(TokenBuf[Idx]);
#endif

    for (idp = IdTable; idp < ENDOF(IdTable); idp++)
    {
        for (Idx = 0; Idx < sizeof(TokenBuf); Idx++)
            if (TokenBuf[Idx] != idp->Token[Idx])
                goto next;
#ifdef DEBUG
        printf(": found, Sym=%d Op=%d\n", idp->Sym, idp->Op);
#endif
        MemWr(WordIndexed(Arg2Ptr, 1), idp->Sym);
        MemWr(WordIndexed(Arg2Ptr, 2), idp->Op);
        return;
      next:
        ;
    }

    MemWr(WordIndexed(Arg2Ptr, 1), SYM_IDENT);
    MemWr(WordIndexed(Arg2Ptr, 2), OP_NOOP);
    for (Idx = 0; Idx < sizeof(TokenBuf); Idx++)
        MemWrByte(WordIndexed(Arg2Ptr, 3), Idx, TokenBuf[Idx]);
#ifdef DEBUG
    printf(": not found\n");
#endif
}


word
CspTreeSearch(word TokenBuf, word ResultPtr, word NodePtr)
{
    word Link;

#ifdef DEBUG
    int i;

    for (i = 0; i < 8; i++)
        putchar(MemRdByte(TokenBuf, i));
    printf(": ");
#endif

    for (;;)
    {
        int cmp = ByteCmp(TokenBuf, NodePtr, 8);
        if (cmp < 0)
        {
            Link = MemRd(WordIndexed(NodePtr, 5));
            if (Link != NIL)
                NodePtr = Link; /* follow RightLink                 */
            else
            {
#ifdef DEBUG
                printf("not found, should be on right node\n");
#endif
                MemWr(ResultPtr, NodePtr);
                return (0xffff);
            }
        }
        else if (cmp > 0)
        {
            Link = MemRd(WordIndexed(NodePtr, 4));
            if (Link != NIL)
                NodePtr = Link; /* follow LeftLink */
            else
            {
#ifdef DEBUG
                printf("not found, should be on left node\n");
#endif
                MemWr(ResultPtr, NodePtr);
                return (1);
            }
        }
        else
        {
#ifdef DEBUG
            printf("found\n");
#endif
            MemWr(ResultPtr, NodePtr);
            return (0);
        }
    }
}
