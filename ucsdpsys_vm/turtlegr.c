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

#include <libexplain/execlp.h>
#include <libexplain/fork.h>
#include <libexplain/socketpair.h>
#include <math.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <unistd.h>

#include <lib/memory.h>
#include <lib/pcode.h>
#include <lib/psystem.h>

#include <ucsdpsys_vm/stack.h>
#include <ucsdpsys_vm/turtlegr.h>

#ifdef TURTLEGRAPHICS

static FILE *TurtleServer = NULL;

#define TURTLE_SPEEDUP

#ifdef TURTLE_SPEEDUP
static Integer CurrentTurtleX;
static Integer CurrentTurtleY;
static Integer CurrentTurtleAng;
#endif


static void
StartTurtleServer(void)
{
    int fd[2];
    int pid;
    int i;

    if (explain_socketpair_on_error(PF_UNIX, SOCK_STREAM, 0, fd) < 0)
        return;
    pid = explain_fork_on_error();
    if (pid < 0)
        return;
    if (!pid)
    {
        if (fd[1] != 0)
        {
            close(0);
            dup2(fd[1], 0);
        }
        if (fd[1] != 1)
        {
            close(1);
            dup2(fd[1], 1);
        }
        for (i = getdtablesize(); i > 2; i--)
            close(i);
        explain_execlp_or_die
        (
            "ucsdpsys_xturtleserver",
            "ucsdpsys_xturtleserver",
            NULL
        );
    }
    close(fd[1]);
    TurtleServer = fdopen(fd[0], "wb+");
}


static void
tprintf(char *format, ...)
{
    va_list ap;
    if (!TurtleServer)
        StartTurtleServer();

    if (TurtleServer)
    {
        va_start(ap, format);
        vfprintf(TurtleServer, format, ap);
        va_end(ap);
        putc('\n', TurtleServer);
    }
}


static Integer
tgetint(void)
{
    int i = 0;
    char Buffer[20];

    fflush(TurtleServer);

    for (i = 0; i < 19; i++)
    {
        Buffer[i] = getc(TurtleServer);
        if (Buffer[i] == '\n')
            break;
    }

    Buffer[i] = '\0';

    sscanf(Buffer, "%d", &i);
    return ((Integer) i);
}


#ifdef TURTLE_SPEEDUP

static void
TurnTo(int Angle)
{
    CurrentTurtleAng = Angle;
    while (CurrentTurtleAng < 0)
        CurrentTurtleAng += 360;
    while (CurrentTurtleAng >= 360)
        CurrentTurtleAng -= 360;
}

#endif


void
TurtleGraphics(word EntryPoint)
{
    size_t i;
    char func[64];
    char *p;

    i = 0;
    for (i = 0; i < sizeof(func); i++)
    {
        if (!(func[i] = MemRdByte(EntryPoint, i)))
            break;
    }

    p = &func[16];
    if (strcmp(p, "INITTURTLE") == 0)
        tprintf("INITTURTLE");
    else if (strcmp(p, "TURN") == 0)
    {
        Integer j = PopInteger();
        tprintf("TURN %d", j);
#ifdef TURTLE_SPEEDUP
        TurnTo(CurrentTurtleAng + j);
#endif
    }
    else if (strcmp(p, "TURNTO") == 0)
    {
        Integer j = PopInteger();
        tprintf("TURNTO %d", j);
#ifdef TURTLE_SPEEDUP
        TurnTo(j);
#endif
    }
    else if (strcmp(p, "MOVE") == 0)
    {
        Integer j = PopInteger();
        tprintf("MOVE %d", j);
#ifdef TURTLE_SPEEDUP
        CurrentTurtleX += round(cos(CurrentTurtleAng * 3.14 / 180) * j);
        CurrentTurtleY += round(sin(CurrentTurtleAng * 3.14 / 180) * j);
#endif
    }
    else if (strcmp(p, "MOVETO") == 0)
    {
        Integer y = PopInteger();
        Integer x = PopInteger();
        tprintf("MOVETO %d %d", x, y);
#ifdef TURTLE_SPEEDUP
        CurrentTurtleX = x;
        CurrentTurtleY = y;
#endif
    }
    else if (strcmp(p, "PENCOLOR") == 0)
        tprintf("PENCOLOR %d", Pop());
    else if (strcmp(p, "TEXTMODE") == 0)
        tprintf("TEXTMODE");
    else if (strcmp(p, "GRAFMODE") == 0)
        tprintf("GRAFMODE");
    else if (strcmp(p, "FILLSCREEN") == 0)
        tprintf("FILLSCREEN %d", Pop());
    else if (strcmp(p, "VIEWPORT") == 0)
    {
        int yMax = PopInteger();
        int yMin = PopInteger();
        int xMax = PopInteger();
        int xMin = PopInteger();
        tprintf("VIEWPORT %d %d %d %d", xMin, xMax, yMin, yMax);
    }
    else if (strcmp(p, "TURTLEX") == 0)
    {
        Pop();
        Pop();
#ifdef TURTLE_SPEEDUP
        Push(CurrentTurtleX);
#else
        tprintf("TURTLEX");
        Push(tgetint());
#endif
    }
    else if (strcmp(p, "TURTLEY") == 0)
    {
        Pop();
        Pop();
#ifdef TURTLE_SPEEDUP
        Push(CurrentTurtleY);
#else
        tprintf("TURTLEY");
        Push(tgetint());
#endif
    }
    else if (strcmp(p, "TURTLEANG") == 0)
    {
        Pop();
        Pop();
#ifdef TURTLE_SPEEDUP
        Push(CurrentTurtleAng);
#else
        tprintf("TURTLEANG");
        Push(tgetint());
#endif
    }
    else if (strcmp(p, "SCREENBIT") == 0)
    {
        Integer y = PopInteger();
        Integer x = PopInteger();
        tprintf("SCREENBIT %d %d", x, y);
        Push(tgetint());
    }
    else if (strcmp(p, "DRAWBLOCK") == 0)
    {
        int x;
        int y;
        Integer Mode = PopInteger();
        Integer YScreen = PopInteger();
        Integer XScreen = PopInteger();
        Integer Height = PopInteger();
        Integer Width = PopInteger();
        Integer YSkip = PopInteger();
        Integer XSkip = PopInteger();
        Integer RowSize = PopInteger();
        word Source = Pop();

        tprintf("DRAWBLOCK %d %d %d %d %d %d %d %d",
            (((XSkip + Width + 7) & ~7) - (XSkip & ~7)) >> 3, XSkip & 7, 0,
            Width, Height, XScreen, YScreen, Mode);
        for (y = 0; y < Height; y++)
        {
            for (x = (XSkip & 7); x < ((XSkip + Width + 7) & ~7); x += 8)
            {
                putc(MemRdByte(Source, (y + YSkip) * RowSize + x / 8),
                    TurtleServer);
            }
        }
    }
    else if (strcmp(p, "WCHAR") == 0)
    {
        tprintf("WCHAR %d", Pop() & 0xff);
#ifdef TURTLE_SPEEDUP
        CurrentTurtleX += 7;
#endif
    }
    else if (strcmp(p, "WSTRING") == 0)
    {
        word Addr = Pop();
        int Len = MemRdByte(Addr, 0);
        int j;

        tprintf("WSTRING %d", Len);
        for (j = 0; j < Len; j++)
            putc(MemRdByte(Addr, j + 1), TurtleServer);
#ifdef TURTLE_SPEEDUP
        CurrentTurtleX += 7 * Len;
#endif
    }
    else if (strcmp(p, "CHARTYPE") == 0)
        tprintf("WCHAR %d", Pop() & 0x0f);
    else
        XeqError(XNOTIMP);
    fflush(TurtleServer);
}


void
GraphicsClear(void)
{
    int status;
    if (TurtleServer)
    {
        fclose(TurtleServer);
        TurtleServer = NULL;
        wait(&status);
    }
}

#endif /* TURTLEGRAPHICS */
