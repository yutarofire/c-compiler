#!/bin/bash
cat <<EOF | gcc -xc -c -o tmp2.o -
int ret3() { return 3; }
int ret5() { return 5; }
int add(int x, int y) { return x+y; }
int sub(int x, int y) { return x-y; }
int add6(int a, int b, int c, int d, int e, int f) {
  return a+b+c+d+e+f;
}
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
    echo "'$input' => $actual"
  else
    echo "'$input' => $expected expected, but got $actual"
    exit 1
  fi
}

assert 0 'main() { return 0; }'
assert 42 'main() { return 42; }'
assert 5 'main() { return 2+3; }'
assert 102 'main() { return 31+71; }'
assert 26 'main() { return 22 + 4; }'
assert 38 'main() { return 22 +4+ 12; }'
assert 18 'main() { return 22 - 4; }'
assert 30 'main() { return 36 +4 - 10; }'
assert 99 'main() { return  102- 5 + 2; }'
assert 6 'main() { return 2*3; }'
assert 4 'main() { return 8/2; }'
assert 7 'main() { return 1+2*3; }'
assert 5 'main() { return 1+8/2; }'
assert 4 'main() { return (3+5)/2; }'
assert 4 'main() { return (3+6)/2; }'
assert 9 'main() { return (1 + 2) * 3; }'
assert 2 'main() { return 3 + -1; }'
assert 2 'main() { return 3 + (-1); }'
assert 1 'main() { return 4 + -1 * 3; }'
assert 9 'main() { return (4 + -1) * 3; }'
assert 0 'main() { return 4 == -1; }'
assert 1 'main() { return 4 == 4; }'
assert 1 'main() { return 4 == 1 + 3; }'
assert 0 'main() { return 4 != 1 + 3; }'
assert 0 'main() { return 4 == 2 + 3; }'
assert 0 'main() { return 1 + 3 == 2 + 3; }'
assert 1 'main() { return 1+2*2==2+3; }'
assert 0 'main() { return 1 + 4 > 2 + 3; }'
assert 0 'main() { return 1 + 4 < 2 + 3; }'
assert 1 'main() { return 1 + 4 <= 2 + 3; }'
assert 0 'main() { return 1 + 3 >= 2 + 3; }'
assert 1 'main() { return 1 + 4 >= 2 + 3; }'
assert 3 'main() { return 1+2; 3+4; }'

assert 5 'main() { a=1; return a+4; }'
assert 0 'main() { a=2==3; return a; }'
assert 8 'main() { r=1+2*3; return 1+r; }'
assert 14 'main() { foo=1+2*3; return foo*2; }'
assert 15 'main() { foo=1+2*3; bar=foo+1; return foo+bar; }'
assert 15 'main() { foo=1+2*3; faa=foo+1; return foo+faa; }'
assert 3 'main() { foo=1+2; return foo; }'
assert 3 'main() { foo=1+2; return foo; return foo+2; }'

assert 10 'main() { if (1) return 10; return 20; }'
assert 20 'main() { if (0) return 10; return 20; }'
assert 10 'main() { if (0+1) return 10; return 20; }'
assert 20 'main() { if (1-1) return 10; return 20; }'
assert 10 'main() { if (1) return 10; else return 20; return 30; }'
assert 20 'main() { if (0) return 10; else return 20; return 30; }'
assert 6 'main() { for (i=0; i<5; i=i+3) 10; return i; }'
assert 10 'main() { for (;;) return 10; }'
assert 6 'main() { i=0; while (i<5) i=i+3; return i; }'

assert 10 'main() { return 10; }'
assert 10 'main() { x=10; return x; }'
assert 10 'main() { if (1) { x=10; return x; } else { y=11; return y; } }'
assert 11 'main() { if (0) { x=10; return x; } else { y=11; return y; } }'
assert 12 'main() { if (0) { x=10; return x; } else {} return 12; }'

assert 3 'main() { return ret3(); }'
assert 5 'main() { return ret5(); }'
assert 4 'main() { return add(1, 3); }'
assert 2 'main() { return sub(3, 1); }'
assert 21 'main() { return add6(1,2,3,4,5,6); }'

assert 42 'main() { return ret42(); } ret42() { return 42; }'
assert 5 'main() { return plus3(2); } plus3(x) { return x+3; }'
assert 12 'main() { return mul(3,4); } mul(x, y) { return x*y; }'
assert 12 'main() { return mul(1+2,4); } mul(x, y) { return x*y; }'
assert 192 'main() { return mul6(3,4,1,1,2,8); } mul6(a,b,c,d,e,f) { return a*b*c*d*e*f; }'
assert 144 'main() { return fib(12); } fib(n) { if (n==0) return 0; if (n==1) return 1; return fib(n-2)+fib(n-1); }'

assert 3 'main() { x=3; return *&x; }'
assert 3 'main() { x=3; y=&x; z=&y; return **z; }'
assert 5 'main() { x=3; y=&x; *y=5; return x; }'

assert 5 'main() { x=3; y=5; z=&x+1; return *z; }'
assert 5 'main() { x=3; y=5; return *(&x+1); }'
assert 3 'main() { x=3; y=5; return *(&y-1); }'
assert 7 'main() { x=3; y=5; *(&y-1)=7; return x; }'
assert 7 'main() { x=3; y=5; *(&x+1)=7; return y; }'
assert 2 'main() { x=3; return (&x+2)-&x; }'

echo OK
