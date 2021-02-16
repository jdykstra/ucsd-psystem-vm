/*
 * UCSD p-System virtual machine
 * Copyright (C) 2000 Mario Klebsch
 * Copyright (C) 2006, 2010 Peter Miller
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

#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <getopt.h>
#include <math.h>
#include <setjmp.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>

#include <lib/diskio.h>
#include <lib/memory.h>
#include <lib/pcode.h>
#include <lib/progname.h>
#include <lib/psystem.h>
#include <lib/version.h>

#include <ucsdpsys_vm/array.h>
#include <ucsdpsys_vm/native6502.h>
#include <ucsdpsys_vm/ptrace.h>
#include <ucsdpsys_vm/search.h>
#include <ucsdpsys_vm/sets.h>
#include <ucsdpsys_vm/stack.h>
#include <ucsdpsys_vm/term.h>

#undef IXP_COMPATIBILITY
#undef TRACE_TRANSLATE
#define TIME_SCALE      1

static FILE     *TraceFile=NULL;
static byte     TraceSeg;
static byte     TraceProc;

#define APPLE_HEAP_BOT          0x0804
#define APPLE_KP_TOP            0xfe7c
#define APPLE_SEG0_LOAD_GAP     0x450a
#define APPLE_SYSCOM            0xbdde

#ifdef TRACE_TRANSLATE
#define KP_TOP                  0xe000
#define HEAP_BOT                0x1000
#else
#define KP_TOP                  0xfe80
#define HEAP_BOT                0x0200
#endif

#define NUMBER(a)       (sizeof(a)/sizeof(a[0]))

typedef struct SegDict
{
  int           UseCount;
  word          OldKp;
  word          Seg;
  word          SegBase;
} SegDict_t;

SegDict_t       SegDict[SEG_DICT_SIZE];

#define MS_KP           -1
#define MS_STAT         0
#define MS_DYN          1
#define MS_JTAB         2
#define MS_SEG          3
#define MS_IPC          4
#define MS_SP           5
#define MS_VAR          5               /* Var-Offset counts from 1.. */
#define MS_FRAME_SIZE   6

/* Official P-Machine registers */
word    Sp;
word    Ipc;
word    IpcBase;
word    Seg;
word    JTab;
word    Kp;
word    Mp;
word    Np;
word    Base;

word    Syscom;

/* Flags */

#ifndef WORD_MEMORY
int     AppleCompatibility=0;
#endif

/* Additional Bookkeeping */
static word CurrentIpc;
static word BaseMp;
unsigned int    Level=0;
unsigned int    TraceLevel=0;
jmp_buf         ProcessNextInstrunction;

#ifdef XXX


/*
 * Two functions required once in debug output were now no longer be
 * called. They are too pity, to be disposed of already in /dev/null.
 */

char *
PStr(word w)
{
    static char Buffer[256];
    int len = MemRdByte(w, 0);
    char *p = Buffer;
    int i;

    for (i = 0; i < len; i++)
        *p++ = MemRdByte(w, i + 1);
    *p++ = '\0';
    return (Buffer);
}


char *
MultipleWords(word Addr, word Len)
{
    static char Buffer[5 * 256 + 1];
    char *p = Buffer;
    while (Len--)
    {
        sprintf(p, ",%04x", MemRd(Addr));
        p += strlen(p);
        Addr = WordIndexed(Addr, 1);
    }
    return (Buffer);
}

void
CheckCallStack(void)
{
    int i;
    word p = Mp;

    for (i = 0; i < Level; i++)
    {
        assert(p);
        p = MemRd(WordIndexed(p, MS_DYN));
    }
    assert(p == (0xb000 - 4));
}

#endif


void
warning(const char *Msg, ...)
{
    va_list ap;
    char Buffer[512];
    va_start(ap, Msg);
    vsnprintf(Buffer, sizeof(Buffer), Msg, ap);
    va_end(ap);
    fprintf(stderr, "warning: %s\n", Buffer);
    /*  TraceLevel=0x7fff; */
}

void
DumpCore(void)
{
    FILE *f;
    char filename[100];
    snprintf(filename, sizeof(filename), "%s.core", progname_get());
    f = fopen(filename, "w");
    if (f)
    {
        MemDump(f, 0, 0xffff);
        fclose(f);
    }
    else
    {
        warning("DumpCore: unable to create core dump %s: %s", filename,
            strerror(errno));
    }
}


void
panic(const char *Msg, ...)
{
    va_list ap;
    char Buffer[512];

    TermClose();
    va_start(ap, Msg);
    vsnprintf(Buffer, sizeof(Buffer), Msg, ap);
    va_end(ap);
    fprintf(stderr, "panic: %s\n", Buffer);
    DumpCore();
    abort();
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
MoveLeft(word Dst, Integer DstOffset, word Src, Integer SrcOffset, Integer Len)
{
    while (Len > 0)
    {
        --Len;
        MemWrByte(Dst, DstOffset++, MemRdByte(Src, SrcOffset++));
    }
}


void
MoveRight(word Dst, Integer DstOffset, word Src, Integer SrcOffset, Integer Len)
{
    SrcOffset += Len;
    DstOffset += Len;
    while (Len > 0)
    {
        --Len;
        MemWrByte(Dst, --DstOffset, MemRdByte(Src, --SrcOffset));
    }
}


word
FetchB(void)
{
    byte b;
    b = MemRdByte(IpcBase, Ipc++);
    if (b & 0x80)
        return ((word) ((b & 0x7f) << 8) + (word) MemRdByte(IpcBase, Ipc++));
    else
        return ((word) b);
}


static inline word
FetchW(void)
{
    word w;
    w = MemRdByte(IpcBase, Ipc++);
    w += (MemRdByte(IpcBase, Ipc++) << 8);
    return (w);
}


static inline word
FetchUB(void)
{
    return ((word)MemRdByte(IpcBase, Ipc++));
}


/*
 * Dereference a self relocating pointer. Self relocating pointers are
 * used in the segment dictionary and in procedure activation records.
 */
static inline word
SelfRelPtr(word Addr)
{
#ifdef WORD_MEMORY
    return (Addr - MemRd(Addr) / 2);
#else
    return (Addr - MemRd(Addr));
#endif
}


/*
 * Returns the number of procedures of a segment.
 */
static inline byte
SegNumProc(word segno)
{
    return (MemRd(segno) >> 8);
}


/*
 * Return the segment number of a segment.
 */
static inline byte
SegNumber(word segno)
{
    return (MemRd(segno) & 0xff);
}


/*
 * Returns a pointer to the activation record of a specified procedure
 * in a specified segment.
 */
static inline word
Proc(word segno, byte ProcNr)
{
    PointerCheck(segno);
    if ((ProcNr < 1) || (ProcNr > SegNumProc(segno)))
        panic("Proc: Illegal Procedure Number %d", ProcNr);
    return SelfRelPtr(WordIndexed(segno, -ProcNr));
}


/*
 * Returns the procedure number of a procedure.
 */
static inline signed char
ProcNumber(word jtab)
{
    PointerCheck(jtab);
    return (MemRd(jtab) & 0xff);
}


/*
 * Returns the lex level of a procedure.
 */
static inline signed char
ProcLexLevel(word jtab)
{
    PointerCheck(jtab);
    return (MemRd(jtab) >> 8);
}


/*
 * Returns a pointer to the first instruction of a procedure.
 */
static inline word
ProcBase(word jtab)
{
    PointerCheck(jtab);
    return (SelfRelPtr(WordIndexed(jtab, -1)));
}


/*
 * Returns the byte offset to the exit code of a procedure.
 */
static inline word
ProcExitIpc(word jtab)
{
    PointerCheck(jtab);
    return (MemRd(WordIndexed(jtab, -1)) - MemRd(WordIndexed(jtab, -2)) - 2);
}


/*
 * Returns the size of the parameters, which are passed to a
 * procedure.
 */
static inline word
ProcParamSize(word jtab)
{
    PointerCheck(jtab);
    return (MemRd(WordIndexed(jtab, -3)));
}


/*
 * Returns the size of the storage a procedure needs for its local
 * variables.
 */
static inline word
ProcDataSize(word jtab)
{
    PointerCheck(jtab);
    return (MemRd(WordIndexed(jtab, -4)));
}


/*
 * Returns a pointer to a local variable.
 */
static inline word
LocalAddr(word Offset)
{
    return (WordIndexed(Mp, MS_VAR + Offset));
}


/*
 * Returns a pointer to a global variable.
 */
static inline word
GlobalAddr(word Offset)
{
    return (WordIndexed(Base, MS_VAR + Offset));
}


/*
 * Traverse the static link chain.
 */
static inline word
Intermediate(byte Count)
{
    word p;
    for (p = Mp; Count; Count--)
        p = MemRd(WordIndexed(p, MS_STAT));
    return (p);
}


/*
 * Returns a pointer to a variable of an enclosing procedure.
 */
static inline word
IntermediateAddr(word Offset, byte Count)
{
    return (WordIndexed(Intermediate(Count), MS_VAR + Offset));
}


/*
 * Returns a pointer to a variable in a data segment (a global
 * variable in a UNIT)
 */
static inline word
ExternalAddr(word Offset, byte SegNo)
{
    assert(SegNo < SEG_DICT_SIZE);
    return (WordIndexed(SegDict[SegNo].Seg, Offset));
}


/*
 * calculates the target address of a jump operation. Positive
 * displacements perform relative jumps, negative displacements are
 * used as indices into the jump table.
 */
word
jump(signed char disp)
{
    if (disp >= 0)
        return (Ipc + disp);
    disp = -disp;
#ifdef WORD_MEMORY
    return (MemRd(WordIndexed(JTab, -1)) + 2 - (MemRd(JTab - disp / 2) + disp));
#else
    return (MemRd(WordIndexed(JTab, -1)) + 2 - (MemRd(JTab - disp) + disp));
#endif
}


/*
 * Calculates the static link pointer for a procedure.
 */
static inline word
StaticLink(word NewSeg, byte ProcNo)
{
    word NewJTab = Proc(NewSeg, ProcNo);

    if (!ProcNumber(NewJTab))
        return (NIL);
    return (Intermediate(ProcLexLevel(JTab) - ProcLexLevel(NewJTab) + 1));
}


/*
 * Load a segment.  If a data segment is to be loaded, just allocate
 * storage on the stack.
 */
void
CspLoadSegment(byte SegNo)
{
    assert(SegNo < SEG_DICT_SIZE);
    if (!SegDict[SegNo].UseCount)
    {
        word SegUnit = MemRd(SEG_UNIT(SegNo));
        word SegBlock = MemRd(SEG_BLOCK(SegNo));
        word SegSize = MemRd(SEG_SIZE(SegNo));

        assert(!(SegSize & 1));
        if (!SegSize)
            XeqError(XNOPROC);

        SegDict[SegNo].OldKp = Kp;
#ifdef WORD_MEMORY
        Kp -= SegSize / 2;
#else
        Kp -= SegSize;
#endif
        SegDict[SegNo].SegBase = Kp;
        if (SegBlock)
        {
            /*
             * if a block number is specified,
             * load a code segment.
             */
            SegDict[SegNo].Seg = WordIndexed(SegDict[SegNo].OldKp, -1);
            DiskRead(SegUnit, Kp, 0, SegSize, SegBlock);
            if (MemRd(IORSLT))
                XeqError(XSYIOER);
        }
        else
        {
            /* otherwise, it is a Data-Segment */
            SegDict[SegNo].Seg = WordIndexed(Kp, -1);
        }
    }
    SegDict[SegNo].UseCount++;
}


void
CspUnloadSegment(byte SegNo)
{
    assert(SegDict[SegNo].UseCount > 0);
    SegDict[SegNo].UseCount--;
    if (!SegDict[SegNo].UseCount)
    {
        Kp = SegDict[SegNo].OldKp;
        SegDict[SegNo].OldKp = 0;
        SegDict[SegNo].Seg = 0;
    }
}


/*
 * Clear the global directory pointer.
 */
void
ClrGDirP(void)
{
    word GDirP = MemRd(GDIRP);
    if (GDirP != NIL)
    {
        Np = GDirP;
        MemWr(GDIRP, NIL);
    }
}


/*
 * check for a gap between heap and stack.
 */
void
StackCheck(void)
{
    if (Np >= Kp)
    {
        MemWr(GDIRP, NIL);
        Kp = 0x8000;
        Np = 0x6200;
        XeqError(XSTKOVR);
    }
}


/*
 * Call a procedure.  It builds a stack frame for the new procedure
 * and sets up all registers of the p-machine.
 *
 * @returns
 *    1 if its a native procedure,
 *    0 if it is a p-code procedure
 */
int
call(word NewSeg, byte ProcNr, word static_link)
{
    word NewJTab = Proc(NewSeg, ProcNr);
    word DataSize = ProcDataSize(NewJTab);
    word ParamSize = ProcParamSize(NewJTab);
    word NewMp = WordIndexed(Kp, -(DataSize + ParamSize) / 2);

    if (!ProcNumber(NewJTab))
    {
        ProcessNative(NewJTab);
        return (1);
    }

    assert(!(ParamSize & 1));

    MoveLeft(NewMp, 0, Sp, 0, ParamSize);
    Sp = WordIndexed(Sp, ParamSize / 2);

    NewMp = WordIndexed(NewMp, -MS_FRAME_SIZE);
    if (ProcLexLevel(NewJTab) <= 0)
    {
        Push(Base);
        Base = NewMp;
        MemWr(STKBASE, Base);
    }

    MemWr(WordIndexed(NewMp, MS_KP), Kp);
    MemWr(WordIndexed(NewMp, MS_STAT), static_link);
    MemWr(WordIndexed(NewMp, MS_DYN), Mp);
    MemWr(WordIndexed(NewMp, MS_JTAB), JTab);
    MemWr(WordIndexed(NewMp, MS_SEG), Seg);
    MemWr(WordIndexed(NewMp, MS_IPC), Ipc);
    MemWr(WordIndexed(NewMp, MS_SP), Sp);

    Kp = WordIndexed(NewMp, -1); /* Small Hack :-( */
    Mp = NewMp;
    Seg = NewSeg;
    JTab = NewJTab;
    MemWr(LASTMP, Mp);
#ifdef SEG
    MemWr(SEG, Seg);
#endif
#ifdef JTAB
    MemWr(JTAB, JTab);
#endif

    IpcBase = ProcBase(JTab);
    Ipc = 0;
    Level++;
    StackCheck();
    return (0);
}


void
ret(byte n)
{
    word OldMp = Mp;
    byte OldSegNo = SegNumber(Seg);

    while (n > 0)
        Push(MemRd(LocalAddr(n--)));

    Kp = MemRd(WordIndexed(OldMp, MS_KP));
    Mp = MemRd(WordIndexed(OldMp, MS_DYN));
    JTab = MemRd(WordIndexed(OldMp, MS_JTAB));
    IpcBase = ProcBase(JTab);
    Seg = MemRd(WordIndexed(OldMp, MS_SEG));
    Ipc = MemRd(WordIndexed(OldMp, MS_IPC));
    MemWr(LASTMP, Mp);
#ifdef SEG
    MemWr(SEG, Seg);
#endif
#ifdef JTAB
    MemWr(JTAB, JTab);
#endif

    if (OldSegNo != SegNumber(Seg))
        if (OldSegNo) /* Segment 0 is not managed. */
            CspUnloadSegment(OldSegNo);
    Level--;
    StackCheck();
}


/*
 * An execution error has occured. Resume executon at segment 2
 * procedure 1, the system error handler.
 */
void
XeqError(word err)
{
    static int Flag = 0;
    word NewSeg = SegDict[0].Seg;

    if (Flag)
        panic("XeqError: recursion");
    Flag++;

    MemWr(XEQERR, err);
#ifdef BOMBPROC
    MemWr(BOMBPROC, ProcNumber(JTab));
#endif
#ifdef BOMBSEG
    MemWr(BOMBSEG, SegNumber(Seg));
#endif

#if 0
#ifndef WORD_MEMORY
    /*
     * The bytewise interpreter used in Apple Pascal did use Pointers
     * for the Ipc. Adding the Offset to IpcBase is sufficient to
     * emulate this behavior.
     */
    MemWr(BOMBIPC, IpcBase + CurrentIpc);
#else
    /*
     * In my wordwise interpreter, I do calculate this value in a way,
     * that the Apple Pascal error printing routine does print the
     * correct result. But, this value for BOMBIPC is not the location
     * of the failed instruction.
     */
    MemWr(BOMBIPC, JTab - MemRd(WordIndexed(JTab, -1)) - 2 + CurrentIpc);
#endif
    MemWr(MISCINFO, MemRd(MISCINFO) & ~(1 << 10));
#else
    /*
     * Early versions of Apple Pascal do contain the code to directly
     * print this offset.  A bit in system.miscinfo is checked in the
     * system error handler to see, whether BOMBIPC does contain a
     * pointer or an offset.
     */
    MemWr(BOMBIPC, CurrentIpc);
    MemWr(MISCINFO, MemRd(MISCINFO) | 1 << 10);
#endif

    call(NewSeg, 2, BaseMp);
    MemWr(BOMBP, Mp);

    /*
     * This code can be used to enter debugging upon entry of the system
     * error handler. It probably is only usefull to debug the system
     * error handler.
     */
#ifdef XXX
    TraceLevel = 0x7fff;
    warning("XeqError(%d)", err);
#endif
    Flag--;
    longjmp(ProcessNextInstrunction, 0);
}


/****************************************************************************/
/*                                                                          */
/*              P-debugger stuff.                                           */
/*                                                                          */


/*
 * Dump memory in decimal and in hex. Used to dump the evaluation stack.
 */
void
ShowMem(word Start, word End)
{
    for (; Start < End; Start = WordIndexed(Start, 1))
        fprintf(stderr, " %d(%x)", MemRd(Start), MemRd(Start));
    fprintf(stderr, "\n");
}


/*
 * Disassemble a procedure.
 */
void
List(FILE *out, int SegNo, word jtab)
{
    word ipc_base = ProcBase(jtab);
    word ipc = 0;
    char Buffer[1024];
    fprintf(out, "Params: %d, Vars: %d\n",
        ProcParamSize(jtab) / 2, ProcDataSize(jtab) / 2);
    while (WordIndexed(ipc_base, ipc / 2) < jtab)
    {
        word OpCode = MemRdByte(ipc_base, ipc);
        sprintf(Buffer, "%d:       ", ipc);
        ipc = DisasmP(Buffer + strlen(Buffer), SegNo, ipc_base, ipc, jtab, 0);
        fprintf(out, "%s\n", Buffer);
        if ((OpCode == RNP) || (OpCode == RBP) || (OpCode == XIT))
            return;
    }
}


void
Debugger(void)
{
    char prompt[256+64]; /* Hold maximum Buffer plus rough guess at other sizes. */
    char Buffer[256];
    unsigned int from;
    unsigned int to;
    char Buf[10];
    char *line;
    FILE *out;
    char *mode;
    int (*close_method)(FILE *);

    if (Level > TraceLevel)
        return;
    TraceLevel = 0x7fff;

    DisasmP(Buffer, SegNumber(Seg), IpcBase, Ipc, JTab, Sp);
    snprintf(prompt, sizeof(prompt), "s%d, p%d, %4d:      %s      > ",
        SegNumber(Seg), ProcNumber(JTab), CurrentIpc, Buffer);

    do
    {
        Buffer[0] = '\0';
        fprintf(stderr, "%s", prompt);
        if (!fgets(Buffer, sizeof(Buffer) - 1, stdin))
            break;
        Buffer[sizeof(Buffer) - 1] = '\0';

        close_method = NULL;
        out = NULL;
        line = Buffer;
        while (*line)
        {
            if ((*line == '|') || (*line == '>'))
                break;
            else
                line++;
        }

        if (*line == '|')
        {
            *line = '\0';
            line++;
            while (*line)
            {
                if (isspace(*line))
                    line++;
                else
                    break;
            }
            out = popen(line, "w");
            close_method = pclose;
        }
        else if (*line == '>')
        {
            *line = '\0';
            line++;
            if (*line == '>')
            {
                line++;
                mode = "a";
            }
            else
                mode = "w";
            while (*line)
            {
                if (isspace(*line))
                    line++;
                else
                    break;
            }
            out = fopen(line, mode);
            close_method = fclose;
        }
        if (!out)
        {
            close_method = NULL;
            out = stderr;
        }

        switch (Buffer[0])
        {
        case 'p':
            /* print stack */
            fprintf(stderr, "Stack:");
            ShowMem(Sp, SP_TOP);
            break;

        case 'd':
            switch (sscanf(Buffer, "%10s %x %x", Buf, &from, &to))
            {
#if 0
            case 1:
                from = NextDumpAddr;
                /* fall through... */
#endif
            case 2:
                to = from + 0x80;
                /* fall through... */

            case 3:
                MemDump(out, from, to);
                break;

            default:
                fprintf(stderr, "d <from> [<to>]\n");
            }
            break;

        case 'l':
            {
                int SegNo;
                int ProcNo;
                switch (sscanf(Buffer, "%10s %d %d", Buf, &SegNo, &ProcNo))
                {
                case 2:
                    ProcNo = SegNo;
                    SegNo = SegNumber(Seg);
                    /* fall through... */

                case 3:
                    if (SegNo < (int)SEG_DICT_SIZE)
                    {
                        CspLoadSegment(SegNo);
                        List(out, SegNo, Proc(SegDict[SegNo].Seg, ProcNo));
                        CspUnloadSegment(SegNo);
                    }
                    break;

                default:
                    fprintf(stderr, "l [<SegNo>] <ProcNo>\n");
                    break;
                }
            }
            break;

        case 't':
            {
                word s = Seg;
                word j = JTab;
                word m = Mp;
                word i = Ipc;

                for (;;)
                {
                    word w;
                    fprintf(out, "\ns%d, p%d, %4d:\n",
                        SegNumber(s), ProcNumber(j), i);
                    w = WordIndexed(m, MS_VAR);
                    MemDump(out, w, w + ProcParamSize(j) + ProcDataSize(j));

                    if (ProcLexLevel(j) < 0)
                        break;
                    j = MemRd(WordIndexed(m, MS_JTAB));
                    s = MemRd(WordIndexed(m, MS_SEG));
                    i = MemRd(WordIndexed(m, MS_IPC));
                    m = MemRd(WordIndexed(m, MS_DYN));
                }
            }
            MemDump(out, Kp, 0xb000);
            break;

        case 'v':
            MemDump
            (
                out,
                WordIndexed(Mp, MS_VAR),
                WordIndexed(Mp, MS_VAR) + ProcDataSize(JTab)
                    + ProcParamSize(JTab)
            );
            break;

        case 'g':
            TraceLevel = 0;
            return;

        case 'n':
            TraceLevel = Level;
            return;

        case 'f':
            TraceLevel = Level - 1;
            return;

        case 'r':
            fprintf(out,
                "Sp=%04x, Kp=%04x, Mp=%04x, Base=%04x, Seg=%04x, JTab=%04x, "
                "Np=%04x\n", Sp, Kp, Mp, Base, Seg, JTab, Np);
            break;

        case 'q':
            if (TraceFile)
                fclose(TraceFile);
            exit(0);
        }
        if (close_method && out)
        {
            close_method(out);
            close_method = NULL;
            out = NULL;
        }
    }
    while (Buffer[0] != '\n');
}


/****************************************************************************/
/*                                                                          */
/*              P-tracing stuff.                                            */
/*                                                                          */


/*
 * To compare traces with byte and word architecture, this routine
 * tries to 'normalize' the value of pointers. Of course, the
 * assumtions are not always true, but the diffs get a lot shorter
 * using this translation. :-)
 */
static inline word
Translate(word Value)
{
#ifdef TRACE_TRANSLATE
#ifdef WORD_MEMORY
    if (Value > KP_TOP)
        ;
    else if (Value > 0x8000)
        Value = (Value - KP_TOP) * 2 + KP_TOP;
    else if (Value > 0x7f00)
        ;
    else if (Value > HEAP_BOT)
        Value = (Value - HEAP_BOT) * 2 + HEAP_BOT;
#endif
#endif
    return (Value);
}


void
Tracer(void)
{
    char Buffer[64000];
    char StackBuf[1024];
    char *p = StackBuf;
    word w = Sp;

    *p = '\0';
    while (w < SP_TOP)
    {
        word Value = MemRd(w);

        sprintf(p, "%04x ", Translate(Value));
        p += strlen(p);
        w = WordIndexed(w, 1);
    }

    DisasmP(Buffer, MemRd(Seg) & 0xff, IpcBase, Ipc, JTab, Sp);

    fprintf(TraceFile, "s%d p%d o%d        %s      Stack: %s\n",
        MemRd(Seg) & 0xff, MemRd(JTab) & 0xff, Ipc, Buffer, StackBuf);

    fflush(TraceFile);
}


void
SetTrace(char *list)
{
    int i;
    int j;
    char *p;

    p = strchr(list, ',');
    switch (sscanf(list, "%d,%d", &i, &j))
    {
    case 1:
        TraceProc = i;
        break;

    case 2:
        TraceSeg = i;
        TraceProc = j;
        break;

    default:
        fprintf(stderr, "invalid trace flags\n");
        exit(1);
    }
}


/****************************************************************************/
/*                                                                          */
/*              The P-machine itself.                                       */
/*                                                                          */

int
AppleHack1(void)
{
    word Save0 = Ipc;

    if (FetchUB() == 145) /* NGI */
    {
        word OpCode = FetchUB();
        if (OpCode == 171) /* SRO */
        {
            word Var = FetchB(); /* Parameter SRO */
            OpCode = FetchUB();
            if
            (
                (
                    /* LDO n */
                    ((OpCode == 169) && (FetchB() == Var))
                ||
                    /* SLDO n */
                    ((Var >= 1) && (Var <= 16) && (OpCode == 231 + Var))
                )
            &&
                (FetchUB() == 0) /* SLDC 0 */
            &&
                (FetchUB() == 190) /* LDB */
            )
            {
                return (1);
            }
        }
        else if (OpCode == 204) /* STL */
        {
            word Var = FetchB(); /* Parameter STL */
            OpCode = FetchUB();
            if
            (
                (
                    /* LDL  n */
                    ((OpCode == 202) && (FetchB() == Var))
                ||
                    /* SLDL n */
                    ((Var >= 1) && (Var <= 16) && (OpCode == 215 + Var))
                )
            &&
                (FetchUB() == 0) /* SLDC 0 */
            &&
                (FetchUB() == 190) /* LDB */
            )
            {
                return (1);
            }
        }
    }
    Ipc = Save0;
    return (0);
}

int
AppleHack2(void)
{
    word Save = Ipc;
    word Var;

    if
    (
        (FetchUB() == 145) /* NGI */
    &&
        (FetchUB() == 171 && (Var = FetchB())) /* SRO */
    &&
        (FetchUB() == 169 && FetchB() == Var) /* LDO n */
    &&
        (FetchUB() == 6) /* SLDC 6 */
    &&
        (FetchUB() == 192 && FetchUB() == 16 && FetchUB() == 1) /* IXP 16,1 */
    &&
        (FetchUB() == 186) /* LDP */
    )
    {
        return (1);
    }
    Ipc = Save;
    return (0);
}


void
Processor(void)
{
    byte Opcode;
    word w;
    float f;
    word p1;
    word p2;

    setjmp(ProcessNextInstrunction);
    for (;;)
    {
        /* CheckCallStack(); */
        if (TraceFile)
        {
            if
            (
                !TraceProc
            ||
                (
                    (TraceProc == ProcNumber(JTab))
                &&
                    (TraceSeg == SegNumber(Seg))
                )
            )
            {
                Tracer();
            }
        }

        Debugger();

        CurrentIpc = Ipc;
        Opcode = FetchUB(); /* fetch next instruction */
        switch (Opcode)
        {
            /* One-word load and stores constant */
        case SLDC_0:
        case SLDC_1:
        case SLDC_2:
        case SLDC_3:
        case SLDC_4:
        case SLDC_5:
        case SLDC_6:
        case SLDC_7:
        case SLDC_8:
        case SLDC_9:
        case SLDC_10:
        case SLDC_11:
        case SLDC_12:
        case SLDC_13:
        case SLDC_14:
        case SLDC_15:
        case SLDC_16:
        case SLDC_17:
        case SLDC_18:
        case SLDC_19:
        case SLDC_20:
        case SLDC_21:
        case SLDC_22:
        case SLDC_23:
        case SLDC_24:
        case SLDC_25:
        case SLDC_26:
        case SLDC_27:
        case SLDC_28:
        case SLDC_29:
        case SLDC_30:
        case SLDC_31:
        case SLDC_32:
        case SLDC_33:
        case SLDC_34:
        case SLDC_35:
        case SLDC_36:
        case SLDC_37:
        case SLDC_38:
        case SLDC_39:
        case SLDC_40:
        case SLDC_41:
        case SLDC_42:
        case SLDC_43:
        case SLDC_44:
        case SLDC_45:
        case SLDC_46:
        case SLDC_47:
        case SLDC_48:
        case SLDC_49:
        case SLDC_50:
        case SLDC_51:
        case SLDC_52:
        case SLDC_53:
        case SLDC_54:
        case SLDC_55:
        case SLDC_56:
        case SLDC_57:
        case SLDC_58:
        case SLDC_59:
        case SLDC_60:
        case SLDC_61:
        case SLDC_62:
        case SLDC_63:
        case SLDC_64:
        case SLDC_65:
        case SLDC_66:
        case SLDC_67:
        case SLDC_68:
        case SLDC_69:
        case SLDC_70:
        case SLDC_71:
        case SLDC_72:
        case SLDC_73:
        case SLDC_74:
        case SLDC_75:
        case SLDC_76:
        case SLDC_77:
        case SLDC_78:
        case SLDC_79:
        case SLDC_80:
        case SLDC_81:
        case SLDC_82:
        case SLDC_83:
        case SLDC_84:
        case SLDC_85:
        case SLDC_86:
        case SLDC_87:
        case SLDC_88:
        case SLDC_89:
        case SLDC_90:
        case SLDC_91:
        case SLDC_92:
        case SLDC_93:
        case SLDC_94:
        case SLDC_95:
        case SLDC_96:
        case SLDC_97:
        case SLDC_98:
        case SLDC_99:
        case SLDC_100:
        case SLDC_101:
        case SLDC_102:
        case SLDC_103:
        case SLDC_104:
        case SLDC_105:
        case SLDC_106:
        case SLDC_107:
        case SLDC_108:
        case SLDC_109:
        case SLDC_110:
        case SLDC_111:
        case SLDC_112:
        case SLDC_113:
        case SLDC_114:
        case SLDC_115:
        case SLDC_116:
        case SLDC_117:
        case SLDC_118:
        case SLDC_119:
        case SLDC_120:
        case SLDC_121:
        case SLDC_122:
        case SLDC_123:
        case SLDC_124:
        case SLDC_125:
        case SLDC_126:
        case SLDC_127:
            Push(Opcode - SLDC_0); /* SLDC 0..127 Short LoaD Constant */
            break;

        case LDCN:
            /* LDCN LoaD Constant Nil */
            Push(NIL);
            break;

        case LDCI:
            /* LDCI LoaD Constant Integer */
            /*
             * This is always a little-endian fetch, even on big-endian
             * hosts, because there is no guarantee of word alignment (and
             * that's how the native compiler is written).
             */
            p1 = FetchW();
            if (p1 == 16607) /* Apple-Hack */
            {
                if (AppleHack1())
                    Push(4);
                else
                    Push(p1);
            }
            else if (p1 == 16606)
            {
                if (AppleHack2())
                    Push(Boolean(0));
                else
                    Push(p1);
            }
            else
                Push(p1);
            break;

            /* One-word load and stores local */
            /* SLDL Short LoaD Local 1..16 */
        case SLDL_1:
        case SLDL_2:
        case SLDL_3:
        case SLDL_4:
        case SLDL_5:
        case SLDL_6:
        case SLDL_7:
        case SLDL_8:
        case SLDL_9:
        case SLDL_10:
        case SLDL_11:
        case SLDL_12:
        case SLDL_13:
        case SLDL_14:
        case SLDL_15:
        case SLDL_16:
            Push(MemRd(LocalAddr(Opcode - SLDL_1 + 1)));
            break;

        case LDL:
            /* LDL LoaD Local */
            Push(MemRd(LocalAddr(FetchB())));
            break;

        case LLA:
            /* LLA Load Local Addres */
            Push(LocalAddr(FetchB()));
            break;

        case STL:
            /* STL STore Local */
            MemWr(LocalAddr(FetchB()), Pop());
            break;

            /* One-word load and stores global */
            /* SLDO Short LoaD glObal word */
        case SLDO_1:
        case SLDO_2:
        case SLDO_3:
        case SLDO_4:
        case SLDO_5:
        case SLDO_6:
        case SLDO_7:
        case SLDO_8:
        case SLDO_9:
        case SLDO_10:
        case SLDO_11:
        case SLDO_12:
        case SLDO_13:
        case SLDO_14:
        case SLDO_15:
        case SLDO_16:
            Push(MemRd(GlobalAddr(Opcode - SLDO_1 + 1)));
            break;

        case LDO:
            /* LDO LoaD glObal */
            Push(MemRd(GlobalAddr(FetchB())));
            break;

        case LAO:
            /* LAO Load Address glObal */
            Push(GlobalAddr(FetchB()));
            break;

        case SRO:
            /* SRO StoRe glObal */
            MemWr(GlobalAddr(FetchB()), Pop());
            break;

            /* One-word load and stores intermediate */
        case LOD:
            /* LOD LOaD */
            p1 = FetchUB();
            Push(MemRd(IntermediateAddr(FetchB(), p1)));
            break;

        case LDA:
            /* LDA LOad Addres */
            p1 = FetchUB();
            Push(IntermediateAddr(FetchB(), p1));
            break;

        case STR:
            /* STR StoRe */
            p1 = FetchUB();
            MemWr(IntermediateAddr(FetchB(), p1), Pop());
            break;

            /* One-word load and stores indirect */
        case SIND_0:
        case SIND_1:
        case SIND_2:
        case SIND_3:
        case SIND_4:
        case SIND_5:
        case SIND_6:
        case SIND_7:
            /* SIND Short INDirect */
            Push(MemRd(WordIndexed(Pop(), Opcode - SIND_0)));
            break;

        case IND:
            /* IND INDirect */
            Push(MemRd(WordIndexed(Pop(), FetchB())));
            break;

        case STO:
            /* STO STOre indirect */
            p1 = Pop();
            MemWr(Pop(), p1);
            break;

            /* One-word load and stores indirect */
        case LDE:
            /* LDE LoaD External */
            p1 = FetchUB();
            Push(MemRd(ExternalAddr(FetchB(), p1)));
            break;

        case LAE:
            /* LAE Load Addres External */
            p1 = FetchUB();
            Push(ExternalAddr(FetchB(), p1));
            break;

        case STE:
            /* STE STore External */
            p1 = FetchUB();
            MemWr(ExternalAddr(FetchB(), p1), Pop());
            break;

            /* multiple-word loads and stores */
        case LDC:
            p1 = FetchUB();
            Ipc = (Ipc + 1) & (~1); /* allowed only on word boundary */
#ifdef WORD_MEMORY
            w = IpcBase + Ipc / 2;
#else
            w = IpcBase + Ipc;
#endif
            /* FIXME: should this be a "native" word fetch? */
            while (p1--)
                Push(FetchW());
            break;

        case LDM:
            p1 = FetchUB();
            w = Pop();
            while (p1--)
                Push(MemRd(WordIndexed(w, p1)));
            break;

        case STM:
            p1 = FetchUB();
            w = MemRd(WordIndexed(Sp, p1));
            while (p1--)
            {
                MemWr(w, Pop());
                w = WordIndexed(w, 1);
            }
            Pop();
            break;

            /* byte array handling */
        case LDB:
            w = Pop();
            Push(MemRdByte(Pop(), w));
            break;

        case STB:
            p1 = Pop();
            w = Pop();
            MemWrByte(Pop(), w, p1);
            break;

            /* string handling */
        case LSA:
            assert(!(Ipc & 1));
            Push(WordIndexed(IpcBase, Ipc / 2));
            Ipc += FetchUB();
            break;

        case SAS:
            p1 = FetchUB();
            if ((w = Pop()) & 0xff00)
            {
                /* copy String */
                byte Len = MemRdByte(w, 0);
                word Dest = Pop();
                if (Len > p1)
                    XeqError(XS2LONG);
                MoveLeft(Dest, 0, w, 0, Len + 1);
            }
            else
            {
                /* store Char */
                word Dest = Pop();
                MemWrByte(Dest, 0, 1); /* make string of len 1             */
                MemWrByte(Dest, 1, w); /* containing char on stack         */
            }
            break;

        case IXS:
            p1 = Pop();
            p2 = Pop();
            Push(p2);
            Push(p1);
            if (p1 > MemRdByte(p2, 0))
                XeqError(XINVNDX);
            break;

            /* record and array handling */
        case MOV:
            p1 = FetchB();
            {
                word Src = Pop();
                word Dst = Pop();
                while (p1--)
                {
                    MemWr(Dst, MemRd(Src));
                    Dst = WordIndexed(Dst, 1);
                    Src = WordIndexed(Src, 1);
                }
            }
            break;

        case INC:
            Push(WordIndexed(Pop(), FetchB()));
            break;

        case IXA:
            w = Pop();
            Push(WordIndexed(Pop(), w * FetchB()));
            break;

        case IXP:
            p1 = FetchUB();
            p2 = FetchUB();
            w = Pop();
            Push(WordIndexed(Pop(), w / p1)); /* Address */
            Push(p2);
            Push((w % p1) * p2
#ifdef IXP_COMPATIBILITY
                * 0x101
#endif
                );
            break;

        case LPA:
            p1 = FetchB();
#ifdef WORD_MEMORY
            Push(IpcBase + Ipc / 2);
#else
            Push(IpcBase + Ipc);
#endif
            Ipc += p1;
            break;

        case LDP:
            {
                word Offset = Pop() & 0xff;
                word Size = Pop();
                word Addr = Pop();
                if (Offset + Size > 16)
                {
                    warning("LDP: Offset(%d)+Size(%d) > Bits per word",
                        Offset, Size);
                    XeqError(XINVNDX);
                }
                Push((MemRd(Addr) >> Offset) & ((1 << Size) - 1));
            }
            break;

        case STP:
            w = Pop();
            {
                word Offset = Pop() & 0xff;
                word Size = Pop();
                word Addr = Pop();
                if (Offset + Size > 16)
                {
                    warning("STP: Offset(%d)+Size(%d) > Bits per word",
                        Offset, Size);
                    XeqError(XINVNDX);
                }
                w &= (1 << Size) - 1;
                MemWr
                (
                    Addr,
                    (
                        (MemRd(Addr) & ~(((1 << Size) - 1) << Offset))
                    |
                        (w << Offset)
                    )
                );
            }
            break;

            /* TOS arithmetic: integers */
        case ABI:
            /* ABI ABsolute Integer */
            Push(abs(PopInteger()));
            break;

        case ADI:
            Push(PopInteger() + PopInteger());
            break;

        case NGI:
            Push(-PopInteger());
            break;

        case SBI:
            {
                Integer i = PopInteger();
                Push(PopInteger() - i);
            }
            break;

        case MPI:
            Push(PopInteger() * PopInteger());
            break;

        case SQI:
            {
                Integer i = PopInteger();
                Push(i * i);
            }
            break;

        case DVI:
            {
                Integer i = PopInteger();
                if (!i)
                    XeqError(XDIVZER);
                Push(PopInteger() / i);
            }
            break;

        case MODI:
            {
                Integer i = PopInteger();
                if (!i)
                    XeqError(XDIVZER);
                Push(PopInteger() % i);
            }
            break;

        case CHK:
            {
                Integer Upper = PopInteger();
                Integer Lower = PopInteger();
                Integer Value = PopInteger();
                Push(Value);
                if ((Value > Upper) || (Value < Lower))
                    XeqError(XINVNDX);
            }
            break;

        case EQUI:
            {
                Integer i = PopInteger();
                Push(Boolean(PopInteger() == i));
            }
            break;

        case NEQI:
            {
                Integer i = PopInteger();
                Push(Boolean(PopInteger() != i));
            }
            break;

        case LEQI:
            {
                Integer i = PopInteger();
                Push(Boolean(PopInteger() <= i));
            }
            break;

        case LESI:
            {
                Integer i = PopInteger();
                Push(Boolean(PopInteger() < i));
            }
            break;

        case GEQI:
            {
                Integer i = PopInteger();
                Push(Boolean(PopInteger() >= i));
            }
            break;

        case GRTI:
            {
                Integer i = PopInteger();
                Push(Boolean(PopInteger() > i));
            }
            break;

            /* TOS arithmetic: reals */
        case FLT:
            PushReal(PopInteger());
            break;

        case FLO:
            f = PopReal();
            PushReal(PopInteger());
            PushReal(f);
            break;

        case ABR:
            PushReal(fabs(PopReal()));
            break;

        case ADR:
            PushReal(PopReal() + PopReal());
            break;

        case NGR:
            PushReal(-PopReal());
            break;

        case SBR:
            f = PopReal();
            PushReal(PopReal() - f);
            break;

        case MPR:
            PushReal(PopReal() * PopReal());
            break;

        case SQR:
            f = PopReal();
            PushReal(f * f);
            break;

        case DVR:
            f = PopReal();
            if (f == 0)
                XeqError(XDIVZER);
            PushReal(PopReal() / f);
            break;

        case EQU:
            switch (FetchUB())
            {
            case 2:
                f = PopReal();
                Push(Boolean(PopReal() == f));
                break;

            case 4:
                w = Pop();
                Push(Boolean(StrCmp(Pop(), w) == 0));
                break;

            case 6:
                Push(Boolean((Pop() & 1) == (Pop() & 1)));
                break;

            case 8:
                {
                    Set_t Set1;
                    Set_t Set2;
                    SetPop(&Set1);
                    SetPop(&Set2);
                    Push(Boolean(!SetNeq(&Set1, &Set2)));
                }
                break;

            case 10:
                w = Pop();
                Push(Boolean(ByteCmp(Pop(), w, FetchB()) == 0));
                break;

            case 12:
                p1 = FetchB();
                w = Pop();
                Push(Boolean(WordCmp(Pop(), w, p1) == 0));
                break;

            default:
                XeqError(XNOTIMP);
            }
            break;

        case NEQ:
            switch (FetchUB())
            {
            case 2:
                f = PopReal();
                Push(Boolean(PopReal() != f));
                break;

            case 4:
                w = Pop();
                Push(Boolean(StrCmp(Pop(), w) != 0));
                break;

            case 6:
                Push(Boolean((Pop() & 1) != (Pop() & 1)));
                break;

            case 8:
                {
                    Set_t Set1;
                    Set_t Set2;
                    SetPop(&Set1);
                    SetPop(&Set2);
                    Push(Boolean(SetNeq(&Set1, &Set2)));
                }
                break;

            case 10:
                w = Pop();
                Push(Boolean(ByteCmp(Pop(), w, FetchB()) != 0));
                break;

            case 12:
                p1 = FetchB();
                w = Pop();
                Push(Boolean(WordCmp(Pop(), w, p1) != 0));
                break;

            default:
                XeqError(XNOTIMP);
            }
            break;

        case LEQ:
            switch (FetchUB())
            {
            case 2:
                f = PopReal();
                Push(Boolean(PopReal() <= f));
                break;

            case 4:
                w = Pop();
                Push(Boolean(StrCmp(Pop(), w) <= 0));
                break;

            case 6:
                w = Pop() & 1;
                Push(Boolean((Pop() & 1) <= w));
                break;

            case 8:
                {
                    Set_t needle;
                    Set_t haystack;
                    /* needle <= haystack */
                    SetPop(&haystack);
                    SetPop(&needle);
                    Push(Boolean(set_is_improper_subset(&haystack, &needle)));
                    break;
                }
                break;

            case 10:
                w = Pop();
                Push(Boolean(ByteCmp(Pop(), w, FetchB()) <= 0));
                break;

            default:
                XeqError(XNOTIMP);
            }
            break;

        case LES:
            switch (FetchUB())
            {
            case 2:
                f = PopReal();
                Push(Boolean(PopReal() < f));
                break;

            case 4:
                w = Pop();
                Push(Boolean(StrCmp(Pop(), w) < 0));
                break;

            case 6:
                w = Pop() & 1;
                Push(Boolean((Pop() & 1) < w));
                break;

            case 8:
                {
                    Set_t needle;
                    Set_t haystack;
                    /* needle < haystack */
                    SetPop(&haystack);
                    SetPop(&needle);
                    Push(Boolean(set_is_proper_subset(&haystack, &needle)));
                    break;
                }
                break;

            case 10:
                w = Pop();
                Push(Boolean(ByteCmp(Pop(), w, FetchB()) < 0));
                break;

            default:
                XeqError(XNOTIMP);
            }
            break;

        case GEQ:
            switch (FetchUB())
            {
            case 2:
                f = PopReal();
                Push(Boolean(PopReal() >= f));
                break;

            case 4:
                w = Pop();
                Push(Boolean(StrCmp(Pop(), w) >= 0));
                break;

            case 6:
                w = Pop() & 1;
                Push(Boolean((Pop() & 1) >= w));
                break;

            case 8:
                {
                    Set_t haystack;
                    Set_t needle;
                    /* haystack >= needle */
                    SetPop(&needle);
                    SetPop(&haystack);
                    Push(Boolean(set_is_improper_subset(&haystack, &needle)));
                }
                break;

            case 10:
                w = Pop();
                Push(Boolean(ByteCmp(Pop(), w, FetchB()) >= 0));
                break;

            default:
                XeqError(XNOTIMP);
            }
            break;

        case GRT:
            switch (FetchUB())
            {
            case 2:
                f = PopReal();
                Push(Boolean(PopReal() > f));
                break;

            case 4:
                w = Pop();
                Push(Boolean(StrCmp(Pop(), w) > 0));
                break;

            case 6:
                w = Pop() & 1;
                Push(Boolean((Pop() & 1) > w));
                break;

            case 8:
                {
                    Set_t haystack;
                    Set_t needle;
                    /* haystack > needle */
                    SetPop(&needle);
                    SetPop(&haystack);
                    Push(Boolean(set_is_proper_subset(&haystack, &needle)));
                }
                break;

            case 10:
                w = Pop();
                Push(Boolean(ByteCmp(Pop(), w, FetchB()) > 0));
                break;

            default:
                XeqError(XNOTIMP);
            }
            break;

            /* TOS arithmetic: logical */
        case LAND:
            Push(Pop() & Pop());
            break;

        case LOR:
            Push(Pop() | Pop());
            break;

        case LNOT:
            Push(~Pop());
            break;

            /* Sets */
        case ADJ:
            p1 = FetchUB();
            w = MemRd(Sp);
            if (p1 != w)
            {
                Set_t Buf;
                SetPop(&Buf);
                SetAdj(&Buf, p1);
                SetPush(&Buf);
            }
            if (p1 != Pop())
                panic("adj failure");
            break;

        case SGS:
            w = Pop();
            if (w < 512)
            {
                int Size = (w + 16) / 16;
                word Addr;
                int i;
                for (i = 0; i < Size; i++)
                    Push(0);
                Addr = WordIndexed(Sp, w / 16);
                MemWr(Addr, MemRd(Addr) | (1 << w % 16));
                Push(Size);
            }
            else
                XeqError(XINVNDX);
            break;

        case SRS:
            p1 = Pop();
            p2 = Pop();
            if ((p1 < 512) && (p1 < 512))
            {
                if (p2 > p1)
                    Push(0);
                else
                {
                    int Size = (p1 + 16) / 16;
                    word Addr;
                    int i;
                    for (i = 0; i < Size; i++)
                        Push(0);
                    while (p2 <= p1)
                    {
                        Addr = WordIndexed(Sp, p2 / 16);
                        MemWr(Addr, MemRd(Addr) | (1 << p2 % 16));
                        p2++;
                    }
                    Push(Size);
                }
            }
            else
                XeqError(XINVNDX);
            break;

        case INN:
            {
                word Size;
                word Addr;
                word Val;
                Size = Pop();
                Addr = Sp;
                Sp = WordIndexed(Sp, Size);
                Val = Pop();
                if (Val >= 16 * Size)
                    Push(Boolean(0));
                else
                {
                    Push(Boolean(MemRd(WordIndexed(Addr, (Val / 16))) &
                            (1 << (Val % 16))));
                }
            }
            break;

        case UNI:
            {
                int i;
                word Size;
                Set_t Set;

                SetPop(&Set);
                Size = Pop();
                if (Size > Set.Size)
                    SetAdj(&Set, Size);

                for (i = 0; i < Size; i++)
                    Set.Data[i] |= Pop();
                SetPush(&Set);
            }
            break;

        case INT:
            {
                int i;
                word Size;
                Set_t Set;

                SetPop(&Set);
                Size = Pop();
                if (Size > Set.Size)
                    SetAdj(&Set, Size);

                for (i = 0; i < Size; i++)
                    Set.Data[i] &= Pop();
                while (i < Set.Size)
                    Set.Data[i++] = 0;
                SetPush(&Set);
            }
            break;

        case DIF:
            {
                int i;
                word Size;
                Set_t Set;

                SetPop(&Set);
                Size = Pop();
                if (Size > Set.Size)
                    SetAdj(&Set, Size);

                for (i = 0; i < Size; i++)
                    Set.Data[i] = Pop() & ~Set.Data[i];
                while (i < Set.Size)
                    Set.Data[i] = 0;
                SetPush(&Set);
            }
            break;

            /* jumps */
        case UJP:
            w = jump((signed char)FetchUB());
            if ((Ipc - w == 5) && /* check for endless loop */
                (MemRdByte(IpcBase, w) == SLDC_1) &&
                (MemRdByte(IpcBase, w + 1) == FJP) &&
                (MemRdByte(IpcBase, w + 2) == 2))
            {
                sleep(1); /* reduce processor load */
            }
            Ipc = w;
            break;

        case FJP:
            p1 = FetchUB();
            if (!(Pop() & 1))
            {
                w = jump((signed char)p1);
                if ((Ipc - w == 3) && /* check for endless loop */
                    (MemRdByte(IpcBase, w) == SLDC_0))
                {
                    sleep(1); /* reduce processor load */
                }
                Ipc = w;
            }
            break;

        case EFJ:
            p1 = FetchUB();
            if (Pop() != Pop())
                Ipc = jump((signed char)p1);
            break;

        case NFJ:
            p1 = FetchUB();
            if (Pop() == Pop())
                Ipc = jump((signed char)p1);
            break;

        case XJP:
            {
                Integer lo;
                Integer hi;
                Integer value;

                Ipc = (Ipc + 1) & (~1);
                /* FIXME: should these be "native" word fetches? */
                lo = FetchW();
                hi = FetchW();
                value = PopInteger();
                if ((value >= lo) && (value <= hi))
                {
                    Ipc = Ipc + 2 * (value - lo) + 2;
                    Ipc -= MemRd(WordIndexed(IpcBase, Ipc / 2));
                }
            }
            break;

            /* procedure and function calls */
        case CLP:
            call(Seg, FetchUB(), Mp);
            break;

        case CGP:
            call(Seg, FetchUB(), Base);
            break;

        case CIP:
            p1 = FetchUB();
            call(Seg, p1, StaticLink(Seg, p1));
            break;

        case CBP:
            call(Seg, FetchUB(), BaseMp);
            break;

        case CXP:
            p1 = FetchUB();
            p2 = FetchUB();
            if (p1) /* Not for Segment 0 */
                CspLoadSegment(p1);
            w = SegDict[p1].Seg;
            if (call(w, p2, StaticLink(w, p2)))
            {
                /*
                 * Only native procedures are unloaded again
                 * immediately, because they have already been executed.
                 * A p-code procedure has yet to be interpreted, so it
                 * isn't unloaded, the ret() function (called by the RBP
                 * opcode) will unload it later.
                 */
                CspUnloadSegment(p1);
            }
            break;

        case RNP:
            Sp = MemRd(WordIndexed(Mp, MS_SP));
            ret(FetchUB());
            break;

        case RBP:
            Sp = MemRd(WordIndexed(Mp, MS_SP));
            Base = Pop();
            MemWr(STKBASE, Base);
            if ((Base < Kp) || (Base > BaseMp))
                panic("RBP: Base %04x out of range", Base);
            ret(FetchUB());
            break;

        case CSP:
            /* CSP, Call Standard Procedure */
            switch (FetchUB())
            {
            case CSP_IOC:
                if (MemRd(IORSLT))
                    XeqError(XUIOERR);
                break;

            case CSP_NEW:
                ClrGDirP();
                w = Pop();
                MemWr(Pop(), Np);
                Np = WordIndexed(Np, w);
                StackCheck();
                break;

            case CSP_MVL:
                {
                    Integer Len = PopInteger();
                    Integer DstOffset = PopInteger();
                    word Dst = Pop();
                    Integer SrcOffset = PopInteger();
                    word Src = Pop();
                    MoveLeft(Dst, DstOffset, Src, SrcOffset, Len);
                }
                break;

            case CSP_MVR:
                {
                    Integer Len = PopInteger();
                    Integer DstOffset = PopInteger();
                    word Dst = Pop();
                    Integer SrcOffset = PopInteger();
                    word Src = Pop();
                    MoveRight(Dst, DstOffset, Src, SrcOffset, Len);
                }
                break;

            case CSP_XIT:
                {
                    word ProcNo = Pop();
                    word SegNo = Pop();
                    word xMp = Mp;
                    word xSeg = Seg;
                    word xJTab = JTab;

                    Ipc = ProcExitIpc(xJTab);
                    while ((ProcNumber(xJTab) != ProcNo) ||
                        (SegNumber(xSeg) != SegNo))
                    {
                        if (!xMp ||
                            !(xJTab = MemRd(WordIndexed(xMp, MS_JTAB))) ||
                            !(xSeg = MemRd(WordIndexed(xMp, MS_SEG))))
                        {
                            XeqError(XNOEXIT);
                        }

                        MemWr(WordIndexed(xMp, MS_IPC), ProcExitIpc(xJTab));
                        xMp = MemRd(WordIndexed(xMp, MS_DYN));
                    }
                }
                break;

            case CSP_UREAD:
                {
                    word w6 = Pop();
                    word w5 = Pop();
                    word w4 = Pop();
                    word w3 = Pop();
                    word w2 = Pop();
                    word w1 = Pop();
                    UnitRead(w1, w2, w3, w4, w5, w6);
                }
                break;

            case CSP_UWRITE:
                {
                    word w6 = Pop();
                    word w5 = Pop();
                    word w4 = Pop();
                    word w3 = Pop();
                    word w2 = Pop();
                    word w1 = Pop();
                    UnitWrite(w1, w2, w3, w4, w5, w6);
                }
                break;

            case CSP_TIM:
                {
                    struct timeval tv;
                    if (gettimeofday(&tv, NULL) < 0)
                    {
                        MemWr(Pop(), 0);
                        MemWr(Pop(), 0);
                        MemWr(LOWTIME, 0);
                        MemWr(HIGHTIME, 0);
                    }
                    else
                    {
                        tv.tv_sec = (tv.tv_usec * 60 * TIME_SCALE / 1000000)
                            + tv.tv_sec * 60 * TIME_SCALE;
                        MemWr(Pop(), (tv.tv_sec >> 0) & 0xffff);
                        MemWr(LOWTIME, (tv.tv_sec >> 0) & 0xffff);

                        MemWr(Pop(), (tv.tv_sec >> 16) & 0xffff);
                        MemWr(HIGHTIME, (tv.tv_sec >> 16) & 0xffff);
                    }
                }
                break;

#ifdef CSP_IDS
            case CSP_IDS:
                {
                    word BufPtr = Pop();
                    word Arg2Ptr = Pop();
                    CspIdSearch(BufPtr, Arg2Ptr);
                }
                break;
#endif

#ifdef CSP_TRS
            case CSP_TRS:
                {
                    word TokenBuf = Pop();
                    word ResultPtr = Pop();
                    word NodePtr = Pop(); /* initialize with root node addr */
                    Push(CspTreeSearch(TokenBuf, ResultPtr, NodePtr));
                }
                break;
#endif

            case CSP_FLC:
                {
                    word ch = Pop();
                    Integer Len = PopInteger();
                    Integer Offset = PopInteger();
                    word Addr = Pop();
                    while (Len > 0)
                    {
                        --Len;
                        MemWrByte(Addr, Offset++, ch);
                    }
                }
                break;

            case CSP_SCN:
                {
                    Integer Offset;
                    word Buf;       /* Buffer Address */
                    word ch;        /* to sign-seeking */
                    word match;     /* 0 search for ==ch,
                                       !=0: Search !=ch */
                    word limit;     /* Limit */
                    word res;

                    Pop();
                    Offset = PopInteger();
                    Buf = Pop();
                    ch = Pop();
                    match = Pop();
                    limit = Pop();

                    if (limit & 0x8000)
                    {
                        limit = 0x10000 - limit;
                        for (res = 0; res < limit; res++)
                            if (MemRdByte(Buf, Offset - res) != ch)
                            {
                                if (match)
                                    break;
                            }
                            else
                            {
                                if (!match)
                                    break;
                            }
                        Push(0x10000 - res);
                    }
                    else
                    {
                        for (res = 0; res < limit; res++)
                            if (MemRdByte(Buf, Offset + res) != ch)
                            {
                                if (match)
                                    break;
                            }
                            else
                            {
                                if (!match)
                                    break;
                            }
                        Push(res);
                    }
                }
                break;

            case CSP_USTAT:
                {
                    word Dummy = Pop();
                    Integer Offset = PopInteger();
                    word Addr = Pop();
                    word Unit = Pop();
                    UnitStat(Unit, Addr, Offset, Dummy);
                }
                break;

#ifdef CSP_LDSEG
            case CSP_LDSEG:
                CspLoadSegment(Pop());
                break;
#endif

#ifdef CSP_ULDSEG
            case CSP_ULDSEG:
                CspUnloadSegment(Pop());
                break;
#endif

            case CSP_TRC:
                f = PopReal();
                if (f < 0)
                    Push(ceil(f));
                else
                    Push(floor(f));
                break;

            case CSP_RND:
                Push(round(PopReal()));
                break;

            case CSP_SIN:
                PushReal(sin(PopReal()));
                break;

            case CSP_COS:
                PushReal(cos(PopReal()));
                break;

            case CSP_SQRT:
                f = PopReal();
                if (f < 0)
                    XeqError(XFPIERR);
                PushReal(sqrt(f));
                break;

            case CSP_ATAN:
                PushReal(atan(PopReal()));
                break;

            case CSP_EXP:
                PushReal(exp(PopReal()));
                break;

            case CSP_LN:
                f = PopReal();
                if (f <= 0)
                    XeqError(XFPIERR);
                PushReal(log(f));
                break;

            case CSP_MRK:
                ClrGDirP();
                MemWr(Pop(), Np);
                break;

            case CSP_RLS:
                Np = MemRd(Pop());
                StackCheck();
                MemWr(GDIRP, NIL);
                break;

            case CSP_IOR:
                Push(MemRd(IORSLT));
                break;

            case CSP_UBUSY:
                Push(UnitBusy(Pop()));
                break;

            case CSP_POT:
                {
                    float PwrOfTen[] = { 1e0, 1e1, 1e2, 1e3, 1e4, 1e5,
                        1e6, 1e7, 1e8, 1e9, 1e10, 1e11,
                        1e12, 1e13, 1e14, 1e15, 1e16, 1e17,
                        1e18, 1e19, 1e20, 1e21, 1e22, 1e23,
                        1e24, 1e25, 1e26, 1e27, 1e28, 1e29,
                        1e30, 1e31, 1e32, 1e33, 1e34, 1e35,
                        1e36, 1e37, 1e38, 1e39
                    };
                    int Value = PopInteger();
                    if ((Value < 0) || (Value > 39))
                        PushReal(0); /* WWW: XeqError(XINVNDX); */
                    else
                        PushReal(PwrOfTen[Value]);
                }
                break;

            case CSP_UWAIT:
                UnitWait(Pop());
                break;

            case CSP_UCLEAR:
                UnitClear(Pop());
                break;

            case CSP_HLT:
                return;

            case CSP_MAV:
                if (MemRd(GDIRP))
                    w = Kp - MemRd(GDIRP);
                else
                    w = Kp - Np;
                Push(w / 2);
                break;

            default:
                XeqError(XNOTIMP);
            }
            break;

        case BPT:
            p1 = FetchB();
            if
            (
                (MemRd(BUGSTATE) >= 3)
            ||
                (p1 == MemRd(BRKPTS0))
            ||
                (p1 == MemRd(BRKPTS1))
            ||
                (p1 == MemRd(BRKPTS2))
            ||
                (p1 == MemRd(BRKPTS3))
            )
            {
                XeqError(XBRKPNT);
            }
            break;

        case XIT:
#if 1
            return;
#else
            XeqError(XHLTBPT);
            break;
#endif

        case NOP:
            break;

        default:
            XeqError(XNOTIMP);
            break;
        }
    }
}


word
LookupFile(word Unit, const char *Name)
{
    int i;
    DiskRead(Unit, Np, 0, 2048, 2);
    if (MemRd(IORSLT))
        return (0);

    for (i = 0; i < MemRd(WordIndexed(Np, 8)); i++)
    {
        word Entry = WordIndexed(Np, 13 + 13 * i);
        int len;
        for (len = 0; len < MemRdByte(WordIndexed(Entry, 3), 0); len++)
        {
            if (toupper(MemRdByte(WordIndexed(Entry, 3), 1 + len)) !=
                toupper(Name[len]))
            {
                goto next;
            }
        }
        if (Name[len])
            continue;
        return (MemRd(WordIndexed(Entry, 0)));
      next:
        ;
    }
    return (0);
}


/*
 * Segment 0 is split, and the pointer in the Procedure Dictionary have
 * been corrected so that after loading the two halves correctly to the
 * respective "correct" the address pointer is.
 *
 * This routine corrects the pointer in the segment dictionary.  In
 * addition, it determines the first address, in which the second half
 * really should be loaded.  Then an offset is determined by the pointers
 * must be corrected in the second half.
 */

static void
FixupSeg0(word LoadAddr)
{
    word seg = SegDict[0].Seg;
    word SegBase = SegDict[0].SegBase;
    word Addr;
    word Offset;
    int i;

    Addr = 0;
    for (i = 1; i <= SegNumProc(seg); i++)
    {
        word jtab = Proc(seg, i);
        if (jtab < SegBase && jtab > Addr)
            Addr = jtab;
    }
    if (!Addr)
        return; /* no Fixup needed */
    Addr = WordIndexed(Addr, 1);
    Offset = LoadAddr - Addr;
    if (!Offset)
        return;

    for (i = 1; i <= SegNumProc(seg); i++)
    {
        word jtab = Proc(seg, i);
        if (jtab < SegBase)
        {
            Addr = WordIndexed(seg, -i);
#ifdef WORD_MEMORY
            MemWr(Addr, MemRd(Addr) - 2 * Offset);
#else
            MemWr(Addr, MemRd(Addr) - Offset);
#endif
        }
    }
}


static void
load(word Unit, word BlockNo)
{
    int i;

    DiskRead(Unit, Np, 0, 512, BlockNo);
    if (MemRd(IORSLT))
        return;

    /* Create the Segment Dictionary */
    for (i = 0; i < 16; i++)
    {
        word CodeAddr = MemRd(WordIndexed(Np, 2 * i)) + BlockNo;
        word CodeLeng = MemRd(WordIndexed(Np, 2 * i + 1));
        word SegInfo = MemRd(WordIndexed(Np, i + 0x80));

        assert(!(CodeLeng & 1));

        if (CodeAddr && CodeLeng)
        {
            int SegNo = SegInfo & 0xff;
            if (SegInfo & 0x0f00)
            {
                MemWr(SEG_UNIT(SegNo), Unit);
                MemWr(SEG_BLOCK(SegNo), CodeAddr);
                MemWr(SEG_SIZE(SegNo), CodeLeng);
            }
            if (SegNo == 0)
            {
                if (!SegDict[0].UseCount)
                {
                    SegDict[0].UseCount++;
                    SegDict[0].OldKp = Kp;
                    SegDict[0].Seg = WordIndexed(Kp, -1);
#ifdef WORD_MEMORY
                    Kp -= CodeLeng / 2;
#else
                    Kp -= CodeLeng;
#endif
                    SegDict[0].SegBase = Kp;
                    DiskRead(Unit, Kp, 0, CodeLeng, CodeAddr);
                }
                else
                {
#ifndef WORD_MEMORY
#ifdef APPLE_SEG0_LOAD_GAP
                    if (AppleCompatibility)
                    {
                        Kp -= APPLE_SEG0_LOAD_GAP;
                        assert(Syscom >= Kp);
                        assert(WordIndexed(Syscom, SYSCOM_SIZE) <
                            Kp + APPLE_SEG0_LOAD_GAP);
                    }
#endif
#endif
                    FixupSeg0(Kp);
#ifdef WORD_MEMORY
                    Kp -= CodeLeng / 2;
#else
                    Kp -= CodeLeng;
#endif
                    DiskRead(Unit, Kp, 0, CodeLeng, CodeAddr);
                }
            }
        }
    }
}


static void
load_system(int *root_unit_p, const char *file_name)
{
    int             unit;
    int             block;
    int             root_unit;

    unit = 0;
    root_unit = *root_unit_p;
    block = LookupFile(root_unit, file_name);
    if (block)
    {
        unit = root_unit;
    }
    else
    {
        for (unit = 4; unit < MAX_UNIT; unit++)
        {
            if (unit == 6)
                unit = 9;
            block = LookupFile(unit, file_name);
            if (block)
                break;
        }
    }
    if (!block || !unit)
        panic("file \"%s\": not found", file_name);

    load(unit, block);
    if (MemRd(IORSLT))
    {
        panic("file \"%s\": unit %d, block %d: Ioerror %d",
            file_name, unit, block, MemRd(IORSLT));
    }
    if (!SegDict[0].UseCount)
        panic("file \"%s\": not a valid system, no segment 0", file_name);

    call(SegDict[0].Seg, 1, NIL);
    *root_unit_p = unit;
}


/**
  * The data_read function is used to emulate the memory occupied by the
  * THEDATE variable in the system variables.  In this way, the system's
  * idea of th ecurrent date will always be up-to-date without manual
  * intervention.
  *
  * @param addr
  *     The address being read
  * @returns
  *     The word or byte value requested.
  */
static word
date_read(word addr)
{
    time_t now;
    struct tm *tmp;
    word value;

    time(&now);
    tmp = localtime(&now);
    value =
        (
            ((tmp->tm_year % 100) << 9)
        |
            (tmp->tm_mon + 1)
        |
            (tmp->tm_mday << 4)
        );
#ifdef WORD_MEMORY
    (void)addr;
    return value;
#else
    if ((addr & 1) == (byte_sex == big_endian))
        return (value & 0xFF);
    else
        return (value >> 8);
#endif
}


static word
crtinfo_width_read(word addr)
{
    int value = term_width();
#ifdef WORD_MEMORY
    (void)addr;
    return value;
#else
    if ((addr & 1) == (byte_sex == big_endian))
        return (value & 0xFF);
    else
        return (value >> 8);
#endif
}


static word
crtinfo_height_read(word addr)
{
    int value = term_height();
#ifdef WORD_MEMORY
    (void)addr;
    return value;
#else
    if ((addr & 1) == (byte_sex == big_endian))
        return (value & 0xFF);
    else
        return (value >> 8);
#endif
}


static word miscinfo;

#define MISCINFO_HASCLOCK  (1 << 0)
#define MISCINFO_HAS8510A  (1 << 1)
#define MISCINFO_HASLCCRT  (1 << 2)
#define MISCINFO_HASXYCRT  (1 << 3)
#define MISCINFO_SLOWTERM  (1 << 4)
#define MISCINFO_STUPID    (1 << 5)
#define MISCINFO_NOBREAK   (1 << 6)
#define MISCINFO_USERKIND  (3 << 7)
#define MISCINFO_IS_FLIPT  (1 << 9)
#define MISCINFO_WORD_MACH (1 << 10)


static word
miscinfo_read(word addr)
{
    int value = miscinfo;

    /*
     * MISCINFO_NOBREAK
     *     This is used to control whether or not the BREAK character is
     *     to be processed.  Gets turned on and off by the system, so we
     *     leave it alone.
     *
     * FIXME: have the emulator honor this bit.
     */

    /*
     * MISCINFO_STUPID
     *     This is used by the compiler to decide whether or not to drop
     *     straight into the editor when the compiler sees an error.
     */

    /*
     * MISCINFO_SLOWTERM
     *     This is used to decide whether or not to use short or
     *     long prompts.  Compared to 1979, this emulator cannot be
     *     considered "slow".
     */
    miscinfo &= ~MISCINFO_SLOWTERM;

    /*
     * MISCINFO_HASXYCRT
     *     This is used to decide whether or not the GOTOXY built-in
     *     procedure does anything useful.  If you are in batch mode,
     *     the ncurses interface isn't used, and thus GOTOXY isn't
     *     useful in batch mode.
     */
    if (term_is_batch_mode())
        miscinfo &= ~MISCINFO_HASXYCRT;
    else
        miscinfo |= MISCINFO_HASXYCRT;

    /*
     * MISCINFO_HASLCCRT
     *     This is used to determine whether or not the output terminal is
     *     capable of displaying lower case characters.  It is no longer 1979,
     *     and this emulator always has lower case available.
     */
    value |= MISCINFO_HASLCCRT;

    /*
     * MISCINFO_HAS8510A
     *     This is used to determine whether or not a Terak graphics
     *     card is available.  This emulator doesn't have one.
     */
    value &= ~MISCINFO_HAS8510A;

    /*
     * MISCINFO_HASCLOCK
     *     This is used to determine whether or not the TIME built-in
     *     procedure returns anything useful.  This emulator always does.
     */
    value |= MISCINFO_HASCLOCK;

    /*
     * MISCINFO_USERKIND
     *     This is very strange. It appears to be some kind of privilege
     *     indicator.  See system/system.a.text and system/syssegs.b.text for
     *     additional mystification.
     */

    /*
     * MISCINFO_WORD_MACH
     *     This is used to determine whether or not the CPU is word-oriented
     *     (true) or byte oriented (false).  None of the UCSD system source
     *     code appears to use this.
     */
#ifdef WORD_MEMORY
    miscinfo |= MISCINFO_WORD_MACH;
#else
    miscinfo &= ~MISCINFO_WORD_MACH;
#endif

    /*
     * MISCINFO_IS_FLIPT
     *     This is used to determine whether the byte sex of the CPU;
     *     false if little endian architecture, true if big endian
     *     architecture.  Most programs use a different method.
     */
    if (byte_sex == big_endian)
        miscinfo |= MISCINFO_IS_FLIPT;
    else
        miscinfo &= ~MISCINFO_IS_FLIPT;

#ifdef WORD_MEMORY
    (void)addr;
    return value;
#else
    if ((addr & 1) == (byte_sex == big_endian))
        return (value & 0xFF);
    else
        return (value >> 8);
#endif
}


static void
miscinfo_write(word addr, word value)
{
#ifdef WORD_MEMORY
    (void)addr;
    miscinfo = value;
#else
    if ((addr & 1) == (byte_sex == big_endian))
        miscinfo = (miscinfo & 0xFF00) | (value & 0x00FF);
    else
        miscinfo = (miscinfo & 0x00FF) | ((value << 8) & 0xFF00);
#endif
}


static void
usage(void)
{
    const char *prog = progname_get();
    fprintf(stderr, "Usage: %s [ <option>... ]\n", prog);
    fprintf(stderr, "       %s -V\n", prog);
    exit(1);
}


static const struct option options[] =
{
    { "apple", 0, 0, 'a' },
    { "batch", 1, 0, 'b' },
    { "dump", 0, 0, 'd' },
    { "forget", 1, 0, 'f' },
    { "name", 1, 0, 'n' },
    { "no-emulation", 0, 0, 'D' },
    { "read", 1, 0, 'r' },
    { "trace", 1, 0, 'T' },
    { "trace-file", 1, 0, 't' },
    { "trace-max", 0, 0, 'g' },
    { "version", 0, 0, 'V' },
    { "write", 1, 0, 'w' },
    { "xterm", 1, 0, 'x' },
    { 0, 0, 0, 0 }
};


int
main(int argc, char **argv)
{
    int             always_dump_core;
    int             i;
    int             Unit;
    int             UseXTerm;
    int             BatchFd;
    const char      *SystemName;
    int             emulate_date;
    int             root_unit;
    const char      *byte_sex_set;

    progname_set(argv[0]);
    always_dump_core = 0;
    Unit = 4;
    UseXTerm = 0;
    BatchFd = -1;
    SystemName = "system.pascal";
    emulate_date = 1;
    memset(SegDict, 0, sizeof(SegDict));
    MemInit();
    DiskInit();
    TraceProc = 0;
    TraceSeg = 1;
    byte_sex_set = 0;

    for (;;)
    {
        i = getopt_long(argc, argv, "ab:Ddgn:t:T:w:r:f:xV", options, 0);
        if (i == EOF)
            break;
        switch (i)
        {
        case 'a':
#ifndef WORD_MEMORY
            AppleCompatibility = 1;
#endif
            break;

        case 'b':
            if (!optarg || !*optarg)
            {
                fprintf(stderr,
                    "-b option requires filename argument or '-' for stdin\n");
                usage();
            }
            if (strcmp(optarg, "-") == 0)
                BatchFd = 0;
            else
                BatchFd = open(optarg, O_RDONLY, 0);
            break;

        case 'D':
            emulate_date = 0;
            break;

        case 'd':
            always_dump_core = 1;
            break;

        case 'g':
            TraceLevel = 0x7fff;
            break;

        case 'n':
            if (!optarg || !*optarg)
            {
                fprintf(stderr, "-n option requires filename argument\n");
                usage();
            }
            SystemName = optarg;
            break;

        case 't':
            if (!optarg || !*optarg)
            {
                fprintf(stderr, "-t option requires filename argument\n");
                usage();
            }
            if (strcmp(optarg, "-") == 0)
            {
                TraceFile = fdopen(dup(1), "w");
            }
            else
            {
                TraceFile = fopen(optarg, "w");
            }
            break;

        case 'T':
            SetTrace(optarg);
            break;

        case 'w':
        case 'r':
        case 'f':
            {
                enum DiskMode Mode = ReadOnly;
                switch (i)
                {
                case 'w':
                    Mode = ReadWrite;
                    break;

                case 'r':
                    Mode = ReadOnly;
                    break;

                case 'f':
                    Mode = Forget;
                    break;
                }
                if (!optarg || !*optarg)
                {
                    fprintf
                    (
                        stderr,
                        "-%c option requires filename argument\n",
                        i
                    );
                    usage();
                }
                if (DiskMount(Unit, optarg, Mode) < 0)
                    exit(1);

                /*
                 * Make sure the byte sex of all the disks is the same
                 * as the byte sex of the first disk.
                 */
                if (byte_sex_set)
                {
                    byte_sex_t bs2 = disk_get_byte_sex(Unit);
                    if (byte_sex != bs2)
                    {
                        fprintf
                        (
                            stderr,
                            "%s: byte sex %s mismatch...\n",
                            optarg,
                            byte_sex_name(bs2)
                        );
                        fprintf
                        (
                            stderr,
                            "%s: ... expected %s\n",
                            byte_sex_set,
                            byte_sex_name(byte_sex)
                        );
                        exit(1);
                    }
                }
                else
                {
                    /*
                     * Set the emulator's byte sex from the byte sex of
                     * the first disk.
                     *
                     * It is essential that teh bytes sex it set before
                     * the first of the initialization code that
                     * estanblishes the contents of emulated memory.
                     */
                    byte_sex = disk_get_byte_sex(Unit);
                    byte_sex_set = optarg;
                }

                Unit++;
                if (Unit == 6)
                    Unit = 9;
            }
            break;

        case 'x':
            UseXTerm++;
            break;

        case 'V':
            version_print();
            return 0;
        }
    }

    TermOpen(UseXTerm, BatchFd);

#ifndef WORD_MEMORY
    if (AppleCompatibility)
    {
        Np = APPLE_HEAP_BOT;
        Kp = APPLE_KP_TOP;
        Syscom = APPLE_SYSCOM;
    }
    else
#endif
    {
        Np = HEAP_BOT;
        Kp = KP_TOP;
        Kp = WordIndexed(Kp, -SYSCOM_SIZE);
        Syscom = Kp;
    }
    Sp = SP_TOP;
    Mp = Kp;

    root_unit = 4;
    load_system(&root_unit, SystemName);
    BaseMp = Mp;
    Sp = WordIndexed(Sp, 1); /* SP correct */

    MemWr(LocalAddr(1), Syscom);

    if (emulate_date)
    {
        if (ProcDataSize(JTab) > (67 * 2))
        {
            /*
             * We are going to emulate the date field in the system globals.
             * This means that the date field for directory entries is more
             * likely to be correct, without manual intervention.
             */
            word the_date = LocalAddr(67);
            memory_emulate_setw(the_date, date_read, 0);
        }

        /*
         * Always consult ncurses(3) as to the size of the screen.
         */
        memory_emulate_setw(CRTINFO_WIDTH, crtinfo_width_read, 0);
        memory_emulate_setw(CRTINFO_HEIGHT, crtinfo_height_read, 0);

        /*
         * Many of the SYSCOM^.MISCINFO bits are required to match the
         * virtual machine.  So we emulate them, to be sure they are
         * correct.
         */
        memory_emulate_setw(MISCINFO, miscinfo_read, miscinfo_write);
    }

    MemWr(GDIRP, NIL);

    /*
     * The SYSCOM^.SYSUNIT field is set to the unit number of the volume
     * that SYSTEM.PASCAL was found on.  This affects things like the
     * expected location of files, such as SYSTEM.LIBRARY and other
     * system data files.
     */
    MemWr(SYSUNIT, root_unit);

#ifndef WORD_MEMORY
    if (AppleCompatibility)
    {
        MemWr(WordIndexed(Syscom, 161), 0x4bd);
        MemWr(WordIndexed(Syscom, 166), 0x6);
        /* Bit field with unknown contents */
        MemWrByte(WordIndexed(Syscom, 169), 1, 0x81);
    }
#endif

    Processor();

    if (always_dump_core)
        DumpCore();

    while (Unit > 4)
    {
        Unit--;
        if (Unit == 8)
            Unit = 5;
        DiskUmount(Unit);
    }
    TermClose();
    if (TraceFile)
        fclose(TraceFile);
    return (0);
}
