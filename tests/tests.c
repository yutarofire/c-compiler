/*
 * This is a block comment.
 */

int printf();
int exit();

int g1;
int g2[4];

typedef int MyInt;

int assert(int expected, int actual, char *code) {
  if (expected == actual) {
    printf("%s => %d\n", code, actual);
    return 1;
  }

  printf("%s => %d expected but got %d\n", code, expected, actual);
  exit(1);
}

int ret3() {
  return 3;
  return 5;
}

int add2(int x, int y) {
  return x + y;
}

int sub2(int x, int y) {
  return x - y;
}

int add5(int a, int b, int c, int d, int e) {
  return a + b + c + d + e;
}

int addx(int *x, int y) {
  return *x + y;
}

int sub_char(char a, char b, char c) {
  return a - b - c;
}

int fib(int x) {
  if (x<=1)
    return 1;
  return fib(x-1) + fib(x-2);
}

static int static_fn() {
  return 3;
}

int main() {
  assert(0, 0, "0");
  assert(42, 42, "42");
  assert(21, 5+20-4, "5+20-4");
  assert(41,  12 + 34 - 5 , " 12 + 34 - 5 ");
  assert(47, 5+6*7, "5+6*7");
  assert(15, 5*(9-6), "5*(9-6)");
  assert(4, (3+5)/2, "(3+5)/2");
  assert(10, -10+20, "-10+20");
  assert(10, - -10, "- -10");
  assert(10, - - +10, "- - +10");

  assert(0, 0==1, "0==1");
  assert(1, 42==42, "42==42");
  assert(1, 0!=1, "0!=1");
  assert(0, 42!=42, "42!=42");

  assert(1, 0<1, "0<1");
  assert(0, 1<1, "1<1");
  assert(0, 2<1, "2<1");
  assert(1, 0<=1, "0<=1");
  assert(1, 1<=1, "1<=1");
  assert(0, 2<=1, "2<=1");

  assert(1, 1>0, "1>0");
  assert(0, 1>1, "1>1");
  assert(0, 1>2, "1>2");
  assert(1, 1>=0, "1>=0");
  assert(1, 1>=1, "1>=1");
  assert(0, 1>=2, "1>=2");

  assert(3, ({ int a; a=3; a; }), "({ int a; a=3; a; })");
  assert(3, ({ int a=3; a; }), "({ int a=3; a; })");
  assert(8, ({ int a=3; int z=5; a+z; }), "({ int a=3; int z=5; a+z; })");

  assert(3, ({ int a=3; a; }), "({ int a=3; a; })");
  assert(8, ({ int a=3; int z=5; a+z; }), "({ int a=3; int z=5; a+z; })");
  assert(6, ({ int a; int b; a=b=3; a+b; }), "({ int a; int b; a=b=3; a+b; })");
  assert(3, ({ int foo=3; foo; }), "({ int foo=3; foo; })");
  assert(8, ({ int foo123=3; int bar=5; foo123+bar; }), "({ int foo123=3; int bar=5; foo123+bar; })");

  assert(3, ({ int x; if (0) x=2; else x=3; x; }), "({ int x; if (0) x=2; else x=3; x; })");
  assert(3, ({ int x; if (1-1) x=2; else x=3; x; }), "({ int x; if (1-1) x=2; else x=3; x; })");
  assert(2, ({ int x; if (1) x=2; else x=3; x; }), "({ int x; if (1) x=2; else x=3; x; })");
  assert(2, ({ int x; if (2-1) x=2; else x=3; x; }), "({ int x; if (2-1) x=2; else x=3; x; })");

  assert(55, ({ int i=0; int j=0; for (i=0; i<=10; i=i+1) j=i+j; j; }), "({ int i=0; int j=0; for (i=0; i<=10; i=i+1) j=i+j; j; })");

  assert(10, ({ int i=0; while(i<10) i=i+1; i; }), "({ int i=0; while(i<10) i=i+1; i; })");

  assert(3, ({ 1; {2;} 3; }), "({ 1; {2;} 3; })");

  assert(10, ({ int i=0; while(i<10) i=i+1; i; }), "({ int i=0; while(i<10) i=i+1; i; })");
  assert(55, ({ int i=0; int j=0; while(i<=10) {j=i+j; i=i+1;} j; }), "({ int i=0; int j=0; while(i<=10) {j=i+j; i=i+1;} j; })");

  assert(3, ({ int x=3; *&x; }), "({ int x=3; *&x; })");
  assert(3, ({ int x=3; int *y=&x; int **z=&y; **z; }), "({ int x=3; int *y=&x; int **z=&y; **z; })");
  assert(5, ({ int x=3; int y=5; *(&x+1); }), "({ int x=3; int y=5; *(&x+1); })");
  assert(3, ({ int x=3; int y=5; *(&y-1); }), "({ int x=3; int y=5; *(&y-1); })");
  assert(5, ({ int x=3; int *y=&x; *y=5; x; }), "({ int x=3; int *y=&x; *y=5; x; })");
  assert(7, ({ int x=3; int y=5; *(&x+1)=7; y; }), "({ int x=3; int y=5; *(&x+1)=7; y; })");
  assert(7, ({ int x=3; int y=5; *(&y-1)=7; x; }), "({ int x=3; int y=5; *(&y-1)=7; x; })");
  assert(2, ({ int x=3; (&x+2)-&x; }), "({ int x=3; (&x+2)-&x; })");
  assert(8, ({ int x; int y; x=3; y=5; x+y; }), "({ int x, y; x=3; y=5; x+y; })");
  assert(8, ({ int x=3; int y=5; x+y; }), "({ int x=3, y=5; x+y; })");

  assert(3, ret3(), "ret3()");
  assert(8, add2(3, 5), "add2(3, 5)");
  assert(2, sub2(5, 3), "sub2(5, 3)");
  assert(15, add5(1,2,3,4,5), "add5(1,2,3,4,5)");
  assert(7, add2(3,4), "add2(3,4)");
  assert(1, sub2(4,3), "sub2(4,3)");
  assert(55, fib(9), "fib(9)");

  assert(3, ({ int x[2]; int *y=&x; *y=3; *x; }), "({ int x[2]; int *y=&x; *y=3; *x; })");

  assert(3, ({ int x[3]; *x=3; *(x+1)=4; *(x+2)=5; *x; }), "({ int x[3]; *x=3; *(x+1)=4; *(x+2)=5; *x; })");
  assert(4, ({ int x[3]; *x=3; *(x+1)=4; *(x+2)=5; *(x+1); }), "({ int x[3]; *x=3; *(x+1)=4; *(x+2)=5; *(x+1); })");
  assert(5, ({ int x[3]; *x=3; *(x+1)=4; *(x+2)=5; *(x+2); }), "({ int x[3]; *x=3; *(x+1)=4; *(x+2)=5; *(x+2); })");

  assert(0, ({ int x[2][3]; int *y=x; *y=0; **x; }), "({ int x[2][3]; int *y=x; *y=0; **x; })");
  assert(1, ({ int x[2][3]; int *y=x; *(y+1)=1; *(*x+1); }), "({ int x[2][3]; int *y=x; *(y+1)=1; *(*x+1); })");
  assert(2, ({ int x[2][3]; int *y=x; *(y+2)=2; *(*x+2); }), "({ int x[2][3]; int *y=x; *(y+2)=2; *(*x+2); })");
  assert(3, ({ int x[2][3]; int *y=x; *(y+3)=3; **(x+1); }), "({ int x[2][3]; int *y=x; *(y+3)=3; **(x+1); })");
  assert(4, ({ int x[2][3]; int *y=x; *(y+4)=4; *(*(x+1)+1); }), "({ int x[2][3]; int *y=x; *(y+4)=4; *(*(x+1)+1); })");
  assert(5, ({ int x[2][3]; int *y=x; *(y+5)=5; *(*(x+1)+2); }), "({ int x[2][3]; int *y=x; *(y+5)=5; *(*(x+1)+2); })");

  assert(3, ({ int x[3]; *x=3; x[1]=4; x[2]=5; *x; }), "({ int x[3]; *x=3; x[1]=4; x[2]=5; *x; })");
  assert(4, ({ int x[3]; *x=3; x[1]=4; x[2]=5; *(x+1); }), "({ int x[3]; *x=3; x[1]=4; x[2]=5; *(x+1); })");
  assert(5, ({ int x[3]; *x=3; x[1]=4; x[2]=5; *(x+2); }), "({ int x[3]; *x=3; x[1]=4; x[2]=5; *(x+2); })");
  assert(5, ({ int x[3]; *x=3; x[1]=4; x[2]=5; *(x+2); }), "({ int x[3]; *x=3; x[1]=4; x[2]=5; *(x+2); })");

  assert(0, ({ int x[2][3]; int *y=x; y[0]=0; x[0][0]; }), "({ int x[2][3]; int *y=x; y[0]=0; x[0][0]; })");
  assert(1, ({ int x[2][3]; int *y=x; y[1]=1; x[0][1]; }), "({ int x[2][3]; int *y=x; y[1]=1; x[0][1]; })");
  assert(2, ({ int x[2][3]; int *y=x; y[2]=2; x[0][2]; }), "({ int x[2][3]; int *y=x; y[2]=2; x[0][2]; })");
  assert(3, ({ int x[2][3]; int *y=x; y[3]=3; x[1][0]; }), "({ int x[2][3]; int *y=x; y[3]=3; x[1][0]; })");
  assert(4, ({ int x[2][3]; int *y=x; y[4]=4; x[1][1]; }), "({ int x[2][3]; int *y=x; y[4]=4; x[1][1]; })");
  assert(5, ({ int x[2][3]; int *y=x; y[5]=5; x[1][2]; }), "({ int x[2][3]; int *y=x; y[5]=5; x[1][2]; })");

  assert(4, ({ int x; sizeof(x); }), "({ int x; sizeof(x); })");
  assert(4, ({ int x; sizeof x; }), "({ int x; sizeof x; })");
  assert(8, ({ int *x; sizeof(x); }), "({ int *x; sizeof(x); })");
  assert(16, ({ int x[4]; sizeof(x); }), "({ int x[4]; sizeof(x); })");
  assert(48, ({ int x[3][4]; sizeof(x); }), "({ int x[3][4]; sizeof(x); })");
  assert(16, ({ int x[3][4]; sizeof(*x); }), "({ int x[3][4]; sizeof(*x); })");
  assert(4, ({ int x[3][4]; sizeof(**x); }), "({ int x[3][4]; sizeof(**x); })");
  assert(5, ({ int x[3][4]; sizeof(**x) + 1; }), "({ int x[3][4]; sizeof(**x) + 1; })");
  assert(5, ({ int x[3][4]; sizeof **x + 1; }), "({ int x[3][4]; sizeof **x + 1; })");
  assert(4, ({ int x[3][4]; sizeof(**x + 1); }), "({ int x[3][4]; sizeof(**x + 1); })");

  assert(4, ({ int x=1; sizeof(x=2); }), "({ int x=1; sizeof(x=2); })");
  assert(1, ({ int x=1; sizeof(x=2); x; }), "({ int x=1; sizeof(x=2); x; })");

  assert(0, g1, "g1");
  assert(3, ({ g1=3; g1; }), "({ g1=3; g1; })");
  assert(0, ({ g2[0]=0; g2[1]=1; g2[2]=2; g2[3]=3; g2[0]; }), "({ g2[0]=0; g2[1]=1; g2[2]=2; g2[3]=3; g2[0]; })");
  assert(1, ({ g2[0]=0; g2[1]=1; g2[2]=2; g2[3]=3; g2[1]; }), "({ g2[0]=0; g2[1]=1; g2[2]=2; g2[3]=3; g2[1]; })");
  assert(2, ({ g2[0]=0; g2[1]=1; g2[2]=2; g2[3]=3; g2[2]; }), "({ g2[0]=0; g2[1]=1; g2[2]=2; g2[3]=3; g2[2]; })");
  assert(3, ({ g2[0]=0; g2[1]=1; g2[2]=2; g2[3]=3; g2[3]; }), "({ g2[0]=0; g2[1]=1; g2[2]=2; g2[3]=3; g2[3]; })");

  assert(4, sizeof(g1), "sizeof(g1)");
  assert(16, sizeof(g2), "sizeof(g2)");

  assert(1, ({ char x=1; x; }), "({ char x=1; x; })");
  assert(1, ({ char x=1; char y=2; x; }), "({ char x=1; char y=2; x; })");
  assert(2, ({ char x=1; char y=2; y; }), "({ char x=1; char y=2; y; })");

  assert(1, ({ char x; sizeof(x); }), "({ char x; sizeof(x); })");
  assert(10, ({ char x[10]; sizeof(x); }), "({ char x[10]; sizeof(x); })");
  assert(1, ({ sub_char(7, 3, 3); }), "({ sub_char(7, 3, 3); })");

  assert(97, "abc"[0], "abc[0]");
  assert(98, "abc"[1], "abc[1]");
  assert(99, "abc"[2], "abc[2]");
  assert(0, "abc"[3], "abc[3]");
  assert(4, sizeof("abc"), "sizeof(abc)");

  assert(2, ({ int x=2; { int x=3; } x; }), "({ int x=2; { int x=3; } x; })");
  assert(2, ({ int x=2; { int x=3; } int y=4; x; }), "({ int x=2; { int x=3; } int y=4; x; })");
  assert(3, ({ int x=2; { x=3; } x; }), "({ int x=2; { x=3; } x; })");

  assert(2, ({ struct { int a; int b; } x; x.a = 2; x.b = 3; x.a; }), "({ struct { int a; int b; } x; x.a = 2; x.b = 3; x.a; })");
  assert(3, ({ struct { int a; int b; } x; x.a = 2; x.b = 3; x.b; }), "({ struct { int a; int b; } x; x.a = 2; x.b = 3; x.b; })");

  assert(1, ({ struct {char a; int b; char c;} x; x.a=1; x.b=2; x.c=3; x.a; }), "({ struct {char a; int b; char c;} x; x.a=1; x.b=2; x.c=3; x.a; })");
  assert(2, ({ struct {char a; int b; char c;} x; x.b=1; x.b=2; x.c=3; x.b; }), "({ struct {char a; int b; char c;} x; x.b=1; x.b=2; x.c=3; x.b; })");
  assert(3, ({ struct {char a; int b; char c;} x; x.a=1; x.b=2; x.c=3; x.c; }), "({ struct {char a; int b; char c;} x; x.a=1; x.b=2; x.c=3; x.c; })");

  assert(0, ({ struct {int a; int b;} x[3]; int *p=x; p[0]=0; x[0].a; }), "({ struct {int a; int b;} x[3]; int *p=x; p[0]=0; x[0].a; })");
  assert(1, ({ struct {int a; int b;} x[3]; int *p=x; p[1]=1; x[0].b; }), "({ struct {int a; int b;} x[3]; int *p=x; p[1]=1; x[0].b; })");
  assert(2, ({ struct {int a; int b;} x[3]; int *p=x; p[2]=2; x[1].a; }), "({ struct {int a; int b;} x[3]; int *p=x; p[2]=2; x[1].a; })");
  assert(3, ({ struct {int a; int b;} x[3]; int *p=x; p[3]=3; x[1].b; }), "({ struct {int a; int b;} x[3]; int *p=x; p[3]=3; x[1].b; })");

  assert(4, ({ struct {int a;} x; sizeof(x); }), "({ struct {int a;} x; sizeof(x); })");
  assert(8, ({ struct {int a; int b;} x; sizeof(x); }), "({ struct {int a; int b;} x; sizeof(x); })");
  assert(12, ({ struct {int a[3];} x; sizeof(x); }), "({ struct {int a[3];} x; sizeof(x); })");
  assert(16, ({ struct {int a;} x[4]; sizeof(x); }), "({ struct {int a;} x[4]; sizeof(x); })");
  assert(24, ({ struct {int a[3];} x[2]; sizeof(x); }), "({ struct {int a[3];} x[2]; sizeof(x); })");
  assert(2, ({ struct {char a; char b;} x; sizeof(x); }), "({ struct {char a; char b;} x; sizeof(x); })");
  assert(8, ({ struct {char a; int b;} x; sizeof(x); }), "({ struct {char a; int b;} x; sizeof(x); })");
  assert(8, ({ struct {int a; char b;} x; sizeof(x); }), "({ struct {int a; char b;} x; sizeof(x); })");

  assert(2, ({ struct t {char a[2];}; { struct t {char a[4];}; } struct t y; sizeof(y); }), "({ struct t {char a[2];}; { struct t {char a[4];}; } struct t y; sizeof(y); })");
  assert(3, ({ struct t {int x;}; int t=1; struct t y; y.x=2; t+y.x; }), "({ struct t {int x;}; int t=1; struct t y; y.x=2; t+y.x; })");

  assert(3, ({ struct t {char a;} x; struct t *y = &x; x.a=3; y->a; }), "({ struct t {char a;} x; struct t *y = &x; x.a=3; y->a; })");
  assert(3, ({ struct t {char a;} x; struct t *y = &x; y->a=3; x.a; }), "({ struct t {char a;} x; struct t *y = &x; y->a=3; x.a; })");

  assert(8, ({ struct t {int a; int b;} x; struct t y; sizeof(y); }), "({ struct t {int a; int b;} x; struct t y; sizeof(y); })");
  assert(8, ({ struct t {int a; int b;}; struct t y; sizeof(y); }), "({ struct t {int a; int b;}; struct t y; sizeof(y); })");

  { void *x; }

  assert(1, ({ typedef int t; t x=1; x; }), "({ typedef int t; t x=1; x; })");
  assert(1, ({ typedef struct {int a;} t; t x; x.a=1; x.a; }), "({ typedef struct {int a;} t; t x; x.a=1; x.a; })");
  assert(1, ({ typedef int t; t t=1; t; }), "({ typedef int t; t t=1; t; })");
  assert(2, ({ typedef struct {int a;} t; { typedef int t; } t x; x.a=2; x.a; }), "({ typedef struct {int a;} t; { typedef int t; } t x; x.a=2; x.a; })");
  assert(3, ({ MyInt x=3; x; }), "({ MyInt x=3; x; })");

  assert(97, 'a', "'a'");
  assert(10, '\n', "'\\n'");
  assert(4, sizeof('a'), "sizeof('a')");

  assert(0, ({ _Bool x = 0; x; }), "({ _Bool x = 0; x; })");
  assert(1, ({ _Bool x = 1; x; }), "({ _Bool x = 1; x; })");
  assert(1, ({ _Bool x = 2; x; }), "({ _Bool x = 2; x; })");

  assert(0, ({ enum { zero, one, two }; zero; }), "({ enum { zero, one, two }; zero; })");
  assert(1, ({ enum { zero, one, two }; one; }), "({ enum { zero, one, two }; one; })");
  assert(2, ({ enum { zero, one, two }; two; }), "({ enum { zero, one, two }; two; })");
  assert(4, ({ enum { zero, one, two } x; sizeof(x); }), "({ enum { zero, one, two } x; sizeof(x); })");
  assert(2, ({ enum { zero, one, two }; one + 1; }), "({ enum { zero, one, two }; one + 1; })");

  assert(10, ({ int one = 10; { enum { zero, one, two } x; } one; }), "({ int one = 10; { enum { zero, one, two } x; } one; })");
  assert(1, ({ char x = 'a'; { enum { zero, one, two } x; } sizeof(x); }), "({ char x = 'a'; { enum { zero, one, two } x; } sizeof(x); })");
  assert(4, ({ enum { zero, one, two } x; { char x = 'a'; } sizeof(x); }), "({ enum { zero, one, two } x; { char x = 'a'; } sizeof(x); })");

  assert(3, static_fn(), "static_fn()");

  assert(55, ({ int j=0; for (int i=0; i<=10; i=i+1) j=j+i; j; }), "({ int j=0; for (int i=0; i<=10; i = i+1) j=j+i; j; })");
  assert(3, ({ int i=3; int j=0; for (int i=0; i<=10; i=i+1) j=j+i; i; }), "({ int i=3; int j=0; for (int i=0; i<=10; i=i+1) j=j+i; i; })");

  assert(7, ({ int i=2; i+=5; i; }), "({ int i=2; i+=5; i; })");
  assert(7, ({ int i=2; i+=5; }), "({ int i=2; i+=5; })");
  assert(3, ({ int i=5; i-=2; i; }), "({ int i=5; i-=2; i; })");
  assert(3, ({ int i=5; i-=2; }), "({ int i=5; i-=2; })");
  assert(6, ({ int i=3; i*=2; i; }), "({ int i=3; i*=2; i; })");
  assert(6, ({ int i=3; i*=2; }), "({ int i=3; i*=2; })");
  assert(3, ({ int i=6; i/=2; i; }), "({ int i=6; i/=2; i; })");
  assert(3, ({ int i=6; i/=2; }), "({ int i=6; i/=2; })");

  assert(3, ({ int i=2; ++i; }), "({ int i=2; ++i; })");
  assert(3, ({ int i=2; ++i; i; }), "({ int i=2; ++i; i; })");
  assert(1, ({ int i=2; --i; }), "({ int i=2; --i; })");
  assert(1, ({ int i=2; --i; i; }), "({ int i=2; --i; i; })");
  assert(2, ({ int a[3]; a[0]=0; a[1]=1; a[2]=2; int *p=a+1; ++*p; }), "({ int a[3]; a[0]=0; a[1]=1; a[2]=2; int *p=a+1; ++*p; })");
  assert(0, ({ int a[3]; a[0]=0; a[1]=1; a[2]=2; int *p=a+1; --*p; }), "({ int a[3]; a[0]=0; a[1]=1; a[2]=2; int *p=a+1; --*p; })");

  assert(2, ({ int i=2; i++; }), "({ int i=2; i++; })");
  assert(3, ({ int i=2; i++; i; }), "({ int i=2; i++; i; })");
  assert(2, ({ int i=2; i--; }), "({ int i=2; i--; })");
  assert(1, ({ int i=2; i--; i; }), "({ int i=2; i--; i; })");

  assert(11, ({ int a[3]; a[0]=0; a[1]=11; a[2]=22; int *p=a; p++; *p; }), "({ int a[3]; a[0]=0; a[1]=1; a[2]=2; int *p=a+1; *p++; })");
  assert(11, ({ int a[3]; a[0]=0; a[1]=11; a[2]=22; int *p=a; p++; p++; p--; *p; }), "({ int a[3]; a[0]=0; a[1]=1; a[2]=2; int *p=a+1; *p++; })");
  assert(1, ({ int a[3]; a[0]=0; a[1]=1; a[2]=2; int *p=a+1; *p++; }), "({ int a[3]; a[0]=0; a[1]=1; a[2]=2; int *p=a+1; *p++; })");
  assert(1, ({ int a[3]; a[0]=0; a[1]=1; a[2]=2; int *p=a+1; *p--; }), "({ int a[3]; a[0]=0; a[1]=1; a[2]=2; int *p=a+1; *p--; })");

  assert(-1, ~0, "~0");
  assert(0, ~-1, "~-1");

  assert(0, 0&1, "0&1");
  assert(1, 3&1, "3&1");
  assert(3, 7&3, "7&3");
  assert(10, -1&10, "-1&10");

  assert(1, 0||1, "0||1");
  assert(1, 0||(2-2)||5, "0||(2-2)||5");
  assert(0, 0||0, "0||0");
  assert(0, 0||(2-2), "0||(2-2)");

  assert(0, 0&&1, "0&&1");
  assert(0, (2-2)&&5, "(2-2)&&5");
  assert(1, 1&&5, "1&&5");

  assert(3, ({ int i; for (i=0; i<10; i++) { if (i==3) break; } i; }), "({ int i; for (i=0; i<10; i++) { if (i==3) break; } i; })");
  assert(3, ({ int i=0; while (i<10) { if (i==3) break; i++; } i; }), "({ int i=0; while (i<10) { if (i==3) break; i++; } i; })");

  assert(3, ({ int i=0; for(;i<10;i++) { if (i == 3) break; } i; }), "({ int i=0; for(;i<10;i++) { if (i == 3) break; } i; })");
  assert(4, ({ int i=0; while (1) { if (i++ == 3) break; } i; }), "({ int i=0; while (1) { if (i++ == 3) break; } i; })");
  assert(3, ({ int i=0; for(;i<10;i++) { for (;;) break; if (i == 3) break; } i; }), "({ int i=0; for(;i<10;i++) { for (;;) break; if (i == 3) break; } i; })");
  assert(4, ({ int i=0; while (1) { while(1) break; if (i++ == 3) break; } i; }), "({ int i=0; while (1) { while(1) break; if (i++ == 3) break; } i; })");

  assert(10, ({ int i; int k=0; for (i=0; i<10; i++) { k++; } k; }), "({ int i; int k=0; for (i=0; i<10; i++) { k++; } k; })");
  assert(9, ({ int i; int k=0; for (i=0; i<10; i++) { if (i==3) continue; k++; } k; }), "({ int i; int k=0; for (i=0; i<10; i++) { if (i==3) continue; k++; } k; })");

  assert(10, ({ int i=0; int j=0; for (;i<10;i++) { if (i>5) continue; j++; } i; }), "({ int i=0; int j=0; for (;i<10;i++) { if (i>5) continue; j++; } i; })");
  assert(6, ({ int i=0; int j=0; for (;i<10;i++) { if (i>5) continue; j++; } j; }), "({ int i=0; int j=0; for (;i<10;i++) { if (i>5) continue; j++; } j; })");
  assert(11, ({ int i=0; int j=0; while (i++<10) { if (i>5) continue; j++; } i; }), "({ int i=0; int j=0; while (i++<10) { if (i>5) continue; j++; } i; })");
  assert(5, ({ int i=0; int j=0; while (i++<10) { if (i>5) continue; j++; } j; }), "({ int i=0; int j=0; while (i++<10) { if (i>5) continue; j++; } j; })");

  assert(22 ,({ int i=2; switch (i) { case 1: i=11; break; case 2: i=22; break; case 3: i=33; break; } i; }), "({ int i=2; switch (i) { case 1: i=11; break; case 2: i=22; break; case 3: i=33; break; } i; })");
  assert(44 ,({ int i=4; switch (i) { case 1: i=11; break; case 2: i=22; break; case 3: i=33; break; default: i=44; } i; }), "({ int i=4; switch (i) { case 1: i=11; break; case 2: i=22; break; case 3: i=33; break; default: i=44; } i; })");

  assert(5, ({ int i=0; switch(0) { case 0:i=5;break; case 1:i=6;break; case 2:i=7;break; } i; }), "({ int i=0; switch(0) { case 0:i=5;break; case 1:i=6;break; case 2:i=7;break; } i; })");
  assert(6, ({ int i=0; switch(1) { case 0:i=5;break; case 1:i=6;break; case 2:i=7;break; } i; }), "({ int i=0; switch(1) { case 0:i=5;break; case 1:i=6;break; case 2:i=7;break; } i; })");
  assert(7, ({ int i=0; switch(2) { case 0:i=5;break; case 1:i=6;break; case 2:i=7;break; } i; }), "({ int i=0; switch(2) { case 0:i=5;break; case 1:i=6;break; case 2:i=7;break; } i; })");
  assert(0, ({ int i=0; switch(3) { case 0:i=5;break; case 1:i=6;break; case 2:i=7;break; } i; }), "({ int i=0; switch(3) { case 0:i=5;break; case 1:i=6;break; case 2:i=7;break; } i; })");
  assert(5, ({ int i=0; switch(0) { case 0:i=5;break; default:i=7; } i; }), "({ int i=0; switch(0) { case 0:i=5;break; default:i=7; } i; })");
  assert(7, ({ int i=0; switch(1) { case 0:i=5;break; default:i=7; } i; }), "({ int i=0; switch(1) { case 0:i=5;break; default:i=7; } i; })");
  assert(2, ({ int i=0; switch(1) { case 0: 0; case 1: 0; case 2: 0; i=2; } i; }), "({ int i=0; switch(1) { case 0: 0; case 1: 0; case 2: 0; i=2; } i; })");
  assert(0, ({ int i=0; switch(3) { case 0: 0; case 1: 0; case 2: 0; i=2; } i; }), "({ int i=0; switch(3) { case 0: 0; case 1: 0; case 2: 0; i=2; } i; })");

  assert(1, ({ int x[3]={1,2,3}; x[0]; }), "({ int x[3]={1,2,3}; x[0]; })");
  assert(2, ({ int x[3]={1,2,3}; x[1]; }), "({ int x[3]={1,2,3}; x[1]; })");
  assert(3, ({ int x[3]={1,2,3}; x[2]; }), "({ int x[3]={1,2,3}; x[2]; })");

  assert(3, (1,2,3), "(1,2,3)");
  assert(5, ({ int i=2; int j=3; (i=5,j)=6; i; }), "({ int i=2; int j=3; (i=5,j)=6; i; })");
  assert(6, ({ int i=2; int j=3; (i=5,j)=6; j; }), "({ int i=2; int j=3; (i=5,j)=6; j; })");

  printf("OK\n");
  return 0;
}
