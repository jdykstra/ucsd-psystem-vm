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

#ifndef UCSDPSYS_VM_TURTLEGR_H
#define UCSDPSYS_VM_TURTLEGR_H

/**
  * The GraphicsClear function is used to clear the graphics screen, if
  * present.
  */
void GraphicsClear(void);

/**
  * The TurtleGraphics function is used to implement UNIT
  * TURTLEGRAPHICS.
  *
  * @param entry_point
  *     The base address of the relevant turtle graphics procedure or
  *     function.
  */
void TurtleGraphics(word entry_point);

#endif /* UCSDPSYS_VM_TURTLEGR_H */
