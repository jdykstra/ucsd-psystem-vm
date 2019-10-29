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

TEST_SUBJECT="idsearch"
. test_prelude

cat > example.text << 'fubar'
(*$U-*)
(*$warning shadow false*)
program example;

  procedure execerror;
  begin
    halt;
  end;

  procedure print(s: string);
  begin
    unitwrite(1, s[1], length(s));
  end;

  procedure chk(b: boolean);
  var
    crlf: char;
  begin
    if b then
      print(' ok')
    else
      print(' wrong');
    crlf := chr(13);
    unitwrite(1, crlf, 2);
  end;

  procedure myconcat(var frontstring: string; rearstring: string);
    var frontlength, rearlength, chcntr: integer;
  begin
    frontlength := ord(frontstring[0]);
    rearlength := ord(rearstring[0]);
    for chcntr := 1 to rearlength do
      frontstring[frontlength + chcntr] := rearstring[chcntr];
    frontstring[0] := chr(frontlength + rearlength);
  end;

  procedure ids;
  type
    cursrange = 0..1023;
    symbol = (ident,comma,colon,semicolon,lparent,rparent,dosy,tosy,
              downtosy,endsy,untilsy,ofsy,thensy,elsesy,becomes,lbrack,
              rbrack,arrow,period,beginsy,ifsy,casesy,repeatsy,whilesy,
              forsy,withsy,gotosy,labelsy,constsy,typesy,varsy,procsy,
              funcsy,progsy,forwardsy,intconst,realconst,stringconst,
              notsy,mulop,addop,relop,setsy,packedsy,arraysy,
              recordsy,filesy,othersy,longconst,usessy,unitsy,intersy,
              implesy,externsy,sepsy);
    operator = (mul,rdiv,andop,idiv,imod,plus,minus,orop,ltop,leop,
                geop,gtop,neop,eqop,inop,nop);
    alpha = packed array [1..8] of char;
    ctp = ^ identifier;
    identifier =
      record
        name: alpha;
        llink,rlink:ctp;
      end;
    symbufarray = packed array [cursrange] of char;

  var
    symbufp: ^symbufarray;
    symcursor:  cursrange;
    sy: symbol;
    op: operator;
    id: alpha;

    reservedwords:
      array[0..200] of
        record
          name: alpha;
          reserveop: operator;
          reservesy: symbol;
        end;
    size, cursnum, j, i, maxreserve: integer;
    descrip, tempdescrip, wordstring, lid: string;

  procedure myinit;
  var wordcnt:integer;
       i: integer;
       lid: string;

    procedure stuff(newname:alpha;newsy:symbol;newop:operator);
    begin
      with reservedwords[maxreserv] do
      begin
        name:=newname;
        reserveop:=newop;
        reservesy:=newsy;
      end;
      maxreserve:=maxreserve+1;
    end;

  begin
    stuff('AND     ',mulop,andop);
    stuff('ARRAY   ',arraysy,nop);
    stuff('BEGIN   ',beginsy,nop);
    stuff('CASE    ',casesy,nop);
    stuff('CONST   ',constsy,nop);
    stuff('DIV     ',mulop,idiv);
    stuff('DO      ',dosy,nop);
    stuff('DOWNTO  ',downtosy,nop);
    stuff('END     ',endsy,nop);
    stuff('ELSE    ',elsesy,nop);
    stuff('FOR     ',forsy,nop);
    stuff('FILE    ',filesy,nop);
    stuff('FORWARD ',forwardsy,nop);
    stuff('FUNCTION',funcsy,nop);
    stuff('GOTO    ',gotosy,nop);
    stuff('IF      ',ifsy,nop);
    stuff('IN      ',relop,inop);
    stuff('LABEL   ',labelsy,nop);
    stuff('MOD     ',mulop,imod);
    stuff('NOT     ',notsy,nop);
    stuff('OF      ',ofsy,nop);
    stuff('OR      ',addop,orop);
    stuff('PROCEDUR',procsy,nop);
    stuff('PROGRAM ',progsy,nop);
    stuff('PACKED  ',packedsy,nop);
    stuff('REPEAT  ',repeatsy,nop);
    stuff('RECORD  ',recordsy,nop);
    stuff('SET     ',setsy,nop);
    stuff('SEGMENT ',progsy,nop);
    stuff('THEN    ',thensy,nop);
    stuff('TO      ',tosy,nop);
    stuff('TYPE    ',typesy,nop);
    stuff('UNTIL   ',untilsy,nop);
    stuff('VAR     ',varsy,nop);
    stuff('WHILE   ',whilesy,nop);
    stuff('WITH    ',withsy,nop);
    stuff('USES    ',usessy,nop);
    stuff('UNIT    ',unitsy,nop);
    stuff('IMPLEMEN',implesy,nop);
    stuff('INTERFAC',intersy,nop);
    stuff('EXTERNAL',externsy,nop);
    stuff('SEPARATE',sepsy,nop);
  end;

  procedure doidschk(name:string;goodsy:symbol;goodop:operator;
                     goodid:alpha;chkid:boolean;wordsize:integer);
  var descrip:string;
  begin
    for j:=1 to length(name) do
      symbufp^[j+cursnum-1]:=name[j];
    symbufp^[length(name)+cursnum]:=' ';
    symcursor:=cursnum;
    descrip:='IDS  on "';
    myconcat(descrip,name);
    tempdescrip:=descrip;
    myconcat(tempdescrip,'"  chking SYMCURSOR value after IDS');
    print(tempdescrip);

    idsearch(symcursor,symbufp^);

    chk(symcursor=cursnum+wordsize-1);
    tempdescrip:=descrip;
    myconcat(tempdescrip,'"  chking for correct symbol value');
    print(tempdescrip);
    chk(sy=goodsy);
    tempdescrip:=descrip;
    if goodsy in [mulop,relop,addop] then
    begin
      myconcat(tempdescrip,'"  chking for correct operator value');
      print(tempdescrip);
      chk(op=goodop);
    end;
    if chkid then
    begin
      myconcat(descrip, '"  chking for correct ID value');
      print(descrip);
      chk(id=goodid);
    end;
    cursnum:=cursnum+1;
  end;

  begin
    new(symbufp);
    fillchar(symbufp^,1024,' ');
    maxreserve:=0;
    cursnum:=0;
    myinit;

    (*check all reserved words for correct operator and symbol values*)
    for i:=0 to maxreserve-1 do
      with reservedwords[i] do
      begin
        wordstring:='        ';
        for j:=1 to 8 do
        begin
          wordstring[j]:=name[j];
          if name[j]<>' ' then
            size:=j;
        end;
        doidschk(wordstring,reservesy,reserveop,name,false,size);
      end;

    doidschk('SMALL;', ident, nop, 'SMALL   ', true, 5);
    doidschk('A', ident, nop, 'A       ', true, 1);
    doidschk('A2345678901234567890', ident, nop, 'A2345678', true, 20);
    doidschk('sChLuMp', ident, nop, 'SCHLUMP ', true, 7);
    doidschk('WitH', withsy, nop, '        ', false, 4);
    doidschk('UNDER_SCORE', ident, nop, 'UNDERSCO', true, 11);
  end;

begin
  ids;
end.
fubar
test $? -eq 0 || no_result

cat > test.ok << 'fubar'
IDS  on "AND     "  chking SYMCURSOR value after IDS ok
IDS  on "AND     "  chking for correct symbol value ok
IDS  on "AND     "  chking for correct operator value ok
IDS  on "ARRAY   "  chking SYMCURSOR value after IDS ok
IDS  on "ARRAY   "  chking for correct symbol value ok
IDS  on "BEGIN   "  chking SYMCURSOR value after IDS ok
IDS  on "BEGIN   "  chking for correct symbol value ok
IDS  on "CASE    "  chking SYMCURSOR value after IDS ok
IDS  on "CASE    "  chking for correct symbol value ok
IDS  on "CONST   "  chking SYMCURSOR value after IDS ok
IDS  on "CONST   "  chking for correct symbol value ok
IDS  on "DIV     "  chking SYMCURSOR value after IDS ok
IDS  on "DIV     "  chking for correct symbol value ok
IDS  on "DIV     "  chking for correct operator value ok
IDS  on "DO      "  chking SYMCURSOR value after IDS ok
IDS  on "DO      "  chking for correct symbol value ok
IDS  on "DOWNTO  "  chking SYMCURSOR value after IDS ok
IDS  on "DOWNTO  "  chking for correct symbol value ok
IDS  on "END     "  chking SYMCURSOR value after IDS ok
IDS  on "END     "  chking for correct symbol value ok
IDS  on "ELSE    "  chking SYMCURSOR value after IDS ok
IDS  on "ELSE    "  chking for correct symbol value ok
IDS  on "FOR     "  chking SYMCURSOR value after IDS ok
IDS  on "FOR     "  chking for correct symbol value ok
IDS  on "FILE    "  chking SYMCURSOR value after IDS ok
IDS  on "FILE    "  chking for correct symbol value ok
IDS  on "FORWARD "  chking SYMCURSOR value after IDS ok
IDS  on "FORWARD "  chking for correct symbol value ok
IDS  on "FUNCTION"  chking SYMCURSOR value after IDS ok
IDS  on "FUNCTION"  chking for correct symbol value ok
IDS  on "GOTO    "  chking SYMCURSOR value after IDS ok
IDS  on "GOTO    "  chking for correct symbol value ok
IDS  on "IF      "  chking SYMCURSOR value after IDS ok
IDS  on "IF      "  chking for correct symbol value ok
IDS  on "IN      "  chking SYMCURSOR value after IDS ok
IDS  on "IN      "  chking for correct symbol value ok
IDS  on "IN      "  chking for correct operator value ok
IDS  on "LABEL   "  chking SYMCURSOR value after IDS ok
IDS  on "LABEL   "  chking for correct symbol value ok
IDS  on "MOD     "  chking SYMCURSOR value after IDS ok
IDS  on "MOD     "  chking for correct symbol value ok
IDS  on "MOD     "  chking for correct operator value ok
IDS  on "NOT     "  chking SYMCURSOR value after IDS ok
IDS  on "NOT     "  chking for correct symbol value ok
IDS  on "OF      "  chking SYMCURSOR value after IDS ok
IDS  on "OF      "  chking for correct symbol value ok
IDS  on "OR      "  chking SYMCURSOR value after IDS ok
IDS  on "OR      "  chking for correct symbol value ok
IDS  on "OR      "  chking for correct operator value ok
IDS  on "PROCEDUR"  chking SYMCURSOR value after IDS ok
IDS  on "PROCEDUR"  chking for correct symbol value ok
IDS  on "PROGRAM "  chking SYMCURSOR value after IDS ok
IDS  on "PROGRAM "  chking for correct symbol value ok
IDS  on "PACKED  "  chking SYMCURSOR value after IDS ok
IDS  on "PACKED  "  chking for correct symbol value ok
IDS  on "REPEAT  "  chking SYMCURSOR value after IDS ok
IDS  on "REPEAT  "  chking for correct symbol value ok
IDS  on "RECORD  "  chking SYMCURSOR value after IDS ok
IDS  on "RECORD  "  chking for correct symbol value ok
IDS  on "SET     "  chking SYMCURSOR value after IDS ok
IDS  on "SET     "  chking for correct symbol value ok
IDS  on "SEGMENT "  chking SYMCURSOR value after IDS ok
IDS  on "SEGMENT "  chking for correct symbol value ok
IDS  on "THEN    "  chking SYMCURSOR value after IDS ok
IDS  on "THEN    "  chking for correct symbol value ok
IDS  on "TO      "  chking SYMCURSOR value after IDS ok
IDS  on "TO      "  chking for correct symbol value ok
IDS  on "TYPE    "  chking SYMCURSOR value after IDS ok
IDS  on "TYPE    "  chking for correct symbol value ok
IDS  on "UNTIL   "  chking SYMCURSOR value after IDS ok
IDS  on "UNTIL   "  chking for correct symbol value ok
IDS  on "VAR     "  chking SYMCURSOR value after IDS ok
IDS  on "VAR     "  chking for correct symbol value ok
IDS  on "WHILE   "  chking SYMCURSOR value after IDS ok
IDS  on "WHILE   "  chking for correct symbol value ok
IDS  on "WITH    "  chking SYMCURSOR value after IDS ok
IDS  on "WITH    "  chking for correct symbol value ok
IDS  on "USES    "  chking SYMCURSOR value after IDS ok
IDS  on "USES    "  chking for correct symbol value ok
IDS  on "UNIT    "  chking SYMCURSOR value after IDS ok
IDS  on "UNIT    "  chking for correct symbol value ok
IDS  on "IMPLEMEN"  chking SYMCURSOR value after IDS ok
IDS  on "IMPLEMEN"  chking for correct symbol value ok
IDS  on "INTERFAC"  chking SYMCURSOR value after IDS ok
IDS  on "INTERFAC"  chking for correct symbol value ok
IDS  on "EXTERNAL"  chking SYMCURSOR value after IDS ok
IDS  on "EXTERNAL"  chking for correct symbol value ok
IDS  on "SEPARATE"  chking SYMCURSOR value after IDS ok
IDS  on "SEPARATE"  chking for correct symbol value ok
IDS  on "SMALL;"  chking SYMCURSOR value after IDS ok
IDS  on "SMALL;"  chking for correct symbol value ok
IDS  on "SMALL;"  chking for correct ID value ok
IDS  on "A"  chking SYMCURSOR value after IDS ok
IDS  on "A"  chking for correct symbol value ok
IDS  on "A"  chking for correct ID value ok
IDS  on "A2345678901234567890"  chking SYMCURSOR value after IDS ok
IDS  on "A2345678901234567890"  chking for correct symbol value ok
IDS  on "A2345678901234567890"  chking for correct ID value ok
IDS  on "sChLuMp"  chking SYMCURSOR value after IDS ok
IDS  on "sChLuMp"  chking for correct symbol value ok
IDS  on "sChLuMp"  chking for correct ID value ok
IDS  on "WitH"  chking SYMCURSOR value after IDS ok
IDS  on "WitH"  chking for correct symbol value ok
IDS  on "UNDER_SCORE"  chking SYMCURSOR value after IDS ok
IDS  on "UNDER_SCORE"  chking for correct symbol value ok
IDS  on "UNDER_SCORE"  chking for correct ID value ok

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
