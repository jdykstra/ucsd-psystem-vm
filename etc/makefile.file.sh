#!/bin/sh
#
# UCSD p-System virtual machine
# Copyright (C) 2006, 2010 Peter Miller
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or (at
# your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program. If not, see <http://www.gnu.org/licenses/>.
#
depfile=no-such-file-or-directory
case $# in
2)
    ;;
3)
    depfile="$3";
    ;;
*)
    echo "usage: $0 filename resolved-filename" 1>&2
    exit 1
    ;;
esac
file="$1"
rfn="$2"

case $file in

*/*.y)
    root=`basename $file .y`
    stem=`echo $file | sed 's/\.y$//'`
    dir=`dirname $file`

    case $file in
    *)
        numconf="no"
        ;;
    esac

    yy=`echo $stem | sed -e 's|^[^/]*/||' -e 's|[^a-zA-Z0-9]|_|g'`
    echo ""
    echo "${stem}.gen.c ${stem}.gen.h: $file"
    echo "      @echo Expect $numconf conflicts:"
    echo "      \$(YACC) -d $file"
    echo "      sed -e 's/[yY][yY]/${yy}_/g' y.tab.c > ${stem}.gen.c"
    echo "      sed -e 's/[yY][yY]/${yy}_/g' y.tab.h > ${stem}.gen.h"
    echo "      rm y.tab.c y.tab.h"
    ;;

*/*.c)
    root=`basename $file .c`
    stem=`echo $file | sed 's/\.c$//'`
    dir=`dirname $file`

    dep=
    if [ -r $depfile ]; then
        dep=`sed -e 's_.arch]/__' $depfile`
    fi

    echo ""
    echo "${stem}.o: $file" $dep
    echo "      \$(CC) \$(CPPFLAGS) \$(CFLAGS) -I. -c $file"
    echo "      mv ${root}.o ${stem}.o"
    ;;

man/man[0-9]/*.[0-9])
    dir=`dirname $file`
    stem=`echo $file | sed 's|^man/\(.*\)|\1|'`

    dep=
    if [ -r $depfile ]; then
        dep=`sed -e 's_.arch]/__' $depfile`
    fi

    echo ""
    echo "\$(mandir)/$stem: $file" $dep $dir/.mandir
    echo "      \$(SOELIM) -I. -Ietc $file > tmp"
    echo "      \$(INSTALL_DATA) tmp \$@"
    echo "      @rm -f tmp"
    ;;

etc/*.man)
    base=`echo $file | sed 's|etc/\(.*\).man$|\1|'`

    dep=
    if [ -r $depfile ]; then
        dep=`sed -e 's_.arch]/__' $depfile`
    fi

    echo ""
    echo "etc/$base.ps: $file" $dep
    echo "      \$(SOELIM) -Ietc -Iman/man1 -Iman/man5 $file |" \
        "\$(GROFF) -t -man > \$@"

    echo ""
    echo "etc/$base.dvi: $file" $dep
    echo "      \$(SOELIM) -Ietc -Iman/man1 -Iman/man5 $file |" \
        "\$(GROFF) -Tdvi -t -man > \$@"

    echo ""
    echo "etc/$base.txt: $file" $dep
    echo "      \$(SOELIM) -Ietc -Iman/man1 -Iman/man5 $file |" \
        "\$(GROFF) -Tascii -t -man > \$@"
    ;;

test/*/*.sh)
    root=`basename $file .sh`
    echo ""
    echo "$root: $file all"
    echo "      \$(SH) $file"
    ;;

*)
    ;;
esac
exit 0
