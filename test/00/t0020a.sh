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

TEST_SUBJECT="inline-math LN"
. test_prelude

cat > example.text << 'fubar'
(*$feature inline-math true*)
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

  procedure print_real(x: real);
  var
    s: string[12];
    j: integer;
    i: integer;
  begin
    (*
     * NOTE: this code rounds DOWN; this should
     * avoid rounding false negatives.
     *)
    s[0] := chr(8);
    s[1] := '+';
    if x < 0 then
      begin
        s[1] := '-';
        x := -x;
      end;
    i := trunc(x);
    s[2] := chr(ord('0') + i);
    x := x - i;
    s[3] := '.';
    for j := 4 to 8 do
      begin
        x := x * 10;
        i := trunc(x);
        s[j] := chr(ord('0') + i);
        x := x - i;
      end;
    println(s);
  end;

  procedure chk_ln(x: real);
  var
    val: real;
  begin
    val := ln(x);
    print_real(val);
  end;

begin
  chk_ln(0.001);
  chk_ln(0.2);
  chk_ln(0.4);
  chk_ln(0.6);
  chk_ln(0.8);
  chk_ln(1.0);
  chk_ln(1.2);
  chk_ln(1.4);
  chk_ln(1.6);
  chk_ln(1.8);
end.
fubar
test $? -eq 0 || no_result

cat > test.ok << 'fubar'
-6.90775
-1.60943
-0.91629
-0.51082
-0.22314
+0.00000
+0.18232
+0.33647
+0.47000
+0.58778

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
