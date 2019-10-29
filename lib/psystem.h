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

#ifndef LIB_PSYSTEM_H
#define LIB_PSYSTEM_H

#include <lib/gcc_attributes.h>

typedef unsigned char   byte;
typedef unsigned short  word;
typedef signed short    Integer;

#define NUMBER(a) (sizeof(a)/sizeof(a[0]))
#define ENDOF(a) ((a) + NUMBER(a))

word Pop(void);
void Push(word Value);
void XeqError(word err);

void IoError    (word Result);
void UnitRead   (word Unit, word Addr, Integer AddrOffset,
                 word Len, word Block, word Mode);
void UnitWrite  (word Unit, word Addr, Integer AddrOffset,
                 Integer Len, word Block, word Mode);
void UnitClear  (word Unit);
void UnitStat   (word Unit, word Addr, Integer AddrOffset, word Dummy);
word UnitBusy   (word Unit);
void UnitWait   (word Unit);

void panic(const char *Msg, ...) FORMAT_PRINTF(1, 2);
void warning(const char *Msg, ...) FORMAT_PRINTF(1, 2);
void XeqError(word err);

extern word Syscom;

#define SEG_DICT_SIZE 32
#define SYSCOM_SIZE     170

#define IORSLT          WordIndexed(Syscom,0)
#define XEQERR          WordIndexed(Syscom,1)
#define SYSUNIT         WordIndexed(Syscom,2)
#define BUGSTATE        WordIndexed(Syscom,3)
#define GDIRP           WordIndexed(Syscom,4)
#define BOMBP           WordIndexed(Syscom,5)
#define STKBASE         WordIndexed(Syscom,6)
#define LASTMP          WordIndexed(Syscom,7)
#ifdef APPLE_1_3
#define BOMBPROC        WordIndexed(Syscom,8)
#define BOMBSEG         WordIndexed(Syscom,9)
#else
#define JTAB            WordIndexed(Syscom,8)
#define SEG             WordIndexed(Syscom,9)
#endif
#define MEMTOP          WordIndexed(Syscom,10)
#define BOMBIPC         WordIndexed(Syscom,11)

#ifdef UCSD_I_3
#define EXTENSION0      WordIndexed(Syscom,12)
#define EXTENSION1      WordIndexed(Syscom,13)
#define EXTENSION2      WordIndexed(Syscom,14)
#define EXTENSION3      WordIndexed(Syscom,15)
#define EXTENSION4      WordIndexed(Syscom,16)
#define EXTENSION5      WordIndexed(Syscom,17)
#define EXTENSION6      WordIndexed(Syscom,18)
#define EXTENSION7      WordIndexed(Syscom,19)
#define EXTENSION8      WordIndexed(Syscom,20)
#define EXTENSION9      WordIndexed(Syscom,21)
#define EXTENSION10     WordIndexed(Syscom,22)
#define EXTENSION11     WordIndexed(Syscom,23)
#define EXTENSION12     WordIndexed(Syscom,24)
#define EXTENSION13     WordIndexed(Syscom,25)
#define EXTENSION14     WordIndexed(Syscom,26)
#else
#define HLTLINE         WordIndexed(Syscom,12)
#define BRKPTS0         WordIndexed(Syscom,13)
#define BRKPTS1         WordIndexed(Syscom,14)
#define BRKPTS2         WordIndexed(Syscom,15)
#define BRKPTS3         WordIndexed(Syscom,16)
#define RETRIES         WordIndexed(Syscom,17)
#define EXTENSION0      WordIndexed(Syscom,18)
#define EXTENSION1      WordIndexed(Syscom,19)
#define EXTENSION2      WordIndexed(Syscom,20)
#define EXTENSION3      WordIndexed(Syscom,21)
#define EXTENSION4      WordIndexed(Syscom,22)
#define EXTENSION5      WordIndexed(Syscom,23)
#define EXTENSION6      WordIndexed(Syscom,24)
#define EXTENSION7      WordIndexed(Syscom,25)
#define EXTENSION8      WordIndexed(Syscom,26)
#endif
#define LOWTIME         WordIndexed(Syscom,27)
#define HIGHTIME        WordIndexed(Syscom,28)
#define MISCINFO        WordIndexed(Syscom,29)
#define CRTTYPE         WordIndexed(Syscom,30)
#define XRTCTRL         WordIndexed(Syscom,31)
#define CRTINFO         WordIndexed(Syscom,37)
#define CRTINFO_HEIGHT    WordIndexed(Syscom,37)
#define CRTINFO_WIDTH     WordIndexed(Syscom,38)

#define SEG_ENTRY_LEN   3
#define SEG_UNIT(s)     WordIndexed(Syscom, 48 + SEG_ENTRY_LEN*(s))
#define SEG_BLOCK(s)    WordIndexed(SEG_UNIT(s),1)
#define SEG_SIZE(s)     WordIndexed(SEG_UNIT(s),2)

/*
 * Note: Apple extended syscom, beyond the end of the segment table.
 * (See AppleCompatibility code, ucsdpsys_cm/main.c)
 * Words 161, 166, 169
 * So there were at least 26 more words in Apple's syscom, after the
 * segment table (32 segments).
 */

#endif /* LIB_PSYSTEM_H */
