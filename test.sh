#!/bin/bash
cat <<EOF | gcc -xc -c -o tmp2.o -
int ret3() { return 3; }
int ret5() { return 5; }
EOF

assert() {
  expected=$1
  input=$2

  ./9cc "$input" > tmp.s
  cc -o tmp tmp.s tmp2.o
  ./tmp
  actual=$?
  rm tmp.s tmp

  if [ "$actual" = "$expected" ]; then
    echo "\"$input\" => $actual"
  else
    echo "\"$input\" => $expected expected, but got $actual"
    exit 1
  fi
}

assert 0 "{ return 0; }"
assert 42 "{ return 42; }"
assert 5 "{ return 2+3; }"
assert 102 "{ return 31+71; }"
assert 26 "{ return 22 + 4; }"
assert 38 "{ return 22 +4+ 12; }"
assert 18 "{ return 22 - 4; }"
assert 30 "{ return 36 +4 - 10; }"
assert 99 "{ return  102- 5 + 2; }"
assert 7 "{ return 1 + 2 * 3; }"
assert 4 "{ return (3+5)/2; }"
assert 4 "{ return (3+6)/2; }"
assert 9 "{ return (1 + 2) * 3; }"
assert 2 "{ return 3 + -1; }"
assert 2 "{ return 3 + (-1); }"
assert 1 "{ return 4 + -1 * 3; }"
assert 9 "{ return (4 + -1) * 3; }"
assert 0 "{ return 4 == -1; }"
assert 1 "{ return 4 == 4; }"
assert 1 "{ return 4 == 1 + 3; }"
assert 0 "{ return 4 != 1 + 3; }"
assert 0 "{ return 4 == 2 + 3; }"
assert 0 "{ return 1 + 3 == 2 + 3; }"
assert 1 "{ return 1+2*2==2+3; }"
assert 0 "{ return 1 + 4 > 2 + 3; }"
assert 0 "{ return 1 + 4 < 2 + 3; }"
assert 1 "{ return 1 + 4 <= 2 + 3; }"
assert 0 "{ return 1 + 3 >= 2 + 3; }"
assert 1 "{ return 1 + 4 >= 2 + 3; }"
assert 3 "{ return 1+2; 3+4; }"

assert 5 "{ a=1; return a+4; }"
assert 0 "{ a=2==3; return a; }"
assert 8 "{ r=1+2*3; return 1+r; }"
assert 14 "{ foo=1+2*3; return foo*2; }"
assert 15 "{ foo=1+2*3; bar=foo+1; return foo+bar; }"
assert 15 "{ foo=1+2*3; faa=foo+1; return foo+faa; }"
assert 3 "{ foo=1+2; return foo; }"
assert 3 "{ foo=1+2; return foo; return foo+2; }"

assert 10 "{ if (1) return 10; return 20; }"
assert 20 "{ if (0) return 10; return 20; }"
assert 10 "{ if (0+1) return 10; return 20; }"
assert 20 "{ if (1-1) return 10; return 20; }"
assert 10 "{ if (1) return 10; else return 20; return 30; }"
assert 20 "{ if (0) return 10; else return 20; return 30; }"
assert 6 "{ for (i=0; i<5; i=i+3) 10; return i; }"
assert 10 "{ for (;;) return 10; }"
assert 6 "{ i=0; while (i<5) i=i+3; return i; }"

assert 10 "{ return 10; }"
assert 10 "{ x=10; return x; }"
assert 10 "{ if (1) { x=10; return x; } else { y=11; return y; } }"
assert 11 "{ if (0) { x=10; return x; } else { y=11; return y; } }"
assert 12 "{ if (0) { x=10; return x; } else {} return 12; }"

assert 3 "{ return ret3(); }"
assert 5 "{ return ret5(); }"

echo OK
