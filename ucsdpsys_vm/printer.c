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

#include <lib/psystem.h>
#include <lib/memory.h>

#include <ucsdpsys_vm/printer.h>


static FILE *Printer = NULL;


void
PrinterWrite(byte ch, word Mode)
{
    static int Dle = 0;

    if (!Printer)
    {
#ifdef PRINT_DEVICE
        Printer = fopen(PRINT_DEVICE, "w");
#else
        Printer = popen("lp -s", "w");
#endif
    }

    if (!Printer)
    {
    }
    else if (!Dle)
    {
        switch (ch)
        {
        case 0:
            if (!(Mode & 0x8))
                return;
            /* fall through... */

        case 13:
            /* carriage return */
            if (!(Mode & 0x4))
                ch = '\n';
            break;

        case 16:
            /* DLE prefix */
            if (!(Mode & 0x8))
            {
                Dle = 1;
                return;
            }
            break;
        }
        fputc(ch, Printer);
    }
    else
    {
        ch -= 0x20;
        while (ch > 0)
        {
            --ch;
            putc(' ', Printer);
        }
        Dle = 0;
    }
}


void
PrinterClear(void)
{
    if (Printer)
    {
#ifdef PRINT_DEVICE
        fclose(Printer);
#else
        pclose(Printer);
#endif
        Printer = NULL;
    }
}
