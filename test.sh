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

assert 0 'int main() { return 0; }'
assert 42 'int main() { return 42; }'
assert 5 'int main() { return 2+3; }'
assert 102 'int main() { return 31+71; }'
assert 26 'int main() { return 22 + 4; }'
assert 38 'int main() { return 22 +4+ 12; }'
assert 18 'int main() { return 22 - 4; }'
assert 30 'int main() { return 36 +4 - 10; }'
assert 99 'int main() { return  102- 5 + 2; }'
assert 6 'int main() { return 2*3; }'
assert 4 'int main() { return 8/2; }'
assert 7 'int main() { return 1+2*3; }'
assert 5 'int main() { return 1+8/2; }'
assert 4 'int main() { return (3+5)/2; }'
assert 4 'int main() { return (3+6)/2; }'
assert 9 'int main() { return (1 + 2) * 3; }'
assert 2 'int main() { return 3 + -1; }'
assert 2 'int main() { return 3 + (-1); }'
assert 1 'int main() { return 4 + -1 * 3; }'
assert 9 'int main() { return (4 + -1) * 3; }'
assert 0 'int main() { return 4 == -1; }'
assert 1 'int main() { return 4 == 4; }'
assert 1 'int main() { return 4 == 1 + 3; }'
assert 0 'int main() { return 4 != 1 + 3; }'
assert 0 'int main() { return 4 == 2 + 3; }'
assert 0 'int main() { return 1 + 3 == 2 + 3; }'
assert 1 'int main() { return 1+2*2==2+3; }'
assert 0 'int main() { return 1 + 4 > 2 + 3; }'
assert 0 'int main() { return 1 + 4 < 2 + 3; }'
assert 1 'int main() { return 1 + 4 <= 2 + 3; }'
assert 0 'int main() { return 1 + 3 >= 2 + 3; }'
assert 1 'int main() { return 1 + 4 >= 2 + 3; }'
assert 3 'int main() { return 1+2; 3+4; }'

assert 5 'int main() { int a; a=1; return a+4; }'
assert 6 'int main() { int a=2; return a+4; }'
assert 0 'int main() { int a=2==3; return a; }'
assert 8 'int main() { int r=1+2*3; return 1+r; }'
assert 14 'int main() { int foo=1+2*3; return foo*2; }'
assert 15 'int main() { int foo=1+2*3; int bar=foo+1; return foo+bar; }'
assert 15 'int main() { int foo=1+2*3; int faa=foo+1; return foo+faa; }'
assert 3 'int main() { int foo=1+2; return foo; }'
assert 3 'int main() { int foo=1+2; return foo; return foo+2; }'

assert 10 'int main() { if (1) return 10; return 20; }'
assert 20 'int main() { if (0) return 10; return 20; }'
assert 10 'int main() { if (0+1) return 10; return 20; }'
assert 20 'int main() { if (1-1) return 10; return 20; }'
assert 10 'int main() { if (1) return 10; else return 20; return 30; }'
assert 20 'int main() { if (0) return 10; else return 20; return 30; }'
assert 6 'int main() { for (int i=0; i<5; i=i+3) 10; return i; }'
assert 10 'int main() { for (;;) return 10; }'
assert 6 'int main() { int i=0; while (i<5) i=i+3; return i; }'

assert 10 'int main() { return 10; }'
assert 10 'int main() { int x=10; return x; }'
assert 10 'int main() { if (1) { int x=10; return x; } else { int y=11; return y; } }'
assert 11 'int main() { if (0) { int x=10; return x; } else { int y=11; return y; } }'
assert 12 'int main() { if (0) { int x=10; return x; } else {} return 12; }'

assert 3 'int main() { return ret3(); }'
assert 5 'int main() { return ret5(); }'
assert 4 'int main() { return add(1, 3); }'
assert 2 'int main() { return sub(3, 1); }'
assert 21 'int main() { return add6(1,2,3,4,5,6); }'

assert 42 'int main() { return ret42(); } int ret42() { return 42; }'
assert 5 'int main() { return plus3(2); } int plus3(int x) { return x+3; }'
assert 12 'int main() { return mul(3,4); } int mul(int x, int y) { return x*y; }'
assert 12 'int main() { return mul(1+2,4); } int mul(int x, int y) { return x*y; }'
assert 192 'int main() { return mul6(3,4,1,1,2,8); } int mul6(int a, int b, int c, int d, int e, int f) { return a*b*c*d*e*f; }'
assert 144 'int main() { return fib(12); } int fib(int n) { if (n==0) return 0; if (n==1) return 1; return fib(n-2)+fib(n-1); }'

assert 3 'int main() { int x=3; return *&x; }'
assert 3 'int main() { int x=3; int *y=&x; int **z=&y; return **z; }'
assert 5 'int main() { int x=3; int *y=&x; *y=5; return x; }'

assert 5 'int main() { int x=3; int y=5; int *z=&x+1; return *z; }'
assert 5 'int main() { int x=3; int y=5; return *(&x+1); }'
assert 3 'int main() { int x=3; int y=5; return *(&y-1); }'
assert 7 'int main() { int x=3; int y=5; *(&y-1)=7; return x; }'
assert 7 'int main() { int x=3; int y=5; *(&x+1)=7; return y; }'
assert 2 'int main() { int x=3; return (&x+2)-&x; }'

assert 8 'int main() { return sizeof 1; }'
assert 8 'int main() { return sizeof(1); }'
assert 8 'int main() { return sizeof(2); }'
assert 8 'int main() { int a=0; return sizeof(a); }'
assert 8 'int main() { int n=2; int *p=&n; return sizeof(p); }'
assert 8 'int main() { return sizeof(2+3); }'
assert 8 'int main() { int n=2; int *p=&n; return sizeof(p+1); }'
assert 2 'int main() { int a=2; sizeof(a+1); return a; }'

assert 3 'int main() { int a[2]; int *p=&a; *p=3; return *a; }'
assert 1 'int main() { int a[3]; *a=1; *(a+1)=2; *(a+1)=3; return *a; }'
assert 2 'int main() { int a[3]; *a=1; *(a+1)=2; *(a+2)=3; return *(a+1); }'
assert 3 'int main() { int a[3]; *a=1; *(a+1)=2; *(a+2)=3; return *(a+2); }'

assert 1 'int main() { int a[3]; a[0]=1; a[1]=2; a[2]=3; return a[0]; }'
assert 2 'int main() { int a[3]; a[0]=1; a[1]=2; a[2]=3; return a[1]; }'
assert 3 'int main() { int a[3]; a[0]=1; a[1]=2; a[2]=3; return a[2]; }'
assert 24 'int main() { int a[3]; return sizeof(a); }'

echo OK
