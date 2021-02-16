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

#include <lib/config.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#ifndef _AMIGA
#include <assert.h>
#include <curses.h>
#include <netinet/in.h>
#include <signal.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <term.h>
#include <termios.h>
#endif

#include <lib/psystem.h>
#include <lib/memory.h>
#include <ucsdpsys_vm/term.h>

#if defined(SIGWINCH) && defined(TIOCGWINSZ)
#define CAN_RESIZE 1
#else
#undef CAN_RESIZE
#error shit
#endif


#ifdef CAN_RESIZE

static int sigwinch_happened;


static RETSIGTYPE
sigwinch(int signum)
{
    (void)signum;
    sigwinch_happened = 1;
    signal(SIGWINCH, sigwinch);
}


static void
sigwinch_process(void)
{
    struct winsize  size;

    if (ioctl(fileno(stdout), TIOCGWINSZ, &size) == 0)
    {
        resize_term(size.ws_row, size.ws_col);
        wrefresh(curscr);
        LINES = size.ws_row;
        COLS = size.ws_col;
    }
    sigwinch_happened = 0;
}


static inline void
sigwinch_check(void)
{
    if (sigwinch_happened)
        sigwinch_process();
}

#endif /* CAN_RESIZE */


static int BatchMode = 0;
static int TermIn = -1;
static int TermOut = -1;
#ifndef _AMIGA
struct termios OldTerm;
#else
/*
 * Watch out for "\x" because it isn't exactly 2 hex digits long, and
 * becomes ambiguous for "\x9bC" and similar.  By using two strings,
 * they will be glued together for us by the C lexer.
 */
#define AMIGA_PREFIX "\x9b"
static char *clr_eol = AMIGA_PREFIX "K";
static char *clr_eos = AMIGA_PREFIX "J";
static char *clear_screen = AMIGA_PREFIX "2J";
static char *cursor_home = AMIGA_PREFIX "H";
static char *cursor_up = AMIGA_PREFIX "A";
static char *cursor_right = AMIGA_PREFIX "C";
static char *cursor_address = AMIGA_PREFIX "%d;%dH";
#define tgoto(cursor_address, x, y) s,sprintf(s, cursor_address, y+1, x+1)
#endif /* _AMIGA */


/*
 * erase line=0
 * erase to end of screen=11
 * erase screen=12
 * insert line=22
 * move cursor home=25
 * move cursor right=28
 * erase to end of line=29
 * gotoxy=30
 * move cursor up=31
 */

void
TermWrite(char ch, word Mode)
{
    static int state = 0;
    static int x;
    int y;
    char *s;

    switch (state)
    {
    case 0:
        switch (ch)
        {
        case 0:
            /* erase line ??? */
            if (!(Mode & 0x8))
                ;
            else
                write(TermOut, &ch, 1);
            break;

        case 7:
            if (!BatchMode)
                write(TermOut, &ch, 1);
            break;

        case 11:
            /* erase to end of screen */
            if (!BatchMode && clr_eos)
                write(TermOut, clr_eos, strlen(clr_eos));
            break;

        case 12:
            /* erase screen */
            if (!BatchMode && clear_screen)
            {
                write(TermOut, clear_screen,
                    strlen(clear_screen));
            }
            break;

        case 13:
            /* carriage return */
            if (!(Mode & 0x4))
            {
                if (!BatchMode)
                    write(TermOut, "\r\n", 2);
                else
                    write(TermOut, "\n", 1);
            }
            else
                write(TermOut, &ch, 1);
            break;

        case 16:
            /* DLE prefix */
            if (!(Mode & 0x8))
                state = 3;
            break;

        case 22:
            if (!BatchMode && insert_line)
                write(TermOut, insert_line, strlen(insert_line));
            break;

        case 25:
            /* move cursor home */
            if (!BatchMode && cursor_home)
                write(TermOut, cursor_home, strlen(cursor_home));
            else
                write(TermOut, "\n", 1);
            break;

        case 28:
            /* move cursor right */
            if (!BatchMode && cursor_right)
            {
                write(TermOut, cursor_right,
                    strlen(cursor_right));
            }
            break;

        case 29:
            /* erase to end of line */
            if (!BatchMode && clr_eol)
                write(TermOut, clr_eol, strlen(clr_eol));
            break;

        case 30:
            /* gotoxy */
            state = 1;
            break;

        case 31:
            /* move cursor up */
            if (!BatchMode && cursor_up)
                write(TermOut, cursor_up, strlen(cursor_up));
            break;

        default:
            write(TermOut, &ch, 1);
            break;
        }
        break;

    case 1:
        x = ch - ' ';
        state = 2;
        break;

    case 2:
        /* gotoxy complete */
        y = ch - ' ';
        if (!BatchMode && cursor_address)
        {
            s = tgoto(cursor_address, x, y);
            write(TermOut, s, strlen(s));
        }
        state = 0;
        break;

    case 3:
        /* char after DLE */
        ch -= 0x20;
        while (ch--)
            write(TermOut, " ", 1);
        state = 0;
        break;
    }
}


struct translator
{
    struct translator *Next;
    char *Pattern;
    byte Key;
    void (*Action) (byte);
};

static struct translator *Keys = NULL;


static void
AddKeyTranslation(byte Key, char *Pattern, void (*Action)(byte))
{
    struct translator *NewKey = malloc(sizeof(struct translator));
    assert(NewKey);
    NewKey->Key = Key;
    NewKey->Pattern = Pattern;
    NewKey->Action = Action;
    NewKey->Next = Keys;
    Keys = NewKey;
}


char
TermRead(void)
{
    unsigned char ch;

    static int state = 0;
    static int BufIdx = 0;
    static char Buffer[10];

    if (state == 0)
    {
        for (;;)
        {
            fd_set fds;
            struct timeval tv;
            FD_ZERO(&fds);
            FD_SET(TermIn, &fds);
            tv.tv_sec = 0;
            tv.tv_usec = 25000;

#if CAN_RESIZE
            sigwinch_check();
#endif
            if (select(TermIn + 1, &fds, NULL, NULL, BufIdx ? &tv : NULL) > 0)
            {
                /* a sign is available */
                struct translator *t;
                int Match = 0;

                if (read(TermIn, &ch, 1) < 1) /* Read and save in buffer */
                    if (BatchMode)
                        exit(0);

#ifndef XXX
                if (ch == '\0') /* XXX Quick-Hack :-( */
                    continue;
#endif

                Buffer[BufIdx++] = ch;
                for (t = Keys; t; t = t->Next)
                {
                    int i;
                    for (i = 0; i < BufIdx; i++)
                        if (Buffer[i] != t->Pattern[i])
                            goto next;
                    if (!(t->Pattern[BufIdx])) /* full Pattern match */
                    {
                        BufIdx = 0;
                        return (t->Key);
                    }
                    Match = 1;
                    break;
                  next:
                    ;
                }
                if (!Match)
                    goto ret;
            }
            else /* Timeout                          */
            {
              ret:
                if (BufIdx > 1)
                    state = 1;
                else
                    BufIdx = 0;
                return (Buffer[0]);
            }
        }
    }
    else
    {
        ch = Buffer[state++];
        if (state >= BufIdx)
        {
            state = 0;
            BufIdx = 0;
        }
        return (ch);
    }
}


int
TermStat(void)
{
    fd_set fds;
    struct timeval tv;
    FD_ZERO(&fds);
    FD_SET(TermIn, &fds);
    tv.tv_sec = 0;
    tv.tv_usec = 0;

    return (select(TermIn + 1, &fds, NULL, NULL, &tv) > 0);
}


static void
OpenXTerm(void)
{
    socklen_t i;
    int s;
    struct sockaddr_in addr;
    char Buffer[256];

    unsigned char TelnetInit[] = { 255, 253, 3, 255, 251, 3, 255, 251, 1 };

    s = socket(PF_INET, SOCK_STREAM, 0);
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    addr.sin_port = htons(0);
    bind(s, (struct sockaddr *)&addr, sizeof(addr));
    listen(s, 1);
    i = sizeof(addr);
    getsockname(s, (struct sockaddr *)&addr, &i);

    snprintf(Buffer, sizeof(Buffer), "xterm -geometry 80x24 -e "
        "telnet -E localhost %d&", ntohs(addr.sin_port));
    system(Buffer);

    i = sizeof(addr);
    TermOut = accept(s, (struct sockaddr *)&addr, &i);

    close(s);

    write(TermOut, TelnetInit, sizeof(TelnetInit));
    for (i = 0; i < sizeof(TelnetInit); i++)
    {
        if (read(TermOut, &s, 1) <= 0)
            break;
    }
}


void
TermOpen(int UseXTerm, int BatchFd)
{
    struct termios NewTerm;

#ifdef CAN_RESIZE
    signal(SIGWINCH, sigwinch);
#endif
    if (BatchFd < 0)
    {
        BatchMode = 0;
    }
    else
    {
        BatchMode = 1;
        TermIn = BatchFd;
    }

    if (UseXTerm)
    {
        OpenXTerm();
        setupterm("xterm", TermOut, NULL);
    }
    else
    {
        if (BatchMode)
        {
            TermOut = 1;
            setupterm("dumb", TermOut, NULL);
        }
        else
        {
            TermOut = open("/dev/tty", O_RDWR, 0666);
            setupterm(NULL, TermOut, NULL);
        }
    }

    if (TermIn < 0)
        TermIn = TermOut;

    if (BatchMode)
    {
        AddKeyTranslation('\r', "\n", NULL);
    }
    else
    {
        tcgetattr(TermOut, &OldTerm);
        NewTerm = OldTerm;
        NewTerm.c_iflag &= ~(PARMRK | ISTRIP | INLCR | IGNCR |
            ICRNL | IXOFF | IXON);
        NewTerm.c_oflag &= ~OPOST;
        NewTerm.c_lflag &= ~(ISIG | ICANON | ECHO | ECHONL | IEXTEN);
        NewTerm.c_cc[VMIN] = 1;
        NewTerm.c_cc[VTIME] = 0;
        tcsetattr(TermOut, TCSANOW, &NewTerm);

        if (init_1string)
            write(TermOut, init_1string, strlen(init_1string));
        if (init_2string)
            write(TermOut, init_2string, strlen(init_2string));
        if (init_3string)
            write(TermOut, init_3string, strlen(init_3string));
        if (enter_ca_mode)
            write(TermOut, enter_ca_mode, strlen(enter_ca_mode));

        AddKeyTranslation(0x0b, "\033[A", NULL);
        AddKeyTranslation(0x0a, "\033[B", NULL);
        AddKeyTranslation(0x15, "\033[C", NULL);
        AddKeyTranslation(0x08, "\033[D", NULL);

        AddKeyTranslation(0x0b, "\033[OA", NULL);
        AddKeyTranslation(0x0a, "\033[OB", NULL);
        AddKeyTranslation(0x15, "\033[OC", NULL);
        AddKeyTranslation(0x08, "\033[OD", NULL);

        if (key_up)
            AddKeyTranslation(0x0b, key_up, NULL);
        if (key_down)
            AddKeyTranslation(0x0a, key_down, NULL);
        if (key_right)
            AddKeyTranslation(0x15, key_right, NULL);
        if (key_left)
            AddKeyTranslation(0x08, key_left, NULL);
    }
}


void
TermClose(void)
{
#ifdef CAN_RESIZE
    signal(SIGWINCH, SIG_IGN);
#endif
    if (!BatchMode)
    {
        write(TermOut, "\r\n", 2);
        if (exit_ca_mode)
            write(TermOut, exit_ca_mode, strlen(exit_ca_mode));
        if (reset_1string)
            write(TermOut, reset_1string, strlen(reset_1string));
        if (reset_2string)
            write(TermOut, reset_2string, strlen(reset_2string));
        if (reset_3string)
            write(TermOut, reset_3string, strlen(reset_3string));
        tcsetattr(TermOut, TCSANOW, &OldTerm);
    }
    else
        write(TermOut, "\n", 1);
    close(TermOut);
    close(TermIn);
}


int
term_width(void)
{
    int             xcolumns;

    sigwinch_check();
    xcolumns = COLS;
    if (xcolumns < 1)
        xcolumns = 1;
    else if (xcolumns >= 32768)
        xcolumns = 32767;
    return xcolumns;
}


int
term_height(void)
{
    int             ylines;

#ifdef CAN_RESIZE
    sigwinch_check();
#endif
    ylines = LINES;
    if (ylines < 1)
        ylines = 1;
    else if (ylines >= 32768)
        ylines = 32767;
    return ylines;
}


int
term_is_batch_mode(void)
{
    return BatchMode;
}
