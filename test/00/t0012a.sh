#!/bin/sh
#
# UCSD p-System virtual machine
# Copyright (C) 2010 Peter Miller
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or (at
# you option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
# General Public License for more details.
#
# You should have received a copy of the GNU General Public License along
# with this program. If not, see <http://www.gnu.org/licenses/>
#

TEST_SUBJECT="CSP ROUND"
. test_prelude

cat > example.text << 'fubar'
(*$U-*)
program example;

  procedure execerror;
  begin
    halt;
  end;

  procedure print(s: string);
  begin
    unitwrite(1, s[1], length(s));
  end;

  procedure println(s: string);
  var
    newline: char;
  begin
    print(s);
    newline := chr(13);
    unitwrite(1, newline, 2);
  end;

  procedure chk(b: boolean);
  begin
    if b then
      println(': pass')
    else
      println(': fail')
  end;

  procedure rnd(a, b, expect: integer);
  begin
    chk(round(a / b) = expect);
  end;

  procedure runnit;
  begin
    print('round(0) = 0');
    rnd(0, 1, 0);

    print('round(1/2) = 1');
    rnd(1, 2, 1);

    print('round(3/4) = 1');
    rnd(3, 4, 1);

    print('round(5/2) = 3');
    rnd(5, 2, 3);

    print('round(7/2) = 4');
    rnd(7, 2, 4);

    print('round(9999/1000) = 10');
    rnd(9999, 1000, 10);

    print('round(-1/4) = 0');
    rnd(-1, 4, 0);

    print('round(-3/4) = -1');
    rnd(-3, 4, -1);

    print('round(-198/100) = -2');
    rnd(-198, 100, -2);
  end;

begin
  runnit;
end.
fubar
test $? -eq 0 || no_result

cat > test.ok << 'fubar'
round(0) = 0: pass
round(1/2) = 1: pass
round(3/4) = 1: pass
round(5/2) = 3: pass
round(7/2) = 4: pass
round(9999/1000) = 10: pass
round(-1/4) = 0: pass
round(-3/4) = -1: pass
round(-198/100) = -2: pass

fubar
test $? -eq 0 || no_result

ucsdpsys_compile example.text
test $? -eq 0 || no_result

ucsdpsys_mkfs -Lsystem example.vol
test $? -eq 0 || no_result

ucsdpsys_disk -f example.vol --put system.pascal=example.code
test $? -eq 0 || no_result

ucsdpsys_vm -b- -w example.vol < /dev/null > test.out 2>&1
if test $? -ne 0
then
    cat test.out
    fail
fi

diff test.ok test.out
test $? -eq 0 || fail

#
# The functionality exercised by this test worked.
# No other assertions are made.
#
pass
