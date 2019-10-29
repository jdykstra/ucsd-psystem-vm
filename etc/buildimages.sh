#!/bin/sh
#
# UCSD p-System virtual machine
# Copyright (C) 2000 Mario Klebsch
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License along
# with this program. If not, see <http://www.gnu.org/licenses/>
#

dd if=/dev/zero count=1024 of=work.svol
dd if=/dev/zero count=1024 of=system.svol

TODAY=`LANG= date +%d-%b-%y`

./ucsd -b -\
        -f apple_pascal_1.dsk \
        -r apple_pascal_2.dsk \
        -r apple_pascal_3.dsk \
        -r library.svol \
        -w work.svol \
        -w system.svol  <<**EOF
fd$TODAY
z#11
n1024
work:
yz#12
n1024
system:
yqxapple3:setup
csHAS LOWER CASE
yT
nsSCREEN WIDTH
y80
nsKEY TO MOVE CURSOR DOWN
y10
nsKEY TO MOVE CURSOR UP
y11
nqqmde
xapple3:library
work:system.library
apple1:system.library
0 0
1 1
2 2
3 3
6 6
nlibrary:turt.lib.code
1 4
2 5
qmodified turtlegraphics for UNIX based p-system
ftapple1:new.miscinfo,work:system.miscinfo,apple1:system.pascal,system:$,apple1:system.filer,system:$,apple1:system.editor,system:$,apple1:system.syntax,work:$,apple2:system.compiler,system:$,apple2:system.linker,system:$
tapple2:system.assmbler,system:$,apple2:6500.opcodes,system:$,apple2:6500.errors,system:$,apple3:library.code,system:$,apple3:libmap.code,system:$,apple3:setup.code,system:$,apple3:balanced.=,system:$
tapple3:crossref.=,system:$,apple3:diskio.=,system:$,apple3:grafchars.=,system:$,apple3:grafdemo.=,system:$,apple3:hilbert.=,system:$,apple3:spirodemo.=,system:$,apple3:tree.=,system:$
q
**EOF
echo
