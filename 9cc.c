#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv) {
  if (argc != 2) {
    fprintf(stderr, "Wrong number of args\n");
    return 1;
  }

  char *p = argv[1];

  printf(".intel_syntax noprefix\n");
  printf(".global main\n");
  printf("\n");
  printf("main:\n");
  printf("  mov rax, %ld\n", strtol(p, &p, 10));

  if (*p == '+') {
    p++;
    printf("  add rax, %ld\n", strtol(p, &p, 10));
  }


  printf("  ret\n");
  return 0;
}
