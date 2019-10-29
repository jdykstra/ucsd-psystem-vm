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

TEST_SUBJECT="LES POWR"
. test_prelude

cat > example.text << 'fubar'
(*$U-*)
(*$feature extra-set-comparisons true*)
(*$warning set-comparisons false*)
program example;

  type set40 = set of 0..63;

  procedure execerror;
  begin
    halt;
  end;

  procedure print(s: string);
  var
    newline: char;
  begin
    unitwrite(1, s[1], length(s));
    newline := chr(13);
    unitwrite(1, newline, 2);
  end;

  procedure lt(a, b: set40; expect: boolean);
  begin
    if ((a < b) = expect)
    then
      print('pass')
    else
      print('fail');
  end;

begin
  lt([1, 4, 7, 31, 37, 52], [1, 4, 7, 31, 37], false);
  lt([1, 4, 7, 31, 37    ], [1, 4, 7, 31, 37], false);
  lt([1, 4, 7, 31        ], [1, 4, 7, 31, 37], true);
end.
fubar
test $? -eq 0 || no_result

cat > test.ok << 'fubar'
pass
pass
pass

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
