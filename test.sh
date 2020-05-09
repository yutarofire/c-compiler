#!/bin/bash
assert() {
  expected=$1
  input=$2

  ./9cc "$input" > tmp.s
  cc -o tmp tmp.s
  ./tmp
  actual=$?
  rm tmp.s tmp

  if [ "$actual" = "$expected" ]; then
    echo "\"$input\" => $actual"
  else
    echo "$input => $expected expected, but got $actual"
    exit 1
  fi
}

assert 0 "0;"
assert 42 "42;"
assert 5 "2+3;"
assert 102 "31+71;"
assert 26 "22 + 4;"
assert 38 "22 +4+ 12;"
assert 18 "22 - 4;"
assert 30 "36 +4 - 10;"
assert 99 " 102- 5 + 2;"
assert 7 "1 + 2 * 3;"
assert 4 "(3+5)/2;"
assert 4 "(3+6)/2;"
assert 9 "(1 + 2) * 3;"
assert 2 "3 + -1;"
assert 2 "3 + (-1);"
assert 1 "4 + -1 * 3;"
assert 9 "(4 + -1) * 3;"
assert 0 "4 == -1;"
assert 1 "4 == 4;"
assert 1 "4 == 1 + 3;"
assert 0 "4 != 1 + 3;"
assert 0 "4 == 2 + 3;"
assert 0 "1 + 3 == 2 + 3;"
assert 1 "1+2*2==2+3;"
assert 0 "1 + 4 > 2 + 3;"
assert 0 "1 + 4 < 2 + 3;"
assert 1 "1 + 4 <= 2 + 3;"
assert 0 "1 + 3 >= 2 + 3;"
assert 1 "1 + 4 >= 2 + 3;"
assert 7 "1+2; 3+4;"
assert 5 "a=1; a+4;"
assert 0 "a=2==3; a;"
assert 8 "r=1+2*3; 1+r;"

echo OK
