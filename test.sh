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
    echo "$input => $actual"
  else
    echo "$input => $expected expected, but got $actual"
    exit 1
  fi
}

assert 0 0
assert 42 42
assert 5 "2+3"
assert 102 "31+71"

echo OK
