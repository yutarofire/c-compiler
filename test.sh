#!/bin/bash
cc -o 9cc 9cc.c

assert() {
  expected=$1
  input=$2

  ./9cc "$input" > tmp.s
  cc -o tmp tmp.s
  ./tmp
  actual=$?
  rm tmp tmp.s

  if [ "$actual" = "$expected" ]; then
    echo "$input => $actual"
  else
    echo "$input => $expected expected, but got $actual"
    exit 1
  fi
}

assert 0 0
assert 42 42

rm 9cc

echo OK
